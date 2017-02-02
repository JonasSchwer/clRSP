clear all;
close all;
clc;

addpath('../bin');
%%
m = 64;
n = 10000;
l = ceil(n / 100);

runs = 1;
order = 'row-major';
layout = 'interleaved';
device = 'cpu';

% Generiere daten
X = single(complex(rand(m, n), rand(m, n)));
y = single(complex(rand(1, l), rand(1, l)));

[oclX,oclY,~] = clTestPulsecompression(X,conj(y),runs,order,layout,device);

mat = zeros(m, n + l - 1);
mat(:,1:n) = X;
for i=1:m
    mat(i,:) = single(conv(conj(y), X(i,:)));
end

max(max(abs((real(oclX(:,1:n+l-1))-real(mat)))))
max(max(abs((imag(oclX(:,1:n+l-1))-imag(mat)))))