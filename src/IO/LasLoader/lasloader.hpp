#pragma once

#include <Core/Asset/FileLoaderInterface.hpp>
#include <Core/Utils/Attribs.hpp> //AttribManager, logs
#include <IO/RaIO.hpp>

using std::string;
using std::vector;

namespace Ra {
namespace IO {

class RA_IO_API LasLoader : public Ra::Core::Asset::FileLoaderInterface
{
  public:
    LasLoader()          = default;
    virtual ~LasLoader() = default;

    vector<string> getFileExtensions() const override;
    bool handleFileExtension( const std::string& extension ) const override;

    /** load vertices from .las file
     *
     * load x, y, z point coordinates properties and; depending on
     * data format parsed from header; red, green, blue and gps_time properties
     *
     * x, y, z coordinates are scaled according to scale and offset properties parsed
     * from the file header
     *
     * support LAS specification version from 1.1 to 1.4
     *
     * @note LAS specification lists other attributes that can be loaded
     * for each point record (intensity, edge of flight line, scan direction, ...) however
     * only the attributes mentionned above are loaded. See the official LAS specification
     * for more details <a
     * href="https://www.asprs.org/divisions-committees/lidar-division/laser-las-file-format-exchange-activities">
     * here</a>
     * @param [in] filename file path
     * @return nullptr if file opening or geometry creation fails
     */
    Ra::Core::Asset::FileData* loadFile( const std::string& filename ) override;
    string name() const override;
};

} // namespace IO
} // namespace Ra
