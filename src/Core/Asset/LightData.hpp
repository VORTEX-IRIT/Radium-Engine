#pragma once

#include <Core/Asset/AssetData.hpp>
#include <Core/RaCore.hpp>
#include <Core/Utils/Color.hpp>
#include <Core/Utils/Log.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Ra {
namespace Core {
namespace Asset {

/**
 * Asset defining how lights must be passed to the Radium systems from a loader.
 *
 * Supported light type are :
 *    Point light : a local, punctual, isotropic spherical light.
 *    Spot light : a local, punctual, isotropic conical light.
 *    Directional light : an infinite, isotropic light.
 *    Area light : a local isotropic area light.
 */
class RA_CORE_API LightData : public AssetData
{

  public:
    /** Supported light type
     *
     */
    enum LightType {
        UNKNOWN           = 1 << 0,
        POINT_LIGHT       = 1 << 1,
        SPOT_LIGHT        = 1 << 2,
        DIRECTIONAL_LIGHT = 1 << 3,
        AREA_LIGHT        = 1 << 4
    };

    /**
     * Define the parameters of the attenuation function.
     * The only supported attenuation function is 1/ (constant + linear*dist + quadradic*dist^2 )
     */
    struct LightAttenuation {
        Scalar constant;
        Scalar linear;
        Scalar quadratic;
        explicit LightAttenuation( Scalar c = 1, Scalar l = 0, Scalar q = 0 ) :
            constant( c ), linear( l ), quadratic( q ) {}
    };
    // TODO : allow to define other attenuation function such
    // \max\left(0,1-\frac{\left|x\right|^2}{r^2}\right)^2

    /**
     * Light constructor.
     * Must be called explicitely with a name for the light and its type.
     */
    explicit LightData( const std::string& name = "", const LightType& type = UNKNOWN );

    /**
     * Copy constructor.
     * As a Light a a union like object, no default copy constructor could be generated. This will
     * take care of copying the good members of the union.
     * \param data
     */
    LightData( const LightData& data );

    /// Destructor
    ~LightData();

    /**
     * Extension of of the AssetData interface so that we can modify an abcet afeter ts creation.
     * This has no impact on the coherence of the LightData object and could be used without
     * restriction.
     */
    inline void setName( const std::string& name );

    /**
     * Acces to the local frame of the light.
     * \return the local frame
     */
    inline const Eigen::Matrix<Scalar, 4, 4>& getFrame() const;

    /**
     * Set the local frame of the light.
     * \param frame the local frame
     */
    inline void setFrame( const Eigen::Matrix<Scalar, 4, 4>& frame );

    /**
     * Construct a directional light.
     * A directional light is only defined by its color and its lighting direction.
     * No attenuation on directional.
     *  \note The object on which this method is called is unconditionally promoted to
     * ``DIRECTIONAL_LIGHT`` light, whatever it was before the call and only the directional light
     * part of the union is consistent after the call
     */
    inline void setLight( const Core::Utils::Color& color,
                          const Eigen::Matrix<Scalar, 3, 1>& direction );

    /**
     * Construct a point light.
     * A point light is defined by its color, its position and its attenuation.
     *  \note The object on which this method is called is unconditionally promoted to
     * ``POINT_LIGHT`` light, whatever it was before the call and only the directional light part of
     * the union is consistent after the call
     */
    inline void setLight( const Core::Utils::Color& color,
                          const Eigen::Matrix<Scalar, 3, 1>& position,
                          LightAttenuation attenuation );

    /**
     * Construct a spot light.
     * A spot light is defined by its color, its position, the cone axis and light distribution
     * (constant inside inangle, quadratically deacreasing toward 0 fron inAngle to ouAngle) and its
     * attenuation. \note The object on which this method is called is unconditionally promoted to
     * ``SPOT_LIGHT`` light, whatever it was before the call and only the directional light part of
     * the union is consistent after the call
     */
    inline void setLight( const Core::Utils::Color& color,
                          const Eigen::Matrix<Scalar, 3, 1>& position,
                          const Eigen::Matrix<Scalar, 3, 1>& direction,
                          Scalar inAngle,
                          Scalar outAngle,
                          LightAttenuation attenuation );

    /**
     * Construct a area light.
     * An area light, (isotropic with constant emision defined by its color) is approximated by its
     * center and the covariance matrices modelling the spatial extent as well as the elliptical
     * distribution of normals. \note The object on which this method is called is unconditionally
     * promoted to ``AREA_LIGHT`` light, whatever it was before the call and only the directional
     * light part of the union is consistent after the call
     */
    inline void setLight( const Core::Utils::Color& color,
                          const Eigen::Matrix<Scalar, 3, 1>& cog,
                          const Eigen::Matrix<Scalar, 3, 3>& spatialCov,
                          const Eigen::Matrix<Scalar, 3, 3>& normalCov,
                          LightAttenuation attenuation );

    /**
     * Access to the type of the object
     */
    inline LightType getType() const;

    /**
     * Returns true if the light is a PointLight
     */
    inline bool isPointLight() const;

    /**
     * Returns true if the light is a SpotLight
     */
    inline bool isSpotLight() const;

    /**
     * Returns true if the light is a DirectionalLight
     */
    inline bool isDirectionalLight() const;

