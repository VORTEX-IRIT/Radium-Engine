#pragma once
#include <Core/RaCore.hpp>

#include <Core/Utils/Log.hpp>
#include <Core/Utils/StdExperimentalTypeTraits.hpp>
#include <Core/Utils/StdOptional.hpp>
#include <Core/Utils/TypesUtils.hpp>

#include <any>
#include <functional>
#include <map>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Ra {
namespace Core {

/// \brief Heterogeneous container storing "Variables", that maps a name (std::string) to a value
/// (of any type T).
///
/// The container is compatible with any type, from simple arithmetic type to wrapped references to
/// any other type.
///
/// "Variables" can be added, accessed and removed from the container given their type, name and
/// value.
///
/// Given a type, the container gives access to the mapping name->value for all variables with the
/// given type.
///
/// Stored variables can be referenced using a VariableHandle returned when a variable is inserted
/// in the container or fetch giving the type and the name of the variable. Validity of the variable
/// handle follows the same rules than the std::iterator attached to the underlying mapping storage
/// (std::map<std::string, T>::iterator for the actual implementation).
///
/// From a variable handle (possibly invalid), the mapping name->value for all variables with the
/// same type can be fetched. This allows generic, type agnostic, usage of the container as soon as
/// a variable handle (even invalid) is available.
///
/// The container could be visited to apply processing, transformation, ... on all or part of the
/// variables. The accept functions of the visitor are defined by the availability of a callable
/// object whose profile is compatible with the visited types.
///
/// Visiting the container can be made
///  - using a statically typed visitor : accepted types are defined at compile time,
/// as Ra::Core::Utils::TypeList, and the visit is pre-processed by the compiler.
///  - using a dynamically configurable visitor where functor accepting types can be added/removed
/// at runtime. This kind of visit is a little more expensive while being more configurable.
/// Tests using empty processing (to evaluate only the cost of visiting the collection) on different
/// types showed a visit from 5 to 8 times slower. This penalty becomes quite low as soon as the
/// processing during the visit is more complex
///
/// The visitor of the collection can accept one user parameter to be forwarded to each visit
/// function.
/// This parameter could be of any type, knowing the same parameter will be forwarded to all
/// processing method when visiting a variable.
/// Some constraints on user provided parameter depends on the visiting strategy
///   - For static visitors, this parameter is strongly typed and ALL the visiting function should
///     be called with the profile void(const std::string&, [const]T[&], [const]U&&), for any
///     variable type T and the user parameter type U.
///   - For dynamic visitors, as they rely on type erasure pattern, the user parameter should be of
///     any type U but this type is erased when calling the visiting function. So, visiting
///     functions must be callable with the profile void(const std::string&, [const]T[&],
///     [const]std::anyU&&) knowing that the std::any contains a parameter of type U.
///
/// \note relies on C++ standard libraries extensions, version 2 or recommended implementation on
/// compilers that do not provide this extension
/// https://en.cppreference.com/w/cpp/experimental/lib_extensions_2
///
/// \see VariableSet unit test source code for examples of usage of this container
class RA_CORE_API VariableSet
{
  public:
    /// \brief Container type for the mapping name->value of variables with type T.
    template <typename T>
    using VariableContainer = std::map<std::string, T>;

    /// \brief Variable type as stored in the VariableSet
    template <typename T>
    using Variable = typename VariableContainer<T>::value_type;

    /// \brief Type of the variable value
    template <typename T>
    using VariableType = typename Variable<T>::second_type;

    /// \brief Handle of a variable
    /// A handle on a variable with type T is an iterator into the BaseContainer.
    /// De-referencing the handle give access to a non const pair
    /// <const std::string, T> (BaseContainer<T>::value_type).
    /// VariableHandle validity follows the rules of BaseContainer<T>::iterator validity.
    template <typename T>
    using VariableHandle = typename VariableContainer<T>::iterator;

    /// \brief Type of the variable referenced by a VariableHandle
    /// \note This is the same as the one denoted by the type T in VariableHandle<T> or in
    /// VariableType<T>
    template <typename H>
    using VariableTypeFromHandle = typename std::iterator_traits<H>::value_type::second_type;

    // ----------------------------------------------------------
    /// Constructors, destructors
    /// \{
    VariableSet() : m_vtable( VariableSetFunctions::getInstance() ) {}
    ~VariableSet() = default;
    /// A VariableSet is copyable
    VariableSet( const VariableSet& other ) noexcept :
        m_vtable( VariableSetFunctions::getInstance() ) {
        *this = other;
    }
    /// A VariableSet is movable
    VariableSet( VariableSet&& other ) noexcept : m_vtable( VariableSetFunctions::getInstance() ) {
        *this = std::move( other );
    }
    /// \}

