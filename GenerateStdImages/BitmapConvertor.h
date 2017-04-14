#pragma once

#include <Windows.h>
#include <GdiPlus.h>

using namespace Gdiplus;

#ifndef FormatType
typedef const WCHAR* FormatType;
#define FT_BMP L"image/bmp"
#define FT_JPG L"image/jpeg"
#define FT_GIF L"image/gif"
#define FT_TIFF L"image/TIFF"
#define FT_PNG L"image/png"
#endif


class CBitmapConvertor
{
public:
	static HRESULT ConvertBitmapToGray(__out BYTE *pLum, Bitmap *pBmp);
	static HRESULT SaveBitmapToFile(const wchar_t *pFile, Bitmap *pBmp, FormatType ftType);
    static HRESULT SaveGrayImgToFile(const wchar_t *pFile, unsigned char *img, int w, int h, FormatType ftType);
private:
	static bool GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
};
