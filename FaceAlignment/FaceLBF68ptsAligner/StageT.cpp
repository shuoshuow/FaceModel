#include "StageT.h"
#include "ehm.h"
#include "iodefs.h"
#include "affine.h"
#include <string.h>

using namespace facesdk::alignmentor;

HRESULT StageT::Predict(__inout float *pShape, __in int length, const float *pMeanShape,
    __in const BYTE *grayImage, int width, int height, int stride)
{
    if (length != m_iPointNum * 2)
    {
        return E_INVALIDARG;
    }

    affine coeffInv;
    coeffInv.compute_diff_similarity_transform(pMeanShape, pShape, &m_shapeForTransform[0], static_cast<size_t>(length));

    const int treeNum = (int) m_trees.size();

    for (int i = 0; i < treeNum; i++)
    {
        int iPointIdx = i / m_iPerPointTreeNum;
        m_trees[i].Predict(m_predIdx[i], grayImage, width, height, stride, pShape[iPointIdx * 2], pShape[iPointIdx * 2 + 1], coeffInv);
    }

    m_regressor.Predict(&m_shapeUpdate[0], length, &m_predIdx[0], treeNum);

    for (int i = 0; i < length / 2; ++i)
    {
        pShape[i * 2] += m_shapeUpdate[i * 2] * coeffInv.coef[0] + m_shapeUpdate[i * 2 + 1] * coeffInv.coef[1];
        pShape[i * 2 + 1] += m_shapeUpdate[i * 2] * coeffInv.coef[3] + m_shapeUpdate[i * 2 + 1] * coeffInv.coef[4];
    }

    return S_OK;
}

HRESULT StageT::Predict(__out const int *&piPredIds, int &iPredNum,
    __in const float *pMeanShape, const float *pShape, int length,
    __in const BYTE *grayImage, unsigned int width, unsigned int height, unsigned int stride)
{
    if (length != m_iPointNum * 2)
    {
        return E_INVALIDARG;
    }

    affine coeffInv;
    coeffInv.compute_diff_similarity_transform(pMeanShape, pShape, &m_shapeForTransform[0], static_cast<size_t>(length));

    const int treeNum = (int) m_trees.size();

    for (int i = 0; i < treeNum; i++)
    {
        int iPointIdx = i / m_iPerPointTreeNum;
        m_trees[i].Predict(m_predIdx[i], grayImage, width, height, stride, pShape[iPointIdx * 2], pShape[iPointIdx * 2 + 1], coeffInv);
    }

    piPredIds = &m_predIdx[0];
    iPredNum = treeNum;

    return S_OK;
}

HRESULT StageT::initialize(const BYTE*& buffer,
    size_t& bufferSize)
{
    BEGIN_READ_FILE;

    HRESULT hr = S_OK;

    int treeNum = 0;

    BUFFER_READ_SINGLE_AND_CHECK(treeNum, buffer, bufferSize);
    BUFFER_READ_SINGLE_AND_CHECK(m_iPerPointTreeNum, buffer, bufferSize);

    m_iPointNum = treeNum / m_iPerPointTreeNum;

    m_predIdx.resize(treeNum);

    m_shapeUpdate.resize(m_iPointNum * 2);

    m_shapeForTransform.resize(m_iPointNum * 2 * 3);

    m_trees.resize(treeNum);

    //printf("treeNum = %d, m_iPointNum = %d\n", treeNum, m_iPointNum);

    for (int i = 0; i < treeNum; i++)
    {
        //printf("m_trees[%d] is about to initialize\n", i);
        CHR(m_trees[i].initialize(buffer, bufferSize));
        //printf("m_trees[%d] is initialized\n", i);
    }

    CHR(m_regressor.initialize(buffer, bufferSize));

Error:
    return hr;
}

HRESULT StageT::saveToFile(FILE *fp)
{
    m_regressor.saveToFile(fp);

    short count = 0;
    fwrite(&count, sizeof(short), 1, fp);
    for (int i = 0; i < m_trees.size(); ++i)
    {
        count += m_trees[i].GetNodeCount();
        fwrite(&count, sizeof(short), 1, fp);
    }

    for (int i = 0; i < m_trees.size(); ++i)
    {
        if (S_OK != m_trees[i].saveToFile(fp))
        {
            return E_FAIL;
        }
    }
    return S_OK;
}