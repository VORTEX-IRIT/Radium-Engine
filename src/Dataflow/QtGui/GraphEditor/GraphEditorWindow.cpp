#include "Dataflow/Core/NodeFactory.hpp"
#include <Dataflow/QtGui/GraphEditor/GraphEditorWindow.hpp>
#include <memory>

namespace Ra {
namespace Dataflow {
namespace QtGui {
namespace GraphEditor {

using namespace Ra::Dataflow::Core;

GraphEditorWindow::~GraphEditorWindow() {
    delete m_graphEdit;
}

GraphEditorWindow::GraphEditorWindow( std::shared_ptr<DataflowGraph> graph ) :
    m_graphEdit { new GraphEditorView( nullptr ) }, m_graph { graph } {

    setCentralWidget( m_graphEdit );
    m_graphEdit->setFocusPolicy( Qt::StrongFocus );

    createActions();
    createStatusBar();

    readSettings();

    connect(
        m_graphEdit, &GraphEditorView::needUpdate, this, &GraphEditorWindow::documentWasModified );

    setCurrentFile( QString() );
    setUnifiedTitleAndToolBarOnMac( true );
    if ( !m_graph ) {
        m_graph = std::make_shared<DataflowGraph>( "unititled.flow" );
        newFile();
    }

    m_graphEdit->editGraph( m_graph );
    m_graphEdit->show();
}

#if 0
void GraphEditorWindow::resetGraph( std::shared_ptr<DataflowGraph> graph ) {
    m_graph = graph;
    m_graphEdit->editGraph( m_graph );
    setCurrentFile( "" );
}
#endif

void GraphEditorWindow::closeEvent( QCloseEvent* event ) {
    if ( maybeSave() ) {
        writeSettings();
        event->accept();
    }
    else { event->ignore(); }
}

void GraphEditorWindow::newFile() {
    if ( maybeSave() ) {
        // Currently edited graph must be deleted only after it is no more used by the editor
        m_graph->destroy();
        setCurrentFile( "" );
    }
}

void GraphEditorWindow::open() {
    if ( maybeSave() ) {
        QString fileName = QFileDialog::getOpenFileName( this );
        if ( !fileName.isEmpty() ) loadFile( fileName );
    }
}

bool GraphEditorWindow::save() {
    if ( m_curFile.isEmpty() ) { return saveAs(); }
    else { return saveFile( m_curFile ); }
}

bool GraphEditorWindow::saveAs() {
    QFileDialog dialog( this );
    dialog.setWindowModality( Qt::WindowModal );
    dialog.setAcceptMode( QFileDialog::AcceptSave );
    dialog.setDefaultSuffix( "json" );
    if ( dialog.exec() != QDialog::Accepted ) return false;
    return saveFile( dialog.selectedFiles().first() );
}

void GraphEditorWindow::about() {
    QMessageBox::about( this,
                        tr( "About Node Editor" ),
                        tr( "This is NodeGraph Editor widget from Radium::Dataflow::QtGui." ) );
}

void GraphEditorWindow::documentWasModified() {
    setWindowModified( m_graph->shouldBeSaved() );
    emit needUpdate();
}

void GraphEditorWindow::createActions() {

    auto fileMenu       = menuBar()->addMenu( tr( "&File" ) );
    auto fileToolBar    = addToolBar( tr( "File" ) );
    const QIcon newIcon = QIcon::fromTheme( "document-new", QIcon( ":/images/new.png" ) );
    auto newAct         = new QAction( newIcon, tr( "&New" ), this );
    newAct->setShortcuts( QKeySequence::New );
    newAct->setStatusTip( tr( "Create a new file" ) );
    connect( newAct, &QAction::triggered, this, &GraphEditorWindow::newFile );
    fileMenu->addAction( newAct );
    fileToolBar->addAction( newAct );

    const QIcon openIcon = QIcon::fromTheme( "document-open", QIcon( ":/images/open.png" ) );
    auto openAct         = new QAction( openIcon, tr( "&Open..." ), this );
    openAct->setShortcuts( QKeySequence::Open );
    openAct->setStatusTip( tr( "Open an existing file" ) );
    connect( openAct, &QAction::triggered, this, &GraphEditorWindow::open );
    fileMenu->addAction( openAct );
    fileToolBar->addAction( openAct );

    const QIcon saveIcon = QIcon::fromTheme( "document-save", QIcon( ":/images/save.png" ) );
    auto saveAct         = new QAction( saveIcon, tr( "&Save" ), this );
    saveAct->setShortcuts( QKeySequence::Save );
    saveAct->setStatusTip( tr( "Save the document to disk" ) );
    connect( saveAct, &QAction::triggered, this, &GraphEditorWindow::save );
    fileMenu->addAction( saveAct );
    fileToolBar->addAction( saveAct );

    const auto saveAsIcon = QIcon::fromTheme( "document-save-as" );
    auto saveAsAct =
        fileMenu->addAction( saveAsIcon, tr( "Save &As..." ), this, &GraphEditorWindow::saveAs );
    saveAsAct->setShortcuts( QKeySequence::SaveAs );
    saveAsAct->setStatusTip( tr( "Save the document under a new name" ) );

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme( "application-exit" );
    auto exitAct         = fileMenu->addAction( exitIcon, tr( "E&xit" ), this, &QWidget::close );
    exitAct->setShortcuts( QKeySequence::Quit );
    exitAct->setStatusTip( tr( "Exit the application" ) );

#if 0
    // Activite this section when editMenu might be filled
    auto editMenu       = menuBar()->addMenu( tr( "&Edit" ) );
    auto editToolBar = addToolBar( tr( "Edit" ) );
#endif

    auto helpMenu = menuBar()->addMenu( tr( "&Help" ) );
    auto aboutAct = helpMenu->addAction( tr( "&About" ), this, &GraphEditorWindow::about );
    aboutAct->setStatusTip( tr( "Show the application's About box" ) );

    auto aboutQtAct = helpMenu->addAction( tr( "About &Qt" ), qApp, &QApplication::aboutQt );
    aboutQtAct->setStatusTip( tr( "Show the Qt library's About box" ) );
    //    if ( !m_ownGraph ) { menuBar()->hide(); }
}

void GraphEditorWindow::createStatusBar() {
    statusBar()->showMessage( tr( "Ready" ) );
}

void GraphEditorWindow::readSettings() {
    QSettings settings( QCoreApplication::organizationName(), QCoreApplication::applicationName() );
    settings.beginGroup( "nodegraph editor" );
    const QByteArray geometry = settings.value( "geometry", QByteArray() ).toByteArray();
    if ( geometry.isEmpty() ) {
        const QRect availableGeometry = screen()->availableGeometry();
        resize( availableGeometry.width() / 3, availableGeometry.height() / 2 );
        move( ( availableGeometry.width() - width() ) / 2,
              ( availableGeometry.height() - height() ) / 2 );
    }
    else { restoreGeometry( geometry ); }
    const QByteArray graphGeometry = settings.value( "graph", QByteArray() ).toByteArray();
    if ( graphGeometry.isEmpty() ) { m_graphEdit->resize( 800, 600 ); }
    else { m_graphEdit->restoreGeometry( graphGeometry ); }
    settings.endGroup();
}

void GraphEditorWindow::writeSettings() {
    QSettings settings( QCoreApplication::organizationName(), QCoreApplication::applicationName() );
    settings.beginGroup( "nodegraph editor" );
    settings.setValue( "geometry", saveGeometry() );
    settings.setValue( "graph", m_graphEdit->saveGeometry() );
    settings.endGroup();
}

bool GraphEditorWindow::maybeSave() {
    if ( m_graph == nullptr ) { return true; }
    if ( !m_graph->shouldBeSaved() ) { return true; }
    const QMessageBox::StandardButton ret =
        QMessageBox::warning( this,
                              tr( "Application" ),
                              tr( "The document has been modified.\n"
                                  "Do you want to save your changes?" ),
                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );
    switch ( ret ) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }

