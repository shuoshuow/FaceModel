// 
// Copyright (c) Microsoft Corporation. All rights reserved. 
// 
// Microsoft Face SDK is developed by Microsoft Research Asia. It integrates 
// the latest face technologies from Microsoft Research (MSR Asia, MSR Redmond, 
// MSR Fuse Lab, Israel Innovation Labs, Siena) and many other contributors. 
// 
// Microsoft owns the full IP and patents and the source code and binaries are 
// strictly Microsoft Confidential. Please contact Face SDK team prior to 
// external disclosure in any forms. 
// 
// THE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES. 
// 
// Face SDK Team <facesdk@microsoft.com>
// http://toolbox/facesdk 
// 
// ---------------------------------------------------------------------------- 
// File: iodefs.h
// 
// Description: 
//     Defines basic file access macro for easy of use.
// 
#pragma once

#include <cassert>

#define BEGIN_READ_FILE \
	size_t		iBytesToRead, itemsRead;

#define READ_MULTIPLE_AND_CHECK(addr, bytesOfBuffer, bytesOfElement, count, fp) \
	assert((addr) != NULL); \
	iBytesToRead = (bytesOfElement) * (count); \
	itemsRead = fread_s((addr), (bytesOfBuffer), (bytesOfElement), (count), (fp)); \
	CBR(itemsRead == (count));

#define READ_SINGLE_AND_CHECK(value, fp) \
	READ_MULTIPLE_AND_CHECK(&value, sizeof(value), sizeof(value), 1, fp)

#define BUFFER_READ_MULTIPLE_AND_CHECK(addr, bytesOfDst, bytesOfElement, count, buffer, bufferSize) \
	assert((addr) != NULL); \
	assert((buffer) != NULL); \
	iBytesToRead = (bytesOfElement) * (count); \
	CBR(iBytesToRead <= bytesOfDst); \
	CBR(iBytesToRead <= bufferSize); \
	memcpy((addr), (buffer), static_cast<UINT>(iBytesToRead)); \
	itemsRead = count; \
	buffer += iBytesToRead; \
	bufferSize -= iBytesToRead;

// Read value from buffer. Move buffer position on a successful read.
#define BUFFER_READ_SINGLE_AND_CHECK(value, buffer, bufferSize) \
	BUFFER_READ_MULTIPLE_AND_CHECK(&value, sizeof(value), sizeof(value), 1, buffer, bufferSize)
