#include <Dataflow/Core/TypeDemangler.hpp>

#include <Core/Utils/StringUtils.hpp>

#include <map>

namespace Ra {
namespace Dataflow {
namespace Core {

namespace TypeInternal {

/** \todo verify windows specific type demangling needs.
 *
 */
RA_DATAFLOW_API auto makeTypeReadable( const std::string& fullType ) -> std::string {
    static std::map<std::string, std::string> knownTypes {
        { "std::", "" },
        { "__cxx11::", "" },
        { ", std::allocator<float>", "" },
        { ", std::allocator<double>", "" },
        { ", std::allocator<int>", "" },
        { ", std::allocator<unsigned int>", "" },
        { "Ra::Core::VectorArray", "RaVector" },
        { "Ra::Core::Utils::ColorBase<float>", "Color" },
        { "Ra::Core::Utils::ColorBase<double>", "Color" },
        { "Eigen::Matrix<float, 2, 1, 0, 2, 1>", "Vector2" },
        { "Eigen::Matrix<double, 2, 1, 0, 2, 1>", "Vector2d" },
        { "Eigen::Matrix<int, 2, 1, 0, 2, 1>", "Vector2i" },
        { "Eigen::Matrix<unsigned int, 2, 1, 0, 2, 1>", "Vector2ui" },
        { "Eigen::Matrix<float, 3, 1, 0, 3, 1>", "Vector3" },
        { "Eigen::Matrix<double, 3, 1, 0, 3, 1>", "Vector3d" },
        { "Eigen::Matrix<int, 3, 1, 0, 3, 1>", "Vector3i" },
        { "Eigen::Matrix<unsigned int, 3, 1, 0, 3, 1>", "Vector3ui" },
        { "Eigen::Matrix<float, 4, 1, 0, 4, 1>", "Vector4" },
        { "Eigen::Matrix<double, 4, 1, 0, 4, 1>", "Vector4d" },
        { "Eigen::Matrix<int, 4, 1, 0, 4, 1>", "Vector4i" },
        { "Eigen::Matrix<unsigned int, 4, 1, 0, 4, 1>", "Vector4ui" },
        // Windows (visual studio 2022) specific name fix
        { " __ptr64", "" } };

    auto processedType = fullType;
    for ( const auto& [key, value] : knownTypes ) {
        Ra::Core::Utils::replaceAllInString( processedType, key, value );
    }
    return processedType;
}

} // namespace TypeInternal
} // namespace Core
} // namespace Dataflow
} // namespace Ra
