/******************************************************************************
*
*   clRSP.h defines all public interfaces and types that are used by clRSP.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#ifndef CLRSP_H
#define CLRSP_H

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <CL/cl.h>
#include <clFFT/clFFT.h>

/******************************************************************************
*   Definition of enum constatns.
******************************************************************************/

typedef enum clrspStatus_
{
    CLRSP_SUCCESS = 1,  /* Indicates succesfull function return.             */
    CLRSP_ENDSTATUS     /* Last value of the enum.                           */
} clrspStatus;


typedef enum clrspClfftStatus_
{
    CLRSP_CLFFT_NOT_SETUP = 1,  /* Indicates that the clFFT library is not
                                   setup yet.                                */
    CLRSP_CLFFT_SETUP,          /* Indicates that the clFFT library is
                                   allready setup.                           */
    CLRSP_ENDCLFFTSTATUS        /* Last value of the enum.                   */
} clrspClfftStatus;


typedef enum clrspStorageOrder_
{
    CLRSP_COL_MAJOR = 1,    /* Columns lie continually in memory. */
    CLRSP_ROW_MAJOR,        /* Rows lie continually in memory. */
    CLRSP_ENDSTORAGEORDER   /* Last value of the enum. */
} clrspStorageOrder;


typedef enum clrspPlanDimension_
{
    CLRSP_ROW_WISE = 1,         /* Row wise fft.                             */
    CLRSP_COL_WISE,             /* Column wise fft.                          */
    CLRSP_ENDPLANTDIMENSTION    /* Last value of the enum.                   */
} clrspPlanDimension;

/******************************************************************************
*   Interface to the OpenCL environment.
******************************************************************************/

/* Setup OpenCL context for a single device, also create a command queue for
   the device. platform_idx and device_idx reffer to the indexes of the
   platform, repectively device, when clling clGetPlatformIDs, repectively
   clGetDeviceIDs. See the OpenCL Reference for the possible values of
   cl_context_properties, cl_queue_properties and cl_device_type. */
cl_int
clrspSetupSingleDeviceContext(cl_context *context,
                              const cl_context_properties *context_properties,
                              size_t num_context_properties,
                              cl_command_queue *queue,
                              const cl_queue_properties *queue_properties,
                              size_t platform_idx,
                              cl_device_type device_type,
                              size_t device_idx);

/* Get computation time of event in ns. */
cl_int
clrspGetEventDuration(cl_event *event,
                      cl_ulong *duration);


/* ... */
cl_int
clrspQueryDevices(cl_uint *num_devices,
                  cl_device_id **devices,
                  cl_platform_id platform,
                  cl_device_type device_type);


/* ... */
cl_int
clrspQueryPlatforms(cl_uint *num_platforms,
                    cl_platform_id **platforms);

/******************************************************************************
*   Complex matrix datatype.
******************************************************************************/

struct clrspComplexMatrix_
{
    size_t rows;                /* Number of rows.                           */
    size_t cols;                /* Number of columns.                        */
    clrspStorageOrder order;    /* Storage order of the matrix.              */
    float *real;                /* Pointer to real data.                     */
    float *imag;                /* Pointer to imaginary data.                */
};
typedef struct clrspComplexMatrix_ clrspComplexMatrix;


/* Creates a new clrspComplexMatrix struct. Sets rows, cols to the given
   values. Sets the data pointers to NULL. */
clrspComplexMatrix*
clrspNewComplexMatrix(size_t rows,
                      size_t cols,
                      clrspStorageOrder order);

/* Allocates memory for the data pointers A->real and A->imag according to
   A->rows and A->cols. */
void
clrspAllocComplexMatrix(clrspComplexMatrix *A);


/* Reallocates memory for the data pointers A->real and A->imag according to
   rows and cols and sets A->rows and A->cols. */
void
clrspReallocComplexMatrix(clrspComplexMatrix *A,
                          size_t rows,
                          size_t cols);


