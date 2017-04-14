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
// File: utils.h
// 
// Description: 
//     Basic utility 
// 
#pragma once


// NOTE: fp should be left-value variable
#define SAFE_CLOSE_FILE(fp)     do { if ((fp)) { fclose((fp)); (fp) = NULL; } } while (0, 0)
// NOTE: p should be left-value variable
#define SAFE_DELETE_POINTER(p)  do { if (NULL != (p)) { delete (p); (p) = NULL; } } while (0, 0)
// NOTE: p should be left-value variable
#define SAFE_DELETE_ARRAY(p)    do { if (NULL != (p)) { delete[] (p); (p) = NULL;} } while (0, 0)
// NOTE: p should be left-value variable
#define SAFE_DELETE_2DARRAY(pp, dim1)    \
	do  { \
			if (NULL != (pp)) \
			{ \
				for (int __i = 0; __i < dim1; __i++) \
				{  \
					if (NULL != pp[__i]) { delete[](pp[__i]); (pp[__i]) = NULL; } \
				} \
				delete[]pp; pp = NULL; \
			} \
		} while (0, 0)
