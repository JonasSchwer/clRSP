/******************************************************************************
*
*   Implementation of the FFT/iFFT interface with the clFFT library.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

cl_int
clrspIFFT(cl_mem *real,
          cl_mem *imag,
          cl_context *context,
          cl_command_queue *queue,
          clfftPlanHandle *plan,
          cl_uint num_wait_events,
          const cl_event *wait_events,
          cl_event *event)
{
    cl_int status;

    cl_mem inputBufs[2] = {*real, *imag};
    /* Enqueue forward FFT on queue. */
    status = clfftEnqueueTransform(*plan, CLFFT_BACKWARD, 1, queue,
                                   num_wait_events, wait_events, event,
                                   inputBufs, NULL, NULL);

    return status;
}
