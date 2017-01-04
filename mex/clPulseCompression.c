/******************************************************************************
*
*   ...
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "mex.h"

#include "../clRSP.h"

void
clError(cl_int status) {
    char err_msg[64];
    snprintf(err_msg, 64, "OpenCL/clFFT error code %i", status);
    mexErrMsgIdAndTxt("MATLAB:clPulsecompression:OpenCLError", err_msg);
}

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
        || (mxGetN(prhs[0]) != 1 && mxGetM(prhs[0]) != 1)
        || !mxIsSingle(prhs[1])
        || !mxIsComplex(prhs[1])
        || mxGetNumberOfDimensions(prhs[1]) != 2) {
        mexErrMsgIdAndTxt("MATLAB:clPulseCompression:invlaidInputs",
                          "First input must be single, complex vector."
                          "Second input must be single, complex matrix.");
    }

    /* Process input arguments. */
    size_t k = mxGetM(prhs[0]) == 1 ? mxGetN(prhs[0]) : mxGetM(prhs[0]);
    float *y_real = (float*)mxGetData(prhs[0]);
    float *y_imag = (float*)mxGetImagData(prhs[0]);

    size_t m = mxGetM(prhs[1]);
    size_t n = mxGetN(prhs[1]);
    float *M_0_real = (float*)mxGetData(prhs[1]);
    float *M_0_imag = (float*)mxGetImagData(prhs[1]);

    /* Prepare output arguments. */
    if (nlhs > 1) {
        mexErrMsgIdAndTxt("MATLAB:clPulseCompression:invalidNumOutputs",
                          "Too many output arguments.");
    }
    plhs[0] = mxCreateNumericMatrix(m, n + k - 1, mxSINGLE_CLASS, mxCOMPLEX);
    float *M_pc_real = (float*)mxGetData(plhs[0]);
    float *M_pc_imag = (float*)mxGetImagData(plhs[0]);

    cl_int status;

    /* Setup OpenCL environment. */
    cl_context context;
    cl_command_queue queue;
    status = clrspSetupSingleDeviceContext(&context,
                                           NULL,
                                           0,
                                           &queue,
                                           NULL,
                                           0,
                                           CL_DEVICE_TYPE_GPU,
                                           0);
    if (status != CL_SUCCESS) { clError(status); }

    /* Setup clfft library. */
    clrspClfftStatus setup;
    status = clrspSetupClfftLibrary(&setup);
    if (status != CL_SUCCESS) { clError(status); }

    /* perform clPulseCompression. */
    clrspComplexMatrix *y = clrspNewComplexMatrix(1,
                                                  k,
                                                  CLRSP_COLUMN_MAJOR);
    y->real = y_real;
    y->imag = y_imag;
    clrspComplexMatrix *M_0 = clrspNewComplexMatrix(m,
                                                    n,
                                                    CLRSP_COLUMN_MAJOR);
    M_0->real = M_0_real;
    M_0->imag = M_0_imag;
    clrspComplexMatrix *M_pc = clrspNewComplexMatrix(m,
                                                     n + k - 1,
                                                     CLRSP_COLUMN_MAJOR);
    M_pc->real = M_pc_real;
    M_pc->imag = M_pc_imag;

    status = clrspPulseCompression(y,
                                   M_0,
                                   &context,
                                   &queue,
                                   &setup,
                                   M_pc);
    if (status != CL_SUCCESS) { clError(status); }

    /* Release resources. */
    clFinish(queue);
    clfftTeardown();
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
}
