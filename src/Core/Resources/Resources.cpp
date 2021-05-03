#include <Core/Resources/Resources.hpp>

#include <Core/Utils/Log.hpp>
#include <Core/Utils/StdFilesystem.hpp>

#include <algorithm>
#include <cpplocate/cpplocate.h>

namespace Ra {
namespace Core {
namespace Resources {

using namespace Ra::Core::Utils;
namespace fs = ::std::filesystem;

fs::path searchPath( const std::string& pattern, const std::string& system, void* libSymbol ) {
    std::string p = cpplocate::locatePath( pattern, system, libSymbol );
    return fs::path( p ).lexically_normal();
}

// add a trailing "/" if path point to an existing directory.
fs::path clean( const fs::path& path ) {
    auto status = fs::status( path );
    if ( status.type() == fs::file_type::not_found ) return "";
    if ( status.type() == fs::file_type::directory ) return path / "";
    return path;
}

optional<std::string> getRadiumResourcesPath() {
    auto p =
        searchPath( "Resources/Radium/", "", reinterpret_cast<void*>( getRadiumResourcesPath ) );
    p = clean( p / "Resources/Radium" );

    if ( p.empty() ) return {};
    return p.string();
}

optional<std::string> getRadiumPluginsPath() {
    auto p = searchPath( "Plugins/lib", "", reinterpret_cast<void*>( getRadiumPluginsPath ) );
    p      = clean( p / "Plugins" / "lib" );

    if ( p.empty() ) return {};
    return p.string();
}

/// this one is always found, use optional for consistency ?
optional<std::string> getBasePath() {
    return clean( cpplocate::getModulePath() ).string();
}

optional<std::string> getResourcesPath( void* symbol, const std::string& pattern ) {
    auto p = searchPath( pattern, "", symbol );
    p      = clean( p / pattern );
    if ( p.empty() ) return {};
    return p.string();
}

} // namespace Resources
} // namespace Core
} // namespace Ra