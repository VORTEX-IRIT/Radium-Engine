#pragma once

#include <Core/RaCore.hpp>

#include <memory>

// Singleton utility.
// This file give you two macros to automatically implement a class
// as a singleton.
// I used to be a template like you, then I took a DLL to the knee...

// Usage : The singleton instance is initially set to null.
// To create the instance, call `createInstance( )` with the
// class constructor arguments.
// To access the singleton instance, call getInstance().
// The singleton instance can also be destroyed (reset to null).

/// Add this macro (followed by a semicolon) in your class header.
/// The macro should appear first in the class, before any public:
/// specifier.
#define RA_SINGLETON_INTERFACE( TYPE )                                                 \
  protected:                                                                           \
    TYPE( const TYPE& )           = delete;                                            \
    void operator=( const TYPE& ) = delete;                                            \
    struct Deleter {                                                                   \
        void operator()( TYPE* p ) const {                                             \
            delete p;                                                                  \
        }                                                                              \
    };                                                                                 \
    static std::unique_ptr<TYPE, Deleter> s_instance;                                  \
                                                                                       \
  public:                                                                              \
    template <typename... Args>                                                        \
    inline static TYPE* createInstance( const Args&... args ) {                        \
        s_instance = std::unique_ptr<TYPE, Deleter>( new TYPE( args... ), Deleter() ); \
        return getInstance();                                                          \
    }                                                                                  \
    inline static TYPE* getInstance() {                                                \
        return s_instance.get();                                                       \
    }                                                                                  \
    inline static void destroyInstance() {                                             \
        s_instance.reset( nullptr );                                                   \
    }

/// Add this macro in the singleton cpp, followed by a semicolon.
// Limitations : TYPE cannot be a nested type
// RA_SINGLETON_IMPLEMENTATION(A::MySingleton); will *not* work.
#define RA_SINGLETON_IMPLEMENTATION( TYPE )                          \
    std::unique_ptr<TYPE, TYPE::Deleter> TYPE::s_instance = nullptr; \
    class TYPE
// The line above is just there to make the macro end with a ;
