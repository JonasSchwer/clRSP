/******************************************************************************
*
*   Implementation of the OpenCL environment interface.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

cl_int
clrspQueryPlatforms(cl_uint *num_platforms,
                    cl_platform_id **platforms)
{
    cl_int status;

    /* Retrieve the number of platforms. */
    status = clGetPlatformIDs(0, NULL, num_platforms);
    if (status != CL_SUCCESS) { return status; }

    /* Allocate enough space for each platform. */
    *platforms = (cl_platform_id*)malloc(*num_platforms
                                         * sizeof(cl_platform_id));
    assert(platforms);

    /* Fill in the platforms. */
    status = clGetPlatformIDs(*num_platforms, *platforms, NULL);
    if (status != CL_SUCCESS) { free(*platforms); }

    return status;
}
