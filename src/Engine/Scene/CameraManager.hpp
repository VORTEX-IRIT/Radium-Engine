#pragma once

#include <Engine/RaEngine.hpp>
#include <Engine/Scene/CameraStorage.hpp>
#include <Engine/Scene/System.hpp>

#include <Engine/Scene/CameraComponent.hpp>
#include <memory>

namespace Ra {
namespace Engine {
namespace Data {
class CameraComponent;
}

namespace Scene {
/**
 * Interface providing functions to manage a group or type of Cameras
 * in a specific way.
 * Always have at least one camera (index 0) as "active".
 * Activate camera copy camera content from id to camera index 0.
 */
class RA_ENGINE_API CameraManager : public System
{
    // Radium-V2 : make Camera manager compatible with range for ...
  public:
    /// Constructor
    CameraManager();

    // Make copies impossible
    CameraManager( const CameraManager& ) = delete;
    CameraManager& operator=( const CameraManager& ) = delete;

    /// Virtual destructor
    ~CameraManager() override = default;

    /// Get a pointer to the i-th Camera.
    virtual const CameraComponent* getCamera( size_t i ) const = 0;

    /// Add a Camera to the manager ...
    virtual void addCamera( CameraComponent* cam ) = 0;
    virtual Core::Utils::Index getCameraIndex( const CameraComponent* cam );

    /// Add a default camera
    virtual void initialize();

    void activate( Core::Utils::Index index );

    //
    // Calls for the Renderer
    //

    /**
     * @brief Number of Cameras.
     * This is still a work in progress. The idea is to make it possible for a
     * CameraManager to tell it has only one Camera, for example if it wants to send
     * a lot of sources at once in a single RenderParams, let's say a texture.
     */
    // I dont' get the idea of texture camera storage here, event if it's possible for some kind of
    // multi view synthesis, the camera manager manages Components, do this idea of texture means
    // the components share some data where the know how to read/write the chunk ? (dlyr.)
    virtual size_t count() const;

    //
    // System methods
    //
    void generateTasks( Core::TaskQueue* taskQueue, const Engine::FrameInfo& frameInfo ) override;

    void handleAssetLoading( Entity* entity, const Core::Asset::FileData* data ) override;

    static Ra::Core::Asset::Camera defaultCamera;

  protected:
    /** Inherited method marked as final to ensure correct memory management
     *  even in child classes (e.g. CameraStorage).
     */
    void registerComponent( const Entity* entity, Component* component ) final;

    /// Inherited method marked as final to ensure correct memory management
    /// even in child classes (e.g. CameraStorage).
    void unregisterComponent( const Entity* entity, Component* component ) final;

    /// Inherited method marked as final to ensure correct memory management
    /// even in child classes (e.g. CameraStorage).
    void unregisterAllComponents( const Entity* entity ) final;

  protected:
    /// Stores the object that stores the Cameras...
    std::unique_ptr<CameraStorage> m_data {nullptr};
};

} // namespace Scene
} // namespace Engine
} // namespace Ra
