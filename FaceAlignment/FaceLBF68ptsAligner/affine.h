//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Microsoft Face SDK is developed by the Innovation Engineering Group (IEG) 
// of MSRA to help research and development of novel face related applications. 
// It integrates the latest face technologies from Microsoft research teams 
// (MSR Asia, MSR Redmond, MSR Fuse Lab, Israel Innovation Labs, Siena, etc.) 
// and many other individuals.
// 
// The source code and binaries are strictly Microsoft Confidential, 
// and Microsoft owns the full IP and patents.
// Please contact the team prior to external disclosure in any form.
//
// THE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
// Contributors: 
//                 Jian Sun (MSR ASIA) <jiansun@microsoft.com>
//                 Fang Wen <fangwen@microsoft.com>
//                 Qiufeng Yin <qfyin@microsoft.com>
//                 Xin Zou <xinz@microsoft.com>
//                 Fan Yang (MSRA-TTG) <fyang@microsoft.com>                           
//                 Ning Xu (MSRA) <ningx@microsoft.com>
//                 Chao Wang <chaowa@microsoft.com>
//
// Homepage:
//                 http://toolbox/facesdk
//                 http://toolbox/facesdknative
//
// Contacts:
//                 Face SDK Team facesdk@microsoft.com
//


#pragma once

//#include <WinDef.h>
#include <math.h>
#include <Windows.h>
#include <sal.h>



namespace facesdk { namespace alignmentor
{
    struct affine
    {
        static const int MatrixSize = 9;

        // Affine coefficients
        float coef[MatrixSize];

        static inline void inverse(
            __in_ecount(MatrixSize) const float* mi, 
            __out_ecount(MatrixSize) float* mo
            ) throw()
        {
            float f1 = mi[4] * mi[8] - mi[5] * mi[7];
            float f2 = mi[5] * mi[6] - mi[3] * mi[8];
            float f3 = mi[3] * mi[7] - mi[4] * mi[6];
            float del = mi[0] * f1 + mi[1] * f2 + mi[2] * f3;

            if(del > std::numeric_limits<float>::epsilon())
            {
                float inv_del = 1.0f / del;
                mo[0] = f1 * inv_del;
                mo[1] = (mi[7] * mi[2] - mi[1] * mi[8]) * inv_del;
                mo[2] = (mi[1] * mi[5] - mi[4] * mi[2]) * inv_del;
                mo[3] = f2 * inv_del;
                mo[4] = (mi[0] * mi[8] - mi[6] * mi[2]) * inv_del;
                mo[5] = (mi[2] * mi[3] - mi[0] * mi[5]) * inv_del;
                mo[6] = f3 * inv_del;
                mo[7] = (mi[6] * mi[1] - mi[0] * mi[7]) * inv_del;
                mo[8] = (mi[0] * mi[4] - mi[3] * mi[1]) * inv_del;
            }
            else
            {
                memset(mo, 0, sizeof(float)*MatrixSize);
            }
        }

        static inline void multiply(
            __in_ecount(MatrixSize) const float* m1, 
            __in_ecount(MatrixSize) const float* m2, 
            __out_ecount(MatrixSize) float* mo
            ) throw()
        {
            mo[0] = m1[0] * m2[0] + m1[1] * m2[3] + m1[2] * m2[6];
            mo[1] = m1[0] * m2[1] + m1[1] * m2[4] + m1[2] * m2[7];
            mo[2] = m1[0] * m2[2] + m1[1] * m2[5] + m1[2] * m2[8];

            mo[3] = m1[3] * m2[0] + m1[4] * m2[3] + m1[5] * m2[6];
            mo[4] = m1[3] * m2[1] + m1[4] * m2[4] + m1[5] * m2[7];
            mo[5] = m1[3] * m2[2] + m1[4] * m2[5] + m1[5] * m2[8];

            mo[6] = m1[6] * m2[0] + m1[7] * m2[3] + m1[8] * m2[6];
            mo[7] = m1[6] * m2[1] + m1[7] * m2[4] + m1[8] * m2[7];
            mo[8] = m1[6] * m2[2] + m1[7] * m2[5] + m1[8] * m2[8];
        }