    // ------------------------------------------------------------------------------------------
    // Global variable set operation
    // ------------------------------------------------------------------------------------------
    /// Operators acting on a the whole VariableSet
    /// \{
    /// \brief Copy assignment operator
    auto operator=( const VariableSet& other ) -> VariableSet&;

    /// \brief Move assignment operator
    auto operator=( VariableSet&& other ) noexcept -> VariableSet&;

    /// \brief remove all elements from the container
    void clear();

    /// \brief Merge the VariableSet \b from into this
    /// \param from the VariableSet to merge with the current.
    /// Existing variable into this, with same name as in from, are kept unchanged
    /// \see mergeReplaceVariables
    void mergeKeepVariables( const VariableSet& from );

    /// \brief Merge the VariableSet \b from into this
    /// \param from the VariableSet to merge with the current.
    /// Existing variable into this, with same name as in from, are replaced by from's one.
    /// \see mergeKeepVariables
    void mergeReplaceVariables( const VariableSet& from );

    /// \brief Gets the total number of variables (of any type)
    size_t size() const;

    /// \brief Gets the stored data type
    /// \return constant vector of type_index
    auto getStoredTypes() const -> const std::vector<std::type_index>& { return m_storedType; }

    /// \}
    // ------------------------------------------------------------------------------------------
    // Per variable operations
    // ------------------------------------------------------------------------------------------
    /// Operators acting on a per variable basis
    /// \{
    /// \brief Add a variable, i.e. an association name->value, into the container
    /// \return true if the variable is inserted, false if the name was already associated with
    /// another value (of the same type). In this case, keep the old value.
    ///
    template <typename T>
    auto insertVariable( const std::string& name, const T& value )
        -> std::pair<VariableHandle<T>, bool>;

    /// \brief get the value of the given variable
    /// \return a reference to the value.
    /// \pre The element \b name must exists with type \b T. If not verified (assert in debug mode)
    /// std::bad_any_cast exception could be thrown by the underlying management of type erasure
    template <typename T>
    auto getVariable( const std::string& name ) const -> const T&;
    template <typename T>
    auto getVariable( const std::string& name ) -> T&;

    /// \brief get the handle on the variable with the given name
    /// \tparam T the type of the variable
    /// \param name the name of a variable
    /// \return an handle which can be de-referenced to obtain a std::pair<const std::string, T>
    /// representing the name and the value of the variable.
    template <typename T>
    auto getVariableHandle( const std::string& name ) const -> const VariableHandle<T>;

    /// \brief Test the validity of a handle
    /// \tparam H Type of the handle. Expected to be VariableHandle<T> for some variable type T
    /// \param handle the variable handle
    /// \return true if the handle is valid, false if not.
    template <typename H>
    bool isHandleValid( const H& handle ) const;

    /// \brief reset (or set if the variable does not exist yet) the value of the variable.
    /// \return a pair with the variable handle and a bool : true if the variable value was reset,
    /// false if the variable value was set.
    template <typename T>
    auto setVariable( const std::string& name, const T& value )
        -> std::pair<VariableHandle<T>, bool>;

    /// \brief Remove a variable, i.e. a name->value association
    /// \return true if the variable was removed, false if
    /// \pre The element \b name must exists with type \b T. If not verified (assert in debug mode)
    /// std::bad_any_cast exception could be thrown by the underlying management of type erasure
    template <typename T>
    bool deleteVariable( const std::string& name );

    /// \brief delete a variable from its handle
    /// \tparam H Type of the handle. Expected to be VariableHandle<T> for some variable type T
    /// \param handle the variable handle
    /// \return true variable was removed, false if not.
    /// If the variable was removed, handle is invalidated
    /// \pre the handle must be valid. If not verified (assert in debug mode)
    /// std::bad_any_cast exception could be thrown by the underlying management of type erasure
    template <typename H>
    bool deleteVariable( H& handle );

    /// \brief test the existence of the given variable
    /// \return an optional variable handle which contains a value if a variable with the given
    /// name and type exists in the storage.
    template <typename T>
    auto existsVariable( const std::string& name ) const -> Utils::optional<VariableHandle<T>>;

    /// \}

    // ------------------------------------------------------------------------------------------
    // Per type access
    // ------------------------------------------------------------------------------------------
    /// Operators acting on a per type basis
    /// \{
    /// \brief Test if the storage supports a given variable type
    /// \tparam T The type of variable to test
    /// \return an optional, empty if the type does not exists in the VariableSet or whose value is
    /// a **non owning** pointer to the variable collection if it exists. This **non owning**
    /// pointer remains valid as long as the VariableSet exists and contains the given type.
    template <typename T>
    auto existsVariableType() const -> Utils::optional<VariableContainer<T>*>;

