//  (C) Copyright Eric Jourdanneau, Joel Falcou 2010
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  NVIDIA CUDA C++ compiler setup

#ifndef BHO_COMPILER
#  define BHO_COMPILER "NVIDIA CUDA C++ Compiler"
#endif

#if defined(__CUDACC_VER_MAJOR__) && defined(__CUDACC_VER_MINOR__) && defined(__CUDACC_VER_BUILD__)
#  define BHO_CUDA_VERSION (__CUDACC_VER_MAJOR__ * 1000000 + __CUDACC_VER_MINOR__ * 10000 + __CUDACC_VER_BUILD__)
#else
// We don't really know what the CUDA version is, but it's definitely before 7.5:
#  define BHO_CUDA_VERSION 7000000
#endif

// NVIDIA Specific support
// BHO_GPU_ENABLED : Flag a function or a method as being enabled on the host and device
#define BHO_GPU_ENABLED __host__ __device__

#if !defined(__clang__) || defined(__NVCC__)
// A bug in version 7.0 of CUDA prevents use of variadic templates in some occasions
// https://svn.boost.org/trac/bho/ticket/11897
// This is fixed in 7.5. As the following version macro was introduced in 7.5 an existance
// check is enough to detect versions < 7.5
#if BHO_CUDA_VERSION < 7050000
#   define BHO_NO_CXX11_VARIADIC_TEMPLATES
#endif
// The same bug is back again in 8.0:
#if (BHO_CUDA_VERSION > 8000000) && (BHO_CUDA_VERSION < 8010000)
#   define BHO_NO_CXX11_VARIADIC_TEMPLATES
#endif
// CUDA (8.0) has no constexpr support in msvc mode:
#if defined(_MSC_VER) && (BHO_CUDA_VERSION < 9000000)
#  define BHO_NO_CXX11_CONSTEXPR
#endif

#endif

#ifdef __CUDACC__
//
// When compiing .cu files, there's a bunch of stuff that doesn't work with msvc:
//
#if defined(_MSC_VER)
#  define BHO_NO_CXX14_DIGIT_SEPARATORS
#  define BHO_NO_CXX11_UNICODE_LITERALS
#endif
//
// And this one effects the NVCC front end,
// See https://svn.boost.org/trac/bho/ticket/13049
//
#if (BHO_CUDA_VERSION >= 8000000) && (BHO_CUDA_VERSION < 8010000)
#  define BHO_NO_CXX11_NOEXCEPT
#endif

#endif

