clear all;
clc;

addpath('../bin');

%%
runs = 5;
m = 32;
nMin = 100;
nIt  = 100;
nMax = 10000;

N = nMin : nIt : nMax;
A = zeros(length(N),3);
A(:,1) = m;
A(:,2) = N;

for i = 1:length(N)
    n = N(i);
    X = single(complex(rand(m, n), rand(m, n)));
    y = single(complex(rand(1, n), rand(1, n)));
    
%     b = zeros(1,n);
%     tic
%     for j = 1:m
%         b = X(j,:).*y;
%     end
%     A(i,3) = toc*1e9;
    
    [~, A(i,3)] = clTestElemProd(X,y,runs);
end

save(sprintf('data/elemProd%03i.dat',m),'A','-ascii')