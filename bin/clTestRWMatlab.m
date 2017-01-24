% CLTESTRWMATLAB    Get matrix from Matlab, convert it into
%                   clrspComplexMatrix, convert it back into Matlab matrix
%                   and return convertet matrix.
%
%   Y = CLTESTRWMatlab(X,storage_order,storage_layout)
%
% INPUT:
%                X - single complex input matrix
%    storage_order - C storage order, either 'row-major' or 'col-major'
%   storage_layout - C storage layout, either 'planar' or 'interleaved'
%
% OUTPUT:
%   Y - single complex matrix
%
% Author(s): Michael Thoma