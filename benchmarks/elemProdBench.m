clear all;
clc;

addpath('../bin');

%%
runs = 1;
m = 64;
nMin = 100;
nIt  = 100;
nMax = 10000;

N = nMin : nIt : nMax;
A = zeros(length(N),3);
A(:,1) = m;
A(:,2) = N;

for i = 1:length(N)
    n = N(i);
    X = single(complex(rand(n, m), rand(n, m)));
    y = single(complex(rand(1, m), rand(1, m)));
    
%     b = zeros(1,n);
%     tic
%     for j = 1:m
%         b = X(j,:).*y;
%     end
%     A(i,3) = toc*1e9;
    
    [~, A(i,3)] = clTestElemProd(X,y,runs);
end

save(sprintf('data/elemProd%03i.xx.dat',m),'A','-ascii')