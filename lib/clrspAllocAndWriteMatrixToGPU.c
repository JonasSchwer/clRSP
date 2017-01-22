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
                              size_t padding[2],
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
    size_t prows = padding[0];
    size_t pcols = padding[1];

    /* Allocate memory on device. */
    size_t bufsize = (m + 2 * prows) * (n + 2 * pcols) * sizeof(float);

    *A_real = clCreateBuffer(*context,
                             flags,
                             bufsize,
                             NULL,
                             &status);
    if (status != CL_SUCCESS) { return status; }

    *A_imag = clCreateBuffer(*context,
                             flags,
                             bufsize,
                             NULL,
                             &status);
    if (status != CL_SUCCESS) { return status; }

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
    if (status != CL_SUCCESS) { return status; }

    status = clEnqueueFillBuffer(*queue,
                                 *A_imag,
                                 (const void*)&pattern,
                                 sizeof(float),
                                 0,
                                 bufsize,
                                 num_wait_list,
                                 wait_list,
                                 &events[1]);
    if (status != CL_SUCCESS) { return status; }

    /* Copy data to device with zero-padding. */
    size_t buffer_origin[3] = {prows * sizeof(float), pcols, 0};
    size_t host_origin[3] = {0, 0, 0};
    size_t region[3] = {n * sizeof(float), m, 1};
    size_t buffer_row_pitch;
    size_t host_row_pitch;
    if (A->order == CLRSP_ROW_MAJOR) {
        region[0] = n * sizeof(float);
        region[1] = m;
        region[2] = 1;
        buffer_row_pitch = (n + 2 * pcols) * sizeof(float);
        host_row_pitch = n * sizeof(float);
    } else {
        region[0] = m * sizeof(float);
        region[1] = n;
        region[2] = 1;
        buffer_row_pitch = (m + 2 * pcols) * sizeof(float);
        host_row_pitch = m * sizeof(float);
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
    if (status != CL_SUCCESS) { return status; }

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
    if (status != CL_SUCCESS) { return status; }

    return status;
}
