% clTestCFAR        Perform CFAR
%
%       Y = clTestCFAR(X,guardLength,refWidth,refHeight,pfa, order,layout,device,runs);
%   [Y,t] = clTestCFAR(X,guardLength,refWidth,refHeight,pfa, order,layout,device,runs);
%
% INPUT:
%      X            - single complex input matrix (m-by-n)
%      quardLength  - CFAR guard area size
%      refWidth     - CFAR reference field width
%      refHeight    - CFAR reference field height
%      pfa          - factor to calculate CFAR threshold with
%      order        - C storage order, either 'row-major' or 'col-major'
%      layout       - C storage layout, either 'planar' or 'interleaved'
%      device       - OpenCL device type, either 'cpu' or 'gpu'
%      runs         - Benchmark Option. Defines number of times CFAR is done
%
% OUTPUT:
%   Y - single complex matrix with result in imag
%   t - average duration of product execution in nanoseconds
%
% Author(s): Michael Thoma