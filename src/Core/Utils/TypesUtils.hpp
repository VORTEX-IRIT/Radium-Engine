#pragma once
#include <Core/RaCore.hpp>

#ifndef _WIN32
#    include <cxxabi.h>
#    include <memory>
#else
#    include <typeinfo>
#endif

#include <string>
#include <typeindex>

#include <Core/Utils/StringUtils.hpp>

namespace Ra {
namespace Core {
namespace Utils {

/// Return the human readable version of the type name T
template <typename T>
std::string demangleType() noexcept;

/// Return the human readable version of the given object's type
template <typename T>
std::string demangleType( const T& ) noexcept;

/// Return the human readable version of the given type name
std::string demangleType( const std::type_index& typeIndex ) noexcept;

// Check if a type is a container with access to its element type and number
// adapted from https://stackoverflow.com/questions/13830158/check-if-a-variable-type-is-iterable
namespace detail {

using std::begin;
using std::end;

template <typename T>
auto is_container_impl( int )
    -> decltype( begin( std::declval<T&>() ) !=
                     end( std::declval<T&>() ), // begin/end and operator !=
                 void(),                        // Handle evil operator ,
                 std::declval<T&>().empty(),
                 std::declval<T&>().size(),
                 ++std::declval<decltype( begin( std::declval<T&>() ) )&>(), // operator ++
                 void( *begin( std::declval<T&>() ) ),                       // operator*
                 std::true_type {} );

template <typename T>
std::false_type is_container_impl( ... );

} // namespace detail

template <typename T>
using is_container = decltype( detail::is_container_impl<T>( 0 ) );

// TypeList taken and adapted from
// https://github.com/AcademySoftwareFoundation/openvdb/blob/master/openvdb/openvdb/TypeList.h
// Only took small part of TypeList utilities

// forward declarations
template <typename... Ts>
struct TypeList;

namespace TypeListInternal {

/// \brief   Append any number of types to a \c TypeList
/// \details Defines a new \c TypeList with the provided types appended
/// \tparam ListT  The \c TypeList to append to
/// \tparam Ts     Types to append
template <typename ListT, typename... Ts>
struct TSAppendImpl;

/// \brief  Partial specialization for a \c TypeList with a list of zero or more
///         types to append
/// \tparam Ts Current types within the \c TypeList
/// \tparam OtherTs Other types to append
template <typename... Ts, typename... OtherTs>
struct TSAppendImpl<TypeList<Ts...>, OtherTs...> {
    using type = TypeList<Ts..., OtherTs...>;
};

/// \brief  Partial specialization for a \c TypeList with another \c TypeList.
///         Appends the other TypeList's members.
/// \tparam Ts Types within the first \c TypeList
/// \tparam OtherTs Types within the second \c TypeList
template <typename... Ts, typename... OtherTs>
struct TSAppendImpl<TypeList<Ts...>, TypeList<OtherTs...>> {
    using type = TypeList<Ts..., OtherTs...>;
};

} // namespace TypeListInternal

template <typename... Ts>
struct TypeList {
    /// \brief The type of this list
    using Self = TypeList;
    /// \brief The number of types in the type list
    static constexpr size_t Size = sizeof...( Ts );

    /// \brief Append types, or the members of another TypeList, to this list.
    /// \warning Appending nested TypeList<> objects causes them to expand to
    ///          their contained list of types.
    /// \details Example:
    /// \code
    /// {
    ///     using IntTypes = Ra::Core::Utils::TypeList<Int16, Int32, Int64>;
    ///     using RealTypes = Ra::Core::Utils::TypeList<float, double>;
    ///     using NumericTypes = IntTypes::Append<RealTypes>;
    /// }
    /// {
    ///     using IntTypes = Ra::Core::Utils::TypeList<Int16>::Append<Int32, Int64>;
    ///     using NumericTypes = IntTypes::Append<float>::Append<double>;
    /// }
    /// \endcode
    template <typename... TypesToAppend>
    using Append = typename TypeListInternal::TSAppendImpl<Self, TypesToAppend...>::type;
};

#ifdef _WIN32
// On windows (since MSVC 2019), typeid( T ).name() (and then typeIndex.name() returns the demangled
// name
inline std::string demangleType( const std::type_index& typeIndex ) noexcept {
    std::string retval = typeIndex.name();
    removeAllInString( retval, "class " );
    removeAllInString( retval, "struct " );
    removeAllInString( retval, "__cdecl" );
    replaceAllInString( retval, "& __ptr64", "&" );
    replaceAllInString( retval, ",", ", " );
    replaceAllInString( retval, " >", ">" );
    replaceAllInString( retval, "__int64", "long" );
    replaceAllInString( retval, "const &", "const&" );
    return retval;
}
#else
// On Linux/macos, use the C++ ABI demangler
inline std::string demangleType( const std::type_index& typeIndex ) noexcept {
    int error = 0;
    std::string retval;
    char* name = abi::__cxa_demangle( typeIndex.name(), 0, 0, &error );
    if ( error == 0 ) { retval = name; }
    else {
        // error : -1 --> memory allocation failed
        // error : -2 --> not a valid mangled name
        // error : other --> __cxa_demangle
        retval = std::string( "Type demangler error : " ) + std::to_string( error );
    }
    std::free( name );
    removeAllInString( retval, "__1::" ); // or "::__1" ?
    replaceAllInString( retval, " >", ">" );
    return retval;
}
#endif
template <typename T>
std::string demangleType() noexcept {
    // once per one type
    static auto demangled_name = demangleType( std::type_index( typeid( T ) ) );
    return demangled_name;
}

// calling with instances
template <typename T>
std::string demangleType( const T& ) noexcept {
    return demangleType<T>();
}

} // namespace Utils
} // namespace Core
} // namespace Ra
