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
    if (nrhs != 8) {
        mexErrMsgIdAndTxt("MATLAB:clPulseCompression:invalidNumInputs",
                          "Five input arguments required.");
    }
    if (!mxIsSingle(prhs[0])
        || !mxIsComplex(prhs[0])
        || mxGetNumberOfDimensions(prhs[0]) != 2
        || !mxIsScalar(prhs[1])
        || !mxIsScalar(prhs[2])
        || !mxIsScalar(prhs[3])
        || !mxIsScalar(prhs[4])) {
        mexErrMsgIdAndTxt("MATLAB:clPulseCompression:invlaidInputs",
                          "First input must be single, complex matrix."
                          "Second to fifth input must be positive scalar.");
    }
    if (nlhs > 4) {
        mexErrMsgIdAndTxt("MATLAB:clPulseCompression:invalidNumOutputs",
                          "Too many output arguments.");
    }

    /* Process input arguments. */
    char *storage_option = mxArrayToString(prhs[5]);
    clrspStorageOrder order = CLRSP_ROW_MAJOR;
    if (strcmp(storage_option, "row-major") == 0) {
        order = CLRSP_ROW_MAJOR;
    } else if (strcmp(storage_option, "col-major") == 0) {
        order = CLRSP_COL_MAJOR;
    } else {
        mexErrMsgIdAndTxt("MATLAB:clTestRWGPU:invalidInputs",
                          "Unknown storage option.");
    }
    char *layout_option = mxArrayToString(prhs[6]);
    clrspComplexLayout layout = CLRSP_PLANAR;
    if (strcmp(layout_option, "planar") == 0) {
        layout = CLRSP_PLANAR;
    } else if (strcmp(layout_option, "interleaved") == 0) {
        layout = CLRSP_INTERLEAVED;
    } else {
        mexErrMsgIdAndTxt("MATLAB:clTestRWGPU:invalidInputs",
                          "Unknown layout option.");
    }
    char *device_option = mxArrayToString(prhs[7]);
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
    size_t n = mxGetN(prhs[0]);
    clrspComplexMatrix *in = clrspGetComplexMatrix(prhs[0],
                                                   order,
                                                   layout);
    size_t pt = mxGetScalar(prhs[1]);
    size_t pb = mxGetScalar(prhs[2]);
    size_t pl = mxGetScalar(prhs[3]);
    size_t pr = mxGetScalar(prhs[4]);

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

    cl_event events[6];

    /* Allocate memory on device and copy data. */
    cl_mem buf_real;
    cl_mem buf_imag;
    size_t padding[4] = {pt, pb, pl, pr};

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

    /* Prepare host for output. */
    clrspComplexMatrix *out = clrspNewComplexMatrix(m + padding[0] + padding[1],
                                                    n + padding[2] + padding[3],
                                                    in->order,
                                                    in->layout);
    clrspAllocComplexMatrix(out);

    /* Copy zero-padded data back to host. */
    size_t buffer_origin[3] = {0, 0, 0};
    size_t buffer_row_pitch;
    if (out->order == CLRSP_ROW_MAJOR) {
        buffer_row_pitch = (n + padding[2] + padding[3]) * sizeof(float);
    } else {
        buffer_row_pitch = (m + padding[0] + padding[1]) * sizeof(float);
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
                                    2,
                                    &events[2],
                                    &events[4]);
    if (status != CL_SUCCESS) { clError(status, __FILE__, __LINE__); }

    clFinish(queue);

    /* Release OpenCL resources. */
    clFinish(queue);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    /* Prepare output arguments. */
    plhs[0] = clrspGetmxArray(out);
}
