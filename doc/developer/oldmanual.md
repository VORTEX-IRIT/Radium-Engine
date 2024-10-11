\page develoldmanual _deprecated_ Radium Engine programmer manual
\todo pick relevant part, update and move to corresponding page
[TOC]

# Source code organization

* `src` contains the main source code of the engine libs

  * `Core` : the Core module (dynamic library) contains the foundation
 classes such as math classes, containers, adapters to the standard
 library, etc.

  * `Engine` the Engine module (dynamic library) contains all graphics related code and engine subsystems running.

  * `Gui` has the Qt-based GUI classes.

* `Plugins` contains the plugins which add Systems and Components to the engine.

* `Shaders` contains the OpenGL Shaders used by the renderer.

* `Applications` contains applications build with the engine. Currently it contains a minimal example application and the full-fledged `main-app`

* `3rdPartyLibs` contains any library dependency. CMake will automatically look in this folder to find the libraries if they are not installed on your system.

## Import and export

Core and Engine (and the plugins) are compiled as dynamic libraries, therefore you must be careful about which symbols
need exporting, especially on Windows. Every module has a specific header file (`RaCore.hpp` for Core) which must
be included first in all other headers of the module. In particular it defines the `*_API` macros which help exporting
and importing symbols on Windows.

# Core Libs

`Core` contains most of the basic code on which the rest of the software is built.

* `CoreMacros.hpp` has definitions of basic types, build configuration and useful macros.
* `Math` is our math library, which is a wrapper around _Eigen_. The most useful files are `Math.hpp` which has mathematical constants and simple functions, and `LinearAlgebra.hpp` which contains the type definitions of most basic vector and matrix types.
* `Containers` has some specially useful std-like containers, most importantly `VectorArray.hpp` which defines a dynamic array of vectors with both a`std::vector`-like interface and compatibility with Eigen.
* `Mesh` contains our basic mesh geometry primitives, including the representation of a simple triangle mesh and many functions to operate on geometry.
* `String` contains utilities extending `std::string`.
* `Log` is a wrapper around the header-only _EasyLogger_ library which allows us to log various events.
* `Time` contains utilities around `std::chrono` for precise timings.
* `Tasks` contains the definition for the basic Tasks system and the task queue.
* `Utils` contains generic utilities such as a Singleton template.

# Engine Object Model

## Entities

_Entities_ are the base object manipulated by the engine. You can think of them as "game objects", something that
represent an object in the scene. Entity data include a world space to object space transform and a list
of  _Components_

The entity's role is only to hold together this transform and the list of components.
The entity uses double buffering to prevent transforms from being updated
more than once per frame.

When creating an entity, if you set its transform, _do not forget_ to call
`Entity::swapTransformBuffers`, this might prevent you some headache. Example :

~~~{.cpp}
Ra::Engine::Scene::Entity* entity = theEntityManager->getOrCreate( "MyEntity" );
Ra::Core::Transform transform( Ra::Core::Transform::Identity() );
transform.translation = Ra::Core::Vector3( 42, 13, 37 );
entity->setTransform( transform );
entity->swapTransformBuffers();
~~~

## Systems and Components

Each _Component_ represent an aspect of an _Entity_ to a particular engine subsystem.
For example, an animated character may have a skeleton(animation component),
a high-resolution mesh (display component) and simplified convex shape for collision detection (physics component).

Each Component is related to a _System_. The Engine loads several Systems (statically
or from a plugin) and they keep the Components of all Entities in the scene updated.

For example, an Entity may have a Physics Component (which represent its collision shape),
and the engine's corresponding Physics System will update its position according to the physics
engine's result.

## Frame and Tasks

At each frame, the main loop first starts the renderer, then each System is given the opportunity
to add _Tasks_ to the task queue. Tasks are processed in parallel during every frame. They can have
dependencies between them (e.g. the physics update can wait for the animation update to complete).

A Task must implement the interface `Core::Task` which defines a `process()` function which will be called
when the task is executed. However for most cases, it is usually convenient to use a `FunctionTask` which takes
an instance of `std::function` (which will be called by the `process()` method). Calling an object's member function
in a task is thus easily achieved by using `std::bind()` to create the function object.

When a task is added, the task queue assumes ownership of the task and returns a `TaskID`
which identifies the task.

