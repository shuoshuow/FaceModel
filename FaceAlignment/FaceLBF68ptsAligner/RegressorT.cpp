#include "RegressorT.h"
#include "ehm.h"
#include "iodefs.h"

using namespace facesdk::alignmentor;

HRESULT RegressorT::Predict(
    __inout_ecount(iShapeSize) float *pfShapeUpdate,
    __in int iShapeSize,
    __in_ecount(iTotalFeatureLength) const int *piFeatureIds,
    __in int iTotalFeatureLength)
{   
    if (iShapeSize != m_iDataX)
    {
        return E_INVALIDARG;
    }

    memset(pfShapeUpdate, 0, sizeof(float) * iShapeSize);

    for (int i = 0; i < iTotalFeatureLength; i++)
    {
        assert(piFeatureIds[i] >= 0 && piFeatureIds[i] < m_iDataY - 1);
        const float *pR = &m_regressorData[m_iDataX*piFeatureIds[i]];
        float *pDst = pfShapeUpdate;
        for (int j = 0; j < iShapeSize; j++)
        {
            pDst[j] += pR[j];
        }
    }

    float *pDst = pfShapeUpdate;
    const float *pR = &m_regressorData[m_iDataX*(m_iDataY - 1)];
    for (int j = 0; j < iShapeSize; j++)
    {
        pDst[j] += pR[j];
    }

    return S_OK;
}

HRESULT RegressorT::initialize(const BYTE*& buffer, size_t& bufferSize)
{
    HRESULT hr = S_OK;

    BEGIN_READ_FILE;

    BUFFER_READ_SINGLE_AND_CHECK(m_iDataY, buffer, bufferSize);
    BUFFER_READ_SINGLE_AND_CHECK(m_iDataX, buffer, bufferSize);

    m_regressorData.resize(m_iDataX * m_iDataY);

    BUFFER_READ_MULTIPLE_AND_CHECK(&m_regressorData[0], sizeof(float) * m_iDataX * m_iDataY, sizeof(float), m_iDataX * m_iDataY, buffer, bufferSize);
Error:
    return hr;
}

HRESULT RegressorT::saveToFile(FILE *fp)
{
    fwrite(&m_iDataY, sizeof(int), 1, fp);
    fwrite(&m_regressorData[0], sizeof(float), m_iDataX * m_iDataY, fp);
    return S_OK;
}