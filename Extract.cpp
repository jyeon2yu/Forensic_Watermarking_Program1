#pragma once
#include "stdafx.h"
#include "Headers.h"

// SVD ��� ���� / ���� ��� ��
/*extern Mat S_1;
extern Mat S_2;
extern Mat S_3;
extern Mat S_4;

extern Mat LL_U_wm, LL_V_wm;
extern Mat HL_U_wm, HL_V_wm;
extern Mat LH_U_wm, LH_V_wm;
extern Mat HH_U_wm, HH_V_wm;
*/


//////////////////////////////////////////////////////////////////////////////////////////
//// ����
//////////////////////////////////////////////////////////////////////////////////////////

void ExtractWatermark(Mat& marked_img)
{
	Extract(marked_img);
}

void Extract(Mat& Marked_Image)
{
	//imshow("Marked_Image", Marked_Image);
	////////////////////////////////
	//getPSNR(Marked_Image);			// ���� �̹����� ���� �̹����� PSNR �� ����� ����
	////////////////////////////////
	Mat yuv_arr[3];
	cvtColor(Marked_Image, Marked_Image, COLOR_RGB2YCrCb);   // RGB to YCrCb
	split(Marked_Image, yuv_arr);							 // ä�� �и�
	Mat Marked_Y_channel = Mat(Marked_Image.cols, Marked_Image.rows, Marked_Image.type());
	int QRcodeSize;

	yuv_arr[0].copyTo(Marked_Y_channel);					 // Y ä�� �и�

	// ������ QRcode ������� ������ ���
	Mat HH_recoverd_barcode_Pixel = Mat(32, 32, CV_8UC1);
	Mat LH_recoverd_barcode_Pixel = Mat(32, 32, CV_8UC1);
	Mat HL_recoverd_barcode_Pixel = Mat(32, 32, CV_8UC1);

	Mat WT_result;
	WT(Marked_Y_channel, WT_result, 1);               // �и��� Y ä���� ������� 1�ܰ� DWT ����
	//imshow("Extracted_Image_WT", WT_result);

	// �δ뿪�� ����� ������ ��� ����
	Mat HH_subband = Mat(WT_result.cols / 2, WT_result.rows / 2, WT_result.type());
	Mat HL_subband = Mat(WT_result.cols / 2, WT_result.rows / 2, WT_result.type());
	Mat LH_subband = Mat(WT_result.cols / 2, WT_result.rows / 2, WT_result.type());

	HH_subband = WT_result(Rect(WT_result.cols / 2, WT_result.rows / 2, WT_result.cols / 2, WT_result.rows / 2));
	HL_subband = WT_result(Rect(WT_result.cols / 2, 0, WT_result.cols / 2, WT_result.rows / 2));
	LH_subband = WT_result(Rect(0, WT_result.rows / 2, WT_result.cols / 2, WT_result.rows / 2));

	// DCT�� ������ 8x8 ũ���� ����
	Size blockSize(8, 8);
	vector<Mat> HH_blocks;	// �� �δ뿪�� ����
	vector<Mat> HL_blocks;
	vector<Mat> LH_blocks;
	int i = 0;

	// 256x256 ũ���� �δ뿪�� 1024���� 8x8 �� ������� ����
	for (int y = 0; y < 256; y += blockSize.height)
	{
		for (int x = 0; x < 256; x += blockSize.width)
		{
			Rect rect = Rect(x, y, blockSize.width, blockSize.height);
			HH_blocks.push_back(Mat(HH_subband, rect));
			HL_blocks.push_back(Mat(HL_subband, rect));
			LH_blocks.push_back(Mat(LH_subband, rect));
		}
	}

	int x = 0, y = 0;
	// �� �δ뿪�� 1024���� ������ ������� ���Ե� ���͸�ũ ���� ����
	for (int i = 0; i < 1024; i++)
	{
		// DCT ����
		dct(HH_blocks[i], HH_blocks[i]);
		dct(HL_blocks[i], HL_blocks[i]);
		dct(LH_blocks[i], LH_blocks[i]);

		// DCT ���� �� ������ DC ���� �������� ���Ե� QR �����
		float HH_sum = HH_blocks[i].at<float>(0, 0);
		float HL_sum = HL_blocks[i].at<float>(0, 0);
		float LH_sum = LH_blocks[i].at<float>(0, 0);
		
		// ���� ���� ������� QR�� �� �κа� ���� �κ� ����
		HH_recoverd_barcode_Pixel.at<uchar>((int)(y), (int)(x)) = ((HH_sum > 0) ? 255 : 0);
		HL_recoverd_barcode_Pixel.at<uchar>((int)(y), (int)(x)) = ((HL_sum > 0) ? 255 : 0);
		LH_recoverd_barcode_Pixel.at<uchar>((int)(y), (int)(x++)) = ((LH_sum > 0) ? 255 : 0);
		
		if (x == 32)
		{
			y++;
			x = 0;
		}
	}

	// QRcode�� ������ ��� ���� ����
	QRcodeSize = HL_recoverd_barcode_Pixel.rows;
	Mat QR_HH(QRcodeSize + 2, QRcodeSize + 2, HH_recoverd_barcode_Pixel.type(), Scalar(255));
	Mat QR_HL(QRcodeSize + 2, QRcodeSize + 2, HL_recoverd_barcode_Pixel.type(), Scalar(255));
	Mat QR_LH(QRcodeSize + 2, QRcodeSize + 2, LH_recoverd_barcode_Pixel.type(), Scalar(255));

	// ������ QRcode�� �ȼ� ���� ��ġ�� �°� ����
	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 32; j++)
		{
			QR_HH.at<uchar>(i + 1, j + 1) = HH_recoverd_barcode_Pixel.at<uchar>(i, j);
			QR_HL.at<uchar>(i + 1, j + 1) = HL_recoverd_barcode_Pixel.at<uchar>(i, j);
			QR_LH.at<uchar>(i + 1, j + 1) = LH_recoverd_barcode_Pixel.at<uchar>(i, j);
		}
	}

	Mat BIG_QR_2(100, 100, HH_recoverd_barcode_Pixel.type(), Scalar(255));
	Mat BIG_QR_3(100, 100, HL_recoverd_barcode_Pixel.type(), Scalar(255));
	Mat BIG_QR_4(100, 100, LH_recoverd_barcode_Pixel.type(), Scalar(255));

	int n = 0;
	// 32x32 ũ���� QR�� ũ�� Ȯ��
	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 32; j++)
		{
			BIG_QR_2.at<uchar>(n, 3 * j)		 = QR_HH.at<uchar>(i, j);
			BIG_QR_2.at<uchar>(n, 3 * j + 1)	 = QR_HH.at<uchar>(i, j);
			BIG_QR_2.at<uchar>(n, 3 * j + 2)	 = QR_HH.at<uchar>(i, j);
			BIG_QR_2.at<uchar>(n + 1, 3 * j)	 = QR_HH.at<uchar>(i, j);
			BIG_QR_2.at<uchar>(n + 1, 3 * j + 1) = QR_HH.at<uchar>(i, j);
			BIG_QR_2.at<uchar>(n + 1, 3 * j + 2) = QR_HH.at<uchar>(i, j);
			BIG_QR_2.at<uchar>(n + 2, 3 * j)	 = QR_HH.at<uchar>(i, j);
			BIG_QR_2.at<uchar>(n + 2, 3 * j + 1) = QR_HH.at<uchar>(i, j);
			BIG_QR_2.at<uchar>(n + 2, 3 * j + 2) = QR_HH.at<uchar>(i, j);

			BIG_QR_3.at<uchar>(n, 3 * j)		 = QR_HL.at<uchar>(i, j);
			BIG_QR_3.at<uchar>(n, 3 * j + 1)	 = QR_HL.at<uchar>(i, j);
			BIG_QR_3.at<uchar>(n, 3 * j + 2)	 = QR_HL.at<uchar>(i, j);
			BIG_QR_3.at<uchar>(n + 1, 3 * j)	 = QR_HL.at<uchar>(i, j);
			BIG_QR_3.at<uchar>(n + 1, 3 * j + 1) = QR_HL.at<uchar>(i, j);
			BIG_QR_3.at<uchar>(n + 1, 3 * j + 2) = QR_HL.at<uchar>(i, j);
			BIG_QR_3.at<uchar>(n + 2, 3 * j)	 = QR_HL.at<uchar>(i, j);
			BIG_QR_3.at<uchar>(n + 2, 3 * j + 1) = QR_HL.at<uchar>(i, j);
			BIG_QR_3.at<uchar>(n + 2, 3 * j + 2) = QR_HL.at<uchar>(i, j);

			BIG_QR_4.at<uchar>(n, 3 * j)		 = QR_LH.at<uchar>(i, j);
			BIG_QR_4.at<uchar>(n, 3 * j + 1)	 = QR_LH.at<uchar>(i, j);
			BIG_QR_4.at<uchar>(n, 3 * j + 2)	 = QR_LH.at<uchar>(i, j);
			BIG_QR_4.at<uchar>(n + 1, 3 * j)	 = QR_LH.at<uchar>(i, j);
			BIG_QR_4.at<uchar>(n + 1, 3 * j + 1) = QR_LH.at<uchar>(i, j);
			BIG_QR_4.at<uchar>(n + 1, 3 * j + 2) = QR_LH.at<uchar>(i, j);
			BIG_QR_4.at<uchar>(n + 2, 3 * j)	 = QR_LH.at<uchar>(i, j);
			BIG_QR_4.at<uchar>(n + 2, 3 * j + 1) = QR_LH.at<uchar>(i, j);
			BIG_QR_4.at<uchar>(n + 2, 3 * j + 2) = QR_LH.at<uchar>(i, j);
		}
		n += 3;
	}

	imshow("HH_QRcode", BIG_QR_2);
	imshow("HL_QRcode", BIG_QR_3);
	imshow("LH_QRcode", BIG_QR_4);

	imwrite("HH_QRcode.png", BIG_QR_2);
	imwrite("HL_QRcode.png", BIG_QR_3);
	imwrite("LH_QRcode.png", BIG_QR_4);
	//imwrite("HH_recoverd_QRcode.png", HH_recoverd_barcode_Pixel);
	//imwrite("HL_recoverd_QRcode.png", HL_recoverd_barcode_Pixel);
	//imwrite("LH_recoverd_QRcode.png", LH_recoverd_barcode_Pixel);

	//getNCC();		// ���Ե� ���͸�ũ�� ����� ���͸�ũ �� NCC �� ���
}


