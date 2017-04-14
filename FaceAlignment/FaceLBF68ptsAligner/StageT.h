#pragma once

#include <Windows.h>

#include <vector>
#include "Tree.h"
#include "RegressorT.h"
#include <stdio.h>

namespace facesdk { namespace alignmentor {

    class StageT
    {
    public:
        StageT() : m_iPointNum(0), m_iPerPointTreeNum(0) {}

        HRESULT Predict(__inout float *pShape, __in int length, const float *pMeanShape,
            __in const BYTE *grayImage, int width, int height, int stride);

        HRESULT Predict(__out const int *&piPredIds, int &iPredNum, 
            __in const float *pMeanShape, const float *pShape, int length, 
            __in const BYTE *grayImage, unsigned int width, unsigned int height, unsigned int stride);

        HRESULT initialize(const BYTE*& buffer, size_t& bufferSize);

        HRESULT saveToFile(FILE *fp);

        size_t GetStageTreeCount() const { return m_trees.size(); };

    private:
        int m_iPointNum;
        int m_iPerPointTreeNum;

        std::vector<Tree> m_trees;
        std::vector<int> m_predIdx; 
        std::vector<float> m_shapeUpdate;
        std::vector<float> m_shapeForTransform;

        RegressorT m_regressor;
    };
} }