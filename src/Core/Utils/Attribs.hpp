#pragma once
#include <map>

#include <Core/Containers/VectorArray.hpp>
#include <Core/RaCore.hpp>
#include <Core/Utils/ContainerIntrospectionInterface.hpp>
#include <Core/Utils/Index.hpp>
#include <Core/Utils/Observable.hpp>

namespace Ra {
namespace Core {

namespace Geometry {
// need forward declarations for friend classes outside of Utils namespace
class TopologicalMesh;
} // namespace Geometry

namespace Utils {

template <typename T>
class Attrib;

/**
 * AttribBase is the base class for attributes of all type.
 */
class RA_CORE_API AttribBase : public ObservableVoid, public ContainerIntrospectionInterface
{
  public:
    inline explicit AttribBase( const std::string& name );
    virtual ~AttribBase();
    AttribBase( const AttribBase& )            = delete;
    AttribBase& operator=( const AttribBase& ) = delete;

    /// Return the attribute's name.
    inline std::string getName() const;

    ///    Set the attribute's name.
    inline void setName( const std::string& name );

    /// Resize the attribute's array.
    virtual void resize( size_t s ) = 0;

    /// Return true if *this and \p rhs have the same name.
    bool inline operator==( const AttribBase& rhs );

    /// Downcast from AttribBase to Attrib<T>.
    template <typename T>
    inline Attrib<T>& cast();

    /// Downcast from AttribBase to Attrib<T>.
    template <typename T>
    inline const Attrib<T>& cast() const;

    /// Return true if the attribute content is of Scalar type, false otherwise.
    virtual bool isFloat() const = 0;

    /// Return true if the attribute content is of Vector2 type, false otherwise.
    virtual bool isVector2() const = 0;

    /// Return true if the attribute content is of Vector3 type, false otherwise.
    virtual bool isVector3() const = 0;

    /// Return true if the attribute content is of Vector4 type, false otherwise.
    virtual bool isVector4() const = 0;

    /// Return true if data is locked, i.e. has been locked for write access with
    /// getDataWithlock() (defined in subclass Attrib). Double lock is prohebited, so when finished,
    /// call unlock();
    bool inline isLocked() const;

    /// Unlock data so another one can gain write access.
    void inline unlock();

    virtual std::unique_ptr<AttribBase> clone() = 0;

  protected:
    void inline lock( bool isLocked = true );

  private:
    /// The attribute's name.
    std::string m_name;

    /// Is data access locked by a user ?
    bool m_isLocked { false };
};

/**
 * An Attrib stores an element of type \p T for each entry.
 */
template <typename T>
class Attrib : public AttribBase
{
  public:
    using value_type = T;
    using Container  = VectorArray<T>;

    explicit Attrib( const std::string& name );
    virtual ~Attrib();

    /// Resize the container (value_type must have a default ctor).
    void resize( size_t s ) override;

    /// Read-write access to the attribute content.
    /// lock the content, when done call unlock()
    inline Container& getDataWithLock();

    /// @{
    /// ContainerIntrosectionInterface implementation
    size_t getSize() const override;
    size_t getNumberOfComponents() const override;
    int getStride() const override;
    size_t getBufferSize() const override;
    const void* dataPtr() const override;
    /// @}

    ///@{
    /// setAttribData, attrib mustn't be locked (it's asserted).
    void setData( const Container& data );
    void setData( Container&& data );
    ///@}

    /// Read-only acccess to the attribute content.
    inline const Container& data() const;
    bool isFloat() const override;
    bool isVector2() const override;
    bool isVector3() const override;
    bool isVector4() const override;

    /// check if attrib is a given type, as in attr.isType<MyMatrix>()
    template <typename U>
    bool isType();

    std::unique_ptr<AttribBase> clone() override {
        auto ptr    = std::make_unique<Attrib<T>>( getName() );
        ptr->m_data = m_data;
        return ptr;
    }

