%%
clc; clear; close all;

load fire.mat;

n_alines = 2048; n_scans = 2600;
n4_alines = n_alines/4;
n_len = 2^ceil(log2(n_scans));
n2_len = n_len/2;

itn_name = 'intensity_ch%d.flimres';
lft_name = 'lifetime_ch%d.flimres';

for i = 1 : 3
    fid_itn(i) = fopen(sprintf(itn_name,i),'r');
    fid_lft(i) = fopen(sprintf(lft_name,i),'r');
end

for i = 1 : 3   
    temp = fread(fid_itn(i),'float');
    intensity(:,:,i) = reshape(temp,[n4_alines length(temp)/n4_alines]);
    temp = fread(fid_lft(i),'float');
    lifetime(:,:,i)  = reshape(temp,[n4_alines length(temp)/n4_alines]);
end

n_frames = length(temp)/n4_alines;

for i = 1 : 3
    fclose(fid_itn(i));
    fclose(fid_lft(i));
end

intensity_thres = 0.001;
intensity1 = medfilt2(intensity(:,:,1),[5 3],'symmetric');
intensity2 = medfilt2(intensity(:,:,2),[5 3],'symmetric');
intensity3 = medfilt2(intensity(:,:,3),[5 3],'symmetric');
intensity(:,:,1) = intensity1;
intensity(:,:,2) = intensity2;
intensity(:,:,3) = intensity3;
lifetime1 = medfilt2(lifetime(:,:,1).*double(intensity1>intensity_thres),[5 3],'symmetric');
lifetime2 = medfilt2(lifetime(:,:,2).*double(intensity2>intensity_thres),[5 3],'symmetric');
lifetime3 = medfilt2(lifetime(:,:,3).*double(intensity3>intensity_thres),[5 3],'symmetric');
lifetime(:,:,1) = lifetime1;
lifetime(:,:,2) = lifetime2;
lifetime(:,:,3) = lifetime3;

intensity_contrast = [0 0.5]; % AU
lifetime_contrast = [0 5]; % nsec

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
figure(1); set(gcf,'Position',[540 160 630 1100]);
subplot(311); imagesc(intensity1); caxis(intensity_contrast); 
colormap(fire); colorbar; set(gca,'FontSize',15); freezeColors;
xlabel('pullback direction (distal -> proximal)'); ylabel('angle');
title(sprintf('ch1 intensity [%d %d]',intensity_contrast(1),intensity_contrast(2)));

subplot(312); imagesc(intensity2); caxis(intensity_contrast); 
colormap(fire); colorbar; set(gca,'FontSize',15); freezeColors;
xlabel('pullback direction (distal -> proximal)'); ylabel('angle');
title(sprintf('ch2 intensity [%d %d]',intensity_contrast(1),intensity_contrast(2)));

subplot(313); imagesc(intensity3); caxis(intensity_contrast); 
colormap(fire); colorbar; set(gca,'FontSize',15); freezeColors;
xlabel('pullback direction (distal -> proximal)'); ylabel('angle');
title(sprintf('ch3 intensity [%d %d]',intensity_contrast(1),intensity_contrast(2)));

im_intensity = frame2im(getframe(gcf));
imwrite(im_intensity,'intensity_map.tif');
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
figure(2); set(gcf,'Position',[540+631 160 630 1100]);
subplot(311); imagesc(lifetime1); caxis(lifetime_contrast); 
colormap(jet); colorbar; set(gca,'FontSize',15); freezeColors;
xlabel('pullback direction (distal -> proximal)'); ylabel('angle');
title(sprintf('ch1 lifetime [%d %d]',lifetime_contrast(1),lifetime_contrast(2)));

subplot(312); imagesc(lifetime2); caxis(lifetime_contrast); 
colormap(jet); colorbar; set(gca,'FontSize',15); freezeColors;
xlabel('pullback direction (distal -> proximal)'); ylabel('angle');
title(sprintf('ch2 lifetime [%d %d]',lifetime_contrast(1),lifetime_contrast(2)));

subplot(313); imagesc(lifetime3); caxis(lifetime_contrast); 
colormap(jet); colorbar; set(gca,'FontSize',15); freezeColors;
xlabel('pullback direction (distal -> proximal)'); ylabel('angle');
title(sprintf('ch3 lifetime [%d %d]',lifetime_contrast(1),lifetime_contrast(2)));

