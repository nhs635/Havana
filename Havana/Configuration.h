#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#define GET_NEAR_2_POWER(x)      1 << (int)ceil(log2(x))

#define NI_ENABLE                TRUE

#define ADC_RATE			     340 // MHz
#define N_ALINES		         2048

#define POWER_2_15				 32768.0f
#define POWER_2_16				 65536.0f

#define NUM_BUFFER_FRAME		 500 // 최대 저장 가능 프레임 개수
#define NUM_BUFFER_THREAD		 50 // Multithread 돌릴 때 버퍼 개수

#define FLIM_CH_START_5          30 // FLIM channel 3 이후에 더 받아들일 index 개수
 
#define HSV_MULTIFICATION_FACTOR 3

#define CIRC_RADIUS			     1300

#define PIXEL_SIZE               1.0 // micrometer/pixel

#define FLIM_SPLINE_FACTOR       20 // Spline interpolation scaling factor
#define GAUSSIAN_FILTER_WIDTH    200 // Software Broadening Filter Width (FLIM_SPLINE_FACTOR * 10)
#define GAUSSIAN_FILTER_STD      48 // Software Broadening Filter Sigma  (FLIM_SPLINE_FACTOR * 2.4)

#define MAX_INTENSITY            2.0f // fluorescence normalized intensity (AU)
#define MIN_INTENSITY            0.0f // fluorescence normalized intensity (AU)
#define MAX_LIFETIME             5.0f // nsec
#define MIN_LIFETIME             0.0f // nsec

#define INTENSITY_THRES          0.001f

#define IS_CIRC_WRITE            TRUE
#define IS_RECT_WRITE            FALSE // RECT WRITING => Need to modify! (only available when IS_TRIPLE_RING is TRUE)
#define IS_TRIPLE_RING           FALSE // triple ring mode writing results => Need to modify!

#define PROJECTION_OFFSET	     230

#define BENCHTOP_MODE			 FALSE // Benchtop mode (TRUE) / Catheter mode (FALSE)

#define ELFORLIGHT_PORT			 "COM1"
#define FAULHABER_PORT			 "COM2"
#define ZABER_PORT				 "COM9"

#define INTENSITY_MAP_FIRE       1 // Intensity Map을 color로 할지 fire으로 할지 (fire = 1)

#define FRAME_INTERVAL   		 40 // 화면 갱신 주기 (기본 25~30 정도)

#define LUT_GRAY				 0
#define LUT_INV_GRAY		     1
#define LUT_SEPIA			     2
#define LUT_JET			         3
#define LUT_PARULA		         4
#define LUT_HOT			         5
#define LUT_FIRE				 6
#define LUT_HSV					 7
#define LUT_PURPLE				 8
#define LUT_BLUE				 9
#define LUT_GREEN				 10

#endif