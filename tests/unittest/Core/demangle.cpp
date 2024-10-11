#include <Core/Utils/TypesUtils.hpp>
#include <catch2/catch.hpp>

#include <string>
#include <vector>

#include <iostream>

namespace TypeTests {
struct TypeName_struct {};
} // namespace TypeTests
TEST_CASE( "Core/Utils/TypesUtils", "[Core][Core/Utils][TypesUtils]" ) {
    SECTION( "Demangle from typename" ) {
        using Ra::Core::Utils::demangleType;

        REQUIRE( std::string( demangleType<int>() ) == "int" );
        REQUIRE( std::string( demangleType<float>() ) == "float" );
        REQUIRE( std::string( demangleType<uint>() ) == "unsigned int" );
        REQUIRE( std::string( demangleType<size_t>() ) == "unsigned long" );
        auto demangledName = std::string( demangleType<std::vector<int>>() );
        REQUIRE( demangledName == "std::vector<int, std::allocator<int>>" );

        demangledName = std::string( demangleType<TypeTests::TypeName_struct>() );
        REQUIRE( demangledName == "TypeTests::TypeName_struct" );
    }

    SECTION( "Demangle from instance" ) {
        using Ra::Core::Utils::demangleType;

        int i { 1 };
        float f { 2 };
        unsigned int u { 3 };
        size_t s { 4 };

        REQUIRE( std::string( demangleType( i ) ) == "int" );
        REQUIRE( std::string( demangleType( f ) ) == "float" );
        REQUIRE( std::string( demangleType( u ) ) == "unsigned int" );
        REQUIRE( std::string( demangleType( s ) ) == "unsigned long" );
        std::vector<int> v;
        auto demangledName = std::string( demangleType( v ) );
        REQUIRE( demangledName == "std::vector<int, std::allocator<int>>" );

        TypeTests::TypeName_struct tns;
        demangledName = std::string( demangleType( tns ) );
        REQUIRE( demangledName == "TypeTests::TypeName_struct" );
    }
}
