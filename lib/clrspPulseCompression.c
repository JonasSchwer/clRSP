/******************************************************************************
*
*   Implementation of the pulsecompression.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

size_t
clrspNearestPower(const size_t n,
                  const size_t num_primes,
                  const size_t *primes)
{
    size_t exponents[num_primes];
    size_t i;
    for (i = 0; i < num_primes; ++i) {
        exponents[i] = (size_t)ceil(log(n)/log(primes[i]));
    }
    size_t result = pow(primes[num_primes - 1], exponents[num_primes - 1]);
    size_t temp;
    for (i = 0; i < num_primes - 1; ++i) {
        temp = pow(primes[i], exponents[i]);
        result = (temp < result) ? temp : result;
    }

    return result;
}

cl_int
clrspElementwiseProduct(const clrspComplexMatrix *X,
                        cl_mem *X_real,
                        cl_mem *X_imag,
                        const clrspComplexMatrix *y,
                        cl_mem *y_real,
                        cl_mem *y_imag,
                        cl_context *context,
                        cl_command_queue *queue,
                        cl_uint num_wait_list,
                        cl_event *wait_list,
                        cl_event *event)
{
    cl_int status;

    /* Load kernel source code. */
    char *src = clrspLoadKernelSource("../kernels/elemProdKernel.cl");

    /* Create the program. */
    cl_program program;
    program = clCreateProgramWithSource(*context,
                                        1,
                                        (const char**)&src,
                                        NULL,
                                        &status);
    if (status != CL_SUCCESS) { return status; }
    free(src);

    /* Compile the program. */
    cl_uint num_devices;
    status = clGetContextInfo(*context,
                              CL_CONTEXT_NUM_DEVICES,
                              sizeof(cl_uint),
                              (void*)&num_devices,
                              NULL);
    if (status != CL_SUCCESS) { return status; }

    cl_device_id devices[num_devices];
    status = clGetContextInfo(*context,
                              CL_CONTEXT_DEVICES,
                              num_devices * sizeof(cl_device_id),
                              (void*)devices,
                              NULL);
    if (status != CL_SUCCESS) { return status; }

    status = clBuildProgram(program, num_devices, devices, NULL, NULL, NULL);

    /*
    size_t ret_size;
    clGetProgramInfo(program, CL_PROGRAM_KERNEL_NAMES, 0, NULL, &ret_size);
    char *kernel_names = (char*)malloc(ret_size);
    clGetProgramInfo(program,
                     CL_PROGRAM_KERNEL_NAMES,
                     ret_size,
                     kernel_names,
                     NULL);
    printf("Kernel names:\n%s\n", kernel_names);

    clGetProgramBuildInfo(program,
                          devices[0],
                          CL_PROGRAM_BUILD_LOG,
                          0,
                          NULL,
                          &ret_size);
    char *log = malloc(ret_size);
    clGetProgramBuildInfo(program,
                          devices[0],
                          CL_PROGRAM_BUILD_LOG,
                          ret_size,
                          (void*)log,
                          NULL);
    printf("Build log:\n%s\n", log);
    */

    if (status != CL_SUCCESS) { return status; }

    /* Create the kernel. */
    cl_kernel kernel;
    kernel = clCreateKernel(program, "elemProdKernel", &status);
    if (status != CL_SUCCESS) { return status; }

    /* Set kernel arguments. */
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), X_real);
    if (status != CL_SUCCESS) { return status; }

    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), X_imag);
    if (status != CL_SUCCESS) { return status; }

    int inc_row = X->cols;
    status = clSetKernelArg(kernel, 2, sizeof(int), &inc_row);
    if (status != CL_SUCCESS) { return status; }

    int inc_col = 1;
    status = clSetKernelArg(kernel, 3, sizeof(int), &inc_col);
    if (status != CL_SUCCESS) { return status; }

    status = clSetKernelArg(kernel, 4, sizeof(cl_mem), y_real);
    if (status != CL_SUCCESS) { return status; }

    status = clSetKernelArg(kernel, 5, sizeof(cl_mem), y_imag);
    if (status != CL_SUCCESS) { return status; }

    /* Enqueue kernel. */
    size_t globalSize[2] = {X->cols, X->rows};
    status = clEnqueueNDRangeKernel(*queue,
                                    kernel,
                                    2,
                                    NULL,
                                    globalSize,
                                    NULL,
                                    num_wait_list,
                                    wait_list,
                                    event);

    return status;
}