im_lifetime = frame2im(getframe(gcf));
imwrite(im_lifetime,'lifetime_map.tif');
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

figure(3); 
subplot(211); plot(intensity(:,1,2)); axis([0 512 0 2]); ylabel('normalized intensity');
title(sprintf('normalized intensity (fire): %.4f +- %.4f AU', mean(intensity(:,1,2)), std(intensity(:,1,2))));
subplot(212); plot(lifetime(:,1,2)); axis([0 512 0 10]); ylabel('lifetime'); 
title(sprintf('lifetime (jet): %.4f +- %.4f nsec', mean(lifetime (:,1,2)), std(lifetime (:,1,2))));

%% Generate FLIM rings

path = cd;
for i = 1 : length(path)
    if path(i) == '\'
        slash_pos = i;
    end
end

path1 = path(1:slash_pos);
slash_poses = find(path1=='\');
for i = 1 : length(slash_poses)
    path1 = [path1(1:slash_poses(i)+i-1) '\' path1(slash_poses(i)+1+i-1:end)];
end

rect_path = [path1 'rect_image\\rect_1pullback_%03d.bmp'];

oct_contrast = [50 255];

for i = 65 % : n_frames
    
    clc; i
    ring_rect = zeros(n2_len,n_alines,3,'uint8');
    
    im = imread(sprintf(rect_path,i));
    im = uint8(round(255*mat2gray(im,oct_contrast)));
    
    for j = 1 : 3
        temp_intensity = interp1(linspace(1,n_alines,n4_alines),intensity(:,i,j),linspace(1,n_alines,n_alines));
        temp_lifetime  = interp1(linspace(1,n_alines,n4_alines),lifetime (:,i,j),linspace(1,n_alines,n_alines));
        
        temp_intensity1 = uint8(255*ind2rgb(round(255*mat2gray(temp_intensity,intensity_contrast)),fire));
        temp_lifetime1  = uint8(255*ind2rgb(round(255*mat2gray(temp_lifetime, lifetime_contrast)),jet(256)));
        
        temp_lifetime1(1,temp_lifetime==0,:) = 0;
        
        ring_rect([1:50] + 50*(2*j-2),:,:) = repmat(temp_intensity1,[50 1 1]);
        ring_rect([1:50] + 50*(2*j-1),:,:) = repmat(temp_lifetime1,[50 1 1]);        
    end
    
    for j = 1 : 3
        ring_rect(:,:,j) = circshift(ring_rect(:,:,j),-300);
        ring_rect(1:end-300,:,j) = im(1:end-300,:);
    end
    
    im2write = ring_rect(861:end,:,:);
    im2write_resize = zeros(size(im2write,1),size(im2write,2)/2,3,'uint8');
    im2write_resize_circ = zeros(1024,1024,3,'uint8');
    [x_map,y_map] = circ_init(im2write_resize(:,:,1),1024);
    for j = 1 : 3        
        im2write_resize(:,:,j) = imresize(im2write(:,:,j),[size(im2write,1) size(im2write,2)/2]);
        im2write_resize_circ(:,:,j) = uint8(circularize(double(im2write_resize(:,:,j)),x_map,y_map));
    end    
    
    figure(41); imshow(im2write_resize); drawnow;
    figure(42); imshow(im2write_resize_circ); drawnow;
    
%     rect_res_name = sprintf('rect_merged_results_itn[%.1f %.1f]lft[%.1f %.1f].tif',...
%         intensity_contrast(1),intensity_contrast(2),...
%         lifetime_contrast(1),lifetime_contrast(2));    
%     circ_res_name = sprintf('circ_merged_results_itn[%.1f %.1f]lft[%.1f %.1f].tif',...
%         intensity_contrast(1),intensity_contrast(2),...
%         lifetime_contrast(1),lifetime_contrast(2)); 
%     
%     if (i == 1)
%         imwrite(im2write_resize,rect_res_name,'WriteMode','Overwrite');
%         imwrite(im2write_resize_circ,circ_res_name,'WriteMode','Overwrite');
%     else
%         imwrite(im2write_resize,rect_res_name,'WriteMode','Append');
%         imwrite(im2write_resize_circ,circ_res_name,'WriteMode','Append');        
%     end
end
