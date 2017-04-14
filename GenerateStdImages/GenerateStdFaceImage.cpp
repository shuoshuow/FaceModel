#pragma once
#pragma warning(push)
#pragma warning(disable:4996)
#include "FolderFileHelper.h"
#include "BitmapConvertor.h"
#include <strsafe.h>
#include <gdiplus.h>
#include <vector>
#include "Geometry.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace Gdiplus;

using namespace DNNTestLib;
#define MAX_LINE_LEN 20000

const float std_points[] = {
    77.862474f, 77.989120f,
    121.039278f, 77.241471f,
    99.519567f, 104.563127f,
    80.387235f, 124.122528f,
    120.133260f, 123.433507f };


bool GenerateStdFaceImage(const unsigned char *pInput,
    int iInputWidth, int iInputHeight, int iInputChannels, int iInputStride,
    const float *pFacialPoints, int iPointsNum,
    const float *pStdFacialPoints, int iStdPointsNum,
    int iOutputWidth, int iOutputHeight,
    __out unsigned char *pOutput, __out float *pAlignedFacialPoints)
{
    if (iPointsNum < iStdPointsNum)
        return false;

    float coeffOriToStd[9];
    ComputeSimTransCoeff(coeffOriToStd, pFacialPoints, pStdFacialPoints, iStdPointsNum * 2);
	AffineTransform(pAlignedFacialPoints, iPointsNum * 2, coeffOriToStd, pFacialPoints);
	ImageTransform(pInput, iInputWidth, iInputHeight, iInputChannels, iInputStride, coeffOriToStd,
        pOutput, iOutputWidth, iOutputHeight, iOutputWidth*iInputChannels);
    
    return true;
}

void GenerateStdFaceImage(wchar_t* path_input, float* landmark_points, int landmark_num, wchar_t* path_output, __out float *vecFacialPoints)
{
	Bitmap bmp(path_input);
	int w = bmp.GetWidth();
	int h = bmp.GetHeight();
	int w_align = 200;
	int h_align = 200;
	FILE *fpPts = NULL;
	
	vector<unsigned char> aligned_image(w_align * h_align * 3);

	BYTE *pLum = new BYTE[w * h * 3];
	BYTE *pImg = pLum;
	BitmapData bmpData;
	if ((&bmp)->LockBits(NULL, ImageLockModeRead, PixelFormat24bppRGB, &bmpData) != Ok)
		return;
	BYTE *pImageLine = (BYTE*)bmpData.Scan0;
	for (UINT i = 0; i < bmpData.Height; i++)
	{
		BYTE *pImage = pImageLine;
		for (UINT j = 0; j < bmpData.Width; j++)
		{
			*pLum = pImage[0];			pLum++;
			*pLum = pImage[1];			pLum++;
			*pLum = pImage[2];			pLum++;
			pImage += 3;
		}
		pImageLine += bmpData.Stride;
	}
	(&bmp)->UnlockBits(&bmpData);

	GenerateStdFaceImage(pImg, w, h, 3, w * 3, landmark_points, landmark_num, std_points, 5, w_align, h_align, &aligned_image[0], &vecFacialPoints[0]);

	// save transformed image
	int w_out = min(120, w_align);
	int h_out = min(120, h_align);
	int x_ori = (w_align - w_out) / 2;
	int y_ori = (h_align - h_out) / 2;
	CLSID encoder;
	GetEncoderClsid(FT_BMP, &encoder);

	int stride = ((w_out * 3 + 3) / 4) * 4;

	unsigned char *pColorPixels = new unsigned char[h_out * stride];
	unsigned char *pColorRow = pColorPixels;
	unsigned char *pAlignedImage = &aligned_image[0];
	pAlignedImage += y_ori * w_align * 3;
	for (int y = 0; y < h_out; y++, pColorRow += stride, pAlignedImage += 3 * w_align)
	{
		for (int x = 0; x < w_out; x++)
		{
			pColorRow[x * 3] = pAlignedImage[(x + x_ori) * 3];
			pColorRow[x * 3 + 1] = pAlignedImage[(x + x_ori) * 3 + 1];
			pColorRow[x * 3 + 2] = pAlignedImage[(x + x_ori) * 3 + 2];
		}
	}
	Bitmap _bmp(w_out, h_out, stride, PixelFormat24bppRGB, pColorPixels);

	// shift standard landmark
	for (int i = 0; i < landmark_num; i++)
	{
		vecFacialPoints[i * 2] -= x_ori;
		vecFacialPoints[i * 2 + 1] -= y_ori;
	}

	Graphics gImg(&_bmp);
	Pen pen(Color::YellowGreen);
	pen.SetWidth(2);

	for (int i = 5; i < landmark_num; i++)
	{
		gImg.DrawEllipse(&pen, (int)vecFacialPoints[i * 2], (int)vecFacialPoints[i * 2 + 1], 2, 2);
	}

	_bmp.Save(path_output, &encoder, NULL);

	delete[] pColorPixels;
	delete[] pImg;
}

