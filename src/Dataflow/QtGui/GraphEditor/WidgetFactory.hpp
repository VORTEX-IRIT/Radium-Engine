#pragma once
#include <Dataflow/RaDataflow.hpp>

#include <Dataflow/Core/EditableParameter.hpp>

#include <QWidget>

#include <QColorDialog>
#include <iostream>
#include <unordered_map>

namespace Ra {
namespace Dataflow {
namespace QtGui {
namespace GraphEditor {
using namespace Ra::Dataflow::Core;

// TODO : instead of a namespace, make it a class ?
/**
 * Set of functions to manage automatic widget creation and update for editable parameters in the Qt
 * Node Editor.
 * \todo use Ra::Gui::ParameterSetEditor ?
 */
namespace WidgetFactory {
/** Type of the function that creates a widget.
 *  @param the editable parameter that defines the widget content
 *  @return the created widget
 */
using WidgetCreatorFunc = std::function<QWidget*( EditableParameterBase* )>;
/** Type of the function that creates a widget.
 * @param the widget whose content must be updated
 *  @param the editable parameter that defines the widget content
 *  @return true if update is done, false if not
 */
using WidgetUpdaterFunc = std::function<bool( QWidget*, EditableParameterBase* )>;

/** private method to manage the factory
 */
RA_DATAFLOW_API void registerWidgetInternal( std::type_index typeIdx,
                                             WidgetCreatorFunc widgetCreator,
                                             WidgetUpdaterFunc widgetUpdater );

/** Register a widget builder and updater in the factory given the type of the editable parameter
 * @tparam T The concrete type of the editable parameter
 * @param widgetCreator a function that build a widget to edit the given parameter.
 * @param widgetUpdater a function to update the widget state according to the given parameter.
 */
template <typename T>
void registerWidget( WidgetCreatorFunc widgetCreator, WidgetUpdaterFunc widgetUpdater ) {
    registerWidgetInternal( typeid( T ), std::move( widgetCreator ), std::move( widgetUpdater ) );
}

/**
 * Create a widget from an editable parameter using the widget factory
 * @param editableParameter the data whose type will define the widget
 * @return the created widget, nullptr if no widget creator is associated with the editable
 * parameter type.
 */
RA_DATAFLOW_API QWidget* createWidget( EditableParameterBase* editableParameter );

/**
 * Update a widget from an editable parameter using the widget factory
 *
 * @param widget the widget to update
 * @param editableParameter the data whose content will be transfered to the widget
 * @return true if the update is done, false if thereis no updater associated with the
 * editableParrameter type.
 */
RA_DATAFLOW_API bool updateWidget( QWidget* widget, EditableParameterBase* editableParameter );

/**
 * Initialize the factory with pre-defined widgets according to the Radium NodeGraph predefined
 * nodes.
 */
RA_DATAFLOW_API void initializeWidgetFactory();

} // namespace WidgetFactory

} // namespace GraphEditor
} // namespace QtGui
} // namespace Dataflow
} // namespace Ra
