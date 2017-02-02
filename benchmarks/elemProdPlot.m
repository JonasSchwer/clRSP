clear all;
% close all;
clc;

addpath('./data/pulseComp')
listing = dir('./data/pulseComp');

% figure(3)
figure(4)
for i = 3:size(listing,1)
    A = load(listing(i).name);
%     figure(3)
%     plots1(i-2) = semilogy(A(:,2),6*A(:,1).*A(:,2)./A(:,3),'o-');
%     hold on
    figure(4)
    plots2(i-2) = plot(A(:,2),A(:,3)./1e6,'o-');
    hold on
end

% figure(3)
% legend(plots1,listing(3:end).name,'Location','best')
% title('Pulskompression');
% xlabel('N');
% ylabel('GFLOPS');
% grid on
% hold off

figure(4)
% loglogTriangle(2e3,7e3,4e4,-1,'l')
legend(plots2,listing(3:end).name,'Location','best')
title('Pulskompression');
xlabel('N');
ylabel('Millisekunden');
grid on
hold off