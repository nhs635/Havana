%%

clc; clear; close all; 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Image Analysis Code (OCT FLIM Version)
% 
% v161118 drafted (C++ wrote images analysis)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% filename
dfilenm = 'finger';

%% Image parameters

% Please check ini file in the folder
fid = fopen(strcat(dfilenm,'.ini'));
config = textscan(fid,'%s');
fclose(fid); 

ch_start = zeros(1,4);
delay_time_offset = zeros(1,3);
for i = 1 : length(config{:})
    if (strfind(config{1}{i},'nAlines'))
        eq_pos = strfind(config{1}{i},'=');
        n_alines = str2double(config{1}{i}(eq_pos+1:end));
    end
    if (strfind(config{1}{i},'nScans'))
        eq_pos = strfind(config{1}{i},'=');
        n_scans = str2double(config{1}{i}(eq_pos+1:end));
    end
end

n4_alines = n_alines/4;
n2_scans = n_scans/2; nn_scans = n_scans*2; fn_scans = n_scans*4;
n_len = 2^ceil(log2(n_scans)); nn_len = n_len*2;
n2_len = n_len/2; n4_len = n_len/4; n8_len = n_len/8;
n_frame = length(dir('rect_image\rect_1*'));

im_size = n_len*n_alines;
im2_size = n2_len*n_alines;

clear config eq_pos;

%% OCT FLIM data

Nsit = 1; term = 1; 
Nit = n_frame;

pp_range = Nsit : term : Nit;

oct_im = zeros(n2_len,n_alines,n_frame,'uint8');
for pp = pp_range
    clc; pp   
    imgname = sprintf('rect_image\\rect_1pullback_%03d.bmp',pp);
    oct_im(:,:,pp) = imread(imgname); 
end
h = imfinfo(imgname);
cmap = h.Colormap;

fname = 'OCTMaxProjectionMap.bin'; 
fid = fopen(fname,'r');
fdata = fread(fid,'float');
oct_max_proj_map = reshape(fdata,[n_alines,n_frame]);
fclose(fid);

intensity_map = zeros(n4_alines,n_frame,3);
lifetime_map = zeros(n4_alines,n_frame,3);
for i = 1 : 3
    fname1 = sprintf('IntensityMap_Ch%d.bin',i);
    fid1 = fopen(fname1,'r');
    fdata1 = fread(fid1,'float');
    intensity_map(:,:,i) = reshape(fdata1,[n4_alines,n_frame]);
    fclose(fid1);
    
    fname2 = sprintf('LifetimeMap_Ch%d.bin',i);
    fid2 = fopen(fname2,'r');
    fdata2 = fread(fid2,'float');
    lifetime_map(:,:,i) = reshape(fdata2,[n4_alines,n_frame]);
    fclose(fid2);    
end

clear fid* fdata* h imgname;

%%

pp = 1;

vis_ch = 1;
oct_im1 = oct_im(:,:,pp);
flu_intensity = intensity_map(:,pp,vis_ch);
flu_lifetime = lifetime_map(:,pp,vis_ch);

figure(1); set(gcf,'Position',[1 1 1858 1003]);
subplot(241); imagesc(oct_max_proj_map); caxis([90 120]); colormap gray; freezeColors; 
title('OCT maximum projection map'); xlabel('pullback');
subplot(242); imagesc(intensity_map(:,:,1)); caxis([0 2]); colormap hot; freezeColors; 
title('Fluorescence intensity ch 1 [0 2]'); xlabel('pullback');
subplot(243); imagesc(intensity_map(:,:,2)); caxis([0 2]); colormap hot; freezeColors; 
title('Fluorescence intensity ch 2 [0 2]'); xlabel('pullback');
subplot(244); imagesc(intensity_map(:,:,3)); caxis([0 2]); colormap hot; freezeColors; 
title('Fluorescence intensity ch 3 [0 2]'); xlabel('pullback');
subplot(246); imagesc(lifetime_map(:,:,1)); caxis([0 10]); colormap jet; freezeColors;
title('Fluorescence lifetime ch 1 [0 10]'); xlabel('pullback');
subplot(247); imagesc(lifetime_map(:,:,2)); caxis([0 10]); colormap jet; freezeColors; 
title('Fluorescence lifetime ch 2 [0 10]'); xlabel('pullback');
subplot(248); imagesc(lifetime_map(:,:,3)); caxis([0 10]); colormap jet; freezeColors; 
title('Fluorescence lifetime ch 3 [0 10]'); xlabel('pullback');

figure(2); set(gcf,'Position',[420 180 516 778]);
imshow(oct_im1); caxis([0 255]); colormap(cmap); freezeColors;  hold on;
h1 = imshow(imresize(repmat(flu_intensity',[40 1]),[40 n_alines])); caxis([0 2]); colormap hot; freezeColors;
h1.YData = [n2_len-80 n2_len-40];
h2 = imshow(imresize(repmat(flu_lifetime',[40 1]),[40 n_alines])); caxis([0 10]); colormap jet; freezeColors;
h2.YData = [n2_len-40 n2_len]; hold off;

figure(3); set(gcf,'Position',[952 538 560 420]);
subplot(211); plot(flu_intensity); axis([0 n4_alines 0 2]); ylabel('normalized intensity');
title(sprintf('normalized intensity (hot): %.4f +- %.4f AU', mean(flu_intensity), std(flu_intensity))); 
subplot(212); plot(flu_lifetime); axis([0 n4_alines 0 10]); ylabel('lifetime'); 
title(sprintf('lifetime (jet): %.4f +- %.4f nsec', mean(flu_lifetime), std(flu_lifetime)));

