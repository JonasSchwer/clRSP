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

cl_int
clrspSetupSingleDeviceContext(cl_context *context,
                              const cl_context_properties *context_properties,
                              size_t num_context_properties,
                              cl_command_queue *queue,
                              const cl_queue_properties *queue_properties,
                              size_t platform_idx,
                              cl_device_type device_type,
                              size_t device_idx)
{
    /* Output of API calls. */
    cl_int status;

    /* Setup OpenCL environment. */
    cl_uint num_platforms = 0;
    cl_platform_id *platforms;
    status = clrspQueryPlatforms(&num_platforms, &platforms);
    if (status != CL_SUCCESS) { return status; }

    if (platform_idx > num_platforms - 1) {
        fprintf(stderr, "Warning: Requested platform %lu, not existing."
                "Falling back to platform 0\n", platform_idx);
        platform_idx = 0;
    }

    cl_uint num_devices = 0;
    cl_device_id *devices;
    status = clrspQueryDevices(&num_devices, &devices, platforms[platform_idx],
                               device_type);
    if (status != CL_SUCCESS) { return status; }

    if (device_idx > num_devices - 1) {
        fprintf(stderr, "Warning: Requested device %lu, not existing."
                "Falling back to device 0\n", device_idx);
        device_idx = 0;
    }

    /* Setup OpenCL context with specified properties. */
    cl_context_properties cps[num_context_properties + 3];
    cps[0] = CL_CONTEXT_PLATFORM;
    cps[1] = (cl_context_properties)platforms[platform_idx];
    size_t i;
    for (i = 0; i < num_context_properties; ++i) {
        cps[2 + i] = context_properties[i];
    }
    cps[2 + i] = 0;

    *context = clCreateContext(cps, 1, &devices[device_idx], NULL, NULL,
                               &status);
    if (status != CL_SUCCESS) { return status; }

    /* Setup OpenCL command queue with specified properties. */
    /*
    cl_command_queue_properties props = CL_QUEUE_PROFILING_ENABLE;
    *queue = clCreateCommandQueueWithProperties(*context,
                                                devices[device_idx],
                                                &props,
                                                &status);
    */
    *queue = clCreateCommandQueue(*context,
                                  devices[device_idx],
                                  CL_QUEUE_PROFILING_ENABLE,
                                  &status);
    if (status != CL_SUCCESS) {
        clReleaseContext(*context);
        return status;
    }

    return status;
}

cl_int
clrspGetEventProfilingInfo(cl_event *event,
                           const char *name,
                           int verbose)
{
    cl_int status;
    cl_ulong value[4];

    status = clGetEventProfilingInfo(*event,
                                     CL_PROFILING_COMMAND_QUEUED,
                                     sizeof(cl_ulong),
                                     &value[0],
                                     NULL);
    if (status != CL_SUCCESS) { return status; }
    status = clGetEventProfilingInfo(*event,
                                     CL_PROFILING_COMMAND_SUBMIT,
                                     sizeof(cl_ulong),
                                     &value[1],
                                     NULL);
    if (status != CL_SUCCESS) { return status; }
    status = clGetEventProfilingInfo(*event,
                                     CL_PROFILING_COMMAND_START,
                                     sizeof(cl_ulong),
                                     &value[2],
                                     NULL);
    if (status != CL_SUCCESS) { return status; }
    status = clGetEventProfilingInfo(*event,
                                     CL_PROFILING_COMMAND_END,
                                     sizeof(cl_ulong),
                                     &value[3],
                                     NULL);
    if (status != CL_SUCCESS) { return status; }

    if (verbose) {
        printf("\n");
        printf("%19s  %12s\n", " ", "t in ns");
    }
    printf("%19s: %12lu\n", name, (value[3]-value[2]));

    return status;
}
