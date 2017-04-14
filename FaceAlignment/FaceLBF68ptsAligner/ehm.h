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
// File: ehm.h
// 
// Description: 
//     ehm check function definition
// 
#ifndef _EHM_H_
#define _EHM_H_

#include <Windows.h>

#include "ehm_typechecks.h"

// 99% of the time ehm.h "just works".  however 1% of the time the "Error"
// label is already in use (e.g. for some other macro package).  if you hit
// that, do this:
//      #define _ehmErrorLab    EhmError
//      #include <ehm.h>
// ... and then use EhmError as your label.  it may seem a bit silly to add
// this extra indirection vs. just use EhmError always, but given that it's
// a 99%/1%, it seems worthwhile.
//
// our suggestion is that custom clients standardize on "EhmError", but if
// need be, they can choose whatever they want.
#ifndef _ehmErrorLab
#define _ehmErrorLab   Error
#endif

// If you define the value for _ehmOnAssertionFail before #including ehm.h you
// can change what _ehmOnAssertionFail does
// Remember this affects all xxxA macros
#ifdef USE_REPLACEABLE_EHM
#ifndef _ehmOnAssertionFail
#define _ehmOnAssertionFail(eType, dwCode, pszFile, ulLine, pszMessage) \
         OnAssertionFail(eType, dwCode, pszFile, ulLine, pszMessage)
#endif
#else
#ifdef _ehmOnAssertionFail
#error Define USE_REPLACEABLE_EHM to use _ehmOnAssertionFail
#endif
#define _ehmOnAssertionFail(eType, dwCode, pszFile, ulLine, pszMessage) TRUE
#endif

// If you define the value for _ehmOnFail before #including ehm.h you
// can change what _ehmOnFail does
// Remember this affects all non-A macros
#ifndef _ehmOnFail
#define _ehmOnFail(eType, dwCode, pszFile, ulLine, pszMessage) 
#endif

// Special definition for PF_EXPR
#ifdef _PREFAST_
//
// Standard Naming:     PF_*
//                  Keep it SHORT
//
// Current version of PREFast chokes on "if (!(0,0))" so we
// wrote these MACRO helpers.
//
// The reason we have the (0,fResult) thing to begin with 
// (rather than just 'fResult') is that the latter causes warnings 
// "constant in conditional expr" for things like "if (1) ..." and 
// "if (0) ..." (which are what ASSERTs and even non-ASSERTs 
// can sometimes end up generating in a macro-laden world).
//
#pragma warning(disable: 4127)

#if !defined(PF_EXPR)
#define PF_EXPR(fResult)            (fResult)
#endif // !PF_EXPR
#else

#if !defined(PF_EXPR)
#define PF_EXPR(fResult)            (0,(fResult))
#endif // !PF_EXPR

#endif


typedef enum
    {
    eHRESULT,
    eBOOL,
    ePOINTER,
    eWINDOWS
    } eCodeType;

#ifdef __cplusplus
// The following functions should always have C linkage
// because they can be used from both C and C++ code.
extern "C"
{
#endif

BOOL ShouldBreakOnAssert();
BOOL SetBreakOnAssert(BOOL fBreakOnAssert);
BOOL OnAssertionFail(eCodeType eType, DWORD dwCode, const TCHAR* pszFile, unsigned long ulLine, const TCHAR* pszMessage);

#ifdef __cplusplus
}
#endif

// Check HRESULT
#define _CHREx0(hResult) \
        do { \
            hr = CHECK_CHR_TYPE(hResult); \
            if(FAILED(hr)) \
            { \
                return E_FAIL; \
            } \
        } while (0,0)

#define _CHR(hResult, hrFail) \
        do { \
            hr = CHECK_CHR_TYPE(hResult); \
            if(FAILED(hr)) \
            { \
                return E_FAIL; \
            } \
        } while (0,0)

// Check pointer result
#define _CPR(p, hrFail) \
        do { \
            if (PF_EXPR(!CHECK_CPR_TYPE(p))) \
            { \
                return E_FAIL; \
            } \
         } while (0,0)

// Check boolean result
    #define _CBR(fResult, hrFail) \
        do { \
            if (PF_EXPR(!CHECK_CBR_TYPE(fResult))) \
            { \
                return E_FAIL; \
            } \
        } while(0,0)

// Check windows result.  Exactly like CBR for the non-Asserting case - BUT we log differently
#define _CWR(fResult, hrFail) \
    do { \
        if (PF_EXPR(!CHECK_CWR_TYPE(fResult))) \
        { \
            return E_FAIL; \
        } \
    } while(0,0)


// The above macros with Asserts when the condition fails
#ifdef DEBUG

#define _CHRAEx0(hResult) \
    do { \
        hr = CHECK_CHR_TYPE(hResult); \
        if(FAILED(hr)) \
            { \
            return E_FAIL; \
            } \
        } \
    while (0,0)

#define _CHRA(hResult, hrFail) \
    do { \
        hr = CHECK_CHR_TYPE(hResult); \
        if(FAILED(hr)) \
            { \
            return E_FAIL; \
            } \
        } \
    while (0,0)

#define _CPRA(p, hrFail) \
    do  { \
        if (PF_EXPR(!CHECK_CPR_TYPE(p))) \
            { \
            hr = (hrFail); \
            return E_FAIL; \
            } \
        } \
    while (0,0)

