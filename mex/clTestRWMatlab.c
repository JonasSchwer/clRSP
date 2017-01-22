/******************************************************************************
*
*   ...
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "clrspMex.h"


void
mexFunction(int nlhs, mxArray *plhs[],
            int nrhs, const mxArray *prhs[])
{
    /* Check input arguments. */
    if (nrhs != 1) {
        mexErrMsgIdAndTxt("MATLAB:clPulseCompression:invalidNumInputs",
                          "One input arguments required.");
    }
    if (!mxIsSingle(prhs[0])
        || !mxIsComplex(prhs[0])
        || mxGetNumberOfDimensions(prhs[0]) != 2) {
        mexErrMsgIdAndTxt("MATLAB:clPulseCompression:invlaidInputs",
                          "First input must be single, complex matrix.");
    }
    if (nlhs > 1) {
        mexErrMsgIdAndTxt("MATLAB:clPulseCompression:invalidNumOutputs",
                          "Too many output arguments.");
    }

    /* Process input arguments. */
    clrspComplexMatrix *M_0 = clrspGetComplexMatrix(prhs[0],
                                                    CLRSP_COL_MAJOR);

    /* Prepare output arguments. */
    plhs[0] = clrspGetmxArray(M_0);
}
