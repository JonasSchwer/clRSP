% CLTESTRWGPU   Get matrix from Matlab, write it with zero-padding rows
%               and columns to the GPU (device) global memory, read the
%               zero-padded matrix from the GPU (device) and return it
%               as Matlab matrix.
%
%   Y = CLTESTRWGPU(X,k,storage_order,storage_layout,device_type)
%
% INPUT:
%                X - single complex input matrix
%                k - number of rows (colums) to add above (left) and below
%                    (right)
%    storage_order - C storage order, either 'row-major' or 'col-major'
%   storage_layout - C storage layout, either 'planar' or 'interleaved'
%      device_type - OpenCL device type, either 'cpu' or 'gpu'
%
% OUTPUT:
%   Y - zero-padded single complex matrix
%
% Author(s): Michael Thoma