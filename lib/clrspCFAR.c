/******************************************************************************
*
*   Implementation of CFAR (host code).
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"


cl_int
clrspCFAR(const clrspComplexMatrix *X,
          cl_mem *X_real,
          cl_mem *X_imag,
          cl_context *context,
          cl_command_queue *queue,
          cl_uint num_wait_list,
          cl_event *wait_list,
          cl_event *event)
{


// Ab hier anpassen

    cl_int status;

    /* Load kernel source code. */
    char *src = clrspLoadKernelSource("../kernels/cfar2dKernel.cl");

    /* Create the program. */
    cl_program program;
    program = clCreateProgramWithSource(*context,
                                        1,
                                        (const char**)&src,
                                        NULL,
                                        &status);
    if (status != CL_SUCCESS) { return status; }
    free(src);

    /* Compile the program. */
    cl_uint num_devices;
    status = clGetContextInfo(*context,
                              CL_CONTEXT_NUM_DEVICES,
                              sizeof(cl_uint),
                              (void*)&num_devices,
                              NULL);
    if (status != CL_SUCCESS) { return status; }

    cl_device_id devices[num_devices];
    status = clGetContextInfo(*context,
                              CL_CONTEXT_DEVICES,
                              num_devices * sizeof(cl_device_id),
                              (void*)devices,
                              NULL);
    if (status != CL_SUCCESS) { return status; }

    char *options = "-w -cl-fast-relaxed-math";
    status = clBuildProgram(program,
                            num_devices,
                            devices,
                            options,
                            NULL,
                            NULL);
    clrspPrintBuildLog(&program,
                       &devices[0]);

    if (status != CL_SUCCESS) { return status; }


    /* Create the kernel. */
    cl_kernel kernel;
    kernel = clCreateKernel(program, "cfar2dKernel", &status);
    if (status != CL_SUCCESS) { return status; }


    /* Determine total number of work-items needed. */
    size_t global_size[2];
    if (X->order == CLRSP_ROW_MAJOR) {
        global_size[0] = X->cols;
        global_size[1] = X->rows;
    } else {
        global_size[0] = X->rows;
        global_size[1] = X->cols;
    }



    size_t *local_size = NULL;

    int rows = (int)X->rows;
    int cols = (int)X->cols;
    float p_fa = 1e-3;
    int guardLength = 4;
    int refWidth = 16;
    int refHeight = 4;


    /* Set kernel arguments. */
    clSetKernelArg(kernel, 0, sizeof(cl_mem), X_real);
    clSetKernelArg(kernel, 1, sizeof(int), &rows);
    clSetKernelArg(kernel, 2, sizeof(int), &cols);
    clSetKernelArg(kernel, 3, sizeof(int), &(X->order));
    clSetKernelArg(kernel, 4, sizeof(cl_mem), X_imag);
    clSetKernelArg(kernel, 5, sizeof(int), &(X->order));
    clSetKernelArg(kernel, 6, sizeof(float), &p_fa);
    clSetKernelArg(kernel, 7, sizeof(int), &guardLength);
    clSetKernelArg(kernel, 8, sizeof(int), &refWidth);
    clSetKernelArg(kernel, 9, sizeof(int), &refHeight);


    status = clEnqueueNDRangeKernel(*queue,
                                    kernel,
                                    2,
                                    NULL,
                                    global_size,
                                    local_size,
                                    num_wait_list,
                                    wait_list,
                                    event);
    if (status != CL_SUCCESS) { return status; }


    return status;
}
