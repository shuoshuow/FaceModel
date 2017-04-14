//+-----------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  Description:
//      Functions to get file list inside a folder.
//
//  History:
//      2009/11/04-fyang
//            Created
//
//------------------------------------------------------------------------

#include "FolderFileHelper.h"

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

bool IfPathExists(WCHAR *wzDir)
{
	if (PathFileExists(wzDir))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ReadFileToBuffer(WCHAR *wzFile, std::vector<BYTE> &vBuffer)
{
	FILE *fp = NULL;
	_wfopen_s(&fp, wzFile, L"rb");
	if (NULL == fp)
	{
		wprintf(L"cannot open file %s for reading.\n", wzFile);
		return false;
	}

	long lSize;
	size_t result;

	fseek(fp, 0, SEEK_END);
	lSize = ftell(fp);
	rewind(fp);

	vBuffer.resize(lSize);
	BYTE *pBuffer = &vBuffer[0];
	result = fread(pBuffer, 1, lSize, fp);
	if (result != lSize)
	{
		wprintf(L"read file error.\n");
		return false;
	}

	fclose(fp);
	return true;
}

HRESULT GetSubFolders(WCHAR *wzDir, std::vector<CAtlStringW> &vSubFolders)
{
      ATLASSERT(NULL != wzDir);
      if (NULL == wzDir) return E_INVALIDARG;         
      if (!PathFileExists(wzDir)) return E_INVALIDARG;

      vSubFolders.clear();

      WIN32_FIND_DATA wfData;
      WCHAR wzFilter[MAX_PATH];
      StringCchPrintfW(wzFilter, MAX_PATH, L"%s\\*", wzDir);
      HANDLE hFind = ::FindFirstFileW(wzFilter, &wfData);
      if (INVALID_HANDLE_VALUE == hFind)
      {
            return E_INVALIDARG;
      }
      else
      {
            BOOL fNext = TRUE;
            while(TRUE == fNext)
            {
                  if (FILE_ATTRIBUTE_DIRECTORY == 
                        (wfData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                  {
                        CAtlStringW strFolderName(wfData.cFileName);
                        if ( 0 != strFolderName.Compare( L"." ) && 0 != strFolderName.Compare( L".." ))
                        {
                              vSubFolders.push_back(strFolderName);
                        }
                  }
                  fNext = ::FindNextFileW(hFind, &wfData);
            }
            ::FindClose(hFind);
      }

      return S_OK;
}

HRESULT GetPictureFilesinDir(WCHAR * wzPicDir, std::vector<CAtlStringW>& vecPicFile)
{
      ATLASSERT(NULL != wzPicDir);
      if (NULL == wzPicDir) return E_INVALIDARG;            
      if (!PathFileExists(wzPicDir)) return E_INVALIDARG;

      vecPicFile.clear();
      WIN32_FIND_DATA wfData;
      WCHAR wzFilter[MAX_PATH];     

      const int nFileType = 5;
      WCHAR wzFileType[nFileType][MAX_PATH] = { L"jpg", L"bmp", L"png", L"gif", L"jpeg" };

      for ( int i = 0; i < nFileType; i++ )
      {  
          StringCchPrintfW(wzFilter, MAX_PATH, L"%s\\*.%s", wzPicDir, wzFileType[i]);
          HANDLE hFind = ::FindFirstFileW(wzFilter, &wfData);
          if (INVALID_HANDLE_VALUE == hFind) //we couldn't find any jpeg files under specified folder
          {
                //there're no jpg files under specified folder.     
          }
          else
          {
                BOOL fNext = TRUE;
                while(TRUE == fNext)
                {
                      if (FILE_ATTRIBUTE_DIRECTORY ==
                            (wfData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                      {
                            fNext = ::FindNextFileW(hFind, &wfData);
                            continue;
                      }

                      CAtlStringW strPicFile(wfData.cFileName);             
                      vecPicFile.push_back(strPicFile);

                      fNext = ::FindNextFileW(hFind, &wfData);
                      
                }
                ::FindClose(hFind);
          }     
      }

      return S_OK;

}

HRESULT GetBinFilesInDir(WCHAR *wzBinDir, std::vector<CAtlStringW>& vecBinFiles)
{
    ATLASSERT(NULL != wzBinDir);
    if (NULL == wzBinDir) return E_INVALIDARG;            
    if (!PathFileExists(wzBinDir)) return E_INVALIDARG;

    vecBinFiles.clear();
    WIN32_FIND_DATA wfData;
    WCHAR wzFilter[MAX_PATH];     

    const int nFileType = 1;
    WCHAR wzFileType[nFileType][MAX_PATH] = { L"bin"};

    for ( int i = 0; i < nFileType; i++ )
    {  
        StringCchPrintfW(wzFilter, MAX_PATH, L"%s\\*.%s", wzBinDir, wzFileType[i]);
        HANDLE hFind = ::FindFirstFileW(wzFilter, &wfData);
        if (INVALID_HANDLE_VALUE == hFind) //we couldn't find any bin files under specified folder
        {
            //there're no bin files under specified folder.     
        }
        else
        {
            BOOL fNext = TRUE;
            while(TRUE == fNext)
            {
                if (FILE_ATTRIBUTE_DIRECTORY ==
                    (wfData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    fNext = ::FindNextFileW(hFind, &wfData);
                    continue;
                }

                CAtlStringW strBinFile(wfData.cFileName);             
                vecBinFiles.push_back(strBinFile);

                fNext = ::FindNextFileW(hFind, &wfData);

            }
            ::FindClose(hFind);
        }     
    }

    return S_OK;

}


HRESULT GetAllLeafSubFolders_FullPath(WCHAR *wzDir, std::vector<CAtlStringW> &vSubFoldersLeaf)
{
    vSubFoldersLeaf.clear();

    //recursively collect all subforders
    std::vector<CAtlStringW> vSubFolders;
    GetSubFolders( wzDir, vSubFolders );    
    
    while ( vSubFolders.size() > 0 )
    {
        std::vector<CAtlStringW> vSubFoldersTmp;

        for ( size_t i = 0; i < vSubFolders.size(); i++ )
        {
            WCHAR wzSubDirPath[MAX_PATH];
            StringCchPrintfW( wzSubDirPath, MAX_PATH, L"%s\\%s", wzDir, vSubFolders[i] );
            
            std::vector<CAtlStringW> vSubFoldersBottom;
            GetSubFolders( wzSubDirPath, vSubFoldersBottom );   

            if ( vSubFoldersBottom.size() == 0 )
            {
                vSubFoldersLeaf.push_back( wzSubDirPath );
                wprintf( L"%s\n", wzSubDirPath );
            }
            else
            {
                for ( size_t j = 0; j < vSubFoldersBottom.size(); j++ )
                {
                    CAtlStringW strTmp;
                    strTmp.Format( L"%s\\%s", vSubFolders[i], vSubFoldersBottom[j] );
                    vSubFoldersTmp.push_back( strTmp );
                }
            }
        }

        vSubFolders.swap( vSubFoldersTmp );
    }

    return S_OK;
}

//recursively collect all subforders
HRESULT GetAllPicFilesInSubFolders( 
    __in WCHAR *wzDir, 
    __out std::vector<CAtlStringW> &vAllFiles, 
    __in bool bTestReadable)
{
    vAllFiles.clear();

    std::vector<CAtlStringW> vSubFoldersAllLevel;

    vSubFoldersAllLevel.push_back( L"" );   //includ ./
    wprintf( L".\n" );

    //recursively collect all subforders
    std::vector<CAtlStringW> vSubFolders;
    GetSubFolders( wzDir, vSubFolders );    
    
    while ( vSubFolders.size() > 0 )
    {
        std::vector<CAtlStringW> vSubFoldersTmp;

        for ( size_t i = 0; i < vSubFolders.size(); i++ )
        {
            vSubFoldersAllLevel.push_back( vSubFolders[i] );
            
            WCHAR wzSubDirPath[MAX_PATH];
            StringCchPrintfW( wzSubDirPath, MAX_PATH, L"%s\\%s", wzDir, vSubFolders[i] );

            wprintf( L"%s\n", wzSubDirPath );
            
            std::vector<CAtlStringW> vSubFoldersBottom;
            GetSubFolders( wzSubDirPath, vSubFoldersBottom );   

            for ( size_t j = 0; j < vSubFoldersBottom.size(); j++ )
            {
                CAtlStringW strTmp;
                strTmp.Format( L"%s\\%s", vSubFolders[i], vSubFoldersBottom[j] );
                vSubFoldersTmp.push_back( strTmp );
            }
        }

        vSubFolders.clear();
        for ( size_t i = 0; i < vSubFoldersTmp.size(); i++ )
        {
            vSubFolders.push_back( vSubFoldersTmp[i] );
        }
    }
 
    //////////////////////////////////
    size_t iProgress = 0;
    for ( size_t i = 0; i < vSubFoldersAllLevel.size(); i++ )
    {
        WCHAR wzSubDirPath[MAX_PATH];
        std::vector<CAtlStringW> vecPicFile;

        if ( vSubFoldersAllLevel[i].GetLength() > 0 )
            StringCchPrintfW( wzSubDirPath, MAX_PATH, L"%s\\%s", wzDir, vSubFoldersAllLevel[i] );
        else
            StringCchPrintfW( wzSubDirPath, MAX_PATH, L"%s", wzDir );

        GetPictureFilesinDir( wzSubDirPath, vecPicFile );

        for ( size_t j = 0; j < vecPicFile.size(); j++ )
        {
            WCHAR wzPicPath[MAX_PATH];
            StringCchPrintfW( wzPicPath, MAX_PATH, L"%s\\%s", wzSubDirPath, vecPicFile[j] );

            CAtlStringW strFile( wzPicPath );
            if( bTestReadable )
            {
                Gdiplus::Bitmap bmp( wzPicPath );
                if( bmp.GetWidth() != 0 )
                {
                    vAllFiles.push_back( strFile );
                }
                else
                {
                    wprintf( L"Read failed: %s\n", wzPicPath );
                }
            }
            else
            {
                vAllFiles.push_back( strFile );
            }
        }

        size_t iP = (i * 100 ) / vSubFoldersAllLevel.size();
        if ( iP > iProgress )
        {
            iProgress = iP;
            printf( "\b\b\b\b\b\b\b\b\b\b%d%", iProgress );
        }
    }

    return S_OK;
}

HRESULT GetAllFilesFromListFile(
    __in WCHAR *wzFileList,
    __in WCHAR *wzRoot,
    __out std::vector<CAtlStringW> &vAllFiles,
    __in bool bTestReadable)
{


    FILE *fp = NULL;
    _wfopen_s( &fp, wzFileList, L"rt" );
    if (!fp)
    {
        return E_INVALIDARG;
    }

    WCHAR wzTmp[MAX_PATH];
    while ( fgetws( wzTmp, MAX_PATH, fp ) != NULL )
    {
        CAtlStringW strFile(wzTmp);
        strFile.TrimLeft();
        strFile.TrimRight();

        WCHAR wzFilePath[MAX_PATH];
        if (NULL != wzRoot)
        {
            StringCchPrintfW( wzFilePath, MAX_PATH, L"%s\\%s", wzRoot, strFile );
        }
        else
        {
            StringCchPrintfW( wzFilePath, MAX_PATH, L"%s", strFile );
        }
        CAtlStringW strFilePath( wzFilePath );
        if ( bTestReadable )
        {
            Gdiplus::Bitmap bmp( wzFilePath );
            if ( bmp.GetWidth() != 0 )
            {
                vAllFiles.push_back(strFilePath);
            }
        }
        else
        {
            vAllFiles.push_back(strFilePath);
        }

    }
    fclose(fp);
    return S_OK;
}

HRESULT DeleteAllFilesInFolder( WCHAR *wzPicDir )
{
      ATLASSERT(NULL != wzPicDir);
      if (NULL == wzPicDir) return E_INVALIDARG;            
      if (!PathFileExists(wzPicDir)) return E_INVALIDARG;

      WIN32_FIND_DATA wfData;
      WCHAR wzFilter[MAX_PATH];     
      WCHAR wzFile[MAX_PATH]; 

      StringCchPrintfW(wzFilter, MAX_PATH, L"%s\\*.*", wzPicDir );
      HANDLE hFind = ::FindFirstFileW(wzFilter, &wfData);

      if (INVALID_HANDLE_VALUE != hFind) //we couldn't find any file
      {
            BOOL fNext = TRUE;
            while(TRUE == fNext)
            {
                  if (FILE_ATTRIBUTE_DIRECTORY ==
                        (wfData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                  {
                        fNext = ::FindNextFileW(hFind, &wfData);
                        continue;
                  }

                  StringCchPrintfW( wzFile, MAX_PATH, L"%s\\%s", wzPicDir, wfData.cFileName );
                  if( FALSE == ::DeleteFileW( wzFile ) )
                  {
                      wprintf( L"Cannot delete file %s\n", wfData.cFileName );
                  }

                  fNext = ::FindNextFileW(hFind, &wfData);
                  
            }
            ::FindClose(hFind);
      }     

      return S_OK;
}
HRESULT GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
  unsigned int  num = 0;    // number of image encoders
  unsigned int  size = 0;   // size of the image encoder array in bytes
  
  Gdiplus::GetImageEncodersSize(&num, &size);
  if(size == 0)return -1;
  
  Gdiplus::ImageCodecInfo* imageCodecInfo = new Gdiplus::ImageCodecInfo[size];
  Gdiplus::GetImageEncoders(num, size, imageCodecInfo);
  
  for(unsigned int i = 0; i < num; ++i)
  {
    if( wcscmp(imageCodecInfo[i].MimeType, format) == 0 )
    {
       *pClsid = imageCodecInfo[i].Clsid;
       delete[] imageCodecInfo;
       return i;
    }    
  }
  delete[] imageCodecInfo;
  return -1;
}