    return true;
}

void GraphEditorWindow::loadFile( const QString& fileName ) {
    QGuiApplication::setOverrideCursor( Qt::WaitCursor );
    {
        QFile file( fileName );
        if ( !file.open( QFile::ReadOnly | QFile::Text ) ) {
            QMessageBox::warning(
                this,
                tr( "Application" ),
                tr( "Cannot read file %1:\n%2." )
                    .arg( QDir::toNativeSeparators( fileName ), file.errorString() ) );
            return;
        }
    }

    bool loaded( true );
    m_graph->destroy();
    loaded = m_graph->loadFromJson( fileName.toStdString() );

    if ( !loaded ) {
        QMessageBox::warning(
            this,
            tr( "Application" ),
            tr( "Can't load graph from file %1.\n" ).arg( QDir::toNativeSeparators( fileName ) ) );
    }

    QGuiApplication::restoreOverrideCursor();
    if ( loaded ) {
        m_graphEdit->editGraph( m_graph );
        setCurrentFile( fileName );
        statusBar()->showMessage( tr( "File loaded" ), 2000 );
        emit needUpdate();
    }
}

bool GraphEditorWindow::saveFile( const QString& fileName ) {
    QString errorMessage;

    QGuiApplication::setOverrideCursor( Qt::WaitCursor );
    // TODO, if graph do not compile, tell it to the user ?
    // m_graph->compile();

    m_graph->saveToJson( fileName.toStdString() );
#if 0
    QSaveFile file(fileName);

    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << textEdit->toPlainText();
        if (!file.commit()) {
            errorMessage = tr("Cannot write file %1:\n%2.")
                               .arg(QDir::toNativeSeparators(fileName), file.errorString());
        }
    } else {
        errorMessage = tr("Cannot open file %1 for writing:\n%2.")
                           .arg(QDir::toNativeSeparators(fileName), file.errorString());
    }
#endif
    QGuiApplication::restoreOverrideCursor();

    if ( !errorMessage.isEmpty() ) {
        QMessageBox::warning( this, tr( "Application" ), errorMessage );
        return false;
    }

    setCurrentFile( fileName );
    statusBar()->showMessage( tr( "File saved" ), 2000 );
    return true;
}

void GraphEditorWindow::setCurrentFile( const QString& fileName ) {
    m_curFile = fileName;
    // textEdit->document()->setModified(false);
    setWindowModified( false );

    QString shownName = m_curFile;
    if ( m_curFile.isEmpty() ) shownName = "untitled.flow";
    setWindowFilePath( shownName );
}

QString GraphEditorWindow::strippedName( const QString& fullFileName ) {
    return QFileInfo( fullFileName ).fileName();
}

} // namespace GraphEditor
} // namespace QtGui
} // namespace Dataflow
} // namespace Ra
