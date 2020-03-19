#ifndef RADIUMENGINE_TIMELINE_HELP_HPP__
#define RADIUMENGINE_TIMELINE_HELP_HPP__

#include <QDialog>

namespace Ui {
class HelpDialog;
} // namespace Ui

namespace Ra::GuiBase {

/// Dialog to display navigation/interaction controls.
class HelpDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit HelpDialog( QWidget* parent = nullptr );
    ~HelpDialog();

  signals:
    void closed();

  public slots:
    void reject() override;

  private:
    Ui::HelpDialog* ui;
};

} // namespace Ra::GuiBase

#endif // RADIUMENGINE_TIMELINE_HELP_HPP__
