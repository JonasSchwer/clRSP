/******************************************************************************
*
*   ...
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"
#include "clrspMex.h"


void
mexFunction(int nlhs, mxArray *plhs[],
            int nrhs, const mxArray *prhs[])
{
    /* Check input arguments. */
    if (nrhs != 9) {
        mexErrMsgIdAndTxt("MATLAB:clCFAR:invalidNumInputs",
                          "8 input arguments required.");
    }
    if (!mxIsSingle(prhs[0])
        || !mxIsComplex(prhs[0])
        || mxGetNumberOfDimensions(prhs[0]) != 2 ) 
    {
        mexErrMsgIdAndTxt("MATLAB:clCFAR:invlaidInputs",
                          "First input must be single, complex matrix.");
    }



    /*
    if (    !mxIsScalar(prhs[1])
         || !mxIsScalar(prhs[2])
         || !mxIsScalar(prhs[3])                   )
    {
        mexErrMsgIdAndTxt("MATLAB:clCFAR:invlaidInputs",
                          "2nd-4th input must be scalar integer64.");
    }
    */

    if ( !mxIsScalar(prhs[4]))
    {
        mexErrMsgIdAndTxt("MATLAB:clCFAR:invlaidInputs",
                          "Third input must be scalar.");
    }



    if (nlhs > 2) {
        mexErrMsgIdAndTxt("MATLAB:clCFAR:invalidNumOutputs",
                          "Too many output arguments.");
    }

    /* Process input arguments. */
    char *storage_option = mxArrayToString(prhs[5]);
    clrspStorageOrder order = CLRSP_ROW_MAJOR;
    if (strcmp(storage_option, "row-major") == 0) {
        order = CLRSP_ROW_MAJOR;
    } else if (strcmp(storage_option, "col-major") == 0) {
        order = CLRSP_COL_MAJOR;
    } else {
        mexErrMsgIdAndTxt("MATLAB:clCFAR:invalidInputs",
                          "Unknown storage option.");
    }
    char *layout_option = mxArrayToString(prhs[6]);
    clrspComplexLayout layout = CLRSP_PLANAR;
    if (strcmp(layout_option, "planar") == 0) {
        layout = CLRSP_PLANAR;
    } else if (strcmp(layout_option, "interleaved") == 0) {
        layout = CLRSP_INTERLEAVED;
    } else {
        mexErrMsgIdAndTxt("MATLAB:clCFAR:invalidInputs",
                          "Unknown layout option.");
    }
    char *device_option = mxArrayToString(prhs[7]);
    cl_device_type device_type = CL_DEVICE_TYPE_CPU;
    if (strcmp(device_option, "gpu") == 0) {
        device_type = CL_DEVICE_TYPE_GPU;
    } else if (strcmp(device_option, "cpu") == 0) {
        device_type = CL_DEVICE_TYPE_CPU;
    } else {
        mexErrMsgIdAndTxt("MATLAB:clCFAR:invalidInputs",
                          "Unknown device option.");
    }

    size_t m = mxGetM(prhs[0]);
    size_t n = mxGetN(prhs[0]);
    clrspComplexMatrix *X = clrspGetComplexMatrix(prhs[0],
                                                  order,
                                                  layout);

    size_t guardL    = mxGetScalar(prhs[1]);
    size_t refLW     = mxGetScalar(prhs[2]);
    size_t refLH     = mxGetScalar(prhs[3]);
    double tresh_fac = mxGetScalar(prhs[4]);
    size_t runs = mxGetScalar(prhs[8]);


    /* Setup OpenCL environment. */
    cl_int status;
    cl_context context;
    cl_command_queue queue;
    cl_command_queue_properties queue_props = CL_QUEUE_PROFILING_ENABLE;
    status = clrspSetupSingleDeviceContext(&context,
                                           NULL,
                                           0,
                                           &queue,
                                           &queue_props,
                                           0,
                                           device_type,
                                           0);
    if (status != CL_SUCCESS) { clError(status); }

    cl_event events[11];

    /* Allocate memory on device and copy data. */
    cl_mem X_real;
    cl_mem X_imag;
    size_t padding[2] = {0, 0};

    status = clrspAllocAndWriteMatrixToGPU(X,
                                           &X_real,
                                           &X_imag,
                                           padding,
                                           &context,
                                           CL_MEM_READ_WRITE,
                                           &queue,
                                           0,
                                           NULL,
                                           &events[0]);
    if (status != CL_SUCCESS) { clError(status); }

// VON HIER WEITER ANPASSEN

    int i;
    cl_ulong time = 0;
    for (i = 0; i < runs; ++i) {

        puts("check 0.0");

        /* Perform element-wise product. */
        status = clrspCFAR(X,
                           &X_real,
                           &X_imag,
                           &context,
                           &queue,
                           4,
                           &events[0],
                           &events[4]);
        if (status != CL_SUCCESS) { clError(status); }

        puts("check 0.1");

        status = clFinish(queue);
        if (status != CL_SUCCESS) { clError(status); }

        puts("check 0.2");

        cl_ulong duration;
        status = clrspGetEventDuration(&events[4],
                                       &duration);
        if (status != CL_SUCCESS) { clError(status); }

        puts("check 0.3");

        time += duration;
    }
    time /= runs;

    /* Prepare host for output. */
    clrspComplexMatrix *out = clrspNewComplexMatrix(m,
                                                    n,
                                                    order,
                                                    X->layout);
    clrspAllocComplexMatrix(out);

    /* Copy zero-padded data back to host. */
    size_t buffer_origin[3] = {0, 0, 0};
    size_t buffer_row_pitch = n * sizeof(float);
    if (order == CLRSP_ROW_MAJOR) {
        buffer_row_pitch = n * sizeof(float);
    } else {
        buffer_row_pitch = m * sizeof(float);
    }
    if (out->layout == CLRSP_INTERLEAVED) {
        buffer_row_pitch *= 2;
    }

    status = clrspReadMatrixFromGPU(&X_real,
                                    &X_imag,
                                    out,
                                    buffer_origin,
                                    buffer_row_pitch,
                                    &queue,
                                    0,
                                    NULL,
                                    NULL);
    if (status != CL_SUCCESS) { clError(status); }

    clFinish(queue);

    /* Release OpenCL resources. */
    status = clReleaseMemObject(X_real);
    if (status != CL_SUCCESS) { clError(status); }
    if (X->layout == CLRSP_PLANAR) {
        status = clReleaseMemObject(X_imag);
        if (status != CL_SUCCESS) { clError(status); }
    }

    clFinish(queue);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    /* Prepare output arguments. */
    plhs[0] = clrspGetmxArray(out);
    int dims[2] = {1,1};
    plhs[1] = mxCreateNumericArray(2,
                                   dims,
                                   mxUINT64_CLASS,
                                   mxREAL);
    unsigned long *data = mxGetData(plhs[1]);
    data[0] = time;
}
