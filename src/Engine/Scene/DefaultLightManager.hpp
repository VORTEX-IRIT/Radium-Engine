#pragma once

#include <Engine/RadiumEngine.hpp>
#include <Engine/Scene/Light.hpp>
#include <Engine/Scene/LightManager.hpp>

#include <memory>
#include <vector>

namespace Ra {
namespace Engine {
namespace Scene {

/**
 * Associated class.
 */
class RA_ENGINE_API DefaultLightStorage : public LightStorage
{
  public:
    DefaultLightStorage();
    void add( const Scene::Light* i ) override;
    void remove( const Scene::Light* li ) override;
    void upload() const override;
    size_t size() const override;
    void clear() override;
    const Scene::Light* operator[]( unsigned int n ) override;

  private:
    /** Multimap (by light type) of light references. */
    std::multimap<Scene::Light::LightType, const Scene::Light*> m_lights;
};

/**
 * \brief DefaultLightManager. A simple Light Manager with a list of lights.
 */
class RA_ENGINE_API DefaultLightManager : public LightManager
{
  public:
    DefaultLightManager();

    const Scene::Light* getLight( size_t li ) const override;
    void addLight( const Scene::Light* li ) override;
};

} // namespace Scene
} // namespace Engine
} // namespace Ra
