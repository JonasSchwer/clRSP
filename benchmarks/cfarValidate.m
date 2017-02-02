% clear all;
close all;
clc;

addpath('../bin');
%%
m = 500;
n = 500;
runs = 1;

guardLength = 4;
refWidth = 4;
refHeight = 4;
pfa = 1;

order = 'row-major';
layout = 'planar';
device = 'cpu';
%%
 X = single(complex(rand(m,n),rand(m,n)));

 
[ocl, ~] = clTestCFAR(X,guardLength,refWidth,refHeight,pfa, ...
    order,layout,device,runs);
