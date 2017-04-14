//
// Copyright (c) 2014 Microsoft Corporation.  All rights reserved.
//
// File: ModelFileHeader.h
//
// Description:
//  Face SDK model file header definition.
//  All Face SDK algorithm models using .MDL file which must contains this header.
//  FACESDK_MODELCOREHEADER is for public and FACESDK_MODELPRIVATEHEADER for internal use.
//

#pragma once

#include <Windows.h>

#include <time.h>
#define MODEL_FILE_EXTENSION L"MDL"

// Four-Character-Code to indicate Face SDK Model file
#define MODEL_FILE_FOURCC 'FMDL'

// Four-Character-Code for algorithm model types. It is used in data chunk beginning to indicate data types
#define ALGORITHM_DETECTION_PREFILTER_FOURCC 'DPRE'
#define ALGORITHM_DETECTION_CASCADE_FOURCC 'DCAS'
#define ALGORITHM_DETECTION_POSTFILTER_FOURCC 'DPOS'
#define ALGORITHM_ALIGNMENT_LBF_FOURCC 'ALBF'
#define ALGORITHM_RECOGNITION_SIBJ_FOURCC 'RSIJ'

// Stable model types. For temporary experiments, general type can be used with description.
enum MODELTYPE
{
	MT_UNKNOWN = 0,

	// Detection types
	MT_DETECTION_BEGIN = 100,
	MT_DETECTION_JDA = 101,
	MT_DETECTION_JDA_LIGHTWEIGHT_LBF = 102,
	MT_DETECTION_MULTIVIEW = 110,
	MT_DETECTION_END = 199,

	// Alignment types
	MT_ALIGNMENT_BEGIN = 200,
	MT_ALIGNMENT_LBF = 201,
	MT_ALIGNMENT_END = 299,

	// Recognition types
	MT_RECOGNITION_BEGIN = 300,
	MT_RECOGNITION_SIJB = 301,
	MT_RECOGNITION_END = 399,

	// Attribute types
	MT_ATTRIBUTE_BEGIN = 400,
	MT_ATTRIBUTE_GENDER = 401,
	MT_ATTRIBUTE_AGE = 402,
	MT_ATTRIBUTE_END = 499,

	// tracking types
	MT_TRACKING_BEGIN = 500,
	MT_TRACKING_LBF = 501,
	MT_TRACKING_END = 599,

	// Force to be DWORD
	MT_FORCE_DWORD = 0xffffffffUL
};

// Face SDK Model file header public version
struct FACESDK_MODELCOREHEADER
{
	// Magic number to indicate Bin file. It's 'FMDL'
	DWORD dwFourCC;

	// Header structure size
	DWORD dwHeaderSize;

	// Model type
	MODELTYPE dwModelType;

	// model file size
	DWORD dwFileSize;

};

// Face SDK Model file header private version
struct FACESDK_MODELPRIVATEHEADER
{
	// Same structure as FACESDK_MODELCOREHEADER at beginning
	//--------------------------------------------------------
	// Magic number to indicate model file. It's 'FMDL'
	DWORD dwFourCC;

	// Header structure size
	DWORD dwHeaderSize;

	// Model type
	MODELTYPE ModelType;

	// model file size
	DWORD dwFileSize;

	// Private information
	//--------------------------------------------------------
	// Name of algorithm plug-in which generates this .mdl file. ANSI string is enough.
	char szCreatorName[64];

	// Id of algorithm plug-in which generates this .mdl file
	GUID  guidCreatorId;

	// Time building this model. The seconds elapsed since midnight (00:00:00), January 1, 1970
	__time64_t BuildDateTime;

	// Additional information in ANSI string. training parameters could be encode as text string values.
	// e.g. TreeDepth=100;PatchSize=32;
	char szExtraInfo[256];
};


///<summary>
/// Check if the input buffer contains a valid model file
///</summary>
///<return>
/// S_OK if a valid Face SDK model file
/// E_INVALIDARG
/// E_FAIL for invalid file
///</return>
HRESULT ModelVerifyHeader(
	_In_ UINT modelSizeInBytes,
	_In_reads_bytes_(modelSizeInBytes) LPVOID model);

///<summary>
/// Skip model header and point to actual data
///</summary>
BYTE * ModelMoveToDataChunk(_In_ LPVOID model);