        inline void compute_affine_transform(
            __in_ecount(length) const float *pSource, 
            __in_ecount(length) const float *pDestination, 
            __in size_t length) throw()
        {
            float w1[9] = {0};
            float w2[9] = {0};
            float w3[9];

            size_t count = length / 2;
            for (size_t i = 0, id = 0; i < count; i++, id += 2)
            {
                w1[0] += pDestination[id] * pSource[id];
                w1[1] += pDestination[id] * pSource[id+1];
                w1[2] += pDestination[id];
                w1[3] += pDestination[id+1] * pSource[id];
                w1[4] += pDestination[id+1] * pSource[id+1];
                w1[5] += pDestination[id+1];
                w1[6] += pSource[id];
                w1[7] += pSource[id+1];

                w2[0] += pSource[id] * pSource[id];
                w2[1] += pSource[id] * pSource[id+1];
                w2[2] += pSource[id];
                w2[4] += pSource[id+1] * pSource[id+1];
                w2[5] += pSource[id+1];
            }

            w1[8] = (float)count;
            w2[3] = w2[1];
            w2[6] = w2[2];
            w2[7] = w2[5];
            w2[8] = (float)count;

            inverse(w2, w3);
            multiply(w1, w3, coef);
        }

        inline void compute_similar_transfrom(
            __in_ecount(length) const float *pSource, 
            __in_ecount(length) const float *pDestination, 
            __in size_t length) throw()
        {
            size_t count = length / 2;
            float a1 = 0, a3 = 0, a4 = 0;
            float c1 = 0, c2 = 0, c3 = 0, c4 = 0;
            for (size_t i = 0, id = 0; i < count; i++, id += 2)
            {
                float x = pSource[id];
                float y = pSource[id+1];
                float xn = pDestination[id];
                float yn = pDestination[id+1];
                a1 += x*x + y*y;
                a3 += x;
                a4 += y;
                c1 += x*xn + y*yn;
                c2 += y*xn - x*yn;
                c3 += xn;
                c4 += yn;
            }

            float invCoef = 1.0f / count;
            a1 *= invCoef;
            a3 *= invCoef;
            a4 *= invCoef;
            c1 *= invCoef;
            c2 *= invCoef;
            c3 *= invCoef;
            c4 *= invCoef;

            float t = a1 - a3*a3 - a4*a4;
            float b1 = 1.0f / t;
            float b2 = a1 * b1;
            float b3 = -a3 * b1;
            float b4 = -a4 * b1;

            float a = b1*c1 + b3*c3 + b4*c4;
            float b = b1*c2 + b4*c3 - b3*c4;
            float dx = b3*c1 + b4*c2 + b2*c3;
            float dy = b4*c1 - b3*c2 + b2*c4;

            coef[0] = a;
            coef[1] = b;
            coef[2] = dx;
            coef[3] = -b;
            coef[4] = a;
            coef[5] = dy;
            coef[6] = 0;
            coef[7] = 0;
            coef[8] = 1;
        }

        inline void compute_L1_norm(
            __inout_ecount(length) float *pShape, 
            __in size_t length, 
            __out float& fMeanX, 
            __out float& fMeanY) throw()
        {
            fMeanX = 0.f;
            fMeanY = 0.f;
            for (size_t i = 0; i < length; i += 2)
            {
                fMeanX += pShape[i];
                fMeanY += pShape[i + 1];
            }
            fMeanX /= length / 2;
            fMeanY /= length / 2;
            for (size_t i = 0; i < length; i += 2)
            {
                pShape[i] -= fMeanX;
                pShape[i + 1] -= fMeanY;
            }
        }