// SVD ��� ���� 
//void Extract(Mat& Marked_Image)
//{
//  Mat yuv_arr[3];
//
//  cvtColor(Marked_Image, Marked_Image, COLOR_RGB2YUV);
//  split(Marked_Image, yuv_arr);                                              // YUV ������ ��ŷ�� �̹����� �и�
//  Mat Marked_Y_channel = Mat(Marked_Image.cols, Marked_Image.rows, Marked_Image.type());    // ���͸�ũ�� ���Ե� U ä��
//
//  yuv_arr[0].copyTo(Marked_Y_channel);               // U ä�� �и�
//  //imshow("Marked_Image's Y channel", Marked_Y_channel);
//
//  Mat LL_subband_recoverd_barcode_Pixel = Mat(16, 16, CV_8UC1);
//  Mat HL_subband_recoverd_barcode_Pixel = Mat(16, 16, CV_8UC1);
//  Mat LH_subband_recoverd_barcode_Pixel = Mat(16, 16, CV_8UC1);
//  Mat HH_subband_recoverd_barcode_Pixel = Mat(16, 16, CV_8UC1);
//
//  Mat WT_result;
//  WT(Marked_Y_channel, WT_result, 1);
//  //imshow("inverse_WT_result", WT_result);
//
//  Mat LL_subband = Mat(WT_result.cols / 2, WT_result.rows / 2, WT_result.type());
//  Mat HL_subband = Mat(WT_result.cols / 2, WT_result.rows / 2, WT_result.type());
//  Mat LH_subband = Mat(WT_result.cols / 2, WT_result.rows / 2, WT_result.type());
//  Mat HH_subband = Mat(WT_result.cols / 2, WT_result.rows / 2, WT_result.type());
//
//  Mat LL_subband_temp = WT_result(Rect(0, 0, 256, 256));
//  Mat HL_subband_temp = WT_result(Rect(WT_result.cols / 2, 0, 256, 256));
//  Mat LH_subband_temp = WT_result(Rect(0, WT_result.rows / 2, 256, 256));
//  Mat HH_subband_temp = WT_result(Rect(WT_result.cols / 2, WT_result.rows / 2, 256, 256));
//
//  LL_subband_temp.copyTo(LL_subband);
//  HL_subband_temp.copyTo(HL_subband);
//  LH_subband_temp.copyTo(LH_subband);
//  HH_subband_temp.copyTo(HH_subband);
//
//  Mat u_1, s_1, v_1;
//  Mat u_2, s_2, v_2;
//  Mat u_3, s_3, v_3;
//  Mat u_4, s_4, v_4;
//
//  Mat u_wm, v_wm;
//  Mat s_wm_1 = Mat(16, 16, 5);
//  Mat s_wm_2 = Mat(16, 16, 5);
//  Mat s_wm_3 = Mat(16, 16, 5);
//  Mat s_wm_4 = Mat(16, 16, 5);
//
//  Mat s_wm = Mat(256, 256, 5);
//  s_wm = Scalar(0);
//
//  calculate_SVD(LL_subband, u_1, s_1, v_1);   // ��ŷ �̹����� U S V
//  calculate_SVD(HL_subband, u_2, s_2, v_2);   // ��ŷ �̹����� U S V
//  calculate_SVD(LH_subband, u_3, s_3, v_3);   // ��ŷ �̹����� U S V
//  calculate_SVD(HH_subband, u_4, s_4, v_4);   // ��ŷ �̹����� U S V
//
//  int cnt = 0;
//  int k = 0;
//
//  while (cnt < 16)
//  {
//      s_wm_1.at<float>(cnt, cnt) = (s_1.at<float>(cnt, cnt) - S_1.at<float>(cnt, cnt));
//      s_wm_2.at<float>(cnt, cnt) = (s_2.at<float>(cnt, cnt) - S_2.at<float>(cnt, cnt));
//      s_wm_3.at<float>(cnt, cnt) = (s_3.at<float>(cnt, cnt) - S_3.at<float>(cnt, cnt));
//      s_wm_4.at<float>(cnt, cnt) = (s_4.at<float>(cnt, cnt) - S_4.at<float>(cnt, cnt));
//      cnt++;
//  }
//
//  for (int i = 0; i < 16; i++)
//  {
//      for (int j = 0; j < 16; j++)
//      {
//          if (i == j)
//              continue;//s_wm_4.at<float>(i, j) *= 1.5; 
//          else
//          {
//              s_wm_1.at<float>(i, j) = 0;
//              s_wm_2.at<float>(i, j) = 0;
//              s_wm_3.at<float>(i, j) = 0;
//              s_wm_4.at<float>(i, j) = 0;
//          }
//      }
//  }
//
//
//  LL_subband_recoverd_barcode_Pixel = LL_U_wm * s_wm_1 * LL_V_wm;
//  HL_subband_recoverd_barcode_Pixel = HL_U_wm * s_wm_2 * HL_V_wm;
//  LH_subband_recoverd_barcode_Pixel = LH_U_wm * s_wm_3 * LH_V_wm;
//  HH_subband_recoverd_barcode_Pixel = HH_U_wm * s_wm_4 * HH_V_wm;
//
//  Mat result_IWT = Mat(32, 32, 5);
//  LL_subband_recoverd_barcode_Pixel.copyTo(result_IWT(Rect(0, 0, 16, 16)));
//  HL_subband_recoverd_barcode_Pixel.copyTo(result_IWT(Rect(16, 0, 16, 16)));
//  LH_subband_recoverd_barcode_Pixel.copyTo(result_IWT(Rect(0, 16, 16, 16)));
//  HH_subband_recoverd_barcode_Pixel.copyTo(result_IWT(Rect(16, 16, 16, 16)));
//
//  //imshow("result_IWT 1", result_IWT);
//  IWT(result_IWT, result_IWT, 1);
//  //imshow("result_IWT 2", result_IWT);
//  Mat BIG_QR(100, 100, result_IWT.type(), Scalar(255));
//
//  int n = 0;
//  for (int i = 0; i < 32; i++)
//  {
//      for (int j = 0; j < 32; j++)
//      {
//          BIG_QR.at<float>(n, 3 * j)		 = result_IWT.at<float>(i, j);
//          BIG_QR.at<float>(n, 3 * j + 1)	 = result_IWT.at<float>(i, j);
//          BIG_QR.at<float>(n, 3 * j + 2)	 = result_IWT.at<float>(i, j);
//          BIG_QR.at<float>(n + 1, 3 * j)	 = result_IWT.at<float>(i, j);
//          BIG_QR.at<float>(n + 1, 3 * j + 1) = result_IWT.at<float>(i, j);
//          BIG_QR.at<float>(n + 1, 3 * j + 2) = result_IWT.at<float>(i, j);
//          BIG_QR.at<float>(n + 2, 3 * j) = result_IWT.at<float>(i, j);
//          BIG_QR.at<float>(n + 2, 3 * j + 1) = result_IWT.at<float>(i, j);
//          BIG_QR.at<float>(n + 2, 3 * j + 2) = result_IWT.at<float>(i, j);
//      }
//      n += 3;
//  }
//
//  for (int i = 0; i < 100; i++)
//  {
//      for (int j = 0; j < 100; j++)
//      {
//          if (BIG_QR.at<float>(i, j) >= 125)
//              BIG_QR.at<float>(i, j) = 255;
//          else
//              BIG_QR.at<float>(i, j) = 0;
//
//      }
//  }
//  imshow("SVD_Extracted QR", BIG_QR);
//  imwrite("SVD_QR.png", BIG_QR);
//}