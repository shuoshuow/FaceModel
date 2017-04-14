#include "BitmapConvertor.h"

HRESULT CBitmapConvertor::ConvertBitmapToGray(__out BYTE *pLum, Bitmap *pBmp)
{
	BitmapData bmpData;
	if (pBmp->LockBits(NULL, ImageLockModeRead, PixelFormat24bppRGB, &bmpData) != Ok)
		return E_FAIL;
	BYTE *pImageLine = (BYTE*)bmpData.Scan0;
	for (UINT i = 0; i < bmpData.Height; i++)
	{
		BYTE *pImage = pImageLine;
		for (UINT j = 0; j < bmpData.Width; j++)
		{
			BYTE tLuma = BYTE(0.299 * pImage[2] + 0.587 * pImage[1] + 0.114 * pImage[0]);
			*pLum = tLuma;
			pLum++;
			pImage += 3;
		}
		pImageLine += bmpData.Stride;
	}
	pBmp->UnlockBits(&bmpData);
	return S_OK;
}


bool CBitmapConvertor::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num, size;
	Gdiplus::GetImageEncodersSize(&num, &size);
	Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);
	bool found = false;
	for (UINT ix = 0; !found && ix < num; ++ix) 
	{
		if (wcscmp(pImageCodecInfo[ix].MimeType, format) == 0) 
		{
			*pClsid = pImageCodecInfo[ix].Clsid;
			found = true;
		}
	}
	free(pImageCodecInfo);
	return found;
}


HRESULT CBitmapConvertor::SaveBitmapToFile(const wchar_t *pFile, Bitmap *pBmp, FormatType ftType)
{
	CLSID encoder;
	GetEncoderClsid(ftType, &encoder);
	pBmp->Save(pFile, &encoder, NULL);
	return S_OK;
}

HRESULT CBitmapConvertor::SaveGrayImgToFile(const wchar_t *pFile, unsigned char *img, int w, int h, FormatType ftType)
{
    CLSID encoder;
    GetEncoderClsid(ftType, &encoder);

    int stride = ((w * 3 + 3) / 4) * 4;

    unsigned char *pColorPixels = new unsigned char[h * stride];
    
    unsigned char *pColorRow = pColorPixels;
    unsigned char *pGrayRow = img;
    for (int y = 0; y < h; y++, pColorRow += stride, pGrayRow += w)
    {
        for (int x = 0; x < w; x++)
        {
            pColorRow[x * 3] = pColorRow[x * 3 + 1] = pColorRow[x * 3 + 2] = pGrayRow[x];
        }
    }
    Bitmap bmp(w, h, stride, PixelFormat24bppRGB, pColorPixels);
    bmp.Save(pFile, &encoder, NULL);
    delete[] pColorPixels;

    return S_OK;
}
