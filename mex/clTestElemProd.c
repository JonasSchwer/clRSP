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
    if (nrhs != 2) {
        mexErrMsgIdAndTxt("MATLAB:clPulseCompression:invalidNumInputs",
                          "Two input arguments required.");
    }
    if (!mxIsSingle(prhs[0])
        || !mxIsComplex(prhs[0])
        || mxGetNumberOfDimensions(prhs[0]) != 2
        || !mxIsSingle(prhs[1])
        || !mxIsComplex(prhs[1])
        || (mxGetN(prhs[1]) != 1 && mxGetM(prhs[1]) != 1)) {
        mexErrMsgIdAndTxt("MATLAB:clPulseCompression:invlaidInputs",
                          "First input must be single, complex matrix.",
                          "Second input must be single, complex vector.");
    }
    if (nlhs > 1) {
        mexErrMsgIdAndTxt("MATLAB:clPulseCompression:invalidNumOutputs",
                          "Too many output arguments.");
    }

    /* Process input arguments. */
    size_t m = mxGetM(prhs[0]);
    size_t n = mxGetN(prhs[0]);
    clrspComplexMatrix *X = clrspGetComplexMatrix(prhs[0]);

    size_t ny = mxGetM(prhs[1]) > mxGetN(prhs[1])
                        ? mxGetM(prhs[1]) : mxGetN(prhs[1]);
    if (n != ny) {
        mexErrMsgIdAndTxt("MATLAB:clTestElemProd:invalidDimensions",
                          "Length of y must be same as number of rows of X.");
    }
    clrspComplexMatrix *y = clrspGetComplexMatrix(prhs[1]);


    /* Setup OpenCL environment. */
    cl_int status;
    cl_context context;
    cl_command_queue queue;
    cl_command_queue_properties queue_props = CL_QUEUE_PROFILING_ENABLE;
    status = clrspSetupSingleDeviceContext(&context,
                                           NULL,
                                           0,
                                           &queue,
                                           &queue_props,
                                           0,
                                           CL_DEVICE_TYPE_GPU,
                                           0);
    if (status != CL_SUCCESS) { clError(status); }

    cl_event events[11];

    /* Allocate memory on device and copy data. */
    cl_mem X_real;
    cl_mem X_imag;
    size_t padding[2] = {0, 0};

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
    if (status != CL_SUCCESS) { clError(status); }

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
    if (status != CL_SUCCESS) { clError(status); }

    int i;
    cl_ulong time = 0;
    for (i = 0; i < 10; ++i) {

        /* Perform element-wise product. */
        status = clrspElementwiseProduct(X,
                                         &X_real,
                                         &X_imag,
                                         y,
                                         &y_real,
                                         &y_imag,
                                         &context,
                                         &queue,
                                         8,
                                         &events[0],
                                         &events[8]);
        if (status != CL_SUCCESS) { clError(status); }

        clFinish(queue);

        cl_ulong duration;
        status = clrspGetEventDuration(&events[8],
                                       &duration);
        time += duration;
    }
    time /= 10;
    printf("%6lu %6lu %lu\n", m, n, time);

    /* Prepare host for output. */
    clrspComplexMatrix *out = clrspNewComplexMatrix(m,
                                                    n);
    clrspAllocComplexMatrix(out);

    /* Copy zero-padded data back to host. */
    size_t buffer_origin[3] = {0, 0, 0};
    size_t buffer_row_pitch = n * sizeof(float);

    status = clrspReadMatrixFromGPU(&X_real,
                                    &X_imag,
                                    out,
                                    buffer_origin,
                                    buffer_row_pitch,
                                    &queue,
                                    1,
                                    &events[8],
                                    &events[9]);
    if (status != CL_SUCCESS) { clError(status); }

    clFinish(queue);

    /* Release OpenCL resources. */
    clFinish(queue);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    /* Prepare output arguments. */
    plhs[0] = clrspGetmxArray(out);
}
