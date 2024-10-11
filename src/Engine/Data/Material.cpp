#include <Engine/Data/Material.hpp>

namespace Ra {
namespace Engine {
namespace Data {
Material::Material( const std::string& instanceName,
                    const std::string& materialName,
                    MaterialAspect aspect ) :
    ShaderParameterProvider(),
    m_instanceName { instanceName },
    m_aspect { aspect },
    m_materialName { materialName } {}

} // namespace Data
} // namespace Engine
} // namespace Ra
