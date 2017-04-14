#pragma once

#include <Windows.h>

#include "RegressorT.h"
#include "StageT.h"

namespace facesdk { namespace alignmentor {

    class Classifier
    {
    public:
        Classifier():m_pStage(nullptr), m_initialized(false)
        {}

        HRESULT Predict(const float *pShape, int length, const float *pMeanShape, const BYTE *grayImage, int width, int height, int stride, float& score);

        HRESULT initialize(const BYTE*& buffer, size_t& bufferSize, StageT* pStage);

        bool is_initialized() const throw()
        {
            return m_initialized;
        }

    private:
        StageT *m_pStage;
        RegressorT m_regressor;
        bool m_initialized;
    };
} }