wchar_t* StringToWchar(string str)
{
	wchar_t* output = new wchar_t[str.length() + 1];
	std::copy(str.begin(), str.end(), output);
	output[str.length()] = '\0';

	return output;
}

void wmain(int argc, wchar_t *argv[])
{
	wchar_t* path_imagelist = L"D:\\Work\\FaceData\\Face_BeautyGoPro\\Log\\Train_F80s_F_frontface_test_68pt.tsv";
	wchar_t* path_alignedPts = L"D:\\Work\\FaceData\\Face_BeautyGoPro\\Log\\Train_F80s_F_frontface_test_68pt_std.txt";
	string root_output = "D:\\Work\\FaceData\\Face_BeautyGoPro\\68pt_std_landmark";
	
	/*wchar_t* path_imagelist = argv[1];
	wstring wstr = wstring(argv[2]);
	wchar_t* path_alignedPts = argv[3];
	string root_output(wstr.begin(), wstr.end());
	CreateDirectory(wstr.c_str(), NULL);*/

	ifstream pf;
	pf.open(path_imagelist);
	ofstream pfOut;
	pfOut.open(path_alignedPts, ofstream::out);

	string line, str;
	int landmarkNum = 68; //27;// 
	float* landmark_points = new float[(landmarkNum + 5) * 2];
	vector<string> item;
	vector<string> landmarks;
	vector<string> pt;
	wchar_t* path_input;
	wchar_t* path_output;
	int n = 0;

	while (std::getline(pf, line))
	{
		item.clear();
		landmarks.clear();
		pt.clear();

		boost::split(item, line, boost::is_any_of("\t"));

		// read file name
		path_output = StringToWchar(root_output + "\\" + item[0] + "_" + item[4] + ".bmp");
		path_input = StringToWchar(item[1]);

		boost::trim(item[11]);
		boost::split(landmarks, item[11], boost::is_any_of(" "));
		/*boost::trim(item[7]);
		boost::split(landmarks, item[7], boost::is_any_of(" "));*/
		for (int i = 5; i < landmarkNum + 5; i++)
		{
			boost::split(pt, landmarks[i-5], boost::is_any_of(","));

			landmark_points[i * 2] = boost::lexical_cast<double>(pt[0]);
			landmark_points[i * 2 + 1] = boost::lexical_cast<double>(pt[1]);
		}

		landmark_points[0] = (landmark_points[(36 + 5) * 2] + landmark_points[(39+5) * 2]) / 2;
		landmark_points[0 + 1] = (landmark_points[(36 + 5) * 2 + 1] + landmark_points[(39 + 5) * 2 + 1]) / 2;
		landmark_points[1 * 2] = (landmark_points[(42+5) * 2] + landmark_points[(45+5) * 2]) / 2;
		landmark_points[1 * 2 + 1] = (landmark_points[(42 + 5) * 2 + 1] + landmark_points[(45 + 5) * 2 + 1]) / 2;
		landmark_points[2 * 2] = landmark_points[(30 + 5) * 2];
		landmark_points[2 * 2 + 1] = landmark_points[(30 + 5) * 2 + 1];
		landmark_points[3 * 2] = landmark_points[(48 + 5) * 2];
		landmark_points[3 * 2 + 1] = landmark_points[(48 + 5) * 2 + 1];
		landmark_points[4 * 2] = landmark_points[(54+5) * 2];
		landmark_points[4 * 2 + 1] = landmark_points[(54+5) * 2 + 1];

		// crop face and transform -- RGB version
		vector<float> vecFacialPoints((landmarkNum + 5) * 2);
		GenerateStdFaceImage(path_input, landmark_points, landmarkNum + 5, path_output, &vecFacialPoints[0]);

		wstring ws = wstring(path_output);
		item[2] = string(ws.begin(), ws.end());
		for (int i = 0; i < item.size(); i++)
			pfOut << item[i] + "\t";
		for (int i = 5; i < landmarkNum + 5; i++)
		{
			pfOut << vecFacialPoints[i * 2] << "," << vecFacialPoints[i * 2 + 1] << " ";
		}
		pfOut << endl;
		// crop face and transform -- Gray version
		//vector<float> vecFacialPoints(54);
		//vector<unsigned char> aligned_image(200 * 200);
		//Bitmap bmp(path_input);
		//int w = bmp.GetWidth();
		//int h = bmp.GetHeight();
		//BYTE *pLum = new BYTE[w * h];
		//CBitmapConvertor::ConvertBitmapToGray(pLum, &bmp);
		//GenerateStdFaceImage(pLum, w, h, 1, w, landmark_points, 27, std_points, 5, 200, 200, &aligned_image[0], &vecFacialPoints[0]);
		//CBitmapConvertor::SaveGrayImgToFile(path_output, &aligned_image[0], 200, 200, FT_BMP);


		cout << "[" << n << "] has been processed\t" << item[0] << endl;
		n++;		
	}
	
	pf.close();
	pfOut.close();

	system("pause");
}