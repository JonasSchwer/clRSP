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

__kernel
void
elemProdKernel(__global float *X_real,
               __global float *X_imag,
               __global float *y_real,
               __global float *y_imag,
               int rows,
               int cols)
{
    /* Detremine global and local position. */
    int row = get_global_id(1);
    int col = get_global_id(0);

    /* Perform complex product if it is in bounds. */
    int Xidx;
    float real, imag;

    Xidx = row * cols + col;

    real = X_real[Xidx] * y_real[col] - X_imag[Xidx] * y_imag[col];
    imag = X_real[Xidx] * y_imag[col] + X_imag[Xidx] * y_real[col];

    X_real[Xidx] = real;
    X_imag[Xidx] = imag;
}

