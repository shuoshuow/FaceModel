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

#pragma once

#include <windows.h>
#include <unknwn.h>
#include <vector>
#include <atlstr.h>
#include <strsafe.h>

bool IfPathExists(WCHAR *wzDir);
bool ReadFileToBuffer(WCHAR *wzFile, std::vector<BYTE> &vBuffer);

//include .\ and all 1-level subfolders
HRESULT GetPictureFilesinDir(WCHAR * wzPicDir, std::vector<CAtlStringW>& vecPicFile);
HRESULT GetBinFilesInDir(WCHAR *wzBinDir, std::vector<CAtlStringW>& vecBinFiles);


HRESULT GetSubFolders(WCHAR *wzDir, std::vector<CAtlStringW> &vSubFolders);

HRESULT GetAllLeafSubFolders_FullPath(WCHAR *wzDir, std::vector<CAtlStringW> &vSubFoldersLeaf);

HRESULT GetAllPicFilesInSubFolders( 
    __in WCHAR *wzDir, 
    __out std::vector<CAtlStringW> &vAllFiles, 
    __in bool bTestReadable = true);

HRESULT GetAllFilesFromListFile(
    __in WCHAR *wzFileList,
    __in WCHAR *wzRoot,
    __out std::vector<CAtlStringW> &vAllFiles, 
    __in bool bTestReadable = true);

HRESULT DeleteAllFilesInFolder( WCHAR *wzDir );

HRESULT GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

bool ChooseFolder(HWND hParent, const CString& title, CString& folder);