/******************************************************************************
*
*   Implementation of auxiliary.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"


cl_int
clrspAllocAndWriteMatrixToGPU(const clrspComplexMatrix *A,
                              cl_mem *A_real,
                              cl_mem *A_imag,
                              size_t padding[4],
                              cl_context *context,
                              cl_mem_flags flags,
                              cl_command_queue *queue,
                              cl_uint num_wait_list,
                              cl_event *wait_list,
                              cl_event events[4])
{
    cl_int status;

    size_t m = A->rows;
    size_t n = A->cols;
    size_t p_top = padding[0];
    size_t p_bottom = padding[1];
    size_t p_left = padding[2];
    size_t p_right = padding[3];

    /* Allocate memory on device. */
    size_t bufsize
        = (m + p_top + p_bottom) * (n + p_left + p_right) * sizeof(float);
    if (A->layout == CLRSP_INTERLEAVED) {
        bufsize *= 2;
    }

    *A_real = clCreateBuffer(*context,
                             flags,
                             bufsize,
                             NULL,
                             &status);
    if (status != CL_SUCCESS) { return status; }

    if (A->layout == CLRSP_PLANAR) {
        *A_imag = clCreateBuffer(*context,
                                 flags,
                                 bufsize,
                                 NULL,
                                 &status);
        if (status != CL_SUCCESS) { return status; }
    } else {
        *A_imag = NULL;
    }

    /* Initialize device buffers with 0. */
    const float pattern = 0.;

    status = clEnqueueFillBuffer(*queue,
                                 *A_real,
                                 (const void*)&pattern,
                                 sizeof(float),
                                 0,
                                 bufsize,
                                 num_wait_list,
                                 wait_list,
                                 &events[0]);

    if (A->layout == CLRSP_PLANAR) {
        status = clEnqueueFillBuffer(*queue,
                                     *A_imag,
                                     (const void*)&pattern,
                                     sizeof(float),
                                     0,
                                     bufsize,
                                     num_wait_list,
                                     wait_list,
                                     &events[1]);
        if (status != CL_SUCCESS) { puts("check 1"); return status; }
    } else {
        events[1] = events[0];
    }

    /* Copy data to device with zero-padding. */
    size_t host_origin[3] = {0, 0, 0};

    size_t region[3];
    size_t buffer_origin[3];
    size_t buffer_row_pitch;
    size_t host_row_pitch;
    if (A->order == CLRSP_ROW_MAJOR) {
        region[0] = n * sizeof(float);
        region[1] = m;
        region[2] = 1;
        buffer_origin[0] = p_left * sizeof(float);
        buffer_origin[1] = p_top;
        buffer_origin[2] = 0;
        buffer_row_pitch = (n + p_left + p_right) * sizeof(float);
        host_row_pitch = n * sizeof(float);
    } else {
        region[0] = m * sizeof(float);
        region[1] = n;
        region[2] = 1;
        buffer_origin[0] = p_top * sizeof(float);
        buffer_origin[1] = p_left;
        buffer_origin[2] = 0;
        buffer_row_pitch = (m + p_top + p_bottom) * sizeof(float);
        host_row_pitch = m * sizeof(float);
    }
    if (A->layout == CLRSP_INTERLEAVED) {
        region[0] *= 2;
        buffer_origin[0] *= 2;
        buffer_row_pitch *= 2;
        host_row_pitch *= 2;
    }

    status = clEnqueueWriteBufferRect(*queue,
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
                                      1,
                                      &events[0],
                                      &events[2]);
    if (status != CL_SUCCESS) { puts("check 2"); return status; }

    if (A->layout == CLRSP_PLANAR) {
        status = clEnqueueWriteBufferRect(*queue,
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
                                          1,
                                          &events[1],
                                          &events[3]);
        if (status != CL_SUCCESS) { puts("check 3"); return status; }
    } else {
        events[3] = events[2];
    }

    return status;
}