        inline void compute_diff_similarity_transform(
            __in_ecount(length) const float *pSource,
            __in_ecount(length) const float *pDestination,
            __in_ecount(length) float *pBuffer,
            __in size_t length) throw()
        {
            float *pfShapeOriTemp = pBuffer;
            float *pfShapeNewTemp = pBuffer + length;
            float *pfShapeOriRotateTemp = pBuffer + length * 2;

            float fOriMeanX, fOriMeanY;
            memcpy(pfShapeOriTemp, pSource, length * sizeof(float));
            compute_L1_norm(pfShapeOriTemp, length, fOriMeanX, fOriMeanY);

            float fNewMeanX, fNewMeanY;
            memcpy(pfShapeNewTemp, pDestination, length*sizeof(float));
            compute_L1_norm(pfShapeNewTemp, length, fNewMeanX, fNewMeanY);

            float fShapeOriVarX(0.0), fShapeOriVarY(0.0), fShapeNewVarX(0.0), fShapeNewVarY(0.0);
            {
                for (size_t i = 0; i < length; i += 2)
                {
                    float x = pfShapeOriTemp[i];
                    float y = pfShapeOriTemp[i + 1];
                    float x_ = pfShapeNewTemp[i];
                    float y_ = pfShapeNewTemp[i + 1];
                    fShapeOriVarX += x*x;
                    fShapeOriVarY += y*y;
                    fShapeNewVarX += x_*x_;
                    fShapeNewVarY += y_*y_;
                }
                fShapeOriVarX = sqrt(fShapeOriVarX);
                fShapeOriVarY = sqrt(fShapeOriVarY);
                fShapeNewVarX = sqrt(fShapeNewVarX);
                fShapeNewVarY = sqrt(fShapeNewVarY);
            }

            float fScaleX = fShapeOriVarX / fShapeNewVarX;
            float fScaleY = fShapeOriVarY / fShapeNewVarY;

            for (size_t i = 0; i < length; i += 2)
            {
                pfShapeNewTemp[i] *= fScaleX;
                pfShapeNewTemp[i + 1] *= fScaleY;
            }

            compute_similar_transfrom(pfShapeOriTemp, pfShapeNewTemp, length);

            float scale = float(1.0 / sqrt(coef[0] * coef[0] + coef[1] * coef[1]));
            coef[0] *= scale; coef[1] *= scale; coef[2] = 0;
            coef[3] *= scale; coef[4] *= scale; coef[5] = 0;

            apply(pfShapeOriTemp, pfShapeOriRotateTemp, length);

            float fShapeOriRotateVarX(0.0), fShapeOriRotateVarY(0.0);
            {
                for (size_t i = 0; i < length; i += 2)
                {
                    float x = pfShapeOriRotateTemp[i];
                    float y = pfShapeOriRotateTemp[i + 1];
                    fShapeOriRotateVarX += x*x;
                    fShapeOriRotateVarY += y*y;
                }
                fShapeOriRotateVarX = sqrt(fShapeOriRotateVarX);
                fShapeOriRotateVarY = sqrt(fShapeOriRotateVarY);
            }

            fScaleX = fShapeNewVarX / fShapeOriRotateVarX;
            fScaleY = fShapeNewVarY / fShapeOriRotateVarY;

            coef[0] *= fScaleX;
            coef[1] *= fScaleX;
            coef[2] = -(fOriMeanX*coef[0] + fOriMeanY*coef[1]) + fNewMeanX;
            coef[3] *= fScaleY;
            coef[4] *= fScaleY;
            coef[5] = -(fOriMeanX*coef[3] + fOriMeanY*coef[4]) + fNewMeanY;
            coef[6] = 0;
            coef[7] = 0;
        }

        inline void apply(
            __in_ecount(length) const float *pSource, 
            __in_ecount(length) float *pDestination, 
            __in size_t length) throw()
        {
            size_t count = length / 2;
            for (size_t i = 0, id = 0; i < count; i++, id += 2)
            {
                float v1 = pSource[id];
                float v2 = pSource[id + 1];

                *pDestination++ = coef[0] * v1 + coef[1] * v2 + coef[2];
                *pDestination++ = coef[3] * v1 + coef[4] * v2 + coef[5];
            }
        }
    };
} }