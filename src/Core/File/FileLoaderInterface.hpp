#ifndef RADIUMENGINE_FILELOADERINTERFACE_HPP
#define RADIUMENGINE_FILELOADERINTERFACE_HPP

#include <string>
#include <vector>

#include <Core/RaCore.hpp>

namespace Ra
{
    namespace Asset
    {
        class FileData;
    }
}

namespace Ra
{
    namespace Asset
    {
        class FileLoaderInterface
        {
        public:
            virtual ~FileLoaderInterface() {}

            virtual std::vector<std::string> getFileExtensions() const = 0;

            virtual bool handleFileExtension( const std::string& extension ) const = 0;

            virtual FileData * loadFile( const std::string& filename ) = 0;
        };
    }
}

#endif //RADIUMENGINE_FILELOADER_HPP
