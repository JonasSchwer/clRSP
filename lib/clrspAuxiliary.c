/******************************************************************************
*
*   Implementation of auxiliary.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

char*
clrspLoadKernelSource(const char *path)
{
    FILE *fptr;
    if (!(fptr = fopen(path, "r"))) {
        char msg[] = "Error, unable to open";
        char err_msg[strlen(msg) + strlen(path) + 2];
        snprintf(err_msg, strlen(msg)+strlen(path)+2, "%s %s", msg, path);
        perror((const char*)err_msg);
        return NULL;
    }

    size_t n;
    size_t n_total = 0;
    int resizes = 1;
    char read_buffer[128];
    char *kernel_source = (char*)malloc(128 * 10 * resizes * sizeof(char));
    while (!feof(fptr)) {
        n = fread((void*)read_buffer, sizeof(char), 128, fptr);
        n_total += n;
        if (n_total >= 128 * 10 * resizes) {
            kernel_source = (char*)realloc(kernel_source,
                                           128 * 10 * (++resizes)
                                           * sizeof(char));
        }
        memcpy(&kernel_source[n_total-n], (const char*)read_buffer, n);
    }
    fclose(fptr);
    kernel_source[n_total] = '\0';
    kernel_source = (char*)realloc(kernel_source, n_total * sizeof(char));

    return kernel_source;
}

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
    size_t buffer_row_pitch = (n + 2 * pcols) * sizeof(float);

    status = clEnqueueWriteBufferRect(*queue,
                                      *A_real,
                                      CL_FALSE,
                                      buffer_origin,
                                      host_origin,
                                      region,
                                      buffer_row_pitch,
                                      0,
                                      0,
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
                                      0,
                                      0,
                                      A->imag,
                                      1,
                                      &events[1],
                                      &events[3]);
    if (status != CL_SUCCESS) { return status; }

    return status;
}

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
    cl_int status;

    size_t m = A->rows;
    size_t n = A->cols;

    /* Copy data from device to host memory. */
    size_t host_origin[3] = {0, 0, 0};
    size_t region[3] = {n * sizeof(float), m, 1};
    size_t host_row_pitch = n * sizeof(float);

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

    return status;
}
