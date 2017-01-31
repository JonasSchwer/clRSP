/******************************************************************************
*
*   Implementation of auxiliary.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

cl_int
clrspReadMatrixFromGPU(cl_mem *A_real,
                       cl_mem *A_imag,
                       clrspComplexMatrix *A,
                       size_t buffer_origin[3],
                       size_t buffer_row_pitch,
                       cl_command_queue *queue,
                       cl_uint num_wait_list,
                       cl_event *wait_list,
                       cl_event events[2])
{
    puts("check x.1");

    cl_int status;

    size_t m = A->rows;
    size_t n = A->cols;

    puts("check x.2");

    /* Copy data from device to host memory. */
    size_t host_origin[3] = {0, 0, 0};
    size_t region[3];
    size_t host_row_pitch;
    if (A->order == CLRSP_ROW_MAJOR) {
        region[0] = n * sizeof(float);
        region[1] = m;
        region[2] = 1;
        host_row_pitch = n * sizeof(float);
    } else {
        region[0] = m * sizeof(float);
        region[1] = n;
        region[2] = 1;
        host_row_pitch = m * sizeof(float);
    }
    if (A->layout == CLRSP_INTERLEAVED) {
        region[0] *= 2;
        host_row_pitch *= 2;
    }

    puts("check x.3");

    status = clEnqueueReadBufferRect(*queue,
                                     *A_real,
                                     CL_FALSE,
                                     buffer_origin,
                                     host_origin,
                                     region,
                                     buffer_row_pitch,
                                     0,
                                     host_row_pitch,
                                     0,
                                     A->real,
                                     num_wait_list,
                                     wait_list,
                                     &events[0]);
    if (status != CL_SUCCESS) { return status; }

    puts("check x.4");

    if (A->layout == CLRSP_PLANAR) {
        status = clEnqueueReadBufferRect(*queue,
                                         *A_imag,
                                         CL_FALSE,
                                         buffer_origin,
                                         host_origin,
                                         region,
                                         buffer_row_pitch,
                                         0,
                                         host_row_pitch,
                                         0,
                                         A->imag,
                                         num_wait_list,
                                         wait_list,
                                         &events[1]);
        if (status != CL_SUCCESS) { return status; }
    } else {
        events[1] = events[0];
    }

    puts("check x.5");

    return status;
}