    /// \brief Removes all variables of the given type.
    /// \tparam T The type of variable to remove
    /// \return true if the type is deleted, false if it was not managed before the call.
    /// Unregister all the functions associated with the type and remove the storage of
    /// existing variables. If several variables are still stored, they will be destroyed.
    template <typename T>
    bool deleteAllVariables();

    /// \brief Get the whole container for variables of a given type
    /// \tparam T The variable type to get
    /// \return a reference to the storage of the mapping name->value for the given type.
    /// \pre existsVariableType<T>(). If not verified (assert in debug mode) std::bad_any_cast
    /// exception will be thrown by the underlying management of type erasure
    template <typename T>
    auto getAllVariables() const -> VariableContainer<T>&;

    /// \brief Get the whole container for variables of the same type than the given handled
    /// variable. \tparam H Type of the variable handle, should be VariableHandle<T> for some type T
    /// \param handle the handle to an existing variable
    /// \return a reference to the storage of the mapping name->value for the given type.
    /// \pre existsVariableType<HandledType<H>>(). If not verified (assert in debug mode)
    /// std::bad_any_cast exception will be thrown by the underlying management of type erasure
    template <typename H>
    auto getAllVariablesFromHandle( const H& handle )
        -> VariableContainer<VariableTypeFromHandle<H>>&;

    /// \brief Get the number of variables of the given type
    /// \tparam T The type to test
    /// \return the number of variables with type T stored in the container
    template <typename T>
    size_t numberOf() const;
    /// \}

    /// Visiting operators
    /// \{

    /// \brief Base class for visitors with static supported types.
    /// Visiting will be prepared at compile time by unfolding type list and generating all
    /// the required function calls for the visit.
    /// Any class that defines the same alias for a public member "types" can be used as a visitor.
    template <typename... TYPES>
    struct StaticVisitor {
        using types = Utils::TypeList<TYPES...>;
    };

    /// \brief Base class for dynamically configurable visitors
    /// Users can implement this interface to build custom visitors without any restriction.
    /// To ease visitor configuration, see class DynamicVisitor
    /// \see DynamicVisitor
    class DynamicVisitorBase
    {
      public:
        virtual ~DynamicVisitorBase() = default;

        /// \brief Execute a visiting operator on accepted types
        /// \param in The variable to process
        /// \param userParam The optional user define parameter to forward to type associated
        /// functor The variable \b in contains a wrapping of the association name->value whose
        /// visit is accepted. Visiting the association is done by calling the visit operator
        /// associated with the underlying type of the variable \b in.
        virtual void operator()( std::any&& in, std::any&& userParam ) const = 0;

        /// \brief Acceptance function for the visitor
        /// \param id the std::type_index associated to the type to visit
        /// \return true if the type is visitable, false if not
        [[nodiscard]] virtual bool accept( const std::type_index& id ) const = 0;
    };

    /// \brief Base class for visitors with configurable per-type callbacks.
    /// Visiting will be prepared at running time by dynamically adding visitor operators for each
    /// type one want to visit in the container.
    /// The visitor accept type based on either the added operators or an external acceptance
    /// functor which can be associated/changed at runtime. This functor is only allowed to reduce
    /// the number of visited type as only type for which a visitor operator is registered can be
    /// visited.
    /// The visitor operators should be any callable that accept to be called using
    /// f(const std::string&, T&[, std::any&&])
    class RA_CORE_API DynamicVisitor : public DynamicVisitorBase
    {
      public:
        /// allows the class to be derived
        ~DynamicVisitor() override = default;

        /// \brief Execute a visiting operator on accepted types
        /// \param in The variable to process
        /// \param userParam The optional user define parameter to forward to type associated
        /// functor The variable \b in contains a wrapping of the association name->value whose
        /// visit is accepted. Visiting the association is done by calling the visit operator
        /// associated with the underlying type of the variable \b in.
        void operator()( std::any&& in, std::any&& userParam ) const override;

        /// \brief Acceptance function for the visitor
        /// \param id The type to test
        /// \return true if the type is visitable, false if not
        [[nodiscard]] bool accept( const std::type_index& id ) const override;

        /// \brief Add a visiting operator.
        /// \tparam T The accepted type for the visit.
        /// \tparam F The operator type, a callable with profile void(const std::string&, [const
        /// ]T[&], std::any&&).
        /// \param f The operator object.
        /// \return true if the operator was added, false if not (e.g. there is already an operator
        /// associated with the type T)
        template <typename T, typename F>
        bool addOperator( F&& f );

        /// \brief Test the existence of an operator associated with a type
        /// \tparam T
        /// \return
        template <typename T>
        bool hasOperator();

        /// \brief Add or replace a visiting operator
        /// \tparam T
        /// \tparam F
        /// \param f
        /// This method is similar to DynamicVisitor::addOperator but replace any existing operator
        /// previously associated with the type T
        template <typename T, typename F>
        void addOrReplaceOperator( F&& f );

