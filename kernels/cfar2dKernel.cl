/******************************************************************************
 *
 *   OpenCL kernel to perform CFAR on float Matrix
 *
 *   M_RD rows-by-cols float with order
 *   p_fa false-alarm-rate 
 *   
 *
 ******************************************************************************/
//#define DEBUG_BUILD

#ifdef DEBUG_BUILD
    #define DEBUG(X) X
#else
    #define DEBUG(X)
#endif 


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
             int refHeight)
{


    /* Detremine global and local position in M_RD. */
    int row_M_RD = (order_M_RD == 2) ? get_global_id(1) : get_global_id(0);
    int col_M_RD = (order_M_RD == 2) ? get_global_id(0) : get_global_id(1);

DEBUG(printf("Kernel entered for: row=%i, col=%i \n",row_M_RD,col_M_RD);)

    /* Detremine global and local position in M_detect. */
    int row_M_detect = (order_M_detect == 2) ? get_global_id(1) : get_global_id(0);
    int col_M_detect = (order_M_detect == 2) ? get_global_id(0) : get_global_id(1);

    /* Total number of relevant filter elements */
    int num = (2*(refWidth + guardLength) + 1) * (2*(refHeight + guardLength) + 1);
/*    num = num - (2*guardLength + 1) * (2*guardLength + 1); */

    /* sum of filter elements */
    float sum = 0;

DEBUG(printf("Kernel-Checkpoint 1 for: row=%i, col=%i \n",row_M_RD,col_M_RD);)

    int i, j;
    for(i=row_M_RD - (guardLength + refHeight);
        i<= row_M_RD + (guardLength + refHeight);
        ++i)
    {


        for(j=col_M_RD - (guardLength + refWidth);
            j<= col_M_RD + (guardLength + refWidth);
            ++j)
        {

DEBUG(printf("Kernel-Checkpoint 2.%i.%i for: row=%i, col=%i \n",i,j,row_M_RD,col_M_RD);)

            int i_in_bounds     = (i >= 0 && i < rows) ? 1 : 0;
            int j_in_bounds     = (j >= 0 && j < cols) ? 1 : 0;
            


/* ALTE VERSION          
            int i_out_of_guard  =
                (i < row_M_RD-guardLength && i > row_M_RD + guardLength) ? 1 : 0;
            int j_out_of_guard  =
                (j < col_M_RD-guardLength && j > col_M_RD+guardLength) ? 1 : 0;
   
            if(i_in_bounds && j_in_bounds && i_out_of_guard && j_out_of_guard){

*/


            // AENDERUNG IN FOLGENDEN VIER ZEILEN ( || statt && )
            int i_out_of_guard  =
                (i < row_M_RD-guardLength || i > row_M_RD + guardLength) ? 1 : 0;
            int j_out_of_guard  =
                (j < col_M_RD-guardLength || j > col_M_RD+guardLength) ? 1 : 0;

            // AENDERUNG IN FOLGENDER ZEILE ( || statt && und Klammern setzen)
            if(i_in_bounds && j_in_bounds && (i_out_of_guard || j_out_of_guard)){

                /* order_M_RD == 2 -> zeilenweise */
                int M_RD_idx = (order_M_RD == 2) ? i * cols + j : i + j * rows;
                sum = sum + M_RD[M_RD_idx];
            } else {
                num = num - 1;
                continue;
            }
        }
    }

DEBUG(printf("Kernel-Checkpoint 3 for: row=%i, col=%i \n",row_M_RD,col_M_RD);)

    /* mean value calculation */
    float mean = sum/num;

    /* compute tresh_factor for comparison with cell under test */
    float tresh_factor = num*(pow(p_fa, -1/num) - 1);

    /* comparison */
    int M_RD_idx = (order_M_RD == 2) ? row_M_RD * cols + col_M_RD : row_M_RD + col_M_RD * rows;
    int M_detect_idx = (order_M_detect == 2) ? row_M_detect * cols + col_M_detect : row_M_detect + col_M_detect * rows;

    M_detect[M_detect_idx] = mean; //(mean*tresh_factor < M_RD[M_RD_idx]);

DEBUG(printf("Kernel about to exit for: row=%i, col=%i \n",row_M_RD,col_M_RD);)
}
