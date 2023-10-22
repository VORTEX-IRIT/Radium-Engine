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

    m_currentLayout = new QVBoxLayout( this );
    setLayout( m_currentLayout );
    if ( hline ) {
        auto panelName = new QLabel( this );
        if ( !name.empty() ) {
            panelName->setText( name.c_str() );
            // panelName->setStyleSheet("border-bottom-width: 1px; border-bottom-style: solid;
            // border-radius: 0px;");
            panelName->setFrameStyle( QFrame::Box );
        }
        else { panelName->setFrameStyle( QFrame::HLine ); }
        m_currentLayout->addWidget( panelName );
    }
    setVisible( false );
    // setFrameStyle( QFrame::Panel );
    m_layouts.push( m_currentLayout );
}

// Method to populate the panel
void ControlPanel::addOption( const std::string& name,
                              std::function<void( bool )> callback,
                              bool set,
                              const std::string& tooltip ) {
    auto button = new QCheckBox( name.c_str(), this );
    button->setLayoutDirection( Qt::RightToLeft );
    button->setAutoExclusive( false );
    button->setChecked( set );
    if ( !tooltip.empty() ) {
        button->setToolTip(
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() ) );
    }
    m_currentLayout->addWidget( button );
    connect( button, &QCheckBox::stateChanged, std::move( callback ) );
}

void ControlPanel::addLabel( const std::string& text ) {
    auto label = new QLabel( text.c_str(), nullptr );
    m_currentLayout->addWidget( label );
}

void ControlPanel::addButton( const std::string& name,
                              std::function<void()> callback,
                              const std::string& tooltip ) {
    auto button = new QPushButton( name.c_str(), nullptr );
    if ( !tooltip.empty() ) {
        button->setToolTip(
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() ) );
    }
    m_currentLayout->addWidget( button );
    connect( button, &QPushButton::clicked, std::move( callback ) );
}

void ControlPanel::addSliderInput( const std::string& name,
                                   std::function<void( int )> callback,
                                   int initial,
                                   int min,
                                   int max,
                                   const std::string& tooltip ) {
    auto inputLayout = new QHBoxLayout();
    auto inputLabel  = new QLabel( tr( name.c_str() ), this );
    inputLayout->addWidget( inputLabel );
    inputLayout->addStretch();
    auto sliderLayout = new QHBoxLayout();
    auto inputField   = new QSlider( Qt::Horizontal, this );
    if ( !tooltip.empty() ) {
        inputLabel->setToolTip(
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() ) );
    }
    auto spinbox = new QSpinBox( this );
    inputField->setRange( min, max );
    inputField->setValue( initial );
    inputField->setTickPosition( QSlider::TicksAbove );
    inputField->setTickInterval( 1 );
    inputField->setFocusPolicy( Qt::StrongFocus );
    spinbox->setRange( min, max );
    spinbox->setValue( initial );
    connect( inputField, &QSlider::valueChanged, spinbox, &QSpinBox::setValue );
    // Qt6 do not need QOverload<int> but Qt5 do. Keep Qt5 syntax
    connect(
        spinbox, QOverload<int>::of( &QSpinBox::valueChanged ), inputField, &QSlider::setValue );
    connect( inputField, &QSlider::valueChanged, std::move( callback ) );
    sliderLayout->addWidget( inputField );
    sliderLayout->addWidget( spinbox );
    inputLayout->addLayout( sliderLayout );
    m_currentLayout->addLayout( inputLayout );
}

void ControlPanel::addPowerSliderInput( const std::string& name,
                                        std::function<void( double )> callback,
                                        double initial,
                                        double min,
                                        double max,
                                        const std::string& tooltip ) {
    auto inputLayout = new QHBoxLayout();
    auto inputLabel  = new QLabel( tr( name.c_str() ), this );
    auto inputField  = new PowerSlider();
    if ( !tooltip.empty() ) {
        inputLabel->setToolTip(
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() ) );
    }
    inputField->setObjectName( name.c_str() );
    inputField->setRange( min, max );
    inputField->setValue( initial );
    inputField->setSingleStep( 0.01 );
    inputLayout->addWidget( inputLabel );
    inputLayout->addStretch();
    inputLayout->addWidget( inputField );
    connect( inputField, &PowerSlider::valueChanged, std::move( callback ) );
    m_currentLayout->addLayout( inputLayout );
}