        /// \brief Remove a visiting operator
        /// \tparam T
        /// \return
        /// \pre The type \b T must be accepted by an operator.
        template <typename T>
        bool removeOperator();

      private:
        /// \brief Type of the callback function that type-erased the user registered operators
        using CallbackFunction = std::function<void( std::any&, std::any&& )>;

        /// \brief Storage of the type erased operators
        using OperatorsStorageType = std::unordered_map<std::type_index, CallbackFunction>;

        /// \brief Helper struct to build type-erased operators (allowing to test calling profile)
        template <typename T, typename F, bool WithUserParam>
        struct MakeVisitOperatorHelper {
            auto makeOperator( F& f ) -> OperatorsStorageType::value_type;
        };

        /// \brief construct a type-erased operator from a user define functor with profile
        /// void(const std::string, [const]T[&] [, [const] std::any&&])
        template <typename T, typename F>
        auto makeVisitorOperator( F& f ) -> OperatorsStorageType::value_type;

        /// \brief The type erased operators
        OperatorsStorageType m_visitorOperator;
    };

    /// \brief Visit the container using a user defined visitor
    /// \tparam F The type of the visitor to use (could be dynamic or static - see above )
    /// \param visitor The visitor object to use
    /// The type of the visiting functor F should be
    ///     - either derived from VariableSet::StaticVisitor<Type1, Type2, ...> with function
    ///     operators with profile const operator(const std::string& name, T& value) available for
    ///     all the requested types Type1, Type2, ...
    ///     - either a user define class exposing a type list to unfold F::types
    ///     (see Utils::TypeList and VariableSet::StaticVisitor)
    ///     - either derived from DynamicVisitor. In this case, visiting will be less efficient.
    template <typename F>
    void visit( F&& visitor ) const;

    /// \brief overload of the static visit method to allow a parameter pass by reference
    template <typename F, typename T>
    void visit( F&& visitor, T& userParams ) const;

    /// \brief overload of the static visit method to allow a parameter pass by rvalue reference
    template <typename F, typename T>
    void visit( F&& visitor, T&& userParams ) const;

  private:
    /// \brief Visit the container using a dynamically typed visitor
    /// \tparam P The type of the parameter to pass to visitor operators (see below)
    /// \param visitor The visitor to use
    /// \param params  optional parameter to forward to visitor functor
    ///
    /// This visiting method is adapted when the types to visit are only known at running time.
    /// At running time, this visiting approach relies on two loops.
    ///   - The first loop, done by the visiting logic in the class VariableSet iterate over
    ///     the types stored for the mappings name->value to identify the accepted types by the
    ///     visitor.
    ///   - The second loop, done by the visiting logic for accepted types, loop over
    ///     name->value mappings and call the visitor functor for each pair
    ///
    /// The type of the visiting functor F should be
    ///   - either derived from VariableSet::DynamicVisitor with the needed visiting operators
    ///     registered (\see see VariableSet::DynamicVisitor)
    ///   - either derived directly from DynamicVisitorBase to build a full custom visitor. In this
    ///     case, the first given std::any rvalue reference wraps a reference to a
    ///     std::pair<const std::string, T> such
    ///     that the first element of the pair is the name of the variable, and the second its
    ///     value of type T. The second std::any&& rvalue reference contains an optional parameter
    ///     to pass to the visiting functors (the same for all types) if the accept to be called
    ///     with such a parameter.
    template <typename P = bool>
    void visitDynamic( DynamicVisitorBase& visitor, P&& params = P {} ) const;

    /// \brief Visit the container using a statically typed visitor
    /// \tparam F The type of the visitor to use (see below)
    /// \param visitor The visitor object to use
    ///
    /// This visiting method is well adapted when the visited types are known at compile time.
    /// In this case, part of the visit logic is done at compile time by unfolding the call of the
    /// type-related visitors instead of looping over all the stored type.
    /// This visiting strategy loops on visited types at compile time (loop unrolled) then
    /// on each stored values at running time. So, this visiting strategy is more efficient than
    /// the dynamic one if visited types are always the same.
    ///
    /// The type of the visiting functor F should be
    ///     - either derived from VariableSet::StaticVisitor<Type1, Type2, ...> with function
    ///     operators with profile const operator(const std::string& name, T& value) available for
    ///     all the requested types Type1, Type2, ...
    ///     - either a user define class exposing a type list to unfold F::types
    ///     (see Utils::TypeList and VariableSet::StaticVisitor)
    template <typename F>
    void visitStatic( F&& visitor ) const;

    /// \brief overload of the static visit method to allow a parameter pass by reference
    template <typename F, typename T>
    void visitStatic( F&& visitor, T& userParams ) const;

