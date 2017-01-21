clear all;
close all;
clc;

addpath('./data')
listing = dir('./data');

figure
for i = 3:size(listing,1)
    A = load(listing(i).name);
    plots(i-2) = plot(A(:,2),6*A(:,1).*A(:,2)./A(:,3),'o-');
    hold on
end
hold off
legend(plots,listing(3:end).name,'Location','northwest')
title('Elementweises Vector-Product (X 32-by-N, y 1-by-N)');
xlabel('N');
ylabel('GFLOPS');
grid on