/******************************************************************************
 *
 *   OpenCL kernel to perform CFAR on float Matrix
 *
 *   M_RD rows-by-cols float with order
 *   p_fa false-alarm-rate 
 *   
 *
 ******************************************************************************/

__kernel
void
cfar2dKernel(__global float *M_RD,      // __read_only
             int rows,
             int cols,
             int order_M_RD,
             __global float *M_detect,    // __write_only
             int order_M_detect,
             float p_fa,
             int guardLength,
             int refWidth,
             int refHeight
            )
{
    /* Detremine global and local position in M_RD. */
    int row_M_RD = (order_M_RD == 2) ? get_global_id(1) : get_global_id(0);
    int col_M_RD = (order_M_RD == 2) ? get_global_id(0) : get_global_id(1);

    /* Detremine global and local position in M_detect. */
    int row_M_detect = (order_M_detect == 2) ? get_global_id(1) : get_global_id(0);
    int col_M_detect = (order_M_detect == 2) ? get_global_id(0) : get_global_id(1);

    /* Total number of relevant filter elements */
    int num = (2*(refWidth + guardLength) + 1) * (2*(refHeight + guardLength) + 1);
/*    num = num - (2*guardLength + 1) * (2*guardLength + 1); */

    /* sum of filter elements */
    float sum = 0;

    int i, j;
    for(i=row_M_RD - (guardLength + refHeight);
        i<= row_M_RD + (guardLength + refHeight); ++i) {

        for(j=col_M_RD - (guardLength + refWidth);
            j<= col_M_RD + (guardLength + refWidth); ++j) {

            int i_in_bounds     = (i >= 0 && i < rows) ? 1 : 0;
            int j_in_bounds     = (j >= 0 && j < cols) ? 1 : 0;
            int i_out_of_guard  =
                (i < row-guardLength && i > row+guardLength) ? 1 : 0;
            int j_out_of_guard  =
                (j < col-guardLength && j > col+guardLength) ? 1 : 0;

            if(i_in_bounds && j_in_bounds && i_out_of_guard && j_out_of_guard){

                /* order_M_RD == 2 -> zeilenweise */
                int M_RD_idx = (order_M_RD == 2) ? i * cols + j : i + j * rows;
                sum = sum + M_RD[M_RD_idx];
            } else {
                num = num - 1;
                continue;
            }
        }
    }

    /* mean value calculation */
    float mean = sum/num;

    /* compute tresh_factor for comparison with cell under test */
    float tresh_factor = num*(pow(p_fa, -1/num) - 1);

    /* comparison */
    int M_RD_idx = (order_M_RD == 2) ? row * cols + col : row + col * rows;
    int M_detect_idx = (order_M_detect == 2) ? row * cols + col : row + col * rows;

    M_detect[M_detect_idx] = (mean*tresh_factor < M_RD[M_RD_idx]);
}
