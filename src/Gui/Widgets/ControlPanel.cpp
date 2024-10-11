#include <Gui/Widgets/ControlPanel.hpp>

#include <Gui/Widgets/MatrixEditor.hpp>

#include <PowerSlider/PowerSlider.hpp>

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialog>
#include <QFileDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

namespace Ra::Gui::Widgets {

ControlPanel::ControlPanel( const std::string& name, bool hline, QWidget* parent ) :
    QFrame( parent ) {
    setObjectName( name.c_str() );
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setObjectName( "main layout" );
    m_contentLayout = new QGridLayout();
    m_contentLayout->setObjectName( "first content layout" );

    if ( !name.empty() ) {
        auto panelName = new QLabel();
        panelName->setText( name.c_str() );
        m_mainLayout->addWidget( panelName );
    }
    if ( hline ) {
        QFrame* line;
        line = new QFrame();
        line->setFrameShape( QFrame::HLine );
        line->setFrameShadow( QFrame::Sunken );
        m_mainLayout->addWidget( line );
    }

    m_mainLayout->addLayout( m_contentLayout );

    setLayout( m_mainLayout );
}

// Method to populate the panel
void ControlPanel::addOption( const std::string& name,
                              std::function<void( bool )> callback,
                              bool set,
                              const std::string& tooltip ) {
    auto label  = new QLabel( name.c_str() );
    auto button = new QCheckBox();
    button->setAutoExclusive( false );
    button->setChecked( set );
    if ( !tooltip.empty() ) {
        auto tooltipString =
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() );
        label->setToolTip( tooltipString );
        button->setToolTip( tooltipString );
    }
    connect( button, &QCheckBox::stateChanged, std::move( callback ) );

    auto index = m_contentLayout->rowCount();
    m_contentLayout->addWidget( label, index, 0 );
    m_contentLayout->addWidget( button, index, 1 );
}

void ControlPanel::addLabel( const std::string& text ) {
    auto label = new QLabel( text.c_str() );
    auto index = m_contentLayout->rowCount();
    m_contentLayout->addWidget( label, index, 0, 1, -1 );
}

void ControlPanel::addButton( const std::string& name,
                              std::function<void()> callback,
                              const std::string& tooltip ) {
    auto button = new QPushButton( name.c_str(), nullptr );
    if ( !tooltip.empty() ) {
        button->setToolTip(
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() ) );
    }

    connect( button, &QPushButton::clicked, std::move( callback ) );

    auto index = m_contentLayout->rowCount();
    m_contentLayout->addWidget( button, index, 1, 1, -1 );
}

void ControlPanel::addSliderInput( const std::string& name,
                                   std::function<void( int )> callback,
                                   int initial,
                                   int min,
                                   int max,
                                   const std::string& tooltip ) {
    auto inputLabel   = new QLabel( tr( name.c_str() ) );
    auto sliderLayout = new QHBoxLayout();
    auto inputField   = new QSlider( Qt::Horizontal );
    if ( !tooltip.empty() ) {
        auto tooltipString =
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() );
        inputLabel->setToolTip( tooltipString );
        inputField->setToolTip( tooltipString );
    }
    auto spinbox = new QSpinBox();
    inputField->setRange( min, max );
    inputField->setValue( initial );
    inputField->setTickPosition( QSlider::TicksAbove );
    inputField->setTickInterval( 1 );
    inputField->setFocusPolicy( Qt::StrongFocus );
    spinbox->setRange( min, max );
    spinbox->setValue( initial );
    connect( inputField, &QSlider::valueChanged, spinbox, &QSpinBox::setValue );
    connect( spinbox, &QSpinBox::valueChanged, inputField, &QSlider::setValue );
    connect( inputField, &QSlider::valueChanged, std::move( callback ) );
    sliderLayout->addWidget( inputField );
    sliderLayout->addWidget( spinbox );

    auto index = m_contentLayout->rowCount();
    m_contentLayout->addWidget( inputLabel, index, 0 );
    m_contentLayout->addLayout( sliderLayout, index, 1 );
}