Dependencies between tasks can be specified either as _immediate_ dependencies or _pending_ dependencies.
Immediate dependencies are dependencies between tasks which are already present in the task queue.
Adding a dependency with `TaskkQueue::addDependency()` takes two arguments, the predecessor and the successor task,
and the task queue will ensure that the successor task must be executed after all its predecessors have finished.
In `addDependency()`, tasks can be identified with either a `TaskID` or a name; at least one of
the arguments must be a `TaskID`.
If a name is given, _all_ tasks matching with the name will be added as dependencies.

Since the order in which the systems are called is unspecified, a system may want
to add a dependency to a task that is not yet present. Pending dependencies solve
this problem by specifying a dependency between a `TaskID`and a name (which may not be present
in the task queue).
Pending dependencies are resolved just before the task queue starts executing all tasks.

## Rendering

Each Component may have some _RenderObjects_ (aka Drawable) which are (usually) OpenGL objects. When a Component has
changed it needs to tell its drawable to update their internal data (such as OpenGL VBOs).
Render objects are stored together by the _render object manager_ for efficiency, thus components only store an index to reference their render objects.

The Render Objects are drawn by the _Renderer_ which may live in a separated thread. Each frame it grabs all
the drawable (which should be double-buffered in case we are mid-VBO update) and calls `draw()` on them.

\todo a nice class schema

## ItemEntry : Handles to engine objects

The structure `ItemEntry` acts as a general identifier for all three levels of engine objects (entities, components and render objects) by storing a handle to the three objects (a pointer or an index for render objects). An entry representing an entity will have its entity handle set and the other two invalid; an entry representing a component will have the component handle set and the entity handle set to the entity owning the component. Similarly a render object entry will have all three handles sets .
Any other combination is invalid.

Item entries are mainly used to harmonize the interface to manipulate objects in the GUI.

## Object manipulation and editable transforms

Components have an interface through which transforms can be edited with three functions :

* `canEdit()`
* `getTransform()`
* `setTransform()`

Each of these functions takes a render object index as argument, as sometimes a component exposes multiple objects whose
transform can be edited. Function `canEdit()` tells whether a given idnex can be used as a handle for the other two functions.

## The system Entity and Debug Display

One Entity is always created when the engine starts, called the System Entity. Implemented as a singleton, this entity acts as a special object that the engine can use internally to display elements not related to any other Entity. The System Entity is fixed to the world and cannot be moved.

In particular it is expected that UI elements (e.g.  Gizmos) should be attached to the UI component. The Debug Component is useful to display debug primitives which can be useful for graphic debugging.
A series of macros `RA_DISPLAY_...` are defined to conveniently add basic display objects such as points, vectors, lines or 3D frames from anywhere in the code. A compile-time switch (`RA_DISABLE_DEBUG_DISPLAY`) can be activated to disable this feature.

# The plugin framework

Entities and components have been designed so that the engine is modular in terms of features. It is expected that most interesting works will be done by Systems defined in _Plugins_. Each plugin can define its System (and the corresponding Components).

We use a compile-time plugins loading mechanism. When running `cmake`, it will list the contents of the  `src/Plugins/` directory and add them to be compiled with the project, and automatically generate the code to include the plugins Systems in the main application.

## Requirements

For this automated build to work the plugins are required to follow these requirements

* The plugin code must be in a sub-folder of the `src/Plugins/` directory.

* The folder name is taken to be the _base name_ of the plugin. If the directory name is `BaseName`, then
  * Plugin code must be in `namespace BaseNamePlugin`
  * The system defined by the plugin must be named `BaseNamePlugin::BaseNameSystem`
  * It must be defined in a header located at the top of the plugin sub-folder, named `BaseNameSystem.hpp` (its full path should be `src/Plugins/Basename/BasenameSystem.hpp`)
  * The system must inherit from `Engine::Scene::System` and have a default empty constructor.

## Default plugins

See the structure of the default plugins for an example of a working plugin.
So far three default plugin exist:

* _Animation_ handles an animation skeleton and can play keyframe animations.

* _Skinning_ uses a geometric skinning method (like linear blend skinning or dual quaternion).

See also the Radium-PluginExample project: [https://github.com/STORM-IRIT/Radium-PluginExample](https://github.com/STORM-IRIT/Radium-PluginExample) for ToonShader and LaplacianSmoothing.
