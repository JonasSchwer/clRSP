/******************************************************************************
*
*   Implementation of the clrspComplexMatrix datatype.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

void
clrspFreeComplexMatrix(clrspComplexMatrix *A)
{
    free(A->real);
    free(A->imag);
    free(A);
}