void ControlPanel::addPowerSliderInput( const std::string& name,
                                        std::function<void( double )> callback,
                                        double initial,
                                        double min,
                                        double max,
                                        const std::string& tooltip ) {
    auto inputLabel = new QLabel( tr( name.c_str() ) );
    auto inputField = new PowerSlider();
    if ( !tooltip.empty() ) {
        auto tooltipString =
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() );
        inputLabel->setToolTip( tooltipString );
        inputField->setToolTip( tooltipString );
    }
    inputField->setObjectName( name.c_str() );
    inputField->setRange( min, max );
    inputField->setValue( initial );
    inputField->setSingleStep( 0.01 );
    connect( inputField, &PowerSlider::valueChanged, std::move( callback ) );

    auto index = m_contentLayout->rowCount();
    m_contentLayout->addWidget( inputLabel, index, 0 );
    m_contentLayout->addWidget( inputField, index, 1 );
}

void ControlPanel::addMatrixInput( const std::string& name,
                                   std::function<void( const Ra::Core::MatrixN& )> callback,
                                   const Ra::Core::MatrixN& initial,
                                   int dec,
                                   const std::string& tooltip ) {

    auto inputLabel = new QLabel( tr( name.c_str() ) );
    auto inputField = new MatrixEditor( initial, dec );

    if ( !tooltip.empty() ) {
        auto tooltipString =
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() );
        inputLabel->setToolTip( tooltipString );
        inputField->setToolTip( tooltipString );
    }
    connect( inputField, &MatrixEditor::valueChanged, std::move( callback ) );

    auto index = m_contentLayout->rowCount();
    m_contentLayout->addWidget( inputLabel, index, 0 );
    m_contentLayout->addWidget( inputField, index, 1 );
}

void ControlPanel::addColorInput(
    const std::string& name,
    const std::function<void( const Ra::Core::Utils::Color& clr )>& callback,
    Ra::Core::Utils::Color color,
    bool withAlpha,
    const std::string& tooltip ) {

    auto button    = new QPushButton( name.c_str() );
    auto srgbColor = Ra::Core::Utils::Color::linearRGBTosRGB( color );
    auto clrBttn   = QColor::fromRgbF( srgbColor[0], srgbColor[1], srgbColor[2], srgbColor[3] );
    auto clrDlg    = [callback, clrBttn, withAlpha, button, name]() mutable {
        clrBttn = QColorDialog::getColor(
            clrBttn,
            nullptr,
            name.c_str(),
            ( withAlpha ? QColorDialog::ShowAlphaChannel : QColorDialog::ColorDialogOptions() )
#ifndef OS_MACOS
                | QColorDialog::DontUseNativeDialog
#endif
        );
        if ( clrBttn.isValid() ) {
            // update the background color of the viewer
            auto lum = 0.2126_ra * Scalar( clrBttn.redF() ) +
                       0.7151_ra * Scalar( clrBttn.greenF() ) +
                       0.0721_ra * Scalar( clrBttn.blueF() );
            auto bgk = Ra::Core::Utils::Color::sRGBToLinearRGB(
                Ra::Core::Utils::Color( Scalar( clrBttn.redF() ),
                                        Scalar( clrBttn.greenF() ),
                                        Scalar( clrBttn.blueF() ),
                                        Scalar( clrBttn.alphaF() ) ) );
            callback( bgk );
            QString qss = QString( "background-color: %1" ).arg( clrBttn.name() );
            if ( lum > 1_ra / 3_ra ) { qss += QString( "; color: #000000" ); }
            else { qss += QString( "; color: #FFFFFF" ); }
            button->setStyleSheet( qss );
        }
    };

    if ( !tooltip.empty() ) {
        button->setToolTip(
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() ) );
    }
    // for contrast computation : see
    // https://www.w3.org/TR/UNDERSTANDING-WCAG20/visual-audio-contrast-contrast.html
    auto lum    = 0.2126_ra * srgbColor[0] + 0.7151_ra * srgbColor[1] + 0.0721_ra * srgbColor[2];
    QString qss = QString( "background-color: %1" ).arg( clrBttn.name() );
    if ( lum > 1_ra / 3_ra ) { qss += QString( "; color: #000000" ); }
    else { qss += QString( "; color: #FFFFFF" ); }
    button->setStyleSheet( qss );

    connect( button, &QPushButton::clicked, clrDlg );

    auto index = m_contentLayout->rowCount();
    m_contentLayout->addWidget( button, index, 0, 1, -1 );
}

