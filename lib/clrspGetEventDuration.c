/******************************************************************************
*
*   Implementation of the OpenCL environment interface.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

cl_int
clrspGetEventDuration(cl_event *event,
                      cl_ulong *duration)
{
    cl_int status;

    cl_ulong start;
    cl_ulong end;

    status = clGetEventProfilingInfo(*event,
                                     CL_PROFILING_COMMAND_START,
                                     sizeof(cl_ulong),
                                     &start,
                                     NULL);
    if (status != CL_SUCCESS) { return status; }

    status = clGetEventProfilingInfo(*event,
                                     CL_PROFILING_COMMAND_END,
                                     sizeof(cl_ulong),
                                     &end,
                                     NULL);
    if (status != CL_SUCCESS) { return status; }

    *duration = end - start;

    return status;
}
