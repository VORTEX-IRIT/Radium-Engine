cmake_minimum_required(VERSION 3.18)

# -------------------------------------------------------------------------------
# exampleApp executables setup

project(RadiumExamples)

list(APPEND CMAKE_MESSAGE_INDENT "[Examples] ")

# short convenience target
add_custom_target(${PROJECT_NAME})
add_custom_target(Install_${PROJECT_NAME})

foreach(
    APP
    CoreExample
    CustomCameraManipulator
    DataflowExamples
    DrawPrimitives
    EntityAnimation
    EnvMap
    HelloRadium
    KeyEvent
    MaterialEdition
    ParameterEdition
    Picking
    RawShaderMaterial
    SimpleAnimation
    SimpleSimulation
    SimpleSkinning
    TexturedQuad
    TexturedQuadDynamic
)
    add_subdirectory(${APP})
    add_dependencies(${PROJECT_NAME} ${APP})
    add_dependencies(Install_${PROJECT_NAME} Install_${APP})
endforeach()

# VolumeDemoApp
find_package(Radium COMPONENTS IO REQUIRED)
get_target_property(HAS_VOLUMES Radium::IO IO_HAS_VOLUMES)
if(${HAS_VOLUMES})
    add_subdirectory(Volume)
    add_dependencies(${PROJECT_NAME} Volume)
    add_dependencies(Install_${PROJECT_NAME} Install_Volume)
endif()

# HeadlessDemo headless demo is available only if Radium::headless is available
find_package(Radium COMPONENTS Headless QUIET)
if(Radium_FOUND)
    add_subdirectory(HeadlessExample)
    add_dependencies(${PROJECT_NAME} HeadlessExample)
    add_dependencies(Install_${PROJECT_NAME} Install_HeadlessExample)
endif()

# Define specific installation prefix for this example to prevent mixed installation with Radium
# bundle
set(OLD_CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/PluginsExample")

add_subdirectory(PluginsWithLib EXCLUDE_FROM_ALL)
add_dependencies(${PROJECT_NAME} PluginsWithLib)
add_dependencies(Install_${PROJECT_NAME} Install_PluginsWithLib)

set(CMAKE_INSTALL_PREFIX "${OLD_CMAKE_INSTALL_PREFIX}")

list(REMOVE_AT CMAKE_MESSAGE_INDENT -1)
