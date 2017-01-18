/******************************************************************************
*
*   Implementation of the FFT/iFFT interface with the clFFT library.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

cl_int
clrspSetupClfftLibrary(clrspClfftStatus *setup)
{
    /* Output of API calls. */
    cl_int status;

    /* Setup clFFT library. */
    clfftSetupData fft_setup;
    status = clfftInitSetupData(&fft_setup);
    if (status != CL_SUCCESS) { return status; };
    status = clfftSetup(&fft_setup);

    *setup = CLRSP_CLFFT_SETUP;

    return status;
}


cl_int
clrspCreate1DfftPlan(clfftPlanHandle *plan,
                     cl_context *context,
                     cl_command_queue *queue,
                     clrspPlanDimension dim,
                     clrspComplexMatrix *A)
{
    cl_int status;

    /* Create clFFT default plan. */
    size_t clLengths = (dim == CLRSP_ROW_WISE) ? A->cols : A->rows;
    status = clfftCreateDefaultPlan(plan, *context, CLFFT_1D, &clLengths);
    if (status != CL_SUCCESS) { return status; }

    /* Set plan precision. */
    status = clfftSetPlanPrecision(*plan, CLFFT_SINGLE);
    if (status != CL_SUCCESS) { return status; }

    /* Set data layout. */
    status = clfftSetLayout(*plan, CLFFT_COMPLEX_PLANAR, CLFFT_COMPLEX_PLANAR);
    if (status != CL_SUCCESS) { return status; }

    /* Set plan result location. */
    status = clfftSetResultLocation(*plan, CLFFT_INPLACE);
    if (status != CL_SUCCESS) { return status; }

    /* Set data strides. */
    size_t clStrides;
    if (dim == CLRSP_ROW_WISE) {
        clStrides = 1;
    } else {
        clStrides = A->cols;
    }
    status = clfftSetPlanInStride(*plan, CLFFT_1D, &clStrides);
    status = clfftSetPlanOutStride(*plan, CLFFT_1D, &clStrides);
    if (status != CL_SUCCESS) { return status; }

    /* Set plan batch size. */
    size_t batch_size = (dim == CLRSP_ROW_WISE) ? A->rows : A->cols;
    status = clfftSetPlanBatchSize(*plan, batch_size);
    if (status != CL_SUCCESS) { return status; }

    /* Set plan batch distance. */
    size_t dist = dim ? 1 : A->rows;
    if (dim == CLRSP_ROW_WISE) {
        dist = A->cols;
    } else {
        dist = 1;
    }
    status = clfftSetPlanDistance(*plan, dist, dist);
    if (status != CL_SUCCESS) { return status; }

    /* Bake the plan. */
    status = clfftBakePlan(*plan, 1, queue, NULL, NULL);

    return status;
}


cl_int
clrspFFT(cl_mem *real,
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
    status = clfftEnqueueTransform(*plan, CLFFT_FORWARD, 1, queue,
                                   num_wait_events, wait_events, event,
                                   inputBufs, NULL, NULL);

    return status;
}


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
