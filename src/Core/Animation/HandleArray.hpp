#pragma once

#include <Core/Animation/Pose.hpp>
#include <Core/RaCore.hpp>
#include <Core/Types.hpp>

namespace Ra {
namespace Core {
namespace Animation {

/**
 * Type for transform names
 */
using Label = std::string;

/**
 * The HandleArray class describes the base handle structure used for deformation.
 */
class RA_CORE_API HandleArray
{
  public:
    /**
     * The SpaceType specifies the space the transform lives in.
     */
    enum class SpaceType : bool { LOCAL, MODEL };
    static_assert( std::is_same<bool, typename std::underlying_type<SpaceType>::type>::value,
                   "SpaceType is not a boolean" );
    HandleArray();
    explicit HandleArray( const uint n );
    HandleArray( const HandleArray& handle )            = default;
    HandleArray& operator=( const HandleArray& handle ) = default;

    virtual ~HandleArray();

    /**
     * \return the number of transformations
     */
    inline virtual uint size() const { return m_pose.size(); }

    /**
     * Clear the transformations data
     */
    virtual void clear();

    /**
     * \return the current pose in the \p MODE space.
     */
    virtual const Pose& getPose( const SpaceType MODE ) const;

    /**
     * Set the current pose.
     * \param pose the new pose
     * \param MODE the space the psoe lives in
     */
    virtual void setPose( const Pose& pose, const SpaceType MODE );

    /**
     * \return the \p i-th transformation in \p MODE space.
     */
    virtual const Transform& getTransform( const uint i, const SpaceType MODE ) const;

    /**
     * Set the i-th transform to \p T, which is given in the MODE space.
     */
    virtual void setTransform( const uint i, const Transform& T, const SpaceType MODE );

    /**
     * \return the Handle's name
     */
    inline Label getName() const { return m_name; }

    /**
     * Set the Handle's name
     */
    inline void setName( const Label& name ) { m_name = name; }

    /**
     * \return the name of the \p i-th transform.
     */
    inline Label getLabel( const uint i ) const;

    /**
     * Set the name of the \p i-th transform.
     */
    inline void setLabel( const uint i, const Label& text );

    /**
     * The current pose of the Handle
     */
    LocalPose m_pose;

  protected:
    /**
     * The Handle's name
     */
    Label m_name;

    /**
     * The names of the transforms.
     */
    std::vector<Label> m_label;
};

inline Label HandleArray::getLabel( const uint i ) const {
    CORE_ASSERT( ( i < size() ), "Index i out of bound" );
    return m_label.at( i );
}

inline void HandleArray::setLabel( const uint i, const Label& text ) {
    CORE_ASSERT( ( i < size() ), "Index i out of bound" );
    m_label[i] = text;
}

} // namespace Animation
} // Namespace Core
} // Namespace Ra