    /**
     * Returns true if the light is an AreaLight
     */
    inline bool isAreaLight() const;

    /**
     * For debugging purpose, prints out the formated content of the LightData object.
     */
    inline void displayInfo() const;

  protected:
    /// VARIABLE

    Eigen::Matrix<Scalar, 4, 4> m_frame;
    LightType m_type;

    // This part is public so that systems handling lights could access to the data.
    // TODO : make these protected with getters ? Define independant types ?
  public:
    Core::Utils::Color m_color;

    struct DirLight {
        Eigen::Matrix<Scalar, 3, 1> direction;
    };
    struct PointLight {
        Eigen::Matrix<Scalar, 3, 1> position;
        LightAttenuation attenuation;
    };
    struct SpotLight {
        Eigen::Matrix<Scalar, 3, 1> position;
        Eigen::Matrix<Scalar, 3, 1> direction;
        Scalar innerAngle;
        Scalar outerAngle;
        LightAttenuation attenuation;
    };
    struct AreaLight {
        // TODO : this representation is usefull but might be improved
        Eigen::Matrix<Scalar, 3, 1> position;
        Eigen::Matrix<Scalar, 3, 3> spatialCovariance;
        Eigen::Matrix<Scalar, 3, 3> normalCovariance;
        LightAttenuation attenuation;
    };

    union {
        DirLight m_dirlight;
        PointLight m_pointlight;
        SpotLight m_spotlight;
        AreaLight m_arealight;
    };
};

/////////////////////
///  LIGHT DATA   ///
/////////////////////

/// NAME
inline void LightData::setName( const std::string& name ) {
    m_name = name;
}

/// TYPE
inline LightData::LightType LightData::getType() const {
    return m_type;
}

/// FRAME
inline const Eigen::Matrix<Scalar, 4, 4>& LightData::getFrame() const {
    return m_frame;
}

inline void LightData::setFrame( const Eigen::Matrix<Scalar, 4, 4>& frame ) {
    m_frame = frame;
}

/// construct a directional light
inline void LightData::setLight( const Core::Utils::Color& color,
                                 const Eigen::Matrix<Scalar, 3, 1>& direction ) {
    m_type               = DIRECTIONAL_LIGHT;
    m_color              = color;
    m_dirlight.direction = direction;
}

/// construct a point light
inline void LightData::setLight( const Core::Utils::Color& color,
                                 const Eigen::Matrix<Scalar, 3, 1>& position,
                                 LightAttenuation attenuation ) {
    m_type                   = POINT_LIGHT;
    m_color                  = color;
    m_pointlight.position    = position;
    m_pointlight.attenuation = attenuation;
}

/// construct a spot light
inline void LightData::setLight( const Core::Utils::Color& color,
                                 const Eigen::Matrix<Scalar, 3, 1>& position,
                                 const Eigen::Matrix<Scalar, 3, 1>& direction,
                                 Scalar inAngle,
                                 Scalar outAngle,
                                 LightAttenuation attenuation ) {
    m_type                  = SPOT_LIGHT;
    m_color                 = color;
    m_spotlight.position    = position;
    m_spotlight.direction   = direction;
    m_spotlight.innerAngle  = inAngle;
    m_spotlight.outerAngle  = outAngle;
    m_spotlight.attenuation = attenuation;
}

/// construct an area light
inline void LightData::setLight( const Core::Utils::Color& color,
                                 const Eigen::Matrix<Scalar, 3, 1>& cog,
                                 const Eigen::Matrix<Scalar, 3, 3>& spatialCov,
                                 const Eigen::Matrix<Scalar, 3, 3>& normalCov,
                                 LightAttenuation attenuation ) {
    m_type                        = AREA_LIGHT;
    m_color                       = color;
    m_arealight.position          = cog;
    m_arealight.spatialCovariance = spatialCov;
    m_arealight.normalCovariance  = normalCov;
    m_arealight.attenuation       = attenuation;
}

/// QUERY
inline bool LightData::isPointLight() const {
    return ( m_type == POINT_LIGHT );
}

inline bool LightData::isSpotLight() const {
    return ( m_type == SPOT_LIGHT );
}

inline bool LightData::isDirectionalLight() const {
    return ( m_type == DIRECTIONAL_LIGHT );
}

inline bool LightData::isAreaLight() const {
    return ( m_type == AREA_LIGHT );
}

/// DEBUG
inline void LightData::displayInfo() const {
    using namespace Core::Utils; // log
    std::string type;
    switch ( m_type ) {
    case POINT_LIGHT:
        type = "POINT LIGHT";
        break;
    case SPOT_LIGHT:
        type = "SPOT LIGHT";
        break;
    case DIRECTIONAL_LIGHT:
        type = "DIRECTIONAL LIGHT";
        break;
    case AREA_LIGHT:
        type = "AREA LIGHT";
        break;
    case UNKNOWN:
    default:
        type = "UNKNOWN";
        break;
    }
    LOG( logINFO ) << "======== LIGHT INFO ========";
    LOG( logINFO ) << " Name           : " << m_name;
    LOG( logINFO ) << " Type           : " << type;
}

} // namespace Asset
} // namespace Core
} // namespace Ra
