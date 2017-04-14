#pragma once

#include <stdio.h>
#include <vector>
#include "Classifier.h"

using namespace std;

namespace facesdk { namespace alignmentor {

    class LBFAlignment
    {
    public:

        HRESULT initialize(
            __in_bcount(length) const unsigned char* buffer,
            __in size_t length
            );

        HRESULT saveToFile(__in const char* filename);

        HRESULT predict(
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
            __in BOOL useMeanShape = true
            );

        // tracking
        HRESULT predict(
            __in_bcount(stride * height) const unsigned char* image,
            __in size_t width,
            __in size_t height,
            __in size_t stride,
            __inout_ecount(dimension) float* shape,
            __in size_t dimension,
            __out ::RECT& faceRect,
            __out float& score
            );

        size_t get_point_count() const
        {
            return m_mean_shape.size() / 2;
        }

    private:

        HRESULT predict_impl(
            __in_bcount(stride * height) const unsigned char* image,
            __in size_t width,
            __in size_t height,
            __in size_t stride,
            __inout_ecount(dimension) float* shape,
            __in size_t dimension,
            __out float& score
            );

    private:

        enum { FACE_POINT_COUNT = 4 };

        float m_face_coords[FACE_POINT_COUNT << 1];
        float m_std_face_coords[FACE_POINT_COUNT << 1];

        std::vector<float> m_mean_shape;
        std::vector<float> m_curr_shape;
        std::vector<float> m_tran_shape;

        std::vector<StageT> m_stages;
        Classifier m_classifier;
    };
} }
