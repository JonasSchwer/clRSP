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
        || !mxIsScalar(prhs[1])) {
        mexErrMsgIdAndTxt("MATLAB:clPulseCompression:invlaidInputs",
                          "First input must be single, complex matrix.",
                          "Second input must be positive scalar.");
    }
    if (nlhs > 4) {
        mexErrMsgIdAndTxt("MATLAB:clPulseCompression:invalidNumOutputs",
                          "Too many output arguments.");
    }

    /* Process input arguments. */
    size_t m = mxGetM(prhs[0]);
    size_t n = mxGetN(prhs[0]);
    clrspComplexMatrix *in = clrspGetComplexMatrix(prhs[0]);
    size_t k = mxGetScalar(prhs[1]);

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
                                           CL_DEVICE_TYPE_GPU,
                                           0);
    if (status != CL_SUCCESS) { clError(status); }

    cl_event events[6];

    /* Allocate memory on device and copy data. */
    cl_mem buf_real;
    cl_mem buf_imag;
    size_t padding[2] = {k, k};

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
    if (status != CL_SUCCESS) { clError(status); }

    /* Prepare host for output. */
    clrspComplexMatrix *out = clrspNewComplexMatrix(m + 2 * k,
                                                    n + 2 * k);
    clrspAllocComplexMatrix(out);

    /* Copy zero-padded data back to host. */
    size_t buffer_origin[3] = {0, 0, 0};
    size_t buffer_row_pitch = (n + 2 * k) * sizeof(float);

    status = clrspReadMatrixFromGPU(&buf_real,
                                    &buf_imag,
                                    out,
                                    buffer_origin,
                                    buffer_row_pitch,
                                    &queue,
                                    2,
                                    &events[2],
                                    &events[4]);
    if (status != CL_SUCCESS) { clError(status); }

    clFinish(queue);

    /* Release OpenCL resources. */
    clFinish(queue);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    /* Prepare output arguments. */
    plhs[0] = clrspGetmxArray(out);
}
