clear all;
clc;

addpath('../bin');

%%
runs = 10;
order = 'row-major';
layout = 'interleaved';
device = 'gpu';

m = 64;
nMin = 5000;
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
    
    [~, A(i,3)] = clTestElemProd(X,y,runs,order,layout,device);
end

save(sprintf('data/elemProd/%sElemProd%03i.%s.%s.2.dat',device,m,order,layout),'A','-ascii')
% save(sprintf('data/test.dat'),'A','-ascii')