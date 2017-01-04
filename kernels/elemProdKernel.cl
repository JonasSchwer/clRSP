/******************************************************************************
*
*   OpenCL kernel to perform an element-wise vector product, with the complex
*   conjugated of y.
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
               int inc_row,
               int inc_col,
               __constant float *y_real,
               __constant float *y_imag)
{
    /* Get global position in Y direction, i.e. the current row. */
    int i = get_global_id(1);
    /* Get global position in X direction, i.e. the current column. */
    int j = get_global_id(0);
    /* Get global size in X direction, i.e. the number of columns. */
    int n = get_global_size(0);

    int idX = i * inc_row + j * inc_col;
    int idy = j;

    /* Compute real- and imag-part of complex product. */
    float real = X_real[idX] * y_real[idy] - X_imag[idX] * y_imag[idy];
    float imag = X_real[idX] * y_imag[idy] + X_imag[idX] * y_real[idy];

    /* Overwrite entries in X. */
    X_real[idX] = real;
    X_imag[idX] = imag;
}
