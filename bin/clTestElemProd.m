% CLTESTELEMPROD    Perform element-wise product of vector y and each row
%                   of matrix X on the GPU (device).
%
%       Y = CLTESTELEMPROD(X,y,k,storage_order,storage_layout,device_type)
%   [Y,t] = CLTESTELEMPROD(X,y,k,storage_order,storage_layout,device_type)
%
% INPUT:
%                X - single complex input matrix (m-by-n)
%                y - single complex input vector (n)
%                k - number of element-wise product repetition for
%                    benchmarking
%    storage_order - C storage order, either 'row-major' or 'col-major'
%   storage_layout - C storage layout, either 'planar' or 'interleaved'
%      device_type - OpenCL device type, either 'cpu' or 'gpu'
%
% OUTPUT:
%   Y - single complex matrix afte k element-wise products on each row with
%       y
%   t - average duration of product execution in nanoseconds
%
% Author(s): Michael Thoma