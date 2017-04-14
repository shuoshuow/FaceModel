#pragma once
#include "RFShapeRegressor.h"

class FaceLbfAlignmentor {
public:
	facesdk::alignmentor::LBFAlignment *m_pFaceAlign;
	std::vector<float> faceLandmark;

	FaceLbfAlignmentor(byte* ptr, int len);
	FaceLbfAlignmentor(string modelPath);

    ~FaceLbfAlignmentor();

	bool DoFaceAlignment(unsigned char* image, int width, int height, int stride, int faceLeft, int faceTop, int faceWidth, int faceHeight);
};
