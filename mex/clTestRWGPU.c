/******************************************************************************
*
*   ...
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"
#include "clrspMex.h"

cl_int
clrspGetEventProfilingInfo(cl_event *event, const char *name, int verbose);

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

    /* Allocate memory on device. */
    cl_mem buf_real;
    buf_real = clCreateBuffer(context,
                              CL_MEM_READ_WRITE,
                              (m + 2 * k) * (n + 2 * k) * sizeof(float),
                              NULL,
                              &status);
    if (status != CL_SUCCESS) { clError(status); }

    cl_mem buf_imag;
    buf_imag = clCreateBuffer(context,
                              CL_MEM_READ_WRITE,
                              (m + 2 * k) * (n + 2 * k) * sizeof(float),
                              NULL,
                              &status);
    if (status != CL_SUCCESS) { clError(status); }

    /* Events for profiling. */
    cl_event events[6];

    /* Initialize device buffers with 0. */
    const float pattern = 0.;
    status = clEnqueueFillBuffer(queue,
                                 buf_real,
                                 (const void*)&pattern,
                                 sizeof(float),
                                 0,
                                 (m + 2 * k) * (n + 2 * k) * sizeof(float),
                                 0,
                                 NULL,
                                 &events[0]);
    if (status != CL_SUCCESS) { clError(status); }

    status = clEnqueueFillBuffer(queue,
                                 buf_imag,
                                 (const void*)&pattern,
                                 sizeof(float),
                                 0,
                                 (m + 2 * k) * (n + 2 * k) * sizeof(float),
                                 0,
                                 NULL,
                                 &events[1]);
    if (status != CL_SUCCESS) { clError(status); }

    /* Copy data to device with zero-padding. */
    size_t buffer_origin[3] = {k * sizeof(float), k, 0};
    size_t host_origin[3] = {0, 0, 0};
    size_t region[3] = {n * sizeof(float), m, 1};

    status = clEnqueueWriteBufferRect(queue,
                                      buf_real,
                                      CL_FALSE,
                                      buffer_origin,
                                      host_origin,
                                      region,
                                      (n + 2 * k) * sizeof(float),
                                      0,
                                      0,
                                      0,
                                      in->real,
                                      2,
                                      &events[0],
                                      &events[2]);
    if (status != CL_SUCCESS) { clError(status); }

    status = clEnqueueWriteBufferRect(queue,
                                      buf_imag,
                                      CL_FALSE,
                                      buffer_origin,
                                      host_origin,
                                      region,
                                      (n + 2 * k) * sizeof(float),
                                      0,
                                      0,
                                      0,
                                      in->imag,
                                      2,
                                      &events[0],
                                      &events[3]);
    if (status != CL_SUCCESS) { clError(status); }

    /* Prepare host for output. */
    clrspComplexMatrix *out = clrspNewComplexMatrix(m + 2 * k,
                                                    n + 2 * k);
    clrspAllocComplexMatrix(out);

    /* Copy zero-padded data back to host. */
    status = clEnqueueReadBuffer(queue,
                                 buf_real,
                                 CL_FALSE,
                                 0,
                                 (m + 2 * k) * (n + 2 * k) * sizeof(float),
                                 out->real,
                                 2,
                                 &events[2],
                                 &events[4]);
    if (status != CL_SUCCESS) { clError(status); }

    status = clEnqueueReadBuffer(queue,
                                 buf_imag,
                                 CL_FALSE,
                                 0,
                                 (m + 2 * k) * (n + 2 * k) * sizeof(float),
                                 out->imag,
                                 2,
                                 &events[2],
                                 &events[5]);
    if (status != CL_SUCCESS) { clError(status); }

    clFinish(queue);

    /* Get and print profiling information. */
    status = clrspGetEventProfilingInfo(&events[0],
                                        "fill real",
                                        1);
    if (status != CL_SUCCESS) { clError(status); }
    status = clrspGetEventProfilingInfo(&events[1],
                                        "fill imag",
                                        0);
    if (status != CL_SUCCESS) { clError(status); }
    status = clrspGetEventProfilingInfo(&events[2],
                                        "write real",
                                        0);
    if (status != CL_SUCCESS) { clError(status); }
    status = clrspGetEventProfilingInfo(&events[3],
                                        "write imag",
                                        0);
    if (status != CL_SUCCESS) { clError(status); }
    status = clrspGetEventProfilingInfo(&events[4],
                                        "read real",
                                        0);
    if (status != CL_SUCCESS) { clError(status); }
    status = clrspGetEventProfilingInfo(&events[5],
                                        "read imag",
                                        0);
    if (status != CL_SUCCESS) { clError(status); }

    /* Release OpenCL resources. */
    clFinish(queue);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    /* Prepare output arguments. */
    plhs[0] = clrspGetmxArray(out);
}