  private:
    Container m_data;
};

/// An attrib handle basically store an Index and a name.
template <typename T>
class AttribHandle
{
  public:
    using value_type = T;
    using AttribType = Attrib<T>;
    using Container  = typename Attrib<T>::Container;
    /// compare two handle, there are the same if they both represent the same
    /// attrib (type and value).
    template <typename U>
    bool operator==( const AttribHandle<U>& lhs ) const {
        return std::is_same<T, U>::value && m_idx == lhs.idx();
    }

    /// return the index of the attrib.
    Index idx() const { return m_idx; }

    /// return the name of the attrib.
    /// attrib name are unique in a given attribManager.
    std::string attribName() const { return m_name; }

  private:
    Index m_idx        = Index::Invalid();
    std::string m_name = "";

    friend class AttribManager;
};

/*!
 * \brief The AttribManager provides attributes management by handles.
 *
 * The AttribManager stores a container of AttribBase *, which can
 * be accessed (#getAttrib) and deleted (#removeAttrib) using a AttribHandle.
 * Handles are created from an attribute name using #addAttrib, and retrieved
 * using #findAttrib.
 *
 * Example of typical use case:
 * \code
 * // somewhere: creation
 * AttribManager mng;
 * auto inputfattrib = mng.addAttrib<Scalar>("MyAttrib");
 *
 * ...
 *
 * // somewhere else: access
 * auto iattribhandler = mng.findAttrib<Scalar>("MyAttrib"); //  iattribhandler == inputfattrib
 * if ( mng.isValid( iattribhandler ) ) { // true
 *     auto &attrib = mng.getAttrib( iattribhandler );
 *     ...
 * }
 * auto& iattribhandler = mng.findAttrib<Scalar>("InvalidAttrib"); // invalid
 * if ( mng.isValid( iattribhandler ) ) { // false
 *    ...
 * }
 * \endcode
 *
 * \warning There is no error check on the handles attribute type.
 *
 */
class RA_CORE_API AttribManager : public Observable<const std::string&>
{
  public:
    using value_type         = AttribBase;
    using pointer_type       = value_type*;
    using smart_pointer_type = std::unique_ptr<value_type>;
    using Container          = std::vector<smart_pointer_type>;

    inline AttribManager();

    /// Copy constructor and assignment operator are forbidden.
    AttribManager( const AttribManager& m )            = delete;
    AttribManager& operator=( const AttribManager& m ) = delete;

    inline AttribManager( AttribManager&& m );

    inline AttribManager& operator=( AttribManager&& m );
    ~AttribManager() override;

    /// Base copy, does nothing.
    inline void copyAttributes( const AttribManager& m );

    /// Copy the given attributes from \p m.
    /// \note If some attrib already exists, it will be replaced.
    /// \note Invalid handles are ignored.
    template <class T, class... Handle>
    void copyAttributes( const AttribManager& m, const AttribHandle<T>& attr, Handle... attribs );

    /// Copy all attributes from \p m.
    /// \note If some attrib already exists, it will be replaced.
    /// \todo allow to copy all attributes, even those with non-standard type
    void copyAllAttributes( const AttribManager& m );

    /// clear all attribs, invalidate handles.
    void clear();

    /// Return true if \p h correspond to an existing attribute in *this.
    template <typename T>
    bool isValid( const AttribHandle<T>& h ) const;

    /*!
     * \brief contains Check if an attribute with the given \p name exists.
     * \param name Name of the attribute.
     * \warning There is no error check on the attribute type.
     * \complexity \f$ O(\log(n)) \f$
     */
    inline bool contains( const std::string& name ) const;

    /*!
     * \brief findAttrib Grab an attribute handler by \p name.
     * \param name Name of the attribute.
     * \return Attribute handler if found, an invalid handler otherwise.
     * \warning There is no error check on the attribute type.
     * \complexity \f$ O(\log(n)) \f$
     */
    template <typename T>
    inline AttribHandle<T> findAttrib( const std::string& name ) const;

    /*!
     * \brief Get the locked data container from the attrib handle
     * \tparam T The type of the attribute data
     * \param h the attribute handle
     * \return the attribute's data container
     */
    template <typename T>
    typename Attrib<T>::Container& getDataWithLock( const AttribHandle<T>& h );

