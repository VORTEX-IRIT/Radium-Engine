#pragma once

#include <Core/Utils/Color.hpp>
#include <Engine/RaEngine.hpp>
#include <Engine/Scene/Component.hpp>

#include <Engine/Data/RenderParameters.hpp>

namespace Ra {
namespace Engine {
namespace Scene {

class Entity;

// Radium-V2 : this class could totally be renamed LightComponent and get a Light struct embedded.
// Thoughts are welcome !
/**
 * Light base class for rendering
 */
class RA_ENGINE_API Light : public Component
{
  public:
    /** supported light type.
     * \note POLYGONAL is not really supported.
     * \todo : replace this enum by something extensible as we plan to add new light type by plugins
     */
    enum LightType : int { DIRECTIONAL = 0, POINT, SPOT, POLYGONAL };

    /**
     * Define the attenuation of the light source
     * \todo for the moment, the attenuation is non physically coherent. make attenuation be
     * quadratic by default
     */
    struct Attenuation {
        Scalar constant { 1 };
        Scalar linear { 0 };
        Scalar quadratic { 0 };
        Attenuation() = default;
    };

  public:
    /**
     * Create a new light. The new light is a component attached to the given entity
     * \param entity
     * \param type
     * \param name
     */
    Light( Entity* entity, const LightType& type, const std::string& name = "light" );
    ~Light() override = default;

    /**
     *
     * \return the color of the light
     */
    inline const Core::Utils::Color& getColor() const { return m_color; }
    /**
     * Set the color of the light
     * \param color
     */
    inline void setColor( const Core::Utils::Color& color );

    /**
     * set the direction of lighting
     * \todo put this only on directional and spot light sources
     * \param dir
     */
    virtual void setDirection( const Eigen::Matrix<Scalar, 3, 1>& /*dir*/ ) {}
    /**
     * Set the position of the light source
     * \todo put this only on point and spot light sources
     * \param pos
     */
    virtual void setPosition( const Eigen::Matrix<Scalar, 3, 1>& /*pos*/ ) {}
    // ...

    /**
     *
     * \return the type (an enum Light::LightType member) of the light source
     */
    inline const LightType& getType() const { return m_type; }

    /**
     * \brief Extract the set of parameters that must be given to a shader for rendering and
     * lighting with this light.
     * \param params parameters to be filled (using merge-replace) with the one of the light
     */
    void getRenderParameters( Data::RenderParameters& params ) const;

    /**
     * \brief Gives read-only access to the renderParameters of the light.
     * \return a const reference to the light parameters for rendering
     */
    const Data::RenderParameters& getRenderParameters() const { return m_params; }

    /**
     * \brief Gives read-write access to the renderParameters of the light.
     * \return a const reference to the light parameters for rendering
     */
    Data::RenderParameters& getRenderParameters() { return m_params; }
    /**
     * Abstract method that define the glsl code that manage this light type
     * For the moment, this is not use (except by experimental plugins) but will be part of the
     * shader composition process of radium v2
     * \return
     */
    virtual std::string getShaderInclude() const;

    /**
     * Nothing to initialize here ...
     */
    void initialize() override;

  private:
    Data::RenderParameters m_params;
    Core::Utils::Color m_color = Core::Utils::Color::White();
    LightType m_type { LightType::DIRECTIONAL };
};

// ---------------------------------------------------------------------------------------------
// ---- inline methods implementation

inline void Light::setColor( const Core::Utils::Color& color ) {
    m_color = color;
    m_params.setVariable( "light.color", m_color );
}

} // namespace Scene
} // namespace Engine
} // namespace Ra
