clear all;
% close all;
clc;

addpath('../bin');
%%
m = 1;
n = 100000;
l = nearestPower(n,[7,5,3,2]);
runs = 1;

dimension = 'row-wise';
order = 'row-major';
layout = 'interleaved';
device = 'gpu';

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