    /*!
     * \brief Get read access to the data container from the attrib handle
     * \tparam T The type of the attribute data
     * \param h the attribute handle
     * \return the attribute's data container
     */
    template <typename T>
    const typename Attrib<T>::Container& getData( const AttribHandle<T>& h );

    /*!
     * \brief Unlock the handle data
     * \tparam T
     * \param h
     * \return
     */
    template <typename T>
    void unlock( const AttribHandle<T>& h );

    ///@{
    /// Get attribute by handle.
    /// \complexity \f$ O(1) \f$
    /// \warning There is no check on the handle validity. Attrib is statically cast to T without
    /// other checks.
    ///
    template <typename T>
    inline Attrib<T>& getAttrib( const AttribHandle<T>& h );
    template <typename T>
    inline const Attrib<T>& getAttrib( const AttribHandle<T>& h ) const;
    template <typename T>
    inline Attrib<T>* getAttribPtr( const AttribHandle<T>& h );
    template <typename T>
    inline const Attrib<T>* getAttribPtr( const AttribHandle<T>& h ) const;
    ///@}

    ///@{
    /// Get attribute by name.
    /// First search name to index correpondance \complexity \f$ O(getNumAttribs()) \f$
    /// \warning There is no check on the name validity. Attrib is statically cast to T without
    /// other checks.
    template <typename T>
    inline Attrib<T>& getAttrib( const std::string& name );
    template <typename T>
    inline const Attrib<T>& getAttrib( const std::string& name ) const;
    ///@}

    ///@{
    /// Return a AttribBase ptr to the attrib identified by name.
    /// to give access to AttribBase method, regardless of the type of element
    /// stored in the attrib.
    inline AttribBase* getAttribBase( const std::string& name );
    inline const AttribBase* getAttribBase( const std::string& name ) const;
    inline AttribBase* getAttribBase( const Index& idx );
    inline const AttribBase* getAttribBase( const Index& idx ) const;
    ///@}

    ///@{
    /// Set the data of the attrib h.
    /// \warning \p h has to be a valid attrib handle, this is not checked.
    template <typename T>
    inline void setAttrib( const AttribHandle<T>& h,
                           const typename AttribHandle<T>::Container& data );

    /// \see #setAttrib
    template <typename T>
    inline void setAttrib( const AttribHandle<T>& h, typename AttribHandle<T>::Container&& data );
    ///@}

    /// Add attribute by name.
    /// notify(name) is called to trigger the observers
    /// \note If an attribute with the same name already exists,
    ///       just returns a AttribHandle to it.
    /// \complexity \f$ O(n) \f$
    template <typename T>
    AttribHandle<T> addAttrib( const std::string& name );

    /// Remove attribute by handle, invalidates the handle.
    /// notify(name) is called to trigger the observers
    /// \warning If a new attribute is added, old invalidated handles may lead to
    ///          the new attribute.
    /// \complexity \f$ O(\log(n)) \f$
    template <typename T>
    void removeAttrib( AttribHandle<T>& h );
    /// Return true if *this and \p other have the same attributes, same amount
    /// and same names.
    /// \warning There is no check on the attribute type nor data.
    bool hasSameAttribs( const AttribManager& other );

    ///@{
    /// Perform \p fun on each attribute.
    // This is needed by the user to avoid caring about removed attributes (nullptr)
    // \todo reimplement as range for
    template <typename F>
    void for_each_attrib( const F& func ) const;

    // \todo keep non const version private
    template <typename F>
    void for_each_attrib( const F& func );
    ///@}

    /// Return the number of attributes
    inline int getNumAttribs() const;

