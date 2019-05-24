#pragma once
#include "stdafx.h"
#include "Headers.h"

// SVD 방식 삽입 / 추출 사용 시
/*Mat U_1, S_1, V_1;
Mat U_2, S_2, V_2;
Mat U_3, S_3, V_3;
Mat U_4, S_4, V_4;

Mat LL_U_wm, LL_V_wm, LL_S_wm;
Mat HL_U_wm, HL_V_wm, HL_S_wm;
Mat LH_U_wm, LH_V_wm, LH_S_wm;
Mat HH_U_wm, HH_V_wm, HH_S_wm;
*/

//////////////////////////////////////////////////////////////////////////////////////////
//// 삽입
//////////////////////////////////////////////////////////////////////////////////////////
Mat EmbedWatermark(Mat& HostImg, Mat& QrImg)
{
	return Embed(HostImg, QrImg);
}

Mat Embed(Mat& HostImg, Mat& QrImg)
{
	Mat yuv;
	Mat yuv_arr[3];
	Mat WT_result;
	Mat IWT_result;
	Mat Qr_Pixel = Mat(QrImg.rows, QrImg.cols, QrImg.type()); // 32x32 QRcode 각 픽셀 값을 255, 0으로 저장할 행렬 변수 생성

	// QR의 데이터를 0과 255로 설정
	for (int y = 0; y < QrImg.rows; y++)
		for (int x = 0; x < QrImg.cols; x++)
			Qr_Pixel.at<uchar>(y, x) = ((int)QrImg.at<uchar>(y, x) > 125) ? 255 : 0;

	cvtColor(HostImg, yuv, COLOR_RGB2YCrCb);    // RGV to YCrCb
	split(yuv, yuv_arr);						// 채널 분리
	WT(yuv_arr[0], WT_result, 1);               // Y채널을 대상으로 1단계 DWT 진행

	//imshow("Y",yuv_arr[0]);
	//imshow("삽입 전 WT result", WT_result);

	// 부대역의 계수를 저장할 행렬 변수    
	Mat HH_subband = Mat(WT_result.cols / 2, WT_result.rows / 2, WT_result.type());
	Mat HL_subband = Mat(WT_result.cols / 2, WT_result.rows / 2, WT_result.type());
	Mat LH_subband = Mat(WT_result.cols / 2, WT_result.rows / 2, WT_result.type());

	HH_subband = WT_result(Rect(WT_result.cols / 2, WT_result.rows / 2, WT_result.cols / 2, WT_result.rows / 2));
	HL_subband = WT_result(Rect(WT_result.cols / 2, 0, WT_result.cols / 2, WT_result.rows / 2));
	LH_subband = WT_result(Rect(0, WT_result.rows / 2, WT_result.cols / 2, WT_result.rows / 2));

	// DCT를 진행할 8x8 크기의 블럭들
	Size blockSize(8, 8);
	vector<Mat> HH_blocks;	// 각 부대역의 블럭들
	vector<Mat> HL_blocks;
	vector<Mat> LH_blocks;
	int value[1024];		// QR의 삽입 값을 저장할 배열
	int i = 0;

	// 256x256 크기의 부대역을 1024개의 8x8 블럭 사이즈로 분할
	for (int y = 0; y < 256; y += blockSize.height)
	{
		for (int x = 0; x < 256; x += blockSize.width)
		{
			Rect rect = Rect(x, y, blockSize.width, blockSize.height);
			HH_blocks.push_back(Mat(HH_subband, rect));
			HL_blocks.push_back(Mat(HL_subband, rect));
			LH_blocks.push_back(Mat(LH_subband, rect));
			// QR의 삽입 값을 지정
			value[i++] = ((int)Qr_Pixel.at<uchar>((int)(y / 8), (int)(x / 8)) > 125 ? 3 : -3);
		}
	}


	int x = 0, y = 0;
	// 각 부대역의 1024개 블럭들을 대상으로 워터마크 데이터 삽입 진행
	for (int i = 0; i < 1024; i++)
	{
		// DCT 진행
		dct(HH_blocks[i], HH_blocks[i]);
		dct(HL_blocks[i], HL_blocks[i]);
		dct(LH_blocks[i], LH_blocks[i]);

		// DCT 진행 후 값들의 DC 값에 워터마크 삽입
		HH_blocks[i].at<float>(0, 0) += 23 * value[i];
		HL_blocks[i].at<float>(0, 0) += 23 * value[i];
		LH_blocks[i].at<float>(0, 0) += 23 * value[i];

		// 역 DCT 진행
		idct(HH_blocks[i], HH_blocks[i]);
		idct(HL_blocks[i], HL_blocks[i]);
		idct(LH_blocks[i], LH_blocks[i]);

		x += 8;
		if (x == 256)
		{
			x = 0;
			y += 8;
		}
	}

	//imshow("WT result", WT_result);
	// 역 DWT 진행
	IWT(WT_result, IWT_result, 1);

	// 호스트 이미지의 Cb 채널을 워터마크 삽입된 Cb채널로 변경
	for (int i = 0; i < HostImg.rows; i++) {
		for (int j = 0; j < HostImg.cols; j++) {
			yuv.at<Vec3b>(i, j)[0] = (int)IWT_result.at<float>(i, j);
		}
	}

	cvtColor(yuv, yuv, COLOR_YCrCb2RGB); // RGB to YCrCb

	//imshow("* Marked Image", yuv);
	imwrite("Marked_Image.png", yuv);
	
	// 압축률 지정을 위한 부분
	//vector<int> param = vector<int>(2);
	//param[0] = CV_IMWRITE_JPEG_QUALITY;
	//param[1] = 95;//default(95) 0-100

	imwrite("Marked_Image.jpg", yuv);

	return yuv;
}

