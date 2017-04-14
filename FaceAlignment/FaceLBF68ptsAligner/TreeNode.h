#pragma once

#include <Windows.h>


namespace facesdk { namespace alignmentor {

    class TreeNode
    {
    public:
        TreeNode() : fx1(0.f), fy1(0.f), fx2(0.f), fy2(0.f), fThres(0.f), iLeft(-1), iRight(-1), bLeftNodeLeaf(true), bRightNodeLeaf(true){}

        HRESULT initialize(const BYTE*& buffer, size_t& bufferSize);

        float fx1, fy1, fx2, fy2;
        float fThres;
        int iLeft, iRight;	
        bool bLeftNodeLeaf, bRightNodeLeaf;
    };
} } 