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
             int refHeight)
{

    printf("check 1.1\n");

}
