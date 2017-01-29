/******************************************************************************
*
*   Implementation of the FFT/iFFT interface with the clFFT library.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

cl_int
clrspSetupClfftLibrary()
{
    /* Output of API calls. */
    cl_int status;

    /* Setup clFFT library. */
    clfftSetupData fft_setup;
    status = clfftInitSetupData(&fft_setup);
    if (status != CL_SUCCESS) { return status; };
    status = clfftSetup(&fft_setup);

    return status;
}
