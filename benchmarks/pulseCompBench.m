clear all;
close all;
clc;

addpath('../bin');
%%
runs = 10;
order = 'row-major';
layout = 'planar';
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
    l = ceil(n / 100);
    
    % Generiere daten
    X = single(complex(rand(m, n), rand(m, n)));
    y = single(complex(rand(1, l), rand(1, l)));
    
%     b = zeros(1,n);
%     tic
%     for j = 1:m
%         b = X(j,:).*y;
%     end
%     A(i,3) = toc*1e9;
    
    [~, ~, A(i,3)] = clTestPulsecompression(X,conj(y),runs,order,layout,device);
end

save(sprintf('data/pulseComp/%sPulseComp%03i.%s.%s.dat',device,m,order,layout), ...
    'A','-ascii')