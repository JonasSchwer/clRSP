/******************************************************************************
*
*   OpenCL kernel to perform element-wise complex vector-product, between the
*   rows of matrix X and the vector y.
*
*       X(i,:) <- X(i,:) .* y   for i = 1,...,m;
*
*   X m-by-n complex float
*   y 1-by-n complex float
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#ifndef N
#define N=1
#endif

__kernel __attribute__((vec_type_hint(float4)))
void
elemProdKernel(__global float *X_real,
               __global float *X_imag,
               __constant float *y_real,
               __constant float *y_imag,
               int rows,
               int cols,
               int order)
{
    /* Detremine global and local position. */
    int row = get_global_id(1);
    int col = get_global_id(0);

    /* Perform complex product if it is in bounds. */
    int Xidx;
    float4 real, imag, X_realN, X_imagN, y_realN, y_imagN;

    Xidx = (order == 2) ? row * (cols / 4) + col : row + col * rows;

    X_realN = vload4(Xidx, X_real);
    X_imagN = vload4(Xidx, X_imag);
    y_realN = vload4(col, y_real);
    y_imagN = vload4(col, y_imag);

    real = X_realN * y_realN - X_imagN * y_imagN;
    imag = X_realN * y_imagN + X_imagN * y_realN;

    vstore4(real, Xidx, X_real);
    vstore4(imag, Xidx, X_imag);
}
