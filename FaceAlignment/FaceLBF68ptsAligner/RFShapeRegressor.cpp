#include "RFShapeRegressor.h"
#include "ModelFileHeader.h"

#include <algorithm>
#include <functional>
#include <math.h>
#include <assert.h>
#include "ehm.h"
#include "iodefs.h"
#include "affine.h"
#include <stdio.h>

using namespace facesdk::alignmentor;

HRESULT LBFAlignment::predict(
    __in_bcount(stride * height) const unsigned char* image,
    __in size_t width,
    __in size_t height,
    __in size_t stride,
    __in size_t faceLeft,
    __in size_t faceTop,
    __in size_t faceWidth,
    __in size_t faceHeight,
    __inout_ecount(dimension) float* shape,
    __in size_t dimension,
    __out float& score,
    __in BOOL useMeanShape
    ) /*throw()*/
{
    HRESULT hr = S_OK;

    CPP(image);
    CPP(shape);
    CBREx(width > 0, E_INVALIDARG);
    CBREx(height > 0, E_INVALIDARG);
    CBREx(stride >= width, E_INVALIDARG);
    CBREx(faceWidth > 0, E_INVALIDARG);
    CBREx(faceHeight > 0, E_INVALIDARG);
    CBREx(faceLeft + faceWidth < width, E_INVALIDARG);
    CBREx(faceTop + faceHeight < height, E_INVALIDARG);
    CBREx(dimension > 0, E_INVALIDARG);
    CBREx(dimension == m_mean_shape.size(), E_INVALIDARG);
    CBREx(dimension == m_curr_shape.size(), E_INVALIDARG);

    m_face_coords[0] = static_cast<float>(faceLeft);
    m_face_coords[1] = static_cast<float>(faceTop);
    m_face_coords[2] = static_cast<float>(faceLeft + faceWidth - 1);
    m_face_coords[3] = static_cast<float>(faceTop);
    m_face_coords[4] = static_cast<float>(faceLeft);
    m_face_coords[5] = static_cast<float>(faceTop + faceHeight - 1);
    m_face_coords[6] = static_cast<float>(faceLeft + faceWidth - 1);
    m_face_coords[7] = static_cast<float>(faceTop + faceHeight - 1);

    affine coeff;

    if (useMeanShape)
    {
        coeff.compute_similar_transfrom(m_std_face_coords, m_face_coords, _countof(m_std_face_coords));
        coeff.apply(&m_mean_shape[0], &m_curr_shape[0], m_mean_shape.size());
    }
    else
    {
        memcpy_s(&m_curr_shape[0], sizeof(float) * dimension, shape, sizeof(float) * dimension);
    }

    hr = predict_impl(image, width, height, stride, shape, dimension, score);
    CHR(hr);

Error:
    return hr;
}

HRESULT LBFAlignment::predict(
    __in_bcount(stride * height) const unsigned char* image,
    __in size_t width,
    __in size_t height,
    __in size_t stride,
    __inout_ecount(dimension) float* shape,
    __in size_t dimension,
    __out RECT& faceRect,
    __out float& score
    )
{
    HRESULT hr = S_OK;

    CPP(image);
    CPP(shape);
    CBREx(width > 0, E_INVALIDARG);
    CBREx(height > 0, E_INVALIDARG);
    CBREx(stride >= width, E_INVALIDARG);
    CBREx(dimension > 0, E_INVALIDARG);
    CBREx(dimension == m_mean_shape.size(), E_INVALIDARG);
    CBREx(dimension == m_curr_shape.size(), E_INVALIDARG);

    affine coeff;

	// We should do a C++ safe copy and instead use memcpy 
    //std::copy(shape, shape + dimension, &m_curr_shape[0]);
	memcpy(&m_curr_shape[0], shape, dimension * sizeof(float));

    coeff.compute_diff_similarity_transform(&m_mean_shape[0], &m_curr_shape[0], &m_tran_shape[0], m_mean_shape.size());
    coeff.apply(&m_mean_shape[0], &m_curr_shape[0], m_mean_shape.size());

    hr = predict_impl(image, width, height, stride, shape, dimension, score);
    CHR(hr);

    float faceSize = sqrt(coeff.coef[0] * coeff.coef[0] + coeff.coef[1] * coeff.coef[1]);
    faceRect.left = 0;
    faceRect.right = static_cast<LONG>(faceSize);
    faceRect.top = 0;
    faceRect.bottom = static_cast<LONG>(faceSize);

Error:
    return hr;
}