cl_int
clrspPulseCompression(const clrspComplexMatrix *y,
                      clrspComplexMatrix *M_0,
                      cl_context *context,
                      cl_command_queue *queue,
                      clrspClfftStatus *setup,
                      clrspComplexMatrix *M_pc)
{
    cl_int status;

    /* Get dimensions. */
    size_t m, n, k, l;
    m = M_0->rows;
    n = M_0->cols;
    k = y->cols;
    assert(y->rows == 1 && k <= n);

    assert(M_pc->rows == m && M_pc->cols == (n + k - 1));

    /* Get nearest power to one of the given primes of the resulting fft
       length, because clFFT only supports these fft lengths. */
    size_t primes[4] = {7, 5, 3, 2};
    l = clrspNearestPower(n + k - 1,
                          4,
                          primes);

    if (*setup != CLRSP_CLFFT_SETUP) {
        /* Setup clFFT library. */
        status = clrspSetupClfftLibrary(setup);
        if (status != CL_SUCCESS) { return status; }
    }

    /* Create clFFT plans for both signals, take zero-padding into account. */
    clrspComplexMatrix *temp = clrspNewComplexMatrix(1,
                                                     l);

    clfftPlanHandle y_fft_plan;
    status = clrspCreate1DfftPlan(&y_fft_plan,
                                  context,
                                  queue,
                                  CLRSP_ROW_WISE,
                                  temp);
    if (status != CL_SUCCESS) { return status; }

    clfftPlanHandle M_0_fft_plan;
    temp->rows = m;
    status = clrspCreate1DfftPlan(&M_0_fft_plan,
                                  context,
                                  queue,
                                  CLRSP_ROW_WISE,
                                  temp);
    if (status != CL_SUCCESS) { return status; }

    /* Allocate memory on device. */
    cl_mem y_real;
    y_real = clCreateBuffer(*context,
                            CL_MEM_READ_WRITE,
                            l * sizeof(float),
                            NULL,
                            &status);
    if (status != CL_SUCCESS) { return status; }

    cl_mem y_imag;
    y_imag = clCreateBuffer(*context,
                            CL_MEM_READ_WRITE,
                            l * sizeof(float),
                            NULL,
                            &status);
    if (status != CL_SUCCESS) { return status; }

    cl_mem M_0_real;
    M_0_real = clCreateBuffer(*context,
                              CL_MEM_READ_WRITE,
                              m * l * sizeof(float),
                              NULL,
                              &status);
    if (status != CL_SUCCESS) { return status; }

    cl_mem M_0_imag;
    M_0_imag = clCreateBuffer(*context,
                              CL_MEM_READ_WRITE,
                              m * l * sizeof(float),
                              NULL,
                              &status);
    if (status != CL_SUCCESS) { return status; }

    /* Copy data to device an add zero-padding. */
    const float pattern = 0.;

    cl_event w_re_y; /* write real-part of y event. */
    status = clEnqueueWriteBuffer(*queue,
                                  y_real,
                                  CL_FALSE,
                                  0,
                                  k * sizeof(float),
                                  y->real,
                                  0,
                                  NULL,
                                  &w_re_y);
    if (status != CL_SUCCESS) { return status; }

    cl_event w_im_y; /* write imag-part of y event. */
    status = clEnqueueWriteBuffer(*queue,
                                  y_imag,
                                  CL_FALSE,
                                  0,
                                  k * sizeof(float),
                                  y->imag,
                                  0,
                                  NULL,
                                  &w_im_y);
    if (status != CL_SUCCESS) { return status; }

    cl_event f_re_y; /* fill real-part of y event. */
    status = clEnqueueFillBuffer(*queue,
                                 y_real,
                                 (const void*)&pattern,
                                 sizeof(float),
                                 k * sizeof(float), (l - k) * sizeof(float),
                                 0,
                                 NULL,
                                 &f_re_y);
    if (status != CL_SUCCESS) { return status; }

    cl_event f_im_y; /* fill imag-part of y event. */
    status = clEnqueueFillBuffer(*queue,
                                 y_imag,
                                 (const void*)&pattern,
                                 sizeof(float), k * sizeof(float),
                                 (l - k) * sizeof(float),
                                 0,
                                 NULL,
                                 &f_im_y);
    if (status != CL_SUCCESS) { return status; }

//    size_t buffer_origin[3] = {0, 0, 0};
//    size_t host_origin[3] = {0, 0, 0};
//    size_t region[3] = {m * sizeof(float), n, 1};

    cl_event f_re_M_0; /* fill real-part of M_0 event. */
    status = clEnqueueFillBuffer(*queue,
                                 M_0_real,
                                 (const void*)&pattern,
                                 sizeof(float),
                                 0,
                                 m * l * sizeof(float),
                                 0,
                                 NULL,
                                 &f_re_M_0);
    if (status != CL_SUCCESS) { return status; }

    cl_event w_re_M_0; /* write real-part of M_0 event. */
    /*
    status = clEnqueueWriteBufferRect(*queue,
                                      M_0_real,
                                      CL_FALSE,
                                      buffer_origin,
                                      host_origin,
                                      region,
                                      0,
                                      l * m * sizeof(float),
                                      0,
                                      0,
                                      M_0->real,
                                      1,
                                      &f_re_M_0,
                                      &w_re_M_0);
    */
    status = clEnqueueWriteBuffer(*queue,
                                  M_0_real,
                                  CL_FALSE,
                                  0,
                                  m * n * sizeof(float),
                                  M_0->real,
                                  1,
                                  &f_re_M_0,
                                  &w_re_M_0);
    if (status != CL_SUCCESS) { return status; }

    cl_event f_im_M_0; /* fill imag-part of M_0 event. */
    status = clEnqueueFillBuffer(*queue,
                                 M_0_imag,
                                 (const void*)&pattern,
                                 sizeof(float),
                                 0,
                                 m * l * sizeof(float),
                                 0,
                                 NULL,
                                 &f_im_M_0);
    if (status != CL_SUCCESS) { return status; }

    cl_event w_im_M_0; /* write imag-part of M_0 event. */
    /*
    status = clEnqueueWriteBufferRect(*queue,
                                      M_0_imag,
                                      CL_FALSE,
                                      buffer_origin,
                                      host_origin,
                                      region,
                                      0,
                                      m * l * sizeof(float),
                                      0,
                                      0,
                                      M_0->imag,
                                      1,
                                      &f_im_M_0,
                                      &w_im_M_0);
    */
    status = clEnqueueWriteBuffer(*queue,
                                  M_0_imag,
                                  CL_FALSE,
                                  0,
                                  m * n * sizeof(float),
                                  M_0->imag,
                                  1,
                                  &f_im_M_0,
                                  &w_im_M_0);
    if (status != CL_SUCCESS) { return status; }

    /* Perform forward FFT. */
    cl_uint num_wait_events = 4;
    cl_event y_event_list[num_wait_events];
    y_event_list[0] = w_re_y;
    y_event_list[1] = w_im_y;
    y_event_list[2] = f_re_y;
    y_event_list[3] = f_im_y;

    cl_event M_0_event_list[num_wait_events];
    M_0_event_list[0] = w_re_M_0;
    M_0_event_list[1] = w_im_M_0;
    M_0_event_list[2] = f_re_M_0;
    M_0_event_list[3] = f_im_M_0;

    cl_event y_fft;
    status = clrspFFT(&y_real,
                      &y_imag,
                      context,
                      queue,
                      &y_fft_plan,
                      num_wait_events,
                      y_event_list,
                      &y_fft);
    if (status != CL_SUCCESS) { return status; }

    cl_event M_0_fft;
    status = clrspFFT(&M_0_real,
                      &M_0_imag,
                      context,
                      queue,
                      &M_0_fft_plan,
                      num_wait_events,
                      M_0_event_list,
                      &M_0_fft);
    if (status != CL_SUCCESS) { return status; }

    /* Perfomr element-wise product on each row. */
    num_wait_events = 2;
    cl_event elem_prod_event_list[num_wait_events];
    elem_prod_event_list[0] = y_fft;
    elem_prod_event_list[1] = M_0_fft;

    /* Use temporary ComplexMatrices because of zero-padding. */
    clrspComplexMatrix *M_0_ = clrspNewComplexMatrix(M_0->rows,
                                                     l);

    clrspComplexMatrix *y_ = clrspNewComplexMatrix(y->rows,
                                                   l);

    cl_event elem_prod;
    status = clrspElementwiseProduct(M_0_,
                                     &M_0_real,
                                     &M_0_imag,
                                     y_,
                                     &y_real,
                                     &y_imag,
                                     context,
                                     queue,
                                     num_wait_events,
                                     elem_prod_event_list,
                                     &elem_prod);
    if (status != CL_SUCCESS) { return status; }

    clrspFreeComplexMatrix(M_0_);
    clrspFreeComplexMatrix(y_);

    /* Perform backward FFT. */
    cl_event M_0_ifft;
    status = clrspIFFT(&M_0_real,
                       &M_0_imag,
                       context,
                       queue,
                       &M_0_fft_plan,
                       1,
                       &elem_prod,
                       &M_0_ifft);
    if (status != CL_SUCCESS) { return status; }

    /* Get results from device. */

//    region[0] = (n + k - 1) * sizeof(float);
//    region[1] = m;
//    region[2] = 1;

    cl_event r_re_M_0;
    /*
    status = clEnqueueReadBufferRect(*queue,
                                     M_0_real,
                                     CL_FALSE,
                                     buffer_origin,
                                     host_origin,
                                     region,
                                     l * sizeof(float),
                                     0,
                                     0,
                                     0,
                                     M_pc->real,
                                     1,
                                     &M_0_ifft,
                                     &r_re_M_0);
    */
    status = clEnqueueReadBuffer(*queue,
                                 M_0_real,
                                 CL_FALSE,
                                 0,
                                 m * (n + k - 1) * sizeof(float),
                                 M_pc->real,
                                 1,
                                 &M_0_ifft,
                                 &r_re_M_0);
    if (status != CL_SUCCESS) { return status; }

    cl_event r_im_M_0;
    /*
    status = clEnqueueReadBufferRect(*queue,
                                     M_0_imag,
                                     CL_FALSE,
                                     buffer_origin,
                                     host_origin,
                                     region,
                                     l * sizeof(float),
                                     0,
                                     0,
                                     0,
                                     M_pc->imag,
                                     1,
                                     &M_0_ifft,
                                     &r_im_M_0);
    */
    status = clEnqueueReadBuffer(*queue,
                                 M_0_imag,
                                 CL_FALSE,
                                 0,
                                 m * (n + k - 1) * sizeof(float),
                                 M_pc->imag,
                                 1,
                                 &M_0_ifft,
                                 &r_im_M_0);
    if (status != CL_SUCCESS) { return status; }

    /* Release resources. */
    clfftDestroyPlan(&y_fft_plan);
    clfftDestroyPlan(&M_0_fft_plan);
    clFinish(*queue);
    clReleaseMemObject(y_real);
    clReleaseMemObject(y_imag);
    clReleaseMemObject(M_0_real);
    clReleaseMemObject(M_0_imag);

    return status;
}
