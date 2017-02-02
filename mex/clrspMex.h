/******************************************************************************
*
*   Auxiliary functions to Matlab-mex interface.
*
*   Author(s): Michael, Thoma
*
******************************************************************************/

#ifndef CLRSPMEX_H
#define CLRSPMEX_H

#include "mex.h"

#include "../clRSP.h"

/* Give OpenCL error in Matlab. */
void
clError(cl_int status, const char *file, const int line) {
    char err_msg[1024];
    snprintf(err_msg, 1024, "OpenCL/clFFT error code %i,\n"
             "in file: %s\n"
             "in line: %i",
             status, file, line);
    mexErrMsgIdAndTxt("MATLAB:clrspMex:OpenCLError", err_msg);
}

/* Get clrspComplexMatrix form mxArray (Matlab matrix) A. */
clrspComplexMatrix*
clrspGetComplexMatrix(const mxArray *A,
                      clrspStorageOrder order,
                      clrspComplexLayout layout)
{
    size_t m = mxGetM(A);
    size_t n = mxGetN(A);
    float *real = (float*)mxGetData(A);
    float *imag = (float*)mxGetImagData(A);

    clrspComplexMatrix *B = clrspNewComplexMatrix(m,
                                                  n,
                                                  order,
                                                  layout);
    clrspAllocComplexMatrix(B);

    size_t i, j;
    if (order == CLRSP_COL_MAJOR) {
        /* Column major. */
        for (i = 0; i < m; ++i) {
            for (j = 0; j < n; ++j) {
                if (layout == CLRSP_PLANAR) {
                    B->real[i * n + j] = real[i * n + j];
                    B->imag[i * n + j] = imag[i * n + j];
                } else {
                    B->real[2 * (i * n + j)]     = real[i * n + j];
                    B->real[2 * (i * n + j) + 1] = imag[i * n + j];
                }
            }
        }
    } else {
        /* Row major. */
        for (j = 0; j < n; ++j) {
            for (i = 0; i < m; ++i) {
                if (layout == CLRSP_PLANAR) {
                    B->real[i * n + j] = real[j * m + i];
                    B->imag[i * n + j] = imag[j * m + i];
                } else {
                    B->real[2 * (i * n + j)]     = real[j * m + i];
                    B->real[2 * (i * n + j) + 1] = imag[j * m + i];
                }
            }
        }
    }

    return B;
}

/* Get mxArray from clrspComplexMatrix A. */
mxArray*
clrspGetmxArray(clrspComplexMatrix *A)
{
    size_t m = A->rows;
    size_t n = A->cols;

    mxArray *B = mxCreateNumericMatrix(m, n, mxSINGLE_CLASS, mxCOMPLEX);
    float *real = (float*)mxGetData(B);
    float *imag = (float*)mxGetImagData(B);

    size_t i, j;
    if (A->order == CLRSP_COL_MAJOR) {
        /* Column major. */
        for (i = 0; i < m; ++i) {
            for (j = 0; j < n; ++j) {
                if (A->layout == CLRSP_PLANAR) {
                    real[i * n + j] = A->real[i * n + j];
                    imag[i * n + j] = A->imag[i * n + j];
                } else {
                    real[i * n + j] = A->real[2 * (i * n + j)];
                    imag[i * n + j] = A->real[2 * (i * n + j) + 1];
                }
            }
        }
    } else {
        /* Row major. */
        for (i = 0; i < m; ++i) {
            for (j = 0; j < n; ++j) {
                if (A->layout == CLRSP_PLANAR) {
                    real[j * m + i] = A->real[i * n + j];
                    imag[j * m + i] = A->imag[i * n + j];
                } else {
                    real[j * m + i] = A->real[2 * (i * n + j)];
                    imag[j * m + i] = A->real[2 * (i * n + j) + 1];
                }
            }
        }
    }

    return B;
}

#endif // CLRSPMEX_H
