#pragma once

#include <Core/Asset/FileLoaderInterface.hpp>
#include <IO/RaIO.hpp>

using std::string;
using std::vector;

namespace Ra {
namespace IO {

class RA_IO_API AsciiPointCloudLoader : public Ra::Core::Asset::FileLoaderInterface
{
  public:
    AsciiPointCloudLoader()          = default;
    virtual ~AsciiPointCloudLoader() = default;

    vector<string> getFileExtensions() const override;
    bool handleFileExtension( const std::string& extension ) const override;

    /** load vertices properties from ASCII .txt file
     *
     * expected content in file: header line + data records
     *
     * header line content : attribute names separated by blanks ( )
     *
     * example: Time X Y Z Roll Pitch Heading sdX sdY sdZ
     *
     * X, Y, Z and Time properties (case sensitive) are mandatory
     * for file loading. Failure to find any of these properties
     * results in file reading failure
     *
     * X, Y, Z properties are aggregated in geometry vertex array,
     * while all other properties are loaded as custom attribs in
     * Radium
     *
     * @todo add aggregation for normals/colors
     * @param [in] filename file path
     * @return nullptr if file opening fails, geometry creation
     * fails or one of the mandatory properties is not found
     */
    Ra::Core::Asset::FileData* loadFile( const std::string& filename ) override;
    string name() const override;
};

} // namespace IO
} // namespace Ra