// SVD 방식 삽입
//Mat Embed(Mat& original_img, Mat& barcode)
//{
//  Mat yuv;
//  Mat yuv_arr[3];
//  Mat U, V;
//  Mat U_wm, S_wm, V_wm;
//  Mat WT_result;
//  Mat IWT_result;
//
//  Mat qrcode;
//  Mat yuv_qrcode;
//  Mat yuv_qrcode_arr[3];
//  Mat qrcode_WT_result;
//
//  qrcode = barcode;
//
//  cvtColor(qrcode, yuv_qrcode, COLOR_RGB2YCrCb);
//  cvtColor(original_img, yuv, COLOR_RGB2YCrCb);
//
//  split(yuv_qrcode, yuv_qrcode_arr);
//  split(yuv, yuv_arr);
//
//  WT(yuv_qrcode_arr[0], qrcode_WT_result, 1);
//  WT(yuv_arr[0], WT_result, 1);
//
//  Mat LL_subband = Mat(WT_result.cols / 2, WT_result.rows / 2, WT_result.type());
//  Mat HL_subband = Mat(WT_result.cols / 2, WT_result.rows / 2, WT_result.type());
//  Mat LH_subband = Mat(WT_result.cols / 2, WT_result.rows / 2, WT_result.type());
//  Mat HH_subband = Mat(WT_result.cols / 2, WT_result.rows / 2, WT_result.type());
//
//  Mat LL_WM = Mat(16, 16, 5);
//  Mat HL_WM = Mat(16, 16, 5);
//  Mat LH_WM = Mat(16, 16, 5);
//  Mat HH_WM = Mat(16, 16, 5);
//
//  Mat LL_subband_temp = WT_result(Rect(0, 0, 256, 256));                                
//  Mat HL_subband_temp = WT_result(Rect(WT_result.cols / 2, 0, 256, 256));
//  Mat LH_subband_temp = WT_result(Rect(0, WT_result.rows / 2, 256, 256));
//  Mat HH_subband_temp = WT_result(Rect(WT_result.cols / 2, WT_result.rows / 2, 256, 256));
//
//  Mat LL_WM_temp = qrcode_WT_result(Rect(0, 0, 16, 16));                          
//  Mat HL_WM_temp = qrcode_WT_result(Rect(16, 0, 16, 16));
//  Mat LH_WM_temp = qrcode_WT_result(Rect(0, 16, 16, 16));
//  Mat HH_WM_temp = qrcode_WT_result(Rect(16, 16, 16, 16));
//
//  LL_WM_temp.copyTo(LL_WM);
//  HL_WM_temp.copyTo(HL_WM);
//  LH_WM_temp.copyTo(LH_WM);
//  HH_WM_temp.copyTo(HH_WM);
//
//  LL_subband_temp.copyTo(LL_subband);
//  HL_subband_temp.copyTo(HL_subband);
//  LH_subband_temp.copyTo(LH_subband);
//  HH_subband_temp.copyTo(HH_subband);
//
//  /////////////////////////////////////////////////////
//  calculate_SVD(LL_subband, U_1, S_1, V_1);
//  calculate_SVD(HL_subband, U_2, S_2, V_2);
//  calculate_SVD(LH_subband, U_3, S_3, V_3);
//  calculate_SVD(HH_subband, U_4, S_4, V_4);
//
//  calculate_SVD(LL_WM, LL_U_wm, LL_S_wm, LL_V_wm);
//  calculate_SVD(HL_WM, HL_U_wm, HL_S_wm, HL_V_wm);
//  calculate_SVD(LH_WM, LH_U_wm, LH_S_wm, LH_V_wm);
//  calculate_SVD(HH_WM, HH_U_wm, HH_S_wm, HH_V_wm);
//  /////////////////////////////////////////////////////
//
//  Mat __S_1 = Mat(S_1.rows, S_1.cols, S_1.type());
//  Mat __S_2 = Mat(S_2.rows, S_2.cols, S_2.type());
//  Mat __S_3 = Mat(S_3.rows, S_3.cols, S_3.type());
//  Mat __S_4 = Mat(S_4.rows, S_4.cols, S_4.type());
//
//  S_1.copyTo(__S_1);
//  S_2.copyTo(__S_2);
//  S_3.copyTo(__S_3);
//  S_4.copyTo(__S_4);
//
//  int k = 0;
//  while (k < 16)
//  {
//      if (HL_S_wm.at<float>(k, k) == 0)
//          break;
//      else
//      {
//          __S_1.at<float>(k, k) = S_1.at<float>(k, k) + 1 * LL_S_wm.at<float>(k, k);
//          __S_2.at<float>(k, k) = S_2.at<float>(k, k) + 1 * HL_S_wm.at<float>(k, k);
//          __S_3.at<float>(k, k) = S_3.at<float>(k, k) + 1 * LH_S_wm.at<float>(k, k);
//          __S_4.at<float>(k, k) = S_4.at<float>(k, k) + 1 * HH_S_wm.at<float>(k, k);
//          k++;
//      }
//  }
//  ///////////////////////////////////////////////////////////////////// 삽입된거 확인, 추출되는지 확인
//  LL_subband = U_1 * __S_1 * V_1;
//  LL_subband.copyTo(WT_result(Rect(0, 0, 256, 256)));
//  HL_subband = U_2 * __S_2 * V_2;
//  HL_subband.copyTo(WT_result(Rect(WT_result.cols / 2, 0, 256, 256)));
//  LH_subband = U_3 * __S_3 * V_3;
//  LH_subband.copyTo(WT_result(Rect(0, WT_result.rows / 2, 256, 256)));
//  HH_subband = U_4 * __S_4 * V_4;
//  HH_subband.copyTo(WT_result(Rect(WT_result.cols / 2, WT_result.rows / 2, 256, 256)));
//
//  //imshow("삽입 후", WT_result);
//  IWT(WT_result, IWT_result, 1);
//  //imshow("삽입 후 역 웨이블릿", IWT_result);
//
//  //original의 U 채널을 워터마크 삽입된 U채널로 변경
//  for (int i = 0; i < original_img.rows; i++)
//  {
//      for (int j = 0; j < original_img.cols; j++)
//      {
//          yuv.at<Vec3b>(i, j)[0] = (int)IWT_result.at<float>(i, j);
//      }
//  }
//  cvtColor(yuv, yuv, COLOR_YCrCb2RGB); // RGV to YUV
//  //imshow("Marked Image(YUV to RGB)", yuv);
//
//  imwrite("SVD_Marked_Image.png", yuv);
//  imwrite("SVD_Marked_Image.jpg", yuv);
//  return yuv;
//}












