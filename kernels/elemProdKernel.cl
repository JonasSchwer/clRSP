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

__kernel __attribute__((vec_type_hint(float)))
void
elemProdKernel(__global float *X_real,
               __global float *X_imag,
               __global float *y_real,
               __global float *y_imag,
               int rows,
               int cols,
               int order,
               int layout)
{
    /* Detremine global and local position. */
    int row = (order == 2) ? get_global_id(1) : get_global_id(0);
    int col = (order == 2) ? get_global_id(0) : get_global_id(1);

    /* Perform complex product if it is in bounds. */
    int in_bounds = (col < cols && row < rows) ? 1 : 0;
    if (in_bounds && (layout == 1)) {
        int Xidx;
        float real, imag;

        Xidx = (order == 2) ? row * cols + col : row + col * rows;

        real = X_real[Xidx] * y_real[col] - X_imag[Xidx] * y_imag[col];
        imag = X_real[Xidx] * y_imag[col] + X_imag[Xidx] * y_real[col];

        X_real[Xidx] = real;
        X_imag[Xidx] = imag;
    }
    if (in_bounds && (layout == 2)) {
        int Xidx;
        float2 res, X, y;

        Xidx = (order == 2) ? row * cols + col : row + col * rows;
        X = vload2(Xidx, X_real);
        y = vload2(col, y_real);

        res.s0 = X.s0 * y.s0 - X.s1 * y.s1;
        res.s1 = X.s0 * y.s1 + X.s1 * y.s0;

        vstore2(res, Xidx, X_real);
    }
}
