/******************************************************************************
*
*   Implementation of the pulsecompression.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"


cl_int
clrspPulseCompression(clrspComplexMatrix *y,
                      clfftPlanHandle *y_fft_plan,
                      cl_mem *y_real,
                      cl_mem *y_imag,
                      clrspComplexMatrix *X,
                      clfftPlanHandle *X_fft_plan,
                      cl_mem *X_real,
                      cl_mem *X_imag,
                      cl_context *context,
                      cl_command_queue *queue,
                      cl_uint num_wait_events,
                      const cl_event *wait_events,
                      cl_event events[4])
{
    cl_int status;

    /* Perform forward FFT. */
    status = clrspFFT(y_real,
                      y_imag,
                      context,
                      queue,
                      y_fft_plan,
                      num_wait_events,
                      wait_events,
                      &events[0]);
    if (status != CL_SUCCESS) { return status; }

    status = clrspFFT(X_real,
                      X_imag,
                      context,
                      queue,
                      X_fft_plan,
                      num_wait_events,
                      wait_events,
                      &events[1]);
    if (status != CL_SUCCESS) { return status; }

    /* Perform element-wise product on each row. */
    status = clrspElementwiseProduct(X,
                                     X_real,
                                     X_imag,
                                     y,
                                     y_real,
                                     y_imag,
                                     context,
                                     queue,
                                     2,
                                     &events[0],
                                     &events[2]);
    if (status != CL_SUCCESS) {return status; }

    /* Perform backward FFT. */
    status = clrspIFFT(X_real,
                       X_imag,
                       context,
                       queue,
                       X_fft_plan,
                       1,
                       &events[2],
                       &events[3]);
    if (status != CL_SUCCESS) { return status; }
//    events[2] = events[0];
//    events[3] = events[0];

    return status;
}
