#include "TreeNode.h"
#include "ehm.h"
#include "iodefs.h"
#include <string.h>
#include <stdio.h>

using namespace facesdk::alignmentor;

HRESULT TreeNode::initialize(const BYTE*& buffer, size_t& bufferSize)
{
    HRESULT hr = S_OK;

    BEGIN_READ_FILE;

    BUFFER_READ_SINGLE_AND_CHECK(fx1, buffer, bufferSize);
    BUFFER_READ_SINGLE_AND_CHECK(fy1, buffer, bufferSize);
    BUFFER_READ_SINGLE_AND_CHECK(fx2, buffer, bufferSize);
    BUFFER_READ_SINGLE_AND_CHECK(fy2, buffer, bufferSize);
    BUFFER_READ_SINGLE_AND_CHECK(fThres, buffer, bufferSize);
    BUFFER_READ_SINGLE_AND_CHECK(iLeft, buffer, bufferSize);
    BUFFER_READ_SINGLE_AND_CHECK(iRight, buffer, bufferSize);
    BUFFER_READ_SINGLE_AND_CHECK(bLeftNodeLeaf, buffer, bufferSize);
    BUFFER_READ_SINGLE_AND_CHECK(bRightNodeLeaf, buffer, bufferSize);
Error:
    return hr;
}