#define _CBRA(fResult, hrFail) \
    do  { \
        if (PF_EXPR(!CHECK_CBR_TYPE(fResult))) \
            { \
            hr = (hrFail); \
            return E_FAIL; \
            } \
        } \
    while (0,0)

#define _CWRA(fResult, hrFail) \
    do  { \
        if (PF_EXPR(!CHECK_CWR_TYPE(fResult))) \
            { \
            hr = (hrFail); \
            return E_FAIL; \
            } \
        } \
    while (0,0)

#define VBR(fResult) \
    do  { \
        if (PF_EXPR(!CHECK_CBR_TYPE(fResult))) \
            { \
            return E_FAIL; \
        } \
    while (0,0)

#define VPR(fResult) \
    do  { \
        if (PF_EXPR(!CHECK_CPR_TYPE(fResult))) \
            { \
            return E_FAIL; \
        } \
    while (0,0)


// Verify Windows Result
#define VWR(fResult) \
    do  { \
        if (!(PF_EXPR(NULL != CHECK_CWR_TYPE(fResult)))) \
            { \
            return E_FAIL; \
        } \
    while (0,0)


// Verify HRESULT (careful not to modify hr)
#define VHR(hResult) \
    do  { \
        HRESULT _EHM_hrTmp = CHECK_CHR_TYPE(hResult); \
        if(FAILED(_EHM_hrTmp)) \
            { \
            return E_FAIL; \
        } \
    while (0,0)

// NOTE: because the Dxx macros are intended for DEBUG spew - there is no logging extensibility
// make sure you keep the xTmp, can only eval arg 1x
// todo: dump GetLastError in DWR
#define DWR(fResult) \
    do { if (PF_EXPR(!CHECK_CWR_TYPE(fResult))) {DEBUGMSG(1, (TEXT("DWR(") TEXT( # fResult ) TEXT(")\r\n") ));}} while (0,0)
#define DHR(hResult) \
    do { HRESULT hrTmp = CHECK_CHR_TYPE(hResult); if(FAILED(hrTmp)) {DEBUGMSG(1, (TEXT("DHR(") TEXT( # hResult ) TEXT(")=0x%x\r\n"), hrTmp));}} while (0,0)
#define DPR     DWR     // tmp
#define DBR     DWR     // tmp

#define CHRA(e) _CHRAEx0(e)
#define CPRA(e) _CPRA(e, E_OUTOFMEMORY)
#define CBRA(e) _CBRA(e, E_FAIL)
#define CWRA(e) _CWRA(e, E_FAIL)
#define CHRAEx(e, hrFail) _CHRA(e, hrFail)
#define CPRAEx(e, hrFail) _CPRA(e, hrFail)
#define CBRAEx(e, hrFail) _CBRA(e, hrFail)
#define CWRAEx(e, hrFail) _CWRA(e, hrFail)
#else
#define CHRA CHR
#define CPRA CPR
#define CBRA CBR
#define CWRA CWR
#define CHRAEx CHREx
#define CPRAEx CPREx
#define CBRAEx CBREx
#define CWRAEx CWREx
#define VHR(x) (CHECK_CHR_TYPE(x))
#define VPR(x) (CHECK_CPR_TYPE(x))
#define VBR(x) (CHECK_CBR_TYPE(x))
#define VWR(x) (CHECK_CWR_TYPE(x))
#define DHR(x) (CHECK_CHR_TYPE(x))
#define DPR(x) (CHECK_CPR_TYPE(x))
#define DBR(x) (CHECK_CBR_TYPE(x))
#define DWR(x) (CHECK_CWR_TYPE(x))
#endif

#define CHR(e) _CHREx0(e)
#define CPR(e) _CPR(e, E_OUTOFMEMORY)
#define CPP(e) _CPR(e, E_INVALIDARG)
#define CBR(e) _CBR(e, E_FAIL)
#define CWR(e) _CWR(e, E_FAIL)
#define CHREx(e, hrFail) _CHR(e, hrFail)
#define CPREx(e, hrFail) _CPR(e, hrFail)
#define CBREx(e, hrFail) _CBR(e, hrFail)
#define CWREx(e, hrFail) _CWR(e, hrFail)

// obsolete (but still in use)
// - work around various pseudo-pblms:
//  partial images, CEPC no-radio, etc.
// - also things that we want to know about in dev, but not in QA/stress:
//  an e.g. is our DoVerb stuff, there are 'good' failures (END when no call,
// or TALK w/ 0 entries) and 'bad' failures (e.g. TAPI returns an error), we
// don't want to int3 during stress but we do want to on our dev machines
//
// eventually we'll make these do "if (g_Assert) int3;", then we
// can run w/ g_Assert on for dev and off for stress.
#define xCHRA   CHR     // should be CHRA but...
#define xCPRA   CPR     // should be CPRA but...
#define xCBRA   CBR     // should be CBRA but...
#define xCWRA   CWR     // should be CWRA but...
#define xVHR    DHR     // should be VHR  but...
#define xVPR    DPR     // should be VPR  but...
#define xVBR    DBR     // should be VBR  but...
#define xVWR    DWR     // should be VWR  but...

#endif // _EHM_H_
