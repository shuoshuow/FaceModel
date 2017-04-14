#pragma warning (disable : 4996)

#include "iostream"
#include "fstream"
#include "string"
#include "vector"
#include "algorithm"
#include ".\FaceLBF68ptsAligner\RFShapeRegressor.h"
#include ".\FaceLBF68ptsAligner\FaceLbfAlignmentor.h"
#include <highgui/highgui.hpp>
#include <cv.h>

using namespace cv;
using namespace std;

struct FaceRectangle
{
	int Left;
	int Top;
	int Width;
	int Height;
};

byte* matToBytes(Mat image)
{
	int size = image.total() * image.elemSize();
	byte* bytes = new byte[size];
	memcpy(bytes, image.data, size * sizeof(byte));
	
	//int size = image.rows * image.cols;
	//byte* bytes = new byte[size];
	//for (int i = 0; i < image.rows; i++)
	//{		
	//	for (int j = 0; j < image.cols; j++)
	//	{
	//		bytes[i * image.cols + j] = image.data[image.step * i + j];
	//	}
	//}

	return bytes;
}

Mat showLandmarks(string imagePath, vector<float> faceLandmark, vector<float> rect)
{
	Mat image = imread(imagePath);
	int dimension = faceLandmark.size();
	for (int i = 0; i < dimension / 2; i++)
	{
		Point pt;
		pt.x = faceLandmark[2 * i];
		pt.y = faceLandmark[2 * i + 1];
		circle(image, pt, 2, cv::Scalar(0, 255, 0, 255), 2);
	}
	rectangle(image, cv::Rect(rect[0], rect[1], rect[2], rect[3]), cv::Scalar(0, 255, 255, 255), 2);
	//imshow("Image", image);
	//waitKey(0);

	return image;
}

int main()
{
	string modelPath = ".\\Models\\alignment_lbf_68pts.mdl";
	string filePath = "C:\\Users\\shuowan.FAREAST\\Desktop\\MeanFace.txt";//"D:\\Work\\FaceData\\Face_BeautyGoPro\\Log\\Train_F80s_F_frontface_test.tsv";
	string outputPath = "C:\\Users\\shuowan.FAREAST\\Desktop\\MeanFace_68pt.txt"; // "D:\\Work\\FaceData\\Face_BeautyGoPro\\Log\\Train_F80s_F_frontface_test_68pt.tsv";
	string outimageRoot = "D:\\Work\\FaceData\\Face_BeautyGoPro\\68pt_landmark\\";

	// load model
	FaceLbfAlignmentor* alignmentor = new FaceLbfAlignmentor(modelPath);

	ifstream file(filePath, ios::in);
	ofstream output(outputPath, ios::out);	

	string line;
	while (getline(file, line))
	{
		istringstream ss;
		vector<string> items;
		string tmp;

		ss.str(line);
		while (getline(ss, tmp, '\t'))
			items.push_back(tmp);

		string imagePath = items[1];
		replace(imagePath.begin(), imagePath.end(), '\\', '/');
		string faceRect = items[8];
		int beautyLevel = atoi(items[9].c_str());
		string outimagePath = outimageRoot + items[0] + "_" + items[4] + "_68pt.jpg";
				
		ss.clear();
		ss.str(faceRect);
		vector<float> rect;
		while (getline(ss, tmp, ' '))
			rect.push_back(atof(tmp.c_str()));

		//// load 27-pt landmarks
		//vector<float> landmarks_27;
		//ss.clear();
		//ss.str(items[7]);
		//while (getline(ss, tmp, ' '))
		//{
		//	istringstream tss(tmp);
		//	string tt;
		//	while (getline(tss, tt, ','))
		//		landmarks_27.push_back(atof(tt.c_str()));
		//}

		Mat gray_image = imread(imagePath, CV_LOAD_IMAGE_GRAYSCALE);
		byte* grayBuff = matToBytes(gray_image);
		int width = gray_image.cols;
		int height = gray_image.rows;	
		
		if (alignmentor->DoFaceAlignment((unsigned char*)&grayBuff[0], width, height, width, rect[0], rect[1], rect[2], rect[3]))
		{
			Mat img_68 = showLandmarks(imagePath, alignmentor->faceLandmark, rect);
			/*Mat img_27 = showLandmarks(imagePath, landmarks_27, rect);

			Mat img(height, 2 * width, CV_8UC3);
			Mat left(img, cv::Rect(0, 0, width, height));
			img_68.copyTo(left);
			Mat right(img, cv::Rect(width, 0, width, height));
			img_27.copyTo(right);
			
			imshow("Image", img);
			waitKey(0);
			*/
			
			line += "\t";				
			for (int i = 0; i < alignmentor->faceLandmark.size() / 2; i++)
			{
				line += to_string(alignmentor->faceLandmark[2 * i]) + "," + to_string(alignmentor->faceLandmark[2 * i + 1]) + " ";
			}
			output << line << endl;
			imwrite(outimagePath, img_68);
		}
	}

	file.close();
	output.close();
	//system("pause");
	return 1;
}



