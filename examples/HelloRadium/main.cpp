// Include Radium base application and its simple Gui
#include <Gui/BaseApplication.hpp>
#include <Gui/RadiumWindow/SimpleWindowFactory.hpp>

// include the Engine/entity/component interface
#include <Core/Geometry/MeshPrimitives.hpp>
#include <Engine/Scene/EntityManager.hpp>
#include <Engine/Scene/GeometryComponent.hpp>
#include <Engine/Scene/GeometrySystem.hpp>

#include <QTimer>

int main( int argc, char* argv[] ) {
    //! [Creating the application]
    Ra::Gui::BaseApplication app( argc, argv );
    glbinding::Version glVersion { 4, 4 };
    app.initialize( Ra::Gui::SimpleWindowFactory {}, glVersion );
    app.addRadiumMenu();
    //! [Creating the application]

    //! [Verifying the OpenGL version available to the engine]
    if ( glVersion != app.m_engine->getOpenGLVersion() ) {
        LOG( Ra::Core::Utils::logWARNING )
            << "OpenGL version mismatch : requested " << glVersion.toString() << " -- available "
            << app.m_engine->getOpenGLVersion().toString() << std::endl;
    }
    //! [Verifying the OpenGL version available to the engine]

    //! [Creating the cube]
    auto cube { std::move( Ra::Core::Geometry::makeSharpBox2( { 0.1f, 0.1f, 0.1f } ) ) };
    //! [Creating the cube]

    //! [Colorize the Cube]
    cube.addAttrib(
        Ra::Core::Geometry::getAttribName( Ra::Core::Geometry::VERTEX_COLOR ),
        Ra::Core::Vector4Array { cube.vertices().size(), Ra::Core::Utils::Color::Green() } );
    //! [Colorize the Cube]

    //! [Create the engine entity for the cube]
    auto e = app.m_engine->getEntityManager()->createEntity( "Green cube" );
    //! [Create the engine entity for the cube]

    //! [Create a geometry component with the cube]
    auto c = new Ra::Engine::Scene::GeometryDisplayableComponent(
        "Cube Mesh", e, std::move( cube ), nullptr );
    //! [Create a geometry component with the cube]

    //! [Register the entity/component association to the geometry system ]
    auto geometrySystem = app.m_engine->getSystem( "GeometrySystem" );
    geometrySystem->addComponent( e, c );
    //! [Register the entity/component association to the geometry system ]

    //! [Tell the window that something is to be displayed]
    app.m_mainWindow->prepareDisplay();
    //! [Tell the window that something is to be displayed]

    return app.exec();
}
