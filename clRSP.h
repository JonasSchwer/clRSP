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
#include <assert.h>

#include <CL/cl.h>

/******************************************************************************
*   Definition of enum constatns.
******************************************************************************/

typedef enum clrspStatus_
{
    CLRSP_SUCCESS = 1,  /* Indicates succesfull function return.             */
    CLRSP_ENDSTATUS     /* Last value of the enum.                           */
} clrspStatus;

/******************************************************************************
*   Interface for the OpenCL environment.
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

/******************************************************************************
*   Complex matrix datatype.
******************************************************************************/

typedef enum clrspStorageOrder_
{
    CLRSP_COLUMN_MAJOR = 1, /* Columns lie continually in memory.            */
    CLRSP_ROW_MAJOR,        /* Rows lie continually in memory.               */
    CLRSP_ENDSTORAGEORDER   /* Last value of the enum.                       */
} clrspStorageOrder;


struct clrspComplexMatrix_
{
    size_t rows;                /* Number of rows.                           */
    size_t cols;                /* Number of columns.                        */
    clrspStorageOrder order;    /* Storage order in memory.                  */
    float *real;                /* Pointer to real data.                     */
    float *imag;                /* Pointer to imaginary data.                */
};
typedef struct clrspComplexMatrix_ clrspComplexMatrix;


/* Creates a new clrspComplexMatrix struct. Sets rows, cols and order to the
   given values. Sets the data pointers to NULL. */
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

#endif // CLRSP_H
