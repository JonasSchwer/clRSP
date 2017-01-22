/******************************************************************************
*
*   Implementation of auxiliary.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"


void
clrspPrintBuildLog(cl_program *program,
                   cl_device_id *device)
{
    size_t ret_size;
    clGetProgramInfo(*program,
                     CL_PROGRAM_KERNEL_NAMES,
                     0,
                     NULL,
                     &ret_size);
    char kernel_names[ret_size];
    clGetProgramInfo(*program,
                     CL_PROGRAM_KERNEL_NAMES,
                     ret_size,
                     kernel_names,
                     NULL);
    printf("Kernel names:\n\t%s\n", kernel_names);

    clGetProgramBuildInfo(*program,
                          *device,
                          CL_PROGRAM_BUILD_LOG,
                          0,
                          NULL,
                          &ret_size);
    char log[ret_size];
    clGetProgramBuildInfo(*program,
                          *device,
                          CL_PROGRAM_BUILD_LOG,
                          ret_size,
                          log,
                          NULL);
    printf("Build log:\n%s\n", log);
}
