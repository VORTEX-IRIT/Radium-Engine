#pragma once
#include <Core/Utils/Observable.hpp>

#include <Engine/RaEngine.hpp>
#include <Engine/Scene/CameraStorage.hpp>
#include <Engine/Scene/System.hpp>

#include <memory>

namespace Ra {
namespace Engine {
namespace Scene {

/**
 * Interface providing functions to manage a group or type of Cameras
 * in a specific way.
 * CameraManager handle one "active camera" for rendering as m_activeCamera;
 * this rendering camera has no Component associated, and it can retrieve it's data from any managed
 * camera with activate(index).
 * Manipulator and renderer can access (and even store as a visiting ptr) this data with
 * getActiveCamera()
 * Default camera data is used to intializad camera. Client code can set this static data to control
 * default camera initialization.
 */
class RA_ENGINE_API CameraManager : public System
{
    // Radium-V2 : make Camera manager compatible with range for ...
  public:
    /// Constructor
    CameraManager();

    // Make copies impossible
    CameraManager( const CameraManager& )            = delete;
    CameraManager& operator=( const CameraManager& ) = delete;

    /// Virtual destructor
    ~CameraManager() override = default;

    /// Get a pointer to the i-th Camera.
    virtual const CameraComponent* getCamera( size_t i ) const = 0;

    /// Add a Camera to the manager ...
    virtual void addCamera( CameraComponent* cam ) = 0;

    /// get camera component's index, if this component is managed.
    /// \return valid index if component is managed, invalid index in other case.
    virtual Core::Utils::Index getCameraIndex( const CameraComponent* cam );

    /// Add a default camera
    virtual void initialize();

    /// activate camera at index given its index.
    /// index-th camera data is copied to the "active camera" stored explicitly by the manager.
    /// width and height of previous active camera is kept during this process, since it's likely
    /// to be screen size, while index-th camera width height might be something else.
    /// When a camera is activated, the frame of the active camera is computed to take the
    /// entity's transformation into account.
    /// \param index: camera's index to activate, if invalid or out of bound, activation is ignored.
    void activate( Core::Utils::Index index );

    /// update the active camera data
    void updateActiveCameraData();

    //
    // Calls for the Renderer
    //

    /**
     * \brief Number of managed Cameras.
     */
    virtual size_t count() const;

    //
    // System methods
    //
    void generateTasks( Core::TaskQueue* taskQueue, const Engine::FrameInfo& frameInfo ) override;

    void handleAssetLoading( Entity* entity, const Core::Asset::FileData* data ) override;

    /// this static data member handles default camera values.
    /// can be tweaked after ctor and before initalization, or for any kind of reset.
    static Ra::Core::Asset::Camera defaultCamera;

    /// Get the pointer on the active camera data
    Ra::Core::Asset::Camera* getActiveCamera() { return &m_activeCamera; }

    /// Get the index of the currently active camera
    Core::Utils::Index getActiveCameraIndex() const { return m_activeIndex; }

    /// reset the active camera data to default camera
    void resetActiveCamera() {
        m_activeCamera = defaultCamera;
        m_activeIndex  = -1;
        m_activeCameraObservers.notify( m_activeIndex );
    }

    /// get a ref to active camera observers to add/remove an observer
    inline Core::Utils::Observable<Core::Utils::Index>& activeCameraObservers() {
        return m_activeCameraObservers;
    }

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
    std::unique_ptr<CameraStorage> m_data { nullptr };

    /// active camera data, active camera hasn't any component just pure data.
    Ra::Core::Asset::Camera m_activeCamera;

    /// active camera index
    Core::Utils::Index m_activeIndex;

    /// Observers on active camera changes
    Core::Utils::Observable<Core::Utils::Index> m_activeCameraObservers;
};

} // namespace Scene
} // namespace Engine
} // namespace Ra
