#pragma once

#include <Core/Utils/StdOptional.hpp>
#include <string>

#include <Engine/Data/ShaderConfiguration.hpp>

namespace Ra {
namespace Engine {
namespace Data {

/** Factory that manage the set of named shader configurations
 * Usefull for the management of shader libraries where a shader configuration (source code,
 * defines, properties) are defined once and reuse multiple time.
 */
namespace ShaderConfigurationFactory {
/**
 * Add a configuration to the factory
 * \note In case of name collision, the configuration is not added to the factory
 * \param config
 */
RA_ENGINE_API void addConfiguration( const Data::ShaderConfiguration& config );

/**
 * Remove a configuration from the factory
 * \param config
 * \return true if the configuration was found and removed, else false
 */
RA_ENGINE_API bool removeConfiguration( const std::string& configName );

/**
 * Get a configuration from the factory
 * \param name The configuration to get
 * \return The configuration if it exists, std::nullopt otherwise
 */
RA_ENGINE_API Core::Utils::optional<Data::ShaderConfiguration>
getConfiguration( const std::string& name );
} // namespace ShaderConfigurationFactory

} // namespace Data
} // namespace Engine
} // namespace Ra