    /**
     * \brief Scope lock state management for attributes.
     *
     * Unlock all attribs locked after the creation of the ScopedLockState object when it
     * gets out of scope.
     * \code{.cpp}
     * {
     * auto g = new AttribArrayGeometry();
     *
     * // get write access. The attrib must be explicitly unlocked after usage
     * auto& attrib1 = g->getAttrib<Ra::Core::Vector3>( "attrib1" ).getDataWithLock();
     * // ... write to attribOne
     *
     * // enable auto-unlock for all following write access requests
     * auto unlocker = g->vertexAttribs().getScopedLockState();
     *
     * // get write access to several attribs
     * auto& attrib2 = g->getAttrib<Ra::Core::Vector3>( "attrib2" ).getDataWithLock();
     * // ... write to attrib2
     * auto& attrib3 = g->getAttrib<Ra::Core::Vector3>( "attrib3" ).getDataWithLock();
     * // ... write to attrib3
     *
     * // attrib1 is still locked as it was locked outside of the unlocker scope.
     * // Must be explicitly unlocked.
     * attrib1.unlock();
     *
     * // attrib2 and attrib3 are automatically unlocked when the unlocker's destructor
     * // is called (i.e. gets out of scope)
     * }
     * \endcode
     */
    class ScopedLockState
    {
      public:
        /**
         * \param a the AttribManager on which locking will be supervised
         * \brief Constructor, save lock state of all attribs from attribManager
         */
        explicit ScopedLockState( AttribManager* a ) {
            a->for_each_attrib( [this]( const auto& attr ) {
                return ( v.push_back( std::make_pair( attr, attr->isLocked() ) ) );
            } );
        }
        /**
         * \brief Destructor, unlock all attribs whose have been locked after the
         * initialization of the Unlocker.
         */
        ~ScopedLockState() {
            for ( auto& p : v ) {
                if ( !p.second && p.first->isLocked() ) { p.first->unlock(); }
            }
        }

      private:
        std::vector<std::pair<AttribBase*, bool>> v;
    };

    /// Returns a scope unlocker for managed attribs
    ScopedLockState getScopedLockState() { return ScopedLockState { this }; }

  private:
    /// Attrib list, better using attribs() to go through.
    Container m_attribs;

    /// Map between the attrib's name and its index, used to speedup finding the handle index from
    /// the attribute name.
    std::map<std::string, Index> m_attribsIndex;

    // Ease wrapper
    friend class ::Ra::Core::Geometry::TopologicalMesh;