void ControlPanel::addFileInput( const std::string& name,
                                 std::function<void( std::string )> callback,
                                 const std::string& rootDirectory,
                                 const std::string& filters,
                                 const std::string& tooltip ) {

    auto openFile = [this, callback, rootDirectory, filters]() {
        auto fltrs    = QString::fromStdString( filters );
        auto pathList = QFileDialog::getOpenFileNames(
            this, "Open files", QString::fromStdString( rootDirectory ), fltrs );
        if ( !pathList.empty() ) {
            std::string fileList;
            for ( const auto& file : pathList ) {
                fileList += file.toStdString() + ";";
            }
            fileList.erase( fileList.size() - 1 );
            callback( fileList );
        }
        else { callback( "" ); }
    };

    addButton( name, openFile, tooltip );
}

void ControlPanel::addFileOutput( const std::string& name,
                                  std::function<void( std::string )> callback,
                                  const std::string& rootDirectory,
                                  const std::string& filters,
                                  const std::string& tooltip ) {

    auto saveFile = [this, callback, rootDirectory, filters]() {
        auto fltrs    = QString::fromStdString( filters );
        auto filename = QFileDialog::getSaveFileName(
            this, "Save file", QString::fromStdString( rootDirectory ), fltrs );
        if ( !filename.isEmpty() ) { callback( filename.toStdString() ); }
    };
    addButton( name, saveFile, tooltip );
}

void ControlPanel::addSeparator() {
    QFrame* line = new QFrame();
    line->setFrameShape( QFrame::HLine );
    line->setFrameShadow( QFrame::Sunken );
    m_mainLayout->addWidget( line );
    m_contentLayout = new QGridLayout();
    m_mainLayout->addLayout( m_contentLayout );
}

void ControlPanel::addComboBox( const std::string& name,
                                std::function<void( int )> callback,
                                int initial,
                                const std::vector<std::string>& items,
                                const std::string& tooltip ) {
    auto inputLabel = new QLabel( tr( name.c_str() ) );
    auto inputField = new QComboBox();
    for ( auto v : items ) {
        inputField->addItem( QString::fromStdString( v ) );
    }
    if ( !tooltip.empty() ) {
        inputLabel->setToolTip(
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() ) );
    }
    inputField->setCurrentIndex( initial );
    connect(
        inputField, QOverload<int>::of( &QComboBox::currentIndexChanged ), std::move( callback ) );

    auto index = m_contentLayout->rowCount();
    m_contentLayout->addWidget( inputLabel, index, 0 );
    m_contentLayout->addWidget( inputField, index, 1 );
}

void ControlPanel::addComboBox( const std::string& name,
                                std::function<void( const QString& )> callback,
                                const std::string& initial,
                                const std::vector<std::string>& items,
                                const std::string& tooltip ) {
    auto inputLabel = new QLabel( tr( name.c_str() ) );
    auto inputField = new QComboBox();
    for ( auto v : items ) {
        inputField->addItem( QString::fromStdString( v ) );
    }
    if ( !tooltip.empty() ) {
        inputLabel->setToolTip(
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() ) );
    }
    inputField->setCurrentText( QString::fromStdString( initial ) );
    connect( inputField,
             QOverload<const QString&>::of( &QComboBox::currentTextChanged ),
             std::move( callback ) );

    auto index = m_contentLayout->rowCount();
    m_contentLayout->addWidget( inputLabel, index, 0 );
    m_contentLayout->addWidget( inputField, index, 1 );
}

void ControlPanel::addStretch( int stretch ) {
    m_mainLayout->addStretch( stretch );
    m_contentLayout = new QGridLayout();
    m_contentLayout->setObjectName( "stretch layout" );
    m_mainLayout->addLayout( m_contentLayout );
}

void ControlPanel::addWidget( QWidget* newWidget ) {
    m_mainLayout->addWidget( newWidget );
    m_contentLayout = new QGridLayout();
    m_contentLayout->setObjectName( "Widget content layout" );
    m_mainLayout->addLayout( m_contentLayout );
}

} // namespace Ra::Gui::Widgets
