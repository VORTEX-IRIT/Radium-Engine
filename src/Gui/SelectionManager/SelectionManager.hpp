#pragma once
#include <Gui/RaGui.hpp>

#include <Engine/Scene/ItemEntry.hpp>
#include <Gui/TreeModel/EntityTreeModel.hpp>
#include <QItemSelectionModel>

namespace Ra {
namespace Gui {

class RA_GUI_API SelectionManager : public QItemSelectionModel
{
    Q_OBJECT
  public:
    explicit SelectionManager( ItemModel* model, QObject* parent = nullptr );

    // The following functions adapt QItemSelectionModel functions to
    // operate with ItemEntries instead of model indices.

    /** Returns true if the selection contains the given item.
     *
     * \param ent
     * \return
     */
    bool isSelected( const Engine::Scene::ItemEntry& ent ) const;

    /// Return the set of selected entries. \see selectedIndexes()
    std::vector<Engine::Scene::ItemEntry> selectedEntries() const;

    /** Return the current selected item, or an invalid entry if there is no current item.
     * \seeCurrentIndex;
     *
     * \return
     */
    const Engine::Scene::ItemEntry& currentItem() const;

    /** Select an item through an item entry. \see QItemSelectionModel::Select
     *
     * \param ent
     * \param command
     */
    virtual void select( const Engine::Scene::ItemEntry& ent,
                         QItemSelectionModel::SelectionFlags command );

    /** Set an item as current through an item entry. \see QItemSelectionModel::setCurrent
     *
     * \param ent
     * \param command
     */
    void setCurrentEntry( const Engine::Scene::ItemEntry& ent,
                          QItemSelectionModel::SelectionFlags command );

  protected slots:
    void onModelRebuilt();

  protected slots:
    void printSelection() const;

  protected:
    const ItemModel* itemModel() const { return static_cast<const ItemModel*>( model() ); }
};

} // namespace Gui
} // namespace Ra
