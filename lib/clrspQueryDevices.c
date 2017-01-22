/******************************************************************************
*
*   Implementation of the OpenCL environment interface.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

cl_int
clrspQueryDevices(cl_uint *num_devices,
                  cl_device_id **devices,
                  cl_platform_id platform,
                  cl_device_type device_type)
{
    /* Output of API calls. */
    cl_int status;

    /* Retrieve the number of devices of type device_type. */
    status = clGetDeviceIDs(platform, device_type, 0, NULL, num_devices);
    if (status != CL_SUCCESS) { return status; }

    /* Allocate enough space for each device. */
    *devices = (cl_device_id*)malloc(*num_devices * sizeof(cl_device_id));
    assert(devices);

    /* Fill in the devices. */
    status = clGetDeviceIDs(platform, device_type, *num_devices, *devices,
                            NULL);
    if (status != CL_SUCCESS) { free(*devices); }

    return status;
}
