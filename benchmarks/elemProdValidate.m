clear all;
% close all;
clc;

addpath('../bin');
%%
m = 12;
n = 111;
runs = 1;

X = single(complex(rand(m,n),rand(m,n)));
y = single(complex(rand(m,1),rand(m,1)));
[ocl, ~] = clTestElemProd(X,y,runs);
mat = X;
for k = 1:runs
    for i = 1:n
        mat(:,i) = mat(:,i).*y;
    end
end

norm(real(ocl)-real(mat))
norm(imag(ocl)-imag(mat))