/* Free the resources previously allocated by clrspNewComplexMatrix,
   clrspAllocComplexMatrix and clrspReallocComplexMatrix. */
void
clrspFreeComplexMatrix(clrspComplexMatrix *A);

/******************************************************************************
*   Interface to the clFFT library.
******************************************************************************/

/* Initialize internal clFFT resources. Call once before any clFFT plans are
   created. */
cl_int
clrspSetupClfftLibrary(clrspClfftStatus *setup);


/* Create plan for 1D FFT on ech row or each column of A according to dim. */
cl_int
clrspCreate1DfftPlan(clfftPlanHandle *plan,
                     cl_context *context,
                     cl_command_queue *queue,
                     clrspPlanDimension dim,
                     clrspComplexMatrix *A);

/* Perform a 1D FFT according to plan. */
cl_int
clrspFFT(cl_mem *real,
         cl_mem *imag,
         cl_context *context,
         cl_command_queue *queue,
         clfftPlanHandle *plan,
         cl_uint num_wait_events,
         const cl_event *wait_events,
         cl_event *event);


/* Perform a 1D inverse FFT according to plan. */
cl_int
clrspIFFT(cl_mem *real,
          cl_mem *imag,
          cl_context *context,
          cl_command_queue *queue,
          clfftPlanHandle *plan,
          cl_uint num_wait_events,
          const cl_event *wait_events,
          cl_event *event);

/******************************************************************************
*   Interface to signal processing routines.
******************************************************************************/

/* Perform element-wise complex vector-product, between each row of matrix X
   and vector y. */
cl_int
clrspElementwiseProduct(const clrspComplexMatrix *X,
                        cl_mem *X_real,
                        cl_mem *X_imag,
                        const clrspComplexMatrix *y,
                        cl_mem *y_real,
                        cl_mem *y_imag,
                        cl_context *context,
                        cl_command_queue *queue,
                        cl_uint num_wait_list,
                        cl_event *wait_list,
                        cl_event *event);


/* Performs the pulsecompression between the transmitted pulse y and the
   recieved CPI matrix M0 and returns the resulting compressed matrix M_pc. */
cl_int
clrspPulseCompression(const clrspComplexMatrix *y,
                      clrspComplexMatrix *M_0,
                      cl_context *context,
                      cl_command_queue *queue,
                      clrspClfftStatus *setup,
                      clrspComplexMatrix *M_pc);

/******************************************************************************
*   Interface to auxiliary functions.
******************************************************************************/

/* Reads source code from path into character array. */
char*
clrspLoadKernelSource(const char *path);

/* Prints build log of OpenCL program. */
void
clrspPrintBuildLog(cl_program *program,
                   cl_device_id *device);

/* Allocates memory on device for complex matrix A plus symmetric zero-padding
   rows according to padding[0] and symmetric zero-padding columns according to
   padding[1]. Writes A to the device. Returns events, where events correspond
   to:
        events[0] <--> fill A_real with zeros
        events[1] <--> fill A_imag with zeros
        events[2] <--> write A->real to A_real
        events[3] <--> write A->imag to A_imag. */
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
                              cl_event events[4]);


/* Reads complex matrix from device. Starts reading at buffer_origin and reads
   according to A->rows and A->cols. Host memory must be allocated. Returns
   events, where events correspond to:
        events[0] <--> read A_real into A->real
        events[1] <--> read A_imag into A->imag. */
cl_int
clrspReadMatrixFromGPU(cl_mem *A_real,
                       cl_mem *A_imag,
                       clrspComplexMatrix *A,
                       size_t buffer_origin[3],
                       size_t buffer_row_pitch,
                       cl_command_queue *queue,
                       cl_uint num_wait_list,
                       cl_event *wait_list,
                       cl_event events[2]);


/* Rounds up value to the nearest multiple of multiple. */
size_t
clrspRoundUp(size_t value,
             size_t multiple);

#endif // CLRSP_H
