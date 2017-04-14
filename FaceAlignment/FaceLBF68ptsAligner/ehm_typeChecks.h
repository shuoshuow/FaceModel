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
// File: ehm_typeChecks.h
// 
// Description: 
//     ehm type check function definition
// 
#pragma once

#include <Windows.h>

// Define type checks for parameters of EHM macros
// 
// Disabled if:
//              !__cplusplus    can't use templates in C
//              _PREFAST_       disabled in prefast builds, because type checks don't allow prefast
//                              to make the conclusion about the NULL-ness of pointers passed to the CPR macro
//
#if defined(__cplusplus) && !defined(_PREFAST_)

//
// It is necessary to use 'extern "C++"' because some other headers include us 
// inside of an 'extern "C"'. 
// 
extern "C++" 
{
namespace ce
{
    template<typename T, typename Fn, Fn fnClose, T _Invalid, typename copy_traits>
    class auto_xxx;

    template<typename element_type> class ptr_com;
    template<class element_type, class close_traits> class ptr_scoped;
    template<class element_type, class counter_type> class ptr_shared;
}

namespace Typecheck
{
    template <typename T, typename U>
    struct IsSameType
    {
        enum { Value = false };
    };

    template <typename T>
    struct IsSameType<T, T>
    {
        enum { Value = true };
    };

    template <typename T>
    struct IsPointerType
    {
        enum { Value = false };
    };

    template <typename T>
    struct IsPointerType<T *>
    {
        enum { Value = true };
    };

    template <typename T, typename U>
    struct IsConvertibleToType
    {
        // preparation
        typedef char ConvertibleResultType;
        struct UnconvertibleResultType
        {
            char m_internal[2];
        };

        // one of these functions is chosen depending on whether T is convertible to U
        static ConvertibleResultType    Test(U);
        static UnconvertibleResultType  Test(...);

        static T MakeT();

        // actual test
        enum { Value = sizeof(Test(MakeT())) == sizeof(ConvertibleResultType) };
    };

    template <typename T>
    struct IsConvertibleToPointerType
    {
        enum { Value = IsConvertibleToType<T, void *>::Value
                    || IsConvertibleToType<T, void const *>::Value };
    };

    template <typename T>
    struct IsPodType
    {
        enum { Value = __is_pod(T) };  // change to tr1::is_pod when TR1 is supported
    };

    template <typename T>
    struct DerefPointer
    {
        typedef T Type;
    };

    template <typename T>
    struct SizeOf;

    // Windows handle type is defined as void *, or pointer to POD containing a single int
    template <typename T>
    struct IsHandleType
    {
        enum { Value = IsSameType<void *, T>::Value
                       || (IsPointerType<T>::Value
                           && IsPodType<typename DerefPointer<T>::Type>::Value
                           && SizeOf<typename DerefPointer<T>::Type>::Value == sizeof(int)) };
    };

    

#define DEFINE_DEREF_POINTER(PointerT, T)   \
    template <typename T>                   \
    struct DerefPointer<PointerT>           \
    {                                       \
        typedef T Type;                     \
    }                                       \

    DEFINE_DEREF_POINTER(T *, T);
    DEFINE_DEREF_POINTER(T * const, T);
    DEFINE_DEREF_POINTER(T * volatile, T);
    DEFINE_DEREF_POINTER(T * const volatile, T);

#undef DEFINE_DEREF_POINTER

    template <typename T>
    struct SizeOf
    {
        enum { Value = sizeof(T) };
    };

    template <>
    struct SizeOf<void>
    {
        enum { Value = 0 };
    };

    // Check parameter type for CBR
    // Valid types: bool, BOOL (int)
    // Added due to frequent use: DWORD (unsigned long), pointer
    // Warning for future changes: make sure it still rejects HRESULT (long).
    //
    template <typename T>
    T const & CheckCbrType(T const & arg)
    {
        static_assert((IsSameType<bool, T>::Value
                    || IsSameType<BOOL, T>::Value
                    || IsSameType<DWORD, T>::Value
                    || IsConvertibleToPointerType<T>::Value), "It's not a valid CBR type");
        return arg;
    }

    // Basic check of the parameter type for CBR
    // Designed to catch CBR(HRESULT)
    //
    template <typename T>
    T const & CheckCbrTypeBasic(T const & arg)
    {
        static_assert((!IsSameType<HRESULT, T>::Value), "It's not HRESULT type for CBR");
        return arg;
    }

    // Check parameter type for CPR
    // Valid types: pointer or smart pointer
    //
    template <typename T>
    T const & CheckCprType(T const & arg)
    {
        static_assert(IsConvertibleToPointerType<T>::Value, "It's not a valid pointer type");
        return arg;
    }

    // Check parameter type for CHR
    // Valid types: HRESULT
    //
    template <typename T>
    T const & CheckChrType(T const & arg)
    {
        static_assert((IsSameType<HRESULT, T>::Value), "It's not HRESULT type for CHR");
        return arg;
    }

    // Basic check of the parameter type for CHR
    // Designed to catch CHR(BOOL) and CHR(bool)
    //
    template <typename T>
    T const & CheckChrTypeBasic(T const & arg)
    {
        static_assert((!IsSameType<BOOL, T>::Value
                    && !IsSameType<bool, T>::Value), "It's not a valid boolean type");
        return arg;
    }

    // Check parameter type for CWR
    // Valid types: int, DWORD, handle, ce::auto_xxx<handle>
    // Refer to public\common\sdk\inc\winnt.h for a definition of the handle type.
    //
    template <typename T>
    struct CheckCwrTypeImpl
    {
        enum { Value = IsSameType<int, T>::Value
                    || IsSameType<DWORD, T>::Value
                    || IsHandleType<T>::Value };
    };

    template<typename T, typename Fn, Fn fnClose, T _Invalid, typename copy_traits>
    struct CheckCwrTypeImpl<ce::auto_xxx<T, Fn, fnClose, _Invalid, copy_traits> >
    {
        enum { Value = IsHandleType<T>::Value };
    };

    template <typename T>
    T const & CheckCwrType(T const & arg)
    {
        STATIC_ASSERT(CheckCwrTypeImpl<T>::Value);
        return arg;
    }

    template <typename T, typename U>
    bool CheckCprType(ce::ptr_shared<T, U> const &arg)
    {
        return (bool) arg;
    }

    template <typename T, typename U>
    bool CheckCprType(ce::ptr_scoped<T, U> const &arg)
    {
        return (bool) arg;
    }

    template <typename T>
    bool CheckCprType(ce::ptr_com<T> const &arg)
    {
        return (bool) arg;
    }

    template <typename T, typename U>
    bool CheckCbrType(ce::ptr_shared<T, U> const &arg)
    {
        return (bool) arg;
    }

    template <typename T, typename U>
    bool CheckCbrType(ce::ptr_scoped<T, U> const &arg)
    {
        return (bool) arg;
    }

    template <typename T>
    bool CheckCbrType(ce::ptr_com<T> const &arg)
    {
        return (bool) arg;
    }

}
};

