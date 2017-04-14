#pragma once

#include <Windows.h>


#include <vector>
#include <stdio.h>

namespace facesdk { namespace alignmentor {

    class RegressorT
    {
    public:
        RegressorT() : m_iDataX(-1), m_iDataY(-1) {}

        HRESULT Predict(__inout_ecount(iShapeSize) float *pfShapeUpdate, 
            __in int iShapeSize,
            __in_ecount(iTotalFeatureLength) const int *piFeatureIds, 
            __in int iTotalFeatureLength);							

        HRESULT initialize(const BYTE*& buffer, size_t& bufferSize);

        HRESULT saveToFile(FILE *fp);
    private:
        std::vector<float> m_regressorData;
        int m_iDataY;
        int m_iDataX;
    };
} }