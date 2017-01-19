clear all;
close all;
clc;

addpath('../bin');

m = 64;
for n = 100:100:20000
    X = single(complex(rand(m, n), rand(m, n)));
    y = single(complex(rand(1, n), rand(1, n)));
    clTestElemProd(X,y);
end