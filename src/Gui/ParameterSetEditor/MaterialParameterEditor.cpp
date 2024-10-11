#include <Gui/ParameterSetEditor/MaterialParameterEditor.hpp>

#include <Gui/Widgets/ControlPanel.hpp>

// include the Material Definition
#include <Engine/Data/Material.hpp>

#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QString>
#include <QWidget>

#include <limits>
#include <memory>

namespace Ra::Gui {

MaterialParameterEditor::MaterialParameterEditor( QWidget* parent ) : QWidget( parent ) {
    // setup the GUI
    auto verticalLayout = new QVBoxLayout( this );

    m_matInstanceNameLabel = new QLabel();
    m_matNameLabel         = new QLabel();
    m_matProperties        = new QPlainTextEdit();
    m_matProperties->setReadOnly( true );

    auto nameLayout = new QHBoxLayout();
    nameLayout->addWidget( new QLabel( "Instance name :" ) );
    nameLayout->addWidget( m_matInstanceNameLabel );
    verticalLayout->addLayout( nameLayout );

    m_matInfoGroup = new QGroupBox();
    m_matInfoGroup->setTitle( "Material Informations" );
    m_matInfoGroup->setCheckable( false );
    auto matInfoForm = new QFormLayout( m_matInfoGroup );
    matInfoForm->addRow( tr( "Material Type :" ), m_matNameLabel );
    matInfoForm->addRow( tr( "Instance Properties :" ), m_matProperties );
    m_matInfoGroup->setMaximumHeight( 150 );
    verticalLayout->addWidget( m_matInfoGroup );
    m_matInfoGroup->setVisible( false );

    auto matParamsGroup = new QGroupBox();
    matParamsGroup->setTitle( "Edit Parameters" );
    matParamsGroup->setEnabled( true );
    matParamsGroup->setCheckable( false );

    verticalLayout->addWidget( matParamsGroup );
    m_matParamsLayout        = new QVBoxLayout( matParamsGroup );
    m_parametersControlPanel = new VariableSetEditor( "Material Parameters", this );
    m_matParamsLayout->addWidget( m_parametersControlPanel );

    verticalLayout->addStretch();
}

void MaterialParameterEditor::setupFromMaterial(
    std::shared_ptr<Ra::Engine::Data::Material> material ) {
    auto hasMetadata =
        std::dynamic_pointer_cast<Ra::Engine::Data::ParameterSetEditingInterface>( material );
    m_matNameLabel->setText( QString::fromStdString( material->getMaterialName() ) );
    m_matProperties->clear();
    for ( auto& prop : material->getPropertyList() ) {
        m_matProperties->appendPlainText( QString::fromStdString( prop ) );
    }
    // move the cursor to the top
    auto tmpCursor = m_matProperties->textCursor();
    tmpCursor.movePosition( QTextCursor::Start );
    m_matProperties->setTextCursor( tmpCursor );

    auto& params  = material->getParameters();
    auto metadata = ( hasMetadata ) ? hasMetadata->getParametersMetadata() : nlohmann::json {};

    m_matInstanceNameLabel->setText( material->getInstanceName().c_str() );

    // clear the old control panel
    m_matParamsLayout->removeWidget( m_parametersControlPanel );
    delete m_parametersControlPanel;
    m_parametersControlPanel = new VariableSetEditor( "Material Parameters", this );
    m_parametersControlPanel->setShowUnspecified( m_showUnspecified );
    m_matParamsLayout->addWidget( m_parametersControlPanel );

    m_parametersControlPanel->setupUi( params, metadata );

    connect( m_parametersControlPanel,
             &VariableSetEditor::parameterModified,
             [this]( const std::string& nm ) { emit materialParametersModified( nm ); } );
    m_matInfoGroup->setVisible( true );
}

void MaterialParameterEditor::showUnspecified( bool enable ) {
    m_showUnspecified = enable;
    m_parametersControlPanel->setShowUnspecified( m_showUnspecified );
}
} // namespace Ra::Gui
