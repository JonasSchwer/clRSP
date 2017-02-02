/******************************************************************************
*
*   ...
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"
#include "clrspMex.h"


void
mexFunction(int nlhs, mxArray *plhs[],
            int nrhs, const mxArray *prhs[])
{
    /* Check input arguments. */
    if (nrhs != 6) {
        mexErrMsgIdAndTxt("MATLAB:clTestPulseCompression:invalidNumInputs",
                          "Six input arguments required.");
    }
    if (!mxIsSingle(prhs[0])
        || !mxIsComplex(prhs[0])
        || mxGetNumberOfDimensions(prhs[0]) != 2
        || !mxIsSingle(prhs[1])
        || !mxIsComplex(prhs[1])
        || !mxIsScalar(prhs[2])) {
        mexErrMsgIdAndTxt("MATLAB:clTestPulseCompression:invlaidInputs",
                          "First input must be single, complex vector."
                          "Second input must be single, complex matrix."
                          "Third input must be scalar.");
    }
    if (nlhs > 3) {
        mexErrMsgIdAndTxt("MATLAB:clTestPulseCompression:invalidNumOutputs",
                          "Too many out_Xput arguments.");
    }

    /* Process input arguments. */
    char *storage_option = mxArrayToString(prhs[nrhs - 3]);
    clrspStorageOrder order = CLRSP_ROW_MAJOR;
    if (strcmp(storage_option, "row-major") == 0) {
        order = CLRSP_ROW_MAJOR;
    } else if (strcmp(storage_option, "col-major") == 0) {
        order = CLRSP_COL_MAJOR;
    } else {
        mexErrMsgIdAndTxt("MATLAB:clTestPulseCompression:invalidInputs",
                          "Unknown storage option.");
    }

    char *layout_option = mxArrayToString(prhs[nrhs - 2]);
    clrspComplexLayout layout = CLRSP_PLANAR;
    if (strcmp(layout_option, "planar") == 0) {
        layout = CLRSP_PLANAR;
    } else if (strcmp(layout_option, "interleaved") == 0) {
        layout = CLRSP_INTERLEAVED;
    } else {
        mexErrMsgIdAndTxt("MATLAB:clTestPulseCompression:invalidInputs",
                          "Unknown layout option.");
    }

    char *device_option = mxArrayToString(prhs[nrhs - 1]);
    cl_device_type device = CL_DEVICE_TYPE_GPU;
    if (strcmp(device_option, "gpu") == 0) {
        device = CL_DEVICE_TYPE_GPU;
    } else if (strcmp(device_option, "cpu") == 0) {
        device = CL_DEVICE_TYPE_CPU;
    } else {
        mexErrMsgIdAndTxt("MATLAB:clTestPulseCompression:invalidInputs",
                          "Unknown device option.");
    }

    /* Get number of runs for benchmarking. */
    size_t runs = mxGetScalar(prhs[2]);

    /* Get matrix M_0 and it's size. */
    size_t m = mxGetM(prhs[0]);
    size_t n = mxGetN(prhs[0]);
    clrspComplexMatrix *X = clrspGetComplexMatrix(prhs[0],
                                                  order,
                                                  layout);

    /* Get vector s and it's size. */
    size_t k = mxGetN(prhs[1]);
    clrspComplexMatrix *y = clrspGetComplexMatrix(prhs[1],
                                                  order,
                                                  layout);

    /* Determine size of on device data (including zero-padding and fft
       requirements. */
    size_t primes[4] = {2, 3, 5, 7};
    size_t l = clrspNearestPower(n + k - 1,
                                 4,
                                 &primes[0]);

    /* Setup OpenCL environment. */
    cl_int status;
    cl_context context;
    cl_command_queue queue;
    cl_command_queue_properties queue_props =
        (CL_QUEUE_PROFILING_ENABLE);
    status = clrspSetupSingleDeviceContext(&context,
                                           NULL,
                                           0,
                                           &queue,
                                           &queue_props,
                                           0,
                                           device,
                                           0);
    if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }

    /* Initialize clFFT library. */
    status = clrspSetupClfftLibrary();
    if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }

    /* Create clFFT plans. */
    clfftPlanHandle X_fft_plan;
    clrspComplexMatrix *temp = clrspNewComplexMatrix(m,
                                                     l,
                                                     X->order,
                                                     X->layout);
    status = clrspCreate1DfftPlan(&X_fft_plan,
                                  &context,
                                  &queue,
                                  CLRSP_ROW_WISE,
                                  temp);

    clfftPlanHandle y_fft_plan;
    temp->rows = 1;
    status = clrspCreate1DfftPlan(&y_fft_plan,
                                  &context,
                                  &queue,
                                  CLRSP_ROW_WISE,
                                  temp);

    cl_event events[16];

    /* Allocate memory on device and copy data. */
    size_t padding[4] = {0, 0, 0, (l - n)};

    cl_mem X_real;
    cl_mem X_imag;
    status = clrspAllocAndWriteMatrixToGPU(X,
                                           &X_real,
                                           &X_imag,
                                           padding,
                                           &context,
                                           CL_MEM_READ_WRITE,
                                           &queue,
                                           0,
                                           NULL,
                                           &events[0]);
    if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }

    padding[3] = l - k;

    cl_mem y_real;
    cl_mem y_imag;
    status = clrspAllocAndWriteMatrixToGPU(y,
                                           &y_real,
                                           &y_imag,
                                           padding,
                                           &context,
                                           CL_MEM_READ_WRITE,
                                           &queue,
                                           0,
                                           NULL,
                                           &events[4]);
    if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }

    clrspComplexMatrix *X_padded = clrspNewComplexMatrix(m,
                                                         l,
                                                         X->order,
                                                         X->layout);
    clrspComplexMatrix *y_padded = clrspNewComplexMatrix(1,
                                                         l,
                                                         y->order,
                                                         y->layout);
    /* Perform pulse compression. */
    int i;
    cl_ulong time = 0;
    for (i = 0; i < runs; ++i) {

        status = clrspPulseCompression(y_padded,
                                       &y_fft_plan,
                                       &y_real,
                                       &y_imag,
                                       X_padded,
                                       &X_fft_plan,
                                       &X_real,
                                       &X_imag,
                                       &context,
                                       &queue,
                                       8,
                                       &events[0],
                                       &events[8]);
        if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }

        clFinish(queue);

        cl_ulong start = ULONG_MAX;
        cl_ulong end = 0;
        cl_ulong tmp;
        int j;
        for (j = 0; j < 4; ++j) {
            clGetEventProfilingInfo(events[8 + j],
                                    CL_PROFILING_COMMAND_START,
                                    sizeof(cl_ulong),
                                    &tmp,
                                    NULL);
            start = (tmp < start) ? tmp : start;
            clGetEventProfilingInfo(events[8 + j],
                                    CL_PROFILING_COMMAND_END,
                                    sizeof(cl_ulong),
                                    &tmp,
                                    NULL);
            end = (tmp > end) ? tmp : end;
        }
        time += end - start;
    }
    time /= runs;

    /* Prepare host for out_Xput. */
    clrspComplexMatrix *out_X = clrspNewComplexMatrix(m,
                                                      l,
                                                      X->order,
                                                      X->layout);
    clrspAllocComplexMatrix(out_X);

    clrspComplexMatrix *out_y = clrspNewComplexMatrix(1,
                                                      l,
                                                      y->order,
                                                      y->layout);
    clrspAllocComplexMatrix(out_y);

    /* Copy zero-padded data back to host. */
    size_t buffer_origin[3] = {0, 0, 0};
    size_t buffer_row_pitch;
    if (out_X->order == CLRSP_ROW_MAJOR) {
        buffer_row_pitch = l * sizeof(float);
    } else {
        buffer_row_pitch = m * sizeof(float);
    }
    if (out_X->layout == CLRSP_INTERLEAVED) {
        buffer_row_pitch *= 2;
    }

    status = clrspReadMatrixFromGPU(&X_real,
                                    &X_imag,
                                    out_X,
                                    buffer_origin,
                                    buffer_row_pitch,
                                    &queue,
                                    4,
                                    &events[8],
                                    &events[12]);
    if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }

    if (out_y->order == CLRSP_ROW_MAJOR) {
        buffer_row_pitch = l * sizeof(float);
    } else {
        buffer_row_pitch = 1 * sizeof(float);
    }
    if (out_y->layout == CLRSP_INTERLEAVED) {
        buffer_row_pitch *= 2;
    }

    status = clrspReadMatrixFromGPU(&y_real,
                                    &y_imag,
                                    out_y,
                                    buffer_origin,
                                    buffer_row_pitch,
                                    &queue,
                                    4,
                                    &events[8],
                                    &events[14]);
    if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }

    clFinish(queue);

    /* Release OpenCL resources. */
    status = clReleaseMemObject(X_real);
    if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }
    if (X->layout == CLRSP_PLANAR) {
        status = clReleaseMemObject(X_imag);
        if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }
    }
    status = clReleaseMemObject(y_real);
    if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }
    if (X->layout == CLRSP_PLANAR) {
        status = clReleaseMemObject(y_imag);
        if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }
    }
    clfftDestroyPlan(&X_fft_plan);
    clfftDestroyPlan(&y_fft_plan);
    clfftTeardown();
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    /* Prepare out_Xput arguments. */
    plhs[0] = clrspGetmxArray(out_X);
    plhs[1] = clrspGetmxArray(out_y);
    int dims[2] = {1,1};
    plhs[2] = mxCreateNumericArray(2,
                                   dims,
                                   mxUINT64_CLASS,
                                   mxREAL);
    unsigned long *data = mxGetData(plhs[2]);
    data[0] = time;
}
