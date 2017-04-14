#include "Classifier.h"
#include "ehm.h"

using namespace facesdk::alignmentor;

HRESULT Classifier::Predict(const float *pShape, int length, const float *pMeanShape, const BYTE *grayImage, int width, int height, int stride, float& score)
{
    HRESULT hr = S_OK;

    const int *pPredIds;
    int iPredNum;
    hr = m_pStage->Predict(pPredIds, iPredNum, pMeanShape, pShape, length, grayImage, width, height, stride);
    CHR(hr);

    score = 0.0f;
    hr = m_regressor.Predict(&score, 1, pPredIds, iPredNum);
    CHR(hr);

Error:

    return hr;
}

HRESULT Classifier::initialize(const BYTE*& buffer, size_t& bufferSize, StageT* pStage)
{
    m_pStage = pStage;

    HRESULT hr = S_OK;

    hr = m_regressor.initialize(buffer, bufferSize);
    CHR(hr);

    m_initialized = true;

Error:		
    return hr;
}
