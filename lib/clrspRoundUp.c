/******************************************************************************
*
*   Implementation of auxiliary.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"


size_t
clrspRoundUp(size_t value,
             size_t multiple)
{
    size_t remainder = value % multiple;
    if (remainder != 0) {
        value += multiple - remainder;
    }

    return value;
}