void ControlPanel::addMatrixInput( const std::string& name,
                                   std::function<void( const Ra::Core::MatrixN& )> callback,
                                   const Ra::Core::MatrixN& initial,
                                   int dec,
                                   const std::string& tooltip ) {
    auto inputLayout = new QHBoxLayout();

    auto inputLabel = new QLabel( tr( name.c_str() ), this );
    auto inputField = new MatrixEditor( initial, dec, this );

    if ( !tooltip.empty() ) {
        inputLabel->setToolTip(
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() ) );
    }
    inputLayout->addWidget( inputLabel );
    inputLayout->addWidget( inputField );
    connect( inputField, &MatrixEditor::valueChanged, std::move( callback ) );
    m_currentLayout->addLayout( inputLayout );
}

void ControlPanel::addColorInput(
    const std::string& name,
    const std::function<void( const Ra::Core::Utils::Color& clr )>& callback,
    Ra::Core::Utils::Color color,
    bool withAlpha,
    const std::string& tooltip ) {
    auto button = new QPushButton( name.c_str(), this );
    button->setObjectName( name.c_str() );
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
    m_currentLayout->addWidget( button );
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

void ControlPanel::beginLayout( QBoxLayout::Direction dir ) {
    m_layouts.push( m_currentLayout );
    m_currentLayout = new QBoxLayout( dir );
}

void ControlPanel::endLayout( bool separator ) {
    m_layouts.top()->addLayout( m_currentLayout );
    m_currentLayout = m_layouts.top();
    if ( separator ) { addSeparator(); }
    m_layouts.pop();
}

void ControlPanel::addSeparator() {
    QFrame* line = new QFrame();
    switch ( m_currentLayout->direction() ) {
    case QBoxLayout::LeftToRight:
    case QBoxLayout::RightToLeft:
        line->setFrameShape( QFrame::VLine );
        break;
    case QBoxLayout::TopToBottom:
    case QBoxLayout::BottomToTop:
        line->setFrameShape( QFrame::HLine );
        break;
    default:
        line->setFrameShape( QFrame::HLine );
    }
    line->setFrameShadow( QFrame::Sunken );
    m_currentLayout->addWidget( line );
}

void ControlPanel::addComboBox( const std::string& name,
                                std::function<void( int )> callback,
                                int initial,
                                const std::vector<std::string>& items,
                                const std::string& tooltip ) {
    auto inputLayout = new QHBoxLayout();
    auto inputLabel  = new QLabel( tr( name.c_str() ), this );
    auto inputField  = new QComboBox( this );
    for ( auto v : items ) {
        inputField->addItem( QString::fromStdString( v ) );
    }
    if ( !tooltip.empty() ) {
        inputLabel->setToolTip(
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() ) );
    }
    inputField->setCurrentIndex( initial );
    inputLayout->addWidget( inputLabel );
    inputLayout->addWidget( inputField );
    connect(
        inputField, QOverload<int>::of( &QComboBox::currentIndexChanged ), std::move( callback ) );
    m_currentLayout->addLayout( inputLayout );
}

void ControlPanel::addComboBox( const std::string& name,
                                std::function<void( const QString& )> callback,
                                const std::string& initial,
                                const std::vector<std::string>& items,
                                const std::string& tooltip ) {
    auto inputLayout = new QHBoxLayout();
    auto inputLabel  = new QLabel( tr( name.c_str() ), this );
    auto inputField  = new QComboBox( this );
    for ( auto v : items ) {
        inputField->addItem( QString::fromStdString( v ) );
    }
    if ( !tooltip.empty() ) {
        inputLabel->setToolTip(
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() ) );
    }
    inputField->setCurrentText( QString::fromStdString( initial ) );
    inputLayout->addWidget( inputLabel );
    inputLayout->addWidget( inputField );
    connect( inputField,
             QOverload<const QString&>::of( &QComboBox::currentTextChanged ),
             std::move( callback ) );
    m_currentLayout->addLayout( inputLayout );
}

void ControlPanel::addStretch( int stretch ) {
    m_currentLayout->addStretch( stretch );
}

void ControlPanel::addWidget( QWidget* newWidget ) {
    m_currentLayout->addWidget( newWidget );
}

} // namespace Ra::Gui::Widgets