    /// \brief overload of the static visit method to allow a parameter pass by rvalue reference
    template <typename F, typename T>
    void visitStatic( F&& visitor, T&& userParams ) const;

    /// \brief Helper function that associate an index to a type
    /// \tparam T The type to index
    /// \return the identifier index of the type
    template <typename T>
    static auto getVariableVisitTypeIndex() -> std::type_index;

    /// \brief Add support for a given type.
    /// \tparam T The type to manage
    /// \return true if the type was correctly inserted
    /// This method create functions for destroying, copying, inspecting and visiting variables
    /// of the given type.
    template <typename T>
    auto addVariableType() -> Utils::optional<VariableContainer<T>*>;

    /// Implementation of static visitor
    /// \{
    /// \brief Callable profile of F for variable of type T
    template <typename F, typename T>
    using VisitFunction =
        decltype( std::declval<F>().operator()( std::declval<const std::string&>(),
                                                std::declval<T&>() ) );

    /// \brief Callable profile of F for variable of type T and user parameter of type U&&
    template <typename F, typename T, typename U>
    using VisitFunctionWithUserParam =
        decltype( std::declval<F>().operator()( std::declval<const std::string&>(),
                                                std::declval<T&>(),
                                                std::declval<U&&>() ) );

    /// \brief test if the type F has a callable operator with the right profile
    template <typename F, typename T>
    static constexpr bool has_visit_callable_v =
        Ra::Core::Utils::is_detected<VisitFunction, F, T>::value;

    /// \brief test if the type F has a callable operator with the right profile
    template <typename F, typename T, typename U>
    static constexpr bool has_visit_callable_with_user_param_v =
        Ra::Core::Utils::is_detected<VisitFunctionWithUserParam, F, T, U>::value;

    /// unfold type list to generate visitor func call
    template <typename F, template <typename...> typename TYPESLIST, typename... TYPES>
    void visitImpl( F&& visitor, TYPESLIST<TYPES...> ) const;

    template <typename F, typename T>
    void visitImplHelper( F& visitor ) const;

    /// unfold type list to generate visitor func call
    template <typename F, typename U, template <typename...> typename TYPESLIST, typename... TYPES>
    void visitImplUserParam( F&& visitor, U&& userParam, TYPESLIST<TYPES...> ) const;

    template <typename F, typename U, typename T>
    void visitImplHelperUserParam( F& visitor, U&& userParams ) const;

    /// \}

    /// Storage for the data management functions
    /// \{
    /// Type of the constructed visit function that returns a lambda

    /// Shared VariableSetFunctions among VariableSet instances.
    class VariableSetFunctions
    {
      public:
        using ClearFunctionType = std::function<void( VariableSet& )>;
        using MergeFunctionType = std::function<void( const VariableSet&, VariableSet& )>;
        using SizeFunctionType  = std::function<size_t( const VariableSet& )>;
        // Should be these simplified ????
        using VisitFunctorType =
            std::function<std::pair<bool, std::function<void( DynamicVisitorBase&, std::any&& )>>(
                const VariableSet&,
                const DynamicVisitorBase& )>;

        VariableSetFunctions( const VariableSetFunctions& ) = delete;
        void operator=( const VariableSetFunctions& )       = delete;

      protected:
        VariableSetFunctions()  = default;
        ~VariableSetFunctions() = default;

      public:
        // static variable ref singleton implementation.
        static auto getInstance() -> VariableSetFunctions*;

        std::vector<MergeFunctionType> m_mergeKeepFunctions;
        std::vector<MergeFunctionType> m_mergeReplaceFunctions;
        std::vector<SizeFunctionType> m_sizeFunctions;
        std::vector<VisitFunctorType> m_visitFunctions;
        std::vector<std::type_index> m_storedType;
    };

    VariableSetFunctions* m_vtable;

    /// \}

    /// Storage management
    /// \{
    // Storage of the variable in a type-erased associative container
    mutable std::unordered_map<std::type_index, std::any> m_variables;
    std::unordered_map<std::type_index, size_t> m_typeIndexToVtableIndex;
    /// cache for m_variables keys, could be removed by c++ 20 range view
    std::vector<std::type_index> m_storedType;

    /// \brief Initialize an empty storage for variables of type T
    /// \tparam T
    /// \return a raw pointer on the storage collection
    template <typename T>
    auto createVariableStorage() -> VariableContainer<T>*;

    /// \brief Access to the storage for variables of type T
    /// \tparam T
    template <typename T>
    auto getVariableStorage() const -> VariableContainer<T>&;

    /// \brief destroy the storage for variables of type T
    /// \tparam T
    template <typename T>
    void removeVariableStorage();