HRESULT LBFAlignment::predict_impl(
    __in_bcount(stride * height) const unsigned char* image,
    __in size_t width,
    __in size_t height,
    __in size_t stride,
    __inout_ecount(dimension) float* shape,
    __in size_t dimension,
    __out float& score
    )
{
    HRESULT hr = S_OK;

    for (size_t i = 0; i < m_stages.size(); i++)
    {
        hr = m_stages[i].Predict(
            &m_curr_shape[0],
            static_cast<int>(m_curr_shape.size()),
            &m_mean_shape[0],
            image,
            static_cast<int>(width),
            static_cast<int>(height),
            static_cast<int>(stride)
            );
        CHR(hr);
    }

    memcpy_s(shape, sizeof(float) * dimension, &m_curr_shape[0], sizeof(float) * dimension);

    if (m_classifier.is_initialized())
    {
        hr = m_classifier.Predict(
            &m_curr_shape[0],
            static_cast<int>(m_curr_shape.size()),
            &m_mean_shape[0],
            image,
            static_cast<int>(width),
            static_cast<int>(height),
            static_cast<int>(stride),
            score
            );
        CHR(hr);
    }
    else
    {
        // TODO: use other value to indicate this score is not estimated?
        score = 0.0f;
    }

Error:
    return hr;
}


HRESULT LBFAlignment::saveToFile(__in const char* filename)
{
    FILE* fp = NULL;
    fopen_s(&fp, filename, "wb");
    if (NULL == fp) return E_FAIL;

    DWORD cc = ALGORITHM_ALIGNMENT_LBF_FOURCC;
    int modelLength = 0;

    fwrite(&cc, sizeof(DWORD), 1, fp);
    fwrite(&modelLength, sizeof(int), 1, fp);

    int landmarkCount = this->get_point_count();
    fwrite(&landmarkCount, sizeof(int), 1, fp);
    
    fwrite(&m_mean_shape[0], sizeof(float), 2 * landmarkCount, fp);

    int stageCount = m_stages.size();
    fwrite(&stageCount, sizeof(int), 1, fp);

    for (int i = 0; i < stageCount; ++i)
    {
        int stageModelStart = ftell(fp);
        int stageModelSize = 0;
        int stageTreeCount = m_stages[i].GetStageTreeCount();

        fwrite(&stageModelSize, sizeof(int), 1, fp);
        fwrite(&stageTreeCount, sizeof(int), 1, fp);

        if (S_OK != m_stages[i].saveToFile(fp))
        {
            return E_FAIL;
        }

        stageModelSize = ftell(fp) - stageModelStart;
        fseek(fp, stageModelStart, SEEK_SET);
        fwrite(&stageModelSize, sizeof(int), 1, fp);
        fseek(fp, 0, SEEK_END);
    }

    modelLength = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fwrite(&cc, sizeof(DWORD), 1, fp);
    fwrite(&modelLength, sizeof(int), 1, fp);

    fclose(fp);
    return S_OK;
}

HRESULT LBFAlignment::initialize(
    __in_bcount(bufferSize) const BYTE* pModelBuffer,
    __in size_t bufferSize)
{
    //printf("enter initialize\n");
    BEGIN_READ_FILE;

    HRESULT hr = S_OK;

    const BYTE* pBufferToRead = pModelBuffer;
    size_t sizeToRead = bufferSize;

    //printf("pBufferToRead is 0x%lX\n", reinterpret_cast<uintptr_t>(pBufferToRead));
    //printf("sizeToRead is %lD\n", sizeToRead);

    int pointCount;
    int stageCount;

    //printf("A\n");

    BUFFER_READ_SINGLE_AND_CHECK(pointCount, pBufferToRead, sizeToRead);

    //printf("B\n");

    BUFFER_READ_SINGLE_AND_CHECK(stageCount, pBufferToRead, sizeToRead);

    //printf("C\n");

    //printf("pointCount is %d, stageCount is %d\n", pointCount, stageCount);

    m_mean_shape.resize(pointCount * 2);

    BUFFER_READ_MULTIPLE_AND_CHECK(&m_mean_shape[0], sizeof(float) * 2 * pointCount, sizeof(float), 2 * pointCount, pBufferToRead, sizeToRead);

    m_curr_shape.resize(pointCount * 2);

    m_tran_shape.resize(pointCount * 2 * 3);

    m_std_face_coords[0] = 0;
    m_std_face_coords[1] = 0;
    m_std_face_coords[2] = 1;
    m_std_face_coords[3] = 0;
    m_std_face_coords[4] = 0;
    m_std_face_coords[5] = 1;
    m_std_face_coords[6] = 1;
    m_std_face_coords[7] = 1;

    m_stages.resize(stageCount);

    for (int i = 0; i < stageCount; i++)
    {
        //printf("Stage %d is about to initialize\n", i);
        CHR(m_stages[i].initialize(pBufferToRead, sizeToRead));
        //printf("Stage %d is initialized\n", i);
    }

    if (sizeToRead >= sizeof(int))
    {
        int numOfClassifiers;
        BUFFER_READ_SINGLE_AND_CHECK(numOfClassifiers, pBufferToRead, sizeToRead);

        //printf("numOfClassifiers = %d\n", numOfClassifiers);
        // the model includes classifier for tracking
        CBREx(numOfClassifiers <= 1, E_UNEXPECTED);  // ensure only 1 classifier at most

        if (1 == numOfClassifiers)
        {
            hr = m_classifier.initialize(pBufferToRead, sizeToRead, &m_stages[stageCount - 1]);
            //printf("m_classifier.initialize hr = %lX\n", hr);
            CHR(hr);
        }
    }

Error:
    return hr;
}