#pragma once
#include <Core/CoreMacros.hpp>

/// Defines the correct macro to export dll symbols.
#if defined RA_DATAFLOW_EXPORTS
#    define RA_DATAFLOW_API DLL_EXPORT
#elif defined RA_DATAFLOW_STATIC
#    define RA_DATAFLOW_API
#else
#    define RA_DATAFLOW_API DLL_IMPORT
#endif

/// Allow to define initializers for modules that need to be initialized transparently
#define DATAFLOW_LIBRARY_INITIALIZER_DECL( f ) void f##__Initializer()

#define DATAFLOW_LIBRARY_INITIALIZER_IMPL( f )          \
    struct f##__Initializer_t_ {                        \
        f##__Initializer_t_() { ::f##__Initializer(); } \
    };                                                  \
    static f##__Initializer_t_ f##__Initializer__;      \
    void f##__Initializer()
