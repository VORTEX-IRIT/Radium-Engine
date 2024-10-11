\page componentMessenger _deprecated_ Component Messenger
\todo check and update
[TOC]

This class is used to make componenent share data between each others. The whole system is based on **callbacks**
registered from both parts to publish, suscribe or both to
a data.

# Read #

The **suscriber** must define and register some `Ra::Engine::Scene::ComponentMessenger::CallbackTypes::Setter` that will be used by the `ComponentMessenger` to set the data to the
former.

    // definition of callback
    ComponentMessenger::CallbackTypes<T>::Setter data_in = std::bind( &FooComponent::setDataIn, this, std::placeholders::_1 );

    // registration
    ComponentMessenger::getInstance()->registerInput<T>( entity, this, "name", data_in );

    // fetch
    if (ComponentMessenger::getInstance()->canGet<T>( entity, "name") )
    {
        data = ComponentMessenger::getInstance()->get<T>( entity, "name");
    }

# Write #

    // definition of callback
    ComponentMessenger::CallbackTypes<T>::Getter data_out = std::bind( &FooComponent::getDataOut, this );

    // registration
    ComponentMessenger::getInstance()->registerOutput<T>( entity, this, "name", data_out );

The **publisher** will define and register a `Ra::Engine::Scene::ComponentMessenger::CallbackTypes::Getter` that will be used
by the `ComponentMessenger` to fetch data from the former.

# Read'n'Write #

    // definition of callback
    ComponentMessenger::CallbackTypes<T>::ReadWrite data_rw = std::bind( &FooComponent::getDataRw, this );

    // registration
    ComponentMessenger::getInstance()->registerReadWrite<T>( entity, this, "name", data_rw );

    // fetch
    if (ComponentMessenger::getInstance()->canRw<T>( entity, "name"))
    {
        T& data = ComponentMessenger::getInstance()->rw<T>( entity, "name" );
    }

Here a pointer of the data is basically shared and everyone has only to do:

# The `setupIO()` function #

A convention is to use the `setupIO()` function to interact with the messenger.
