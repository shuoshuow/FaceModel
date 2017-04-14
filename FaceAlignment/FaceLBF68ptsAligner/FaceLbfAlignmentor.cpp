#include "FaceLbfAlignmentor.h"
#include "fstream"
#include "iostream"
#include "vector"

FaceLbfAlignmentor::FaceLbfAlignmentor(byte* ptr, int len)
{
	if (m_pFaceAlign == NULL)
		m_pFaceAlign = new facesdk::alignmentor::LBFAlignment();;
	m_pFaceAlign->initialize(ptr, len);
}

FaceLbfAlignmentor::FaceLbfAlignmentor(string modelPath)
{
	std::ifstream file(modelPath, ios::binary | ios::in);
	int length = 0;
	char* data = NULL;

	if (!file.is_open())
		return;

	file.seekg(0, ios::end);
	length = file.tellg().seekpos();
	file.seekg(0, ios::beg);

	data = new char[length];
	file.read(data, length);

	if (m_pFaceAlign == NULL)
		m_pFaceAlign = new facesdk::alignmentor::LBFAlignment();
	m_pFaceAlign->initialize((unsigned char*)&data[0], length);	
}

FaceLbfAlignmentor::~FaceLbfAlignmentor()
{
}

bool FaceLbfAlignmentor::DoFaceAlignment(unsigned char* image, int width, int height, int stride, int faceLeft, int faceTop, int faceWidth, int faceHeight)
{	
	if (m_pFaceAlign == NULL)
		return false;
	
	float alignScore;
	const int dimension = (int)m_pFaceAlign->get_point_count() * 2;
	faceLandmark.resize(dimension);

	HRESULT hr = m_pFaceAlign->predict(image, width, height, stride,
		faceLeft, faceTop, faceWidth, faceHeight,
		&faceLandmark[0], dimension, alignScore);

	return (hr == S_OK) ? true : false;
}
