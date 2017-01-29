clear all;
% close all;
clc;

addpath('../bin');
%%
m = 64;
n = 3300;
l = nearestPower(n,[7,5,3,2]);
runs = 1;

dimension = 'col-wise';
order = 'col-major';
layout = 'planar';
device = 'cpu';

X = single(complex(rand(m,n),rand(m,n)));
[ocl,~] = clTestFFT(X,runs,dimension,order,layout,device);
mat = single(complex(zeros(m,l),zeros(m,l)));
mat(:,1:n) = X;
for k = 1:runs
    if strcmp(dimension,'row-wise')
        mat = fft(mat,[],2);
    else
        mat = fft(mat,[],1);
    end
end

norm(real(ocl)-real(mat))/(runs*prod(size(ocl)))
norm(imag(ocl)-imag(mat))/(runs*prod(size(ocl)))