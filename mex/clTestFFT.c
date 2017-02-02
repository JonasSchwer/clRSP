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
        mexErrMsgIdAndTxt("MATLAB:clTestFFT:invalidNumInputs",
                          "Six input arguments required.");
    }
    if (!mxIsSingle(prhs[0])
        || !mxIsComplex(prhs[0])
        || mxGetNumberOfDimensions(prhs[0]) != 2
        || !mxIsScalar(prhs[1])) {
        mexErrMsgIdAndTxt("MATLAB:clTestFFT:invlaidInputs",
                          "First input must be single, complex matrix."
                          "Second input must be scalar.");
    }
    if (nlhs > 2) {
        mexErrMsgIdAndTxt("MATLAB:clTestFFT:invalidNumOutputs",
                          "Too many output arguments.");
    }

    /* Process input arguments. */
    char *fft_plan_option = mxArrayToString(prhs[2]);
    clrspPlanDimension fft_plan_dimension = CLRSP_ROW_WISE;
    if (strcmp(fft_plan_option, "row-wise") == 0) {
        fft_plan_dimension = CLRSP_ROW_WISE;
    } else if (strcmp(fft_plan_option, "col-wise") == 0) {
        fft_plan_dimension = CLRSP_COL_WISE;
    } else {
        mexErrMsgIdAndTxt("MATLAB:clTestFFT:invalidInputs",
                          "Unknown fft_plan_option.");
    }

    char *storage_option = mxArrayToString(prhs[nrhs - 3]);
    clrspStorageOrder order = CLRSP_ROW_MAJOR;
    if (strcmp(storage_option, "row-major") == 0) {
        order = CLRSP_ROW_MAJOR;
    } else if (strcmp(storage_option, "col-major") == 0) {
        order = CLRSP_COL_MAJOR;
    } else {
        mexErrMsgIdAndTxt("MATLAB:clTestRWGPU:invalidInputs",
                          "Unknown storage option.");
    }

    char *layout_option = mxArrayToString(prhs[nrhs - 2]);
    clrspComplexLayout layout = CLRSP_PLANAR;
    if (strcmp(layout_option, "planar") == 0) {
        layout = CLRSP_PLANAR;
    } else if (strcmp(layout_option, "interleaved") == 0) {
        layout = CLRSP_INTERLEAVED;
    } else {
        mexErrMsgIdAndTxt("MATLAB:clTestRWGPU:invalidInputs",
                          "Unknown layout option.");
    }

    char *device_option = mxArrayToString(prhs[nrhs - 1]);
    cl_device_type device = CL_DEVICE_TYPE_GPU;
    if (strcmp(device_option, "gpu") == 0) {
        device = CL_DEVICE_TYPE_GPU;
    } else if (strcmp(device_option, "cpu") == 0) {
        device = CL_DEVICE_TYPE_CPU;
    } else {
        mexErrMsgIdAndTxt("MATLAB:clTestRWGPU:invalidInputs",
                          "Unknown device option.");
    }

    size_t m = mxGetM(prhs[0]);
    size_t l = mxGetN(prhs[0]);
    size_t primes[4] = {7, 5, 3, 2};
    size_t n = clrspNearestPower(l,
                                 4,
                                 &primes[0]);
    clrspComplexMatrix *in = clrspGetComplexMatrix(prhs[0],
                                                   order,
                                                   layout);

    size_t runs = mxGetScalar(prhs[1]);

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

    clrspComplexMatrix *temp = clrspNewComplexMatrix(m,
                                                     n,
                                                     in->order,
                                                     in->layout);
    /* Create clFFT plan. */
    clfftPlanHandle plan;
    status = clrspCreate1DfftPlan(&plan,
                                  &context,
                                  &queue,
                                  fft_plan_dimension,
                                  temp);
    cl_event events[7];

    /* Allocate memory on device and copy data. */
    cl_mem buf_real;
    cl_mem buf_imag;
    size_t padding[4] = {0, 0, 0, n-l};

    status = clrspAllocAndWriteMatrixToGPU(in,
                                           &buf_real,
                                           &buf_imag,
                                           padding,
                                           &context,
                                           CL_MEM_READ_WRITE,
                                           &queue,
                                           0,
                                           NULL,
                                           &events[0]);
    if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }

    int i;
    cl_ulong time = 0;
    for (i = 0; i < runs; ++i) {

        /* Perform forward fft. */
        status = clrspFFT(&buf_real,
                          &buf_imag,
                          &context,
                          &queue,
                          &plan,
                          4,
                          &events[0],
                          &events[4]);
        if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }

        clFinish(queue);

        cl_ulong duration;
        status = clrspGetEventDuration(&events[4],
                                       &duration);
        time += duration;
    }
    time /= runs;

    /* Prepare host for output. */
    clrspComplexMatrix *out = clrspNewComplexMatrix(m,
                                                    n,
                                                    in->order,
                                                    in->layout);
    clrspAllocComplexMatrix(out);

    /* Copy zero-padded data back to host. */
    size_t buffer_origin[3] = {0, 0, 0};
    size_t buffer_row_pitch;
    if (out->order == CLRSP_ROW_MAJOR) {
        buffer_row_pitch = n * sizeof(float);
    } else {
        buffer_row_pitch = m * sizeof(float);
    }
    if (out->layout == CLRSP_INTERLEAVED) {
        buffer_row_pitch *= 2;
    }

    status = clrspReadMatrixFromGPU(&buf_real,
                                    &buf_imag,
                                    out,
                                    buffer_origin,
                                    buffer_row_pitch,
                                    &queue,
                                    1,
                                    &events[4],
                                    &events[5]);
    if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }

    /* Release OpenCL resources. */
    clFinish(queue);

    status = clReleaseMemObject(buf_real);
    if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }
    if (in->layout == CLRSP_PLANAR) {
        status = clReleaseMemObject(buf_imag);
        if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }
    }

    clfftDestroyPlan(&plan);
    clfftTeardown();
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    /* Prepare output arguments. */
    plhs[0] = clrspGetmxArray(out);
    int dims[2] = {1,1};
    plhs[1] = mxCreateNumericArray(2,
                                   dims,
                                   mxUINT64_CLASS,
                                   mxREAL);
    unsigned long *data = mxGetData(plhs[1]);
    data[0] = time;
}
