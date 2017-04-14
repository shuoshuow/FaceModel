#include "Tree.h"
#include "ehm.h"
#include "iodefs.h"
#include <float.h>
#include <stdio.h>

using namespace facesdk::alignmentor;

void Tree::ExtractFeature(__out float &feature, __in const BYTE *const grayImage, unsigned int width, unsigned int height, unsigned int stride, float fCurrentX, float fCurrentY, const affine& transform_inv, float sx1, float sy1, float sx2, float sy2) const
{
    unsigned int x1 = static_cast<unsigned int>(fCurrentX + transform_inv.coef[0] * sx1 + transform_inv.coef[1] * sy1 + 0.5);
    unsigned int y1 = static_cast<unsigned int>(fCurrentY + transform_inv.coef[3] * sx1 + transform_inv.coef[4] * sy1 + 0.5);
    unsigned int x2 = static_cast<unsigned int>(fCurrentX + transform_inv.coef[0] * sx2 + transform_inv.coef[1] * sy2 + 0.5);
    unsigned int y2 = static_cast<unsigned int>(fCurrentY + transform_inv.coef[3] * sx2 + transform_inv.coef[4] * sy2 + 0.5);

    // check whether out of boundary
    bool inside1 = x1 < width && y1 < height;
    bool inside2 = x2 < width && y2 < height;

    float p1, p2;
    p1 = inside1 ? (float) (grayImage[y1*stride + x1]) : 0.f;
    p2 = inside2 ? (float) (grayImage[y2*stride + x2]) : 0.f;

    feature = (p1 - p2) / (p1 + p2 + FLT_MIN);
}


void Tree::Predict(__out int &iPredIdx, __in const BYTE *const grayImage, unsigned int width, unsigned int height, unsigned int stride, float fCurrentX, float fCurrentY, const affine& transform_inv) const
{
    int iNodeIdx = 0;
    bool bReachLeaf = false, bLeft;
    float feature;
    while (!bReachLeaf)
    {
        const TreeNode& treeNode = m_treeNodes[iNodeIdx];
        ExtractFeature(feature, grayImage, width, height, stride, fCurrentX, fCurrentY, transform_inv, treeNode.fx1, treeNode.fy1, treeNode.fx2, treeNode.fy2);
        bLeft = feature <= treeNode.fThres;
        bReachLeaf = bLeft ? treeNode.bLeftNodeLeaf : treeNode.bRightNodeLeaf;
        iNodeIdx = bLeft ? treeNode.iLeft : treeNode.iRight;
    }
    iPredIdx = iNodeIdx;
}

HRESULT Tree::initialize(const BYTE*& buffer, size_t& bufferSize)
{
    BEGIN_READ_FILE;

    HRESULT hr = S_OK;

    int nodeNum;
    BUFFER_READ_SINGLE_AND_CHECK(nodeNum, buffer, bufferSize);

    //printf("nodeNum = %d\n", nodeNum);

    CBREx(nodeNum >= 0, E_UNEXPECTED);

    m_treeNodes.resize(nodeNum);

    for (int i = 0; i < nodeNum; ++i)
    {
        //printf("m_treeNodes[%d] is about to initialize\n", i);
        CHR(m_treeNodes[i].initialize(buffer, bufferSize));
        //printf("m_treeNodes[%d] is initialized\n", i);

    }

Error:
    return hr;
}

HRESULT Tree::saveToFile(FILE* fp)
{
    for (int i = 0; i < m_treeNodes.size(); ++i)
    {
        TreeNode& node = m_treeNodes[i];

        short idx = 0;

        idx = node.bLeftNodeLeaf ? -1 : node.iLeft;
        fwrite(&idx, sizeof(short), 1, fp);
        idx = node.bRightNodeLeaf ? -1 : node.iRight;
        fwrite(&idx, sizeof(short), 1, fp);

        idx = node.bLeftNodeLeaf ? node.iLeft : -1;
        fwrite(&idx, sizeof(short), 1, fp);
        idx = node.bRightNodeLeaf ? node.iRight : -1;
        fwrite(&idx, sizeof(short), 1, fp);

        fwrite(&node.fx1, sizeof(float), 1, fp);
        fwrite(&node.fy1, sizeof(float), 1, fp);
        fwrite(&node.fx2, sizeof(float), 1, fp);
        fwrite(&node.fy2, sizeof(float), 1, fp);

        fwrite(&node.fThres, sizeof(float), 1, fp);
    }
    return S_OK;
}