    /// \}
};

// ------------------------------------------------------------------------------------------
// Storage management
// ------------------------------------------------------------------------------------------

template <typename T>
auto VariableSet::createVariableStorage() -> VariableContainer<T>* {
    m_variables[std::type_index { typeid( T ) }].emplace<VariableContainer<T>>();
    return std::any_cast<VariableContainer<T>>( &( m_variables[std::type_index { typeid( T ) }] ) );
}

template <typename T>
auto VariableSet::getVariableStorage() const -> VariableContainer<T>& {
    return std::any_cast<VariableContainer<T>&>( m_variables[std::type_index { typeid( T ) }] );
}

template <typename T>
void VariableSet::removeVariableStorage() {
    auto type = std::type_index { typeid( T ) };
    m_variables.erase( type );
    m_typeIndexToVtableIndex.erase( type );

    auto newEnd = std::remove( m_storedType.begin(), m_storedType.end(), type );
    m_storedType.erase( newEnd, m_storedType.end() );
}

// ------------------------------------------------------------------------------------------
// Templated methods definition
// ------------------------------------------------------------------------------------------
template <typename T>
auto VariableSet::insertVariable( const std::string& name, const T& value )
    -> std::pair<VariableHandle<T>, bool> {
    auto typeAccess = existsVariableType<T>();
    // If it is the first parameter of the given type, first register the type
    if ( !typeAccess ) { typeAccess = addVariableType<T>(); }
    // insert the parameter.
    return ( *typeAccess )->insert( { name, value } );
}

template <typename T>
auto VariableSet::getVariable( const std::string& name ) -> T& {
    return const_cast<T&>( const_cast<const VariableSet*>( this )->getVariable<T>( name ) );
}

template <typename T>
auto VariableSet::getVariable( const std::string& name ) const -> const T& {
    return getVariableHandle<T>( name )->second;
}

template <typename T>
auto VariableSet::getVariableHandle( const std::string& name ) const -> const VariableHandle<T> {
    assert( existsVariableType<T>() );
    return getVariableStorage<T>().find( name );
}

template <typename H>
bool VariableSet::isHandleValid( const H& handle ) const {
    if ( !existsVariableType<VariableTypeFromHandle<H>>() ) { return false; }
    return handle != getVariableStorage<VariableTypeFromHandle<H>>().end();
}

template <typename T>
auto VariableSet::setVariable( const std::string& name, const T& value )
    -> std::pair<VariableHandle<T>, bool> {
    auto typeAccess = existsVariableType<T>();
    // If it is the first parameter of the given type, first register the type
    if ( !typeAccess ) { typeAccess = addVariableType<T>(); }
    // insert the parameter.

    return ( *typeAccess )->insert_or_assign( name, value );
}

template <typename T>
bool VariableSet::deleteVariable( const std::string& name ) {
    if ( auto typeAccess = existsVariableType<T>(); typeAccess ) {
        auto removed = ( *typeAccess )->erase( name );
        // remove the type related function when the container has no more data of this type
        if ( numberOf<T>() == 0 ) { deleteAllVariables<T>(); }
        return removed > 0;
    }
    return false;
}

template <typename H>
bool VariableSet::deleteVariable( H& handle ) {
    assert( isHandleValid( handle ) );
    auto varname = handle->first;
    handle       = getVariableStorage<VariableTypeFromHandle<H>>().end();
    return deleteVariable<VariableTypeFromHandle<H>>( varname );
}

template <typename T>
auto VariableSet::existsVariable( const std::string& name ) const
    -> Utils::optional<VariableHandle<T>> {

    if ( auto typeAccess = existsVariableType<T>(); typeAccess ) {
        auto itr = ( *typeAccess )->find( name );
        if ( itr != ( *typeAccess )->cend() ) { return itr; }
    }
    return {};
}

template <typename T>
auto VariableSet::getVariableVisitTypeIndex() -> std::type_index {
    static std::type_index idT(
        typeid( std::reference_wrapper<typename VariableSet::VariableContainer<T>::value_type> ) );
    return idT;
}

template <typename T>
auto VariableSet::addVariableType() -> Utils::optional<VariableContainer<T>*> {
    auto storage = createVariableStorage<T>();

    auto tidx = std::type_index( typeid( T ) );
    auto it   = std::find( m_vtable->m_storedType.begin(), m_vtable->m_storedType.end(), tidx );

    // remember the stored type and its rank for the instance
    m_typeIndexToVtableIndex[tidx] = it - m_vtable->m_storedType.begin();
    m_storedType.push_back( tidx );

    // If the type is not already there, add type operations and identifier in the global vtable
    if ( it == m_vtable->m_storedType.end() ) {
        // remember the stored type in the global vtable
        m_vtable->m_storedType.emplace_back( tidx );
        // used to merge (keep) the stored data from container "from" to container "to"
        m_vtable->m_mergeKeepFunctions.emplace_back(
            []( const VariableSet& from, VariableSet& to ) {
                auto toStorageAccess = to.existsVariableType<T>();
                if ( !toStorageAccess ) { toStorageAccess = to.addVariableType<T>(); }
                auto& toStorage   = *( toStorageAccess.value() );
                auto& fromStorage = from.getVariableStorage<T>();
                for ( const auto& t : fromStorage ) {
                    toStorage.insert( t );
                }
            } );
        // used to merge (replace) the stored data from container "from" to container "to"
        m_vtable->m_mergeReplaceFunctions.emplace_back(
            []( const VariableSet& from, VariableSet& to ) {
                auto toStorageAccess = to.existsVariableType<T>();
                if ( !toStorageAccess ) { toStorageAccess = to.addVariableType<T>(); }
                auto& toStorage   = *( toStorageAccess.value() );
                auto& fromStorage = from.getVariableStorage<T>();
                for ( const auto& t : fromStorage ) {
                    toStorage.insert_or_assign( t.first, t.second );
                }
            } );
        // use to compute gauge on the stored data
        m_vtable->m_sizeFunctions.emplace_back(
            []( const VariableSet& c ) { return c.getVariableStorage<T>().size(); } );
        // used to visit the variableSet with a dynamic visitor
        m_vtable->m_visitFunctions.emplace_back(
            []( const VariableSet& c, const DynamicVisitorBase& v )
                -> std::pair<bool, std::function<void( DynamicVisitorBase&, std::any&& )>> {
                auto id = getVariableVisitTypeIndex<T>();
                if ( v.accept( id ) ) {
                    auto& visitedStorage = c.getVariableStorage<T>();
                    auto coll            = std::ref( visitedStorage );
                    return { true, [coll]( DynamicVisitorBase& visitor, std::any&& userParam ) {
                                for ( auto&& t : coll.get() ) {
                                    visitor( { std::ref( t ) },
                                             std::forward<std::any>( userParam ) );
                                }
                            } };
                }
                else { return { false, nullptr }; }
            } );
    }
    return storage;
}

template <typename T>
auto VariableSet::existsVariableType() const -> Utils::optional<VariableContainer<T>*> {
    auto iter = m_variables.find( std::type_index { typeid( T ) } );
    if ( iter == m_variables.cend() ) { return {}; }
    else { return std::any_cast<VariableSet::VariableContainer<T>>( &( iter->second ) ); }
}

template <typename T>
bool VariableSet::deleteAllVariables() {
    if ( existsVariableType<T>() ) {
        removeVariableStorage<T>();
        return true;
    }
    return false;
}

template <typename T>
auto VariableSet::getAllVariables() const -> VariableContainer<T>& {
    // just assert on the existence of the type to prevent undefined behavior when dereferencing
    // the optional
    assert( existsVariableType<T>() );
    // trows std::bad_any_cast if type does not exist
    return getVariableStorage<T>();
}

template <typename H>
auto VariableSet::getAllVariablesFromHandle( const H& )
    -> VariableContainer<VariableTypeFromHandle<H>>& {
    assert( existsVariableType<VariableTypeFromHandle<H>>() );
    return getVariableStorage<VariableTypeFromHandle<H>>();
}

template <typename T>
size_t VariableSet::numberOf() const {
    if ( auto variables = existsVariableType<T>(); variables ) { return ( *variables )->size(); }
    return 0;
}

/* --------------- Visitors */

template <typename P>
void VariableSet::visitDynamic( DynamicVisitorBase& visitor, P&& params ) const {
    for ( const auto& [type, index] : m_typeIndexToVtableIndex ) {
        auto [accepted, loop] = m_vtable->m_visitFunctions[index]( *this, visitor );
        if ( accepted ) { loop( visitor, std::forward<P>( params ) ); }
    }
}

template <typename F>
void VariableSet::visitStatic( F&& visitor ) const {
    visitImpl( visitor, typename std::decay_t<F>::types {} );
}

template <typename F, template <typename...> typename TYPESLIST, typename... TYPES>
void VariableSet::visitImpl( F&& visitor, TYPESLIST<TYPES...> ) const {
    ( ..., visitImplHelper<std::decay_t<F>, TYPES>( visitor ) );
}

template <typename F, typename T>
void VariableSet::visitImplHelper( F& visitor ) const {
    static_assert( has_visit_callable_v<F, T>,
                   "Static visitors must provide a function with profile "
                   "void( const std::string& name, [const ]T[&] value) for each "
                   "declared visitable type T" );
    if ( auto variables = existsVariableType<T>() ) {
        for ( auto& element : *( variables.value() ) ) {
            visitor( element.first, element.second );
        }
    }
}

template <typename F, typename U>
void VariableSet::visitStatic( F&& visitor, U& userParams ) const {
    visitImplUserParam(
        visitor, std::forward<U>( userParams ), typename std::decay_t<F>::types {} );
}

template <typename F, typename U>
void VariableSet::visitStatic( F&& visitor, U&& userParams ) const {
    visitImplUserParam(
        visitor, std::forward<U>( userParams ), typename std::decay_t<F>::types {} );
}

template <typename F, typename U, template <typename...> typename TYPESLIST, typename... TYPES>
void VariableSet::visitImplUserParam( F&& visitor, U&& userParam, TYPESLIST<TYPES...> ) const {
    ( ...,
      visitImplHelperUserParam<std::decay_t<F>, U, TYPES>( visitor,
                                                           std::forward<U>( userParam ) ) );
}

template <typename F, typename U, typename T>
void VariableSet::visitImplHelperUserParam( F& visitor, U&& userParams ) const {
    static_assert( has_visit_callable_with_user_param_v<F, T, U>,
                   "Static visitors must provide a function with profile "
                   "void( const std::string& name, [const ]T[&] value, [const] U&&) for each "
                   "declared visitable type T" );
    if ( auto variables = existsVariableType<T>(); variables ) {
        for ( auto& element : *( variables.value() ) ) {
            visitor( element.first, element.second, std::forward<U>( userParams ) );
        }
    }
}

template <typename T, typename F>
bool VariableSet::DynamicVisitor::addOperator( F&& f ) {
    auto [it, inserted] = m_visitorOperator.insert( makeVisitorOperator<T, F>( f ) );
    return inserted;
}

template <typename T>
bool VariableSet::DynamicVisitor::hasOperator() {
    return m_visitorOperator.find( getVariableVisitTypeIndex<T>() ) != m_visitorOperator.end();
}

template <typename T, typename F>
void VariableSet::DynamicVisitor::addOrReplaceOperator( F&& f ) {
    auto op = makeVisitorOperator<T, F>( f );
    m_visitorOperator.insert_or_assign( op.first, op.second );
}

template <typename T>
bool VariableSet::DynamicVisitor::removeOperator() {
    assert( hasOperator<T>() );
    auto res = m_visitorOperator.erase( getVariableVisitTypeIndex<T>() ) > 0;
    return res;
}

template <typename T, typename F>
struct VariableSet::DynamicVisitor::MakeVisitOperatorHelper<T, F, true> {
    inline auto makeOperator( F& f ) -> OperatorsStorageType::value_type {
        return { getVariableVisitTypeIndex<T>(), [&f]( std::any& a, std::any&& userParam ) {
                    auto rp = std::any_cast<std::reference_wrapper<VariableSet::Variable<T>>&>( a );
                    auto& p = rp.get();
                    f( p.first, p.second, std::forward<std::any>( userParam ) );
                } };
    }
};

template <typename T, typename F>
struct VariableSet::DynamicVisitor::MakeVisitOperatorHelper<T, F, false> {
    inline auto makeOperator( F& f ) -> OperatorsStorageType::value_type {
        return { getVariableVisitTypeIndex<T>(), [&f]( std::any& a, std::any&& ) {
                    auto rp = std::any_cast<std::reference_wrapper<VariableSet::Variable<T>>&>( a );
                    auto& p = rp.get();
                    f( p.first, p.second );
                } };
    }
};

template <class T, class F>
inline auto VariableSet::DynamicVisitor::makeVisitorOperator( F& f )
    -> OperatorsStorageType::value_type {
    auto opBuilder = MakeVisitOperatorHelper < T, F,
         std::is_invocable<F, const std::string&, T, std::any&&>::value ||
             std::is_invocable<F, const std::string&, T&, std::any&&>::value > {};
    return opBuilder.makeOperator( f );
}

template <typename F>
inline void VariableSet::visit( F&& visitor ) const {
    if constexpr ( std::is_base_of<DynamicVisitorBase, std::decay_t<F>>::value ) {
        visitDynamic( visitor );
    }
    else { visitStatic( visitor ); }
}

template <typename F, typename T>
inline void VariableSet::visit( F&& visitor, T& userParams ) const {
    if constexpr ( std::is_base_of<DynamicVisitorBase, std::decay_t<F>>::value ) {
        visitDynamic( visitor, std::forward<T&>( userParams ) );
    }
    else { visitStatic( visitor, std::forward<T&>( userParams ) ); }
}

template <typename F, typename T>
inline void VariableSet::visit( F&& visitor, T&& userParams ) const {
    if constexpr ( std::is_base_of<DynamicVisitorBase, std::decay_t<F>>::value ) {
        visitDynamic( visitor, std::forward<T&&>( userParams ) );
    }
    else { visitStatic( visitor, std::forward<T&&>( userParams ) ); }
}

} // namespace Core
} // namespace Ra
