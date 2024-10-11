#include "Engine/Data/Texture.hpp"

#include <catch2/catch.hpp>

#include <Core/CoreMacros.hpp>
#include <Engine/Data/BlinnPhongMaterial.hpp>
#include <Engine/Data/LambertianMaterial.hpp>
#include <Engine/Data/PlainMaterial.hpp>
#include <Engine/Data/VolumetricMaterial.hpp>
#include <Engine/OpenGL.hpp>
#include <Engine/RadiumEngine.hpp>

#include <Headless/CLIViewer.hpp>
#include <glbinding/gl/enum.h>
#include <globjects/Texture.h>
#include <memory>
#ifdef HEADLESS_HAS_EGL
#    include <Headless/OpenGLContext/EglOpenGLContext.hpp>
#else
#    include <Headless/OpenGLContext/GlfwOpenGLContext.hpp>
#endif

using namespace Ra::Headless;
using namespace Ra::Engine::Data;
using namespace Ra::Core::Utils;
using namespace gl;
using namespace gl45core;

TEST_CASE( "Engine/Data/Texture", "[Engine][Engine/Data][Textures]" ) {
    // Get the Engine and materials initialized
    glbinding::Version glVersion { 4, 4 };
#ifdef HEADLESS_HAS_EGL
    CLIViewer viewer { std::make_unique<EglOpenGLContext>( glVersion ) };
#else
    CLIViewer viewer { std::make_unique<GlfwOpenGLContext>( glVersion ) };
#endif
    auto dummy_name = "unittest_textures";
    auto code       = viewer.init( 1, &dummy_name );
    viewer.setCamera();

    SECTION( "Texture Init Now" ) {
        REQUIRE( code == 0 );
        viewer.bindOpenGLContext( true );
        GLuint id1, id2;
        TextureParameters params = { {}, {} };
        {
            Texture texture1( params );
            Texture texture2( params );

            texture1.initializeNow();
            texture2.initializeNow();

            auto gpuTexture1 = texture1.getGpuTexture();
            auto gpuTexture2 = texture2.getGpuTexture();

            REQUIRE( gpuTexture1 != nullptr );
            REQUIRE( gpuTexture2 != nullptr );

            id1 = gpuTexture1->id();
            id2 = gpuTexture2->id();

            REQUIRE( id1 != id2 );

            REQUIRE( glIsTexture( id1 ) );
            REQUIRE( glIsTexture( id2 ) );

            texture1.destroyNow();

            REQUIRE( !glIsTexture( id1 ) );
            REQUIRE( texture1.getGpuTexture() == nullptr );
            REQUIRE( glIsTexture( id2 ) );
            REQUIRE( texture2.getGpuTexture() == gpuTexture2 );
        }

        viewer.bindOpenGLContext( true );
        viewer.oneFrame();

        REQUIRE( !glIsTexture( id1 ) );
        REQUIRE( !glIsTexture( id2 ) );
    }

    SECTION( "Texture Init Delayed" ) {
        REQUIRE( code == 0 );
        viewer.bindOpenGLContext( true );
        GLuint id1, id2;
        TextureParameters params = { {}, {} };
        {
            Texture texture1( params );
            Texture texture2( params );

            texture1.initialize();
            texture2.initialize();

            auto gpuTexture1 = texture1.getGpuTexture();
            auto gpuTexture2 = texture2.getGpuTexture();

            REQUIRE( gpuTexture1 == nullptr );
            REQUIRE( gpuTexture2 == nullptr );

            viewer.oneFrame();

            gpuTexture1 = texture1.getGpuTexture();
            gpuTexture2 = texture2.getGpuTexture();

            REQUIRE( gpuTexture1 != nullptr );
            REQUIRE( gpuTexture2 != nullptr );

            id1 = gpuTexture1->id();
            id2 = gpuTexture2->id();

            REQUIRE( id1 != id2 );

            REQUIRE( glIsTexture( id1 ) );
            REQUIRE( glIsTexture( id2 ) );

            texture1.destroy();

            // id1 is still texture
            REQUIRE( glIsTexture( id1 ) );
            // while gpuTexture ptr directly reset
            REQUIRE( texture1.getGpuTexture() == nullptr );

            viewer.oneFrame();

            REQUIRE( !glIsTexture( id1 ) );
            REQUIRE( texture1.getGpuTexture() == nullptr );

            REQUIRE( glIsTexture( id2 ) );
            REQUIRE( texture2.getGpuTexture() == gpuTexture2 );
            // this will register delayed update tasks, that will be removed in dtor
            texture2.setParameters( { {}, {} } );
        }

        viewer.bindOpenGLContext( true );
        viewer.oneFrame();

        REQUIRE( !glIsTexture( id1 ) );
        REQUIRE( !glIsTexture( id2 ) );
    }

    SECTION( "Texture Update and Resize" ) {
        REQUIRE( code == 0 );

        auto data1 = std::shared_ptr<uchar[]>( new uchar[3] );

        data1[0] = 0;
        data1[1] = 1;
        data1[2] = 2;

        std::array<std::shared_ptr<uchar[]>, 6> data2 {
            { std::shared_ptr<uchar[]>( new uchar[3] ),
              std::shared_ptr<uchar[]>( new uchar[3] ),
              std::shared_ptr<uchar[]>( new uchar[3] ),
              std::shared_ptr<uchar[]>( new uchar[3] ),
              std::shared_ptr<uchar[]>( new uchar[3] ),
              std::shared_ptr<uchar[]>( new uchar[3] ) } };
        std::array<std::shared_ptr<void>, 6> data2void {
            { data2[0], data2[1], data2[2], data2[3], data2[4], data2[5] } };

        TextureParameters params1 = { {}, {} };
        TextureParameters params2 = { {}, {} };

        params1.image.texels = data1;
        params2.image.target = GL_TEXTURE_CUBE_MAP;
        params2.image.texels = data2void;

        Texture texture1( params1 );
        Texture texture2( params2 );

        texture1.initialize();
        texture2.initialize();
        viewer.bindOpenGLContext( true );
        viewer.oneFrame();

        REQUIRE( texture1.getParameters().image.isTexelOfType<ImageParameters::ImageType>() );
        REQUIRE( texture1.getTexels() != nullptr );
        REQUIRE( static_cast<const uchar*>( texture1.getTexels() )[0] == 0 );
        REQUIRE( static_cast<const uchar*>( texture1.getTexels() )[1] == 1 );
        REQUIRE( static_cast<const uchar*>( texture1.getTexels() )[2] == 2 );

        auto gpuTexture1 = texture1.getGpuTexture();
        REQUIRE( gpuTexture1 != nullptr );
        auto id1 = gpuTexture1->id();

        REQUIRE( glIsTexture( id1 ) );

        auto readData1 = gpuTexture1->getImage( 0, params1.image.format, params1.image.type );

        REQUIRE( readData1[0] == data1[0] );
        REQUIRE( readData1[1] == data1[1] );
        REQUIRE( readData1[2] == data1[2] );

        data1[0] = 4;
        data1[1] = 5;
        data1[2] = 6;

        texture1.updateData( data1 );
        // update data is dalayed
        readData1 = gpuTexture1->getImage( 0, params1.image.format, params1.image.type );

        REQUIRE( readData1[0] == 0 );
        REQUIRE( readData1[1] == 1 );
        REQUIRE( readData1[2] == 2 );

        // one frame will update data
        viewer.oneFrame();
        readData1 = gpuTexture1->getImage( 0, params1.image.format, params1.image.type );

        REQUIRE( readData1[0] == data1[0] );
        REQUIRE( readData1[1] == data1[1] );
        REQUIRE( readData1[2] == data1[2] );

        // resize send data immediately
        auto data1Resized = std::shared_ptr<uchar[]>( new uchar[3 * 4] );
        for ( uchar i = 0; i < 3 * 4; ++i )
            data1Resized[i] = i;
        texture1.resize( 2, 2, 1, data1Resized );
        readData1 = gpuTexture1->getImage( 0, params1.image.format, params1.image.type );
        // see  GL_PACK_ROW_LENGTH
        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPixelStore.xhtml
        int a;
        glGetIntegerv( GL_PACK_ALIGNMENT, &a );
        const int n = 3;
        const int l = 2;
        const int s = 1;
        int k       = s >= a ? n * l : a / s * (int)std::ceil( float( s * n * l ) / a );
        // 2 x 2 pixels, with 3 components each
        for ( size_t i = 0; i < 2; ++i ) {
            for ( size_t j = 0; j < l; ++j ) {
                for ( size_t c = 0; c < n; ++c ) {
                    size_t readIndex = i * k + j * n + c;
                    size_t dataIndex = i * ( l * n ) + j * n + c;
                    REQUIRE( readData1[readIndex] == data1Resized[dataIndex] );
                }
            }
        }
    }
}
