/******************************************************************************
*
*   Implementation of the FFT/iFFT interface with the clFFT library.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

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
    if (A->layout == CLRSP_PLANAR) {
        status = clfftSetLayout(*plan,
                                CLFFT_COMPLEX_PLANAR,
                                CLFFT_COMPLEX_PLANAR);
    } else {
        status = clfftSetLayout(*plan,
                                CLFFT_COMPLEX_INTERLEAVED,
                                CLFFT_COMPLEX_INTERLEAVED);
    }
    if (status != CL_SUCCESS) { return status; }

    /* Set plan result location. */
    status = clfftSetResultLocation(*plan, CLFFT_INPLACE);
    if (status != CL_SUCCESS) { return status; }

    /* Set data strides. */
    size_t clStrides;
    if (dim == CLRSP_ROW_WISE) {
        clStrides = (A->order == CLRSP_ROW_MAJOR) ? 1 : A->rows;
    } else {
        clStrides = (A->order == CLRSP_COL_MAJOR) ? 1 : A->cols;
    }
    status = clfftSetPlanInStride(*plan, CLFFT_1D, &clStrides);
    status = clfftSetPlanOutStride(*plan, CLFFT_1D, &clStrides);
    if (status != CL_SUCCESS) { return status; }

    /* Set plan batch size. */
    size_t batch_size = (dim == CLRSP_ROW_WISE) ? A->rows : A->cols;
    status = clfftSetPlanBatchSize(*plan, batch_size);
    if (status != CL_SUCCESS) { return status; }

    /* Set plan batch distance. */
    size_t dist;
    if (dim == CLRSP_ROW_WISE) {
        dist = (A->order == CLRSP_ROW_MAJOR) ? A->cols : 1;
    } else {
        dist = (A->order == CLRSP_COL_MAJOR) ? A->rows : 1;
    }
    status = clfftSetPlanDistance(*plan, dist, dist);
    if (status != CL_SUCCESS) { return status; }

    /* Bake the plan. */
    status = clfftBakePlan(*plan, 1, queue, NULL, NULL);

    return status;
}
