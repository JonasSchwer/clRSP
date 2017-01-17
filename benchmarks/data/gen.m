clear all; close all; clc;

M = 64;
N = 2.^(2:20);

for i = 1:length(N)
    A = rand(M, N(i));
    save(sprintf('real%i', i), 'A', '-ascii');
    A = rand(M, N(i));
    save(sprintf('imag%i', i), 'A', '-ascii');
    A = rand(1, floor(N(i)/30));
    save(sprintf('y_re%i', i), 'A', '-ascii');
    A = rand(1, floor(N(i)/30));
    save(sprintf('y_im%i', i), 'A', '-ascii');
end

clear;