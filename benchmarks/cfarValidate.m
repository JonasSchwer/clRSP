% clear all;
close all;
clc;

addpath('../bin');
%%
m = 5;
n = 5;
runs = 1;

guardLength = 1;
refWidth = 1;
refHeight = 1;
threshFactor = 1;

order = 'row-major';
layout = 'planar';
device = 'cpu';
%%
 X = single(complex(rand(m,n),rand(m,n)));
% y = single(complex(rand(1,n),rand(1,n)));
[ocl, ~] = clTestCFAR(X,guardLength,refWidth,refHeight,threshFactor, ...
    order,layout,device,runs);
% mat = X;
% for k = 1:runs
%     for i = 1:m
%         mat(i,:) = mat(i,:).*y;
%     end
% end
% 
% norm(real(ocl)-real(mat))
% norm(imag(ocl)-imag(mat))