#if !defined(NO_CHECKED_EHM)

// Define macros for type checking
//
#define CHECK_CBR_TYPE(x)       \
    Typecheck::CheckCbrType(x)  \

#define CHECK_CPR_TYPE(x)       \
    Typecheck::CheckCprType(x)  \

#define CHECK_CHR_TYPE(x)       \
    Typecheck::CheckChrType(x)  \

#define CHECK_CWR_TYPE(x)       \
    Typecheck::CheckCwrType(x)  \

#else // !NO_CHECKED_EHM

// Define basic type checks if NO_CHECKED_EHM is set.
// This is done by NO_CHECKED_EHM=1 setting in the sources file.
// If NO_CHECKED_EHM is not set the full type checking will be used.
//
#define CHECK_CBR_TYPE(x)           \
    Typecheck::CheckCbrTypeBasic(x) \

#define CHECK_CHR_TYPE(x)           \
    Typecheck::CheckChrTypeBasic(x) \

#endif // !NO_CHECKED_EHM

#endif // __cplusplus && !_PREFAST_

#ifndef CHECK_CBR_TYPE
#define CHECK_CBR_TYPE(x) (x)
#endif

#ifndef CHECK_CPR_TYPE
#define CHECK_CPR_TYPE(x) (x)
#endif

#ifndef CHECK_CHR_TYPE
#define CHECK_CHR_TYPE(x) (x)
#endif

#ifndef CHECK_CWR_TYPE
#define CHECK_CWR_TYPE(x) (x)
#endif