    /// Count number of valid attribs
    int m_numAttribs { 0 };
};

AttribBase::AttribBase( const std::string& name ) : m_name { name } {}

std::string AttribBase::getName() const {
    return m_name;
}

void AttribBase::setName( const std::string& name ) {
    m_name = name;
}

bool inline AttribBase::operator==( const AttribBase & rhs ) {
    return m_name == rhs.getName();
}

template <typename T>
Attrib<T>& AttribBase::cast() {
    return static_cast<Attrib<T>&>( *this );
}

template <typename T>
const Attrib<T>& AttribBase::cast() const {
    return static_cast<const Attrib<T>&>( *this );
}

bool AttribBase::isLocked() const {
    return m_isLocked;
}

void AttribBase::unlock() {
    lock( false );
}

void AttribBase::lock( bool isLocked ) {
    CORE_ASSERT( isLocked != m_isLocked, "double (un)lock" );
    m_isLocked = isLocked;
    if ( !m_isLocked ) notify();
}

/////////////// Attrib ///////////////////

template <typename T>
Attrib<T>::Attrib( const std::string& name ) : AttribBase( name ) {}

template <typename T>

Attrib<T>::~Attrib() {
    m_data.clear();
}
template <typename T>
void Attrib<T>::resize( size_t s ) {
    m_data.resize( s );
}
template <typename T>
typename Attrib<T>::Container& Attrib<T>::getDataWithLock() {
    lock();
    return m_data;
}

template <typename T>
const void* Attrib<T>::dataPtr() const {
    return m_data.dataPtr();
}

template <typename T>
void Attrib<T>::setData( const Container& data ) {
    CORE_ASSERT( !isLocked(), "try to set onto locked data" );
    m_data = data;
    notify();
}

template <typename T>
void Attrib<T>::setData( Container&& data ) {
    CORE_ASSERT( !isLocked(), "try to set onto locked data" );
    m_data = std::move( data );
    notify();
}

template <typename T>
const typename Attrib<T>::Container& Attrib<T>::data() const {
    return m_data;
}

template <typename T>
size_t Attrib<T>::getSize() const {
    return m_data.getSize();
}

template <typename T>
int Attrib<T>::getStride() const {
    return m_data.getStride();
}

template <typename T>
size_t Attrib<T>::getBufferSize() const {
    return m_data.getBufferSize();
}

template <typename T>
bool Attrib<T>::isFloat() const {
    return std::is_same<Scalar, T>::value;
}

template <typename T>
bool Attrib<T>::isVector2() const {
    return std::is_same<Eigen::Matrix<Scalar, 2, 1>, T>::value;
}

template <typename T>
bool Attrib<T>::isVector3() const {
    return std::is_same<Eigen::Matrix<Scalar, 3, 1>, T>::value;
}

template <typename T>
bool Attrib<T>::isVector4() const {
    return std::is_same<Eigen::Matrix<Scalar, 4, 1>, T>::value;
}

template <typename T>
template <typename U>
bool Attrib<T>::isType() {
    return std::is_same<U, T>::value;
}

// Defer computation to VectorArrayTypeHelper
template <typename T>
size_t Attrib<T>::getNumberOfComponents() const {
    return m_data.getNumberOfComponents();
}

/////////////////// AttribManager ///////////////////

AttribManager::AttribManager() {}

AttribManager::AttribManager( AttribManager&& m ) :
    m_attribs( std::move( m.m_attribs ) ),
    m_attribsIndex( std::move( m.m_attribsIndex ) ),
    m_numAttribs( std::move( m.m_numAttribs ) ) {}

AttribManager& AttribManager::operator=( AttribManager&& m ) {
    m_attribs      = std::move( m.m_attribs );
    m_attribsIndex = std::move( m.m_attribsIndex );
    m_numAttribs   = std::move( m.m_numAttribs );
    return *this;
}

void AttribManager::copyAttributes( const AttribManager& m ) {
    m_numAttribs = m.m_numAttribs;
}

template <class T, class... Handle>
void AttribManager::copyAttributes( const AttribManager& m,
                                    const AttribHandle<T>& attr,
                                    Handle... attribs ) {
    if ( m.isValid( attr ) ) {
        // get attrib to copy
        auto& a = m.getAttrib( attr );
        // add new attrib
        auto h = addAttrib<T>( a.getName() );
        // copy attrib data
        getAttrib( h ).setData( a.data() );
    }
    // deal with other attribs
    copyAttributes( m, attribs... );
}

template <typename T>
bool AttribManager::isValid( const AttribHandle<T>& h ) const {
    auto itr = m_attribsIndex.find( h.attribName() );
    return h.m_idx != Index::Invalid() && itr != m_attribsIndex.end() && itr->second == h.m_idx;
}

inline bool AttribManager::contains( const std::string& name ) const {
    return m_attribsIndex.find( name ) != m_attribsIndex.end();
}

template <typename T>
inline AttribHandle<T> AttribManager::findAttrib( const std::string& name ) const {
    auto c = m_attribsIndex.find( name );
    AttribHandle<T> handle;
    if ( c != m_attribsIndex.end() ) {
        handle.m_idx  = c->second;
        handle.m_name = c->first;
    }
    return handle;
}

template <typename T>
typename Attrib<T>::Container& AttribManager::getDataWithLock( const AttribHandle<T>& h ) {
    return static_cast<Attrib<T>*>( m_attribs.at( h.m_idx ).get() )->getDataWithLock();
}

template <typename T>
const typename Attrib<T>::Container& AttribManager::getData( const AttribHandle<T>& h ) {
    return static_cast<Attrib<T>*>( m_attribs.at( h.m_idx ).get() )->data();
}

template <typename T>
void AttribManager::unlock( const AttribHandle<T>& h ) {
    static_cast<Attrib<T>*>( m_attribs.at( h.m_idx ).get() )->unlock();
}

template <typename T>
inline Attrib<T>& AttribManager::getAttrib( const AttribHandle<T>& h ) {
    return *static_cast<Attrib<T>*>( m_attribs.at( h.m_idx ).get() );
}

template <typename T>
inline const Attrib<T>& AttribManager::getAttrib( const AttribHandle<T>& h ) const {
    return *static_cast<Attrib<T>*>( m_attribs.at( h.m_idx ).get() );
}
template <typename T>
inline Attrib<T>& AttribManager::getAttrib( const std::string& name ) {
    return getAttrib( findAttrib<T>( name ) );
}

template <typename T>
inline const Attrib<T>& AttribManager::getAttrib( const std::string& name ) const {
    return getAttrib( findAttrib<T>( name ) );
}

template <typename T>
inline Attrib<T>* AttribManager::getAttribPtr( const AttribHandle<T>& h ) {
    return static_cast<Attrib<T>*>( m_attribs.at( h.m_idx ).get() );
}

template <typename T>
inline const Attrib<T>* AttribManager::getAttribPtr( const AttribHandle<T>& h ) const {
    return static_cast<Attrib<T>*>( m_attribs.at( h.m_idx ).get() );
}

template <typename T>
inline void AttribManager::setAttrib( const AttribHandle<T>& h,
                                      const typename AttribHandle<T>::Container& data ) {
    static_cast<Attrib<T>*>( m_attribs.at( h.m_idx ).get() )->setData( data );
}

template <typename T>
inline void AttribManager::setAttrib( const AttribHandle<T>& h,
                                      typename AttribHandle<T>::Container&& data ) {
    static_cast<Attrib<T>*>( m_attribs.at( h.m_idx ).get() )->setData( data );
}

inline AttribBase* AttribManager::getAttribBase( const std::string& name ) {
    auto c = m_attribsIndex.find( name );
    if ( c != m_attribsIndex.end() ) return m_attribs[c->second].get();
    return nullptr;
}

inline const AttribBase* AttribManager::getAttribBase( const std::string& name ) const {
    auto c = m_attribsIndex.find( name );
    if ( c != m_attribsIndex.end() ) return m_attribs[c->second].get();
    return nullptr;
}

inline AttribBase* AttribManager::getAttribBase( const Index& idx ) {
    if ( idx.isValid() ) return m_attribs[idx].get();
    return nullptr;
}

inline const AttribBase* AttribManager::getAttribBase( const Index& idx ) const {
    if ( idx.isValid() ) return m_attribs[idx].get();
    return nullptr;
}

template <typename T>
AttribHandle<T> AttribManager::addAttrib( const std::string& name ) {
    // does the attrib already exist?
    AttribHandle<T> h = findAttrib<T>( name );
    if ( isValid( h ) ) return h;

    // create the attrib
    smart_pointer_type attrib = std::make_unique<Attrib<T>>( name );

    // look for a free slot
    auto it = std::find_if(
        m_attribs.begin(), m_attribs.end(), []( const auto& attr ) { return !attr; } );
    if ( it != m_attribs.end() ) {
        it->swap( attrib );
        h.m_idx = std::distance( m_attribs.begin(), it );
    }
    else {
        m_attribs.push_back( std::move( attrib ) );
        h.m_idx = m_attribs.size() - 1;
    }
    m_attribsIndex[name] = h.m_idx;
    h.m_name             = name;
    ++m_numAttribs;

    notify( name );
    return h;
}

template <typename T>
void AttribManager::removeAttrib( AttribHandle<T>& h ) {
    auto c = m_attribsIndex.find( h.m_name );
    if ( c != m_attribsIndex.end() ) {
        Index idx = c->second;
        m_attribs[idx].reset( nullptr );
        m_attribsIndex.erase( c );
    }
    h.m_idx   = Index::Invalid(); // invalidate whatever!
    auto name = h.m_name;
    h.m_name  = ""; // invalidate whatever!
    --m_numAttribs;
    notify( name );
}

template <typename F>
void AttribManager::for_each_attrib( const F& func ) const {
    for ( const auto& attr : m_attribs )
        if ( attr != nullptr ) func( attr.get() );
}

template <typename F>
void AttribManager::for_each_attrib( const F& func ) {
    for ( auto& attr : m_attribs )
        if ( attr != nullptr ) func( attr.get() );
}

int AttribManager::getNumAttribs() const {
    return m_numAttribs;
}

} // namespace Utils
} // namespace Core
} // namespace Ra
