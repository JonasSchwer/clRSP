clear all;
close all;
clc;

addpath('../bin');
%%
m = 3;
n = 4;
runs = 1;
order = 'col-major';
layout = 'planar';
device = 'cpu';

X = single(complex(rand(m,n),rand(m,n)));
y = single(complex(rand(1,n),rand(1,n)));
[ocl, ~] = clTestElemProd(X,y,runs,order,layout,device);
mat = X;
for k = 1:runs
    for i = 1:m
        mat(i,:) = mat(i,:).*y;
    end
end

norm(real(ocl)-real(mat))
norm(imag(ocl)-imag(mat))