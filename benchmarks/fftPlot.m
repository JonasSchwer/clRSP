clear all;
close all;
clc;

addpath('./data/fft')
listing = dir('./data/fft');

% figure(1)
figure(2)
for i = 3:size(listing,1)
    A = load(listing(i).name);
%     figure(1)
%     plots1(i-2) = plot(A(:,2),A(:,1).*A(:,2).*log(A(:,2))./A(:,3),'o-');
%     hold on
    figure(2)
    plots2(i-2) = loglog(A(:,2),A(:,3),'o-');
    hold on
end

% figure(1)
% legend(plots1,listing(3:end).name,'Location','best')
% title('Elementweises Vector-Product');
% xlabel('N');
% ylabel('GFLOPS');
% grid on
% hold off

figure(2)
% loglogTriangle(2e3,7e3,4e4,-1,'l')
legend(plots2,listing(3:end).name,'Location','best')
title('Elementweises Vector-Product');
xlabel('N');
ylabel('time in ns');
grid on
hold off