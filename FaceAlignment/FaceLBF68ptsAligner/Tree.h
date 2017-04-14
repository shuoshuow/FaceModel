#pragma once

#include <Windows.h>

#include <vector>
#include "affine.h"
#include "TreeNode.h"

namespace facesdk { namespace alignmentor {

    class Tree
    {
    public:      
        void Predict(__out int &iPredIdx, 
            __in const BYTE *const grayImage, 
            unsigned int width, 
            unsigned int height, 
            unsigned int stride, 
            float x, 
            float y, 
            const affine& transform_inv) const;

        HRESULT initialize(const BYTE*& buffer, size_t& bufferSize);

        HRESULT saveToFile(FILE* fp);

        size_t GetNodeCount() const { return m_treeNodes.size(); }

    private:
        void ExtractFeature(__out float &feature, 
            __in const BYTE *const grayImage, 
            unsigned int width, 
            unsigned int height, 
            unsigned int stride, 
            float x, 
            float y, 
            const affine&  transform_inv,
            float sx1, 
            float sy1, 
            float sx2, 
            float sy2) const;

        std::vector<TreeNode> m_treeNodes;
    };

} }
