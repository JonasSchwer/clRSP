/******************************************************************************
*
*   Implementation of the clrspComplexMatrix datatype.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

clrspComplexMatrix*
clrspNewComplexMatrix(size_t rows,
                      size_t cols,
                      clrspStorageOrder order,
                      clrspComplexLayout layout)
{
    clrspComplexMatrix *A;
    A = (clrspComplexMatrix*)malloc(sizeof(clrspComplexMatrix));
    assert(A);
    A->rows = rows;
    A->cols = cols;
    A->order = order;
    A->layout = layout;
    A->real = NULL;
    A->imag = NULL;

    return A;
}
