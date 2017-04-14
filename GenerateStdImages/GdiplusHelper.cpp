// 
// Copyright (c) Microsoft Corporation. All rights reserved. 
// 
// File: GdiplusHelper.cpp
// 
// Description: 
//     Helper class to initialize Gdiplus environment.
// 
#include <windows.h>
#include <unknwn.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

namespace helper {

    class GdiplusHelper
    {
    private:
        /// private ctor, this class cannot be ctor by others
        /// only ctor and dtor are used for function closure.
        GdiplusHelper()
        {
            gdiSI.SuppressBackgroundThread = TRUE;
            Gdiplus::GdiplusStartup(&gdiToken,&gdiSI,&gdiSO);
            gdiSO.NotificationHook(&gdiHookToken);
        }

        ~GdiplusHelper()
        {
            gdiSO.NotificationUnhook(gdiHookToken);
            Gdiplus::GdiplusShutdown(gdiToken);
        }

        Gdiplus::GdiplusStartupInput gdiSI;
        Gdiplus::GdiplusStartupOutput gdiSO;
        ULONG_PTR gdiToken;
        ULONG_PTR gdiHookToken;

        static GdiplusHelper theGdiPlus;
    };

#ifndef DISABLE_GDIPLUS_HELPER
    GdiplusHelper GdiplusHelper::theGdiPlus;
#endif

}   /// namespace helper
