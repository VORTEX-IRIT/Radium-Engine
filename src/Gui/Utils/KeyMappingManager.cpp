#include "KeyMappingManager.hpp"

#include <Core/Resources/Resources.hpp>
#include <Core/Utils/Log.hpp>

#include <QMessageBox>

namespace Ra::Gui {

using namespace Core::Utils; // log

KeyMappingManager::KeyMappingManager() : m_file( nullptr ) {

    auto optionalPath { Core::Resources::getRadiumResourcesPath() };
    auto resourcesRootDir { optionalPath.value_or( "[[Default resrouces path not found]]" ) };

    ///\todo how to check here ?
    m_defaultConfigFile = resourcesRootDir + std::string( "Configs/default.xml" );

    QSettings settings;
    QString keyMappingFilename =
        settings.value( "keymapping/config", QString::fromStdString( m_defaultConfigFile ) )
            .toString();
    if ( !keyMappingFilename.contains( QString::fromStdString( m_defaultConfigFile ) ) ) {
        LOG( logDEBUG ) << "Loading key mapping " << keyMappingFilename.toStdString() << " (from "
                        << settings.fileName().toStdString() << ")";
    }
    else { LOG( logDEBUG ) << "Loading default key mapping " << m_defaultConfigFile; }
    loadConfiguration( keyMappingFilename.toStdString() );
}

KeyMappingManager::KeyMappingAction
KeyMappingManager::getAction( const KeyMappingManager::Context& context,
                              const QEvent* event,
                              int key,
                              bool wheel ) {

    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;
    const QMouseEvent* mouseEvent = dynamic_cast<const QMouseEvent*>( event );
    if ( mouseEvent ) {
        buttons   = mouseEvent->buttons();
        modifiers = mouseEvent->modifiers();
    }
    return getAction( context, buttons, modifiers, key, wheel );
}

KeyMappingManager::KeyMappingAction
KeyMappingManager::getAction( const KeyMappingManager::Context& context,
                              const Qt::MouseButtons& buttons,
                              const Qt::KeyboardModifiers& modifiers,
                              int key,
                              bool wheel ) {

    if ( context.isInvalid() ) {
        LOG( logDEBUG ) << "try to get action from an invalid context";
        return KeyMappingAction();
    }
    // skip key as modifiers,
    if ( ( key == Qt::Key_Shift ) || ( key == Qt::Key_Control ) || ( key == Qt::Key_Alt ) ||
         ( key == Qt::Key_Meta ) ) {
        key = -1;
    }

    return getAction( context, EventBinding { buttons, modifiers, key, wheel } );
}

KeyMappingManager::KeyMappingAction
KeyMappingManager::getAction( const KeyMappingManager::Context& context,
                              const KeyMappingManager::EventBinding& binding ) {
    auto action = m_bindingToAction[context].find( binding );
    if ( action != m_bindingToAction[context].end() ) { return action->second; }

    return KeyMappingManager::KeyMappingAction();
}

std::optional<KeyMappingManager::EventBinding>
KeyMappingManager::getBinding( const KeyMappingManager::Context& context,
                               KeyMappingAction action ) {
    for ( const auto& [key, value] : m_bindingToAction[context] )
        if ( value == action ) return key;
    return {};
}

KeyMappingManager::Context KeyMappingManager::addContext( const std::string& contextName ) {
    Ra::Core::Utils::Index contextIndex;

    auto contextItr = m_contextNameToIndex.find( contextName );
    if ( contextItr == m_contextNameToIndex.end() ) {
        contextIndex                      = m_contextNameToIndex.size();
        m_contextNameToIndex[contextName] = contextIndex;
        m_actionNameToIndex.emplace_back();
        m_bindingToAction.emplace_back();
        CORE_ASSERT( m_actionNameToIndex.size() == size_t( contextIndex + 1 ),
                     "Corrupted actionName DB" );
        CORE_ASSERT( m_bindingToAction.size() == size_t( contextIndex + 1 ),
                     "Corrupted mappingAction DB" );
    }
    else
        contextIndex = contextItr->second;

    return contextIndex;
}

KeyMappingManager::Context KeyMappingManager::getContext( const std::string& contextName ) {
    // use find so that it do not insert invalid context
    auto itr = m_contextNameToIndex.find( contextName );
    if ( itr != m_contextNameToIndex.end() ) return itr->second;
    return Context {};
}

KeyMappingManager::KeyMappingAction KeyMappingManager::getAction( const Context& context,
                                                                  const std::string& actionName ) {
    if ( size_t( context ) >= m_actionNameToIndex.size() || context.isInvalid() ) {
        LOG( logWARNING ) << "try to get action index ( " << actionName
                          << " ) from an invalid context ( " << context << " )";

        return KeyMappingAction {};
    }
    auto itr = m_actionNameToIndex[context].find( actionName );
    if ( itr != m_actionNameToIndex[context].end() ) return itr->second;
    LOG( logWARNING ) << "try to get action index from an invalid action name " << actionName
                      << " (context " << getContextName( context ) << " [" << context << "])";
    LOG( logWARNING ) << "consider add to conf: "
                      << "<keymap context=\"" << getContextName( context )
                      << "\" key=\"\" modifiers=\"\" buttons=\"\" action=\"" << actionName
                      << "\"/>";

    return KeyMappingAction {};
}

std::string KeyMappingManager::getActionName( const Context& context,
                                              const KeyMappingAction& action ) {

    if ( size_t( context ) < m_actionNameToIndex.size() && context.isValid() ) {

        auto actionFindItr = std::find_if(
            std::begin( m_actionNameToIndex[context] ),
            std::end( m_actionNameToIndex[context] ),
            [&]( const ActionNameMap::value_type& pair ) { return pair.second == action; } );

        if ( actionFindItr != std::end( m_actionNameToIndex[context] ) ) {
            return actionFindItr->first;
        }
    }
    return "Invalid";
}

std::string KeyMappingManager::getContextName( const Context& context ) {
    auto contextFindItr = std::find_if(
        std::begin( m_contextNameToIndex ),
        std::end( m_contextNameToIndex ),
        [&]( const ContextNameMap ::value_type& pair ) { return pair.second == context; } );

    if ( contextFindItr != std::end( m_contextNameToIndex ) ) { return contextFindItr->first; }
    return "Invalid";
}

int KeyMappingManager::addListener( Observer callback ) {
    auto gid = attach( callback );
    // call the registered listener directly to have it up to date if the
    // config is already loaded
    callback();
    return gid;
}

void KeyMappingManager::removeListener( int callbackId ) {
    detach( callbackId );
}

void KeyMappingManager::setActionBinding( const Context& contextIndex,
                                          const EventBinding& binding,
                                          const KeyMappingAction& actionIndex ) {

    CORE_ASSERT( size_t( contextIndex ) < m_contextNameToIndex.size(),
                 "contextIndex is out of range" );

    // search if an action already correspond to this binding.
    auto f = m_bindingToAction[contextIndex].find( binding );
    if ( f != m_bindingToAction[contextIndex].end() ) {

        // if yes, search for its name
        auto findResult = std::find_if(
            std::begin( m_actionNameToIndex[contextIndex] ),
            std::end( m_actionNameToIndex[contextIndex] ),
            [&]( const ActionNameMap::value_type& pair ) { return pair.second == actionIndex; } );

        // if the name is not present, something bad happens.
        if ( findResult == std::end( m_actionNameToIndex[contextIndex] ) ) {
            LOG( logERROR ) << "Corrupted call to bindKeyToAction, index " << actionIndex
                            << " must have been inserted before !\n";
            return;
        }

        // if name is present, find the other action's name
        auto findResult2 = std::find_if(
            std::begin( m_actionNameToIndex[contextIndex] ),
            std::end( m_actionNameToIndex[contextIndex] ),
            [&]( const ActionNameMap::value_type& pair ) { return pair.second == f->second; } );

        // if the name is not present, something bad happens.
        if ( findResult2 == std::end( m_actionNameToIndex[contextIndex] ) ) {
            LOG( logERROR ) << "Corrupted call to bindKeyToAction, index " << actionIndex
                            << " must have been inserted before !\n";
            return;
        }

        LOG( logWARNING ) << "Binding action " << findResult->first << " to "
                          << "buttons [" << enumNamesFromMouseButtons( binding.m_buttons ) << "] "
                          << "modifiers [" << enumNamesFromKeyboardModifiers( binding.m_modifiers )
                          << "]"
                          << " keycode [" << binding.m_key << "]"
                          << " wheel [" << binding.m_wheel << "]"
                          << ", which is already used for action " << findResult2->first << ".";
    }

    LOG( logDEBUG2 ) << "In context " << getContextName( contextIndex ) << " [" << contextIndex
                     << "]"
                     << " binding action " << getActionName( contextIndex, actionIndex ) << " ["
                     << actionIndex << "]"
                     << " buttons [" << enumNamesFromMouseButtons( binding.m_buttons ) << "]"
                     << " modifiers [" << enumNamesFromKeyboardModifiers( binding.m_modifiers )
                     << "]"
                     << " keycode [" << binding.m_key << "]"
                     << " wheel [" << binding.m_wheel << "]";

    m_bindingToAction[contextIndex][binding] = actionIndex;
}

void KeyMappingManager::loadConfiguration( const std::string& inFilename ) {
    // if no filename is given, load default configuration
    std::string filename = inFilename;
    if ( filename.empty() ) { filename = m_defaultConfigFile.c_str(); }

    delete m_file;
    m_file = new QFile( QString::fromStdString( filename ) );

    if ( !m_file->open( QIODevice::ReadOnly ) ) {
        if ( filename != m_defaultConfigFile ) {
            LOG( logERROR ) << "Failed to open key mapping configuration file ! "
                            << m_file->fileName().toStdString();
            LOG( logERROR ) << "Trying to load default configuration...";
            loadConfiguration();
            return;
        }
        else {
            LOG( logERROR ) << "Failed to open default key mapping configuration file !";
            return;
        }
    }
    QDomDocument domDocument;
    if ( !domDocument.setContent( m_file ) ) {
        LOG( logERROR ) << "Can't associate XML file to QDomDocument !";
        LOG( logERROR ) << "Trying to load default configuration...";
        m_file->close();
        loadConfiguration();
        return;
    }

    // Store setting only if not default
    if ( filename != m_defaultConfigFile ) {
        QSettings settings;
        settings.setValue( "keymapping/config", m_file->fileName() );
    }
    m_file->close();

    loadConfigurationInternal( domDocument );

    // notify observer that keymapping has changed.
    notify();
}

bool KeyMappingManager::saveConfiguration( const std::string& inFilename ) {
    QString filename { m_file->fileName() };
    if ( !inFilename.empty() ) { filename = QString::fromStdString( inFilename ); }

    QFile saveTo( filename );
    saveTo.open( QIODevice::WriteOnly );
    QXmlStreamWriter stream( &saveTo );
    stream.setAutoFormatting( true );
    stream.setAutoFormattingIndent( 4 );
    stream.writeStartDocument();
    stream.writeComment( "\tRadium KeyMappingManager configuration file\t" );
    stream.writeComment(
        "\n<keymap context=\"theContext\" action=\"theAction\" buttons=\"QButton\" "
        "modifier=\"QModifier\" key=\"QKey\" wheel=\"boolean\"/>\n" );

    saveKeymap( stream );

    stream.writeEndDocument();

    if ( stream.hasError() ) {
        LOG( logERROR ) << "Fail to write Canonical XML.";
        return false;
    }
    return true;
}

void KeyMappingManager::saveKeymap( QXmlStreamWriter& stream ) {

    // helper functor to write attrib
    auto saveAttrib = [&stream]( const QString& attribName,
                                 const QString& attribValue,
                                 const QString& attribDefault ) {
        if ( attribValue != attribDefault ) stream.writeAttribute( attribName, attribValue );
    };

    if ( stream.hasError() ) { return; }

    stream.writeStartElement( "keymaps" );

    for ( const auto& contextPair : m_contextNameToIndex ) {
        const auto& contextName = contextPair.first;
        const auto& context     = contextPair.second;
        for ( const auto& actionPair : m_bindingToAction[context] ) {
            const auto& binding     = actionPair.first;
            const auto& action      = actionPair.second;
            const auto& actionNames = m_actionNameToIndex[context];
            auto actionItr =
                std::find_if( actionNames.begin(), actionNames.end(), [&action]( const auto& a ) {
                    return a.second == action;
                } );
            if ( actionItr != actionNames.end() ) {
                const auto& actionName = actionItr->first;
                stream.writeStartElement( "keymap" );
                stream.writeAttribute( "context", QString::fromStdString( contextName ) );
                stream.writeAttribute( "action", QString::fromStdString( actionName ) );

                saveAttrib(
                    "buttons",
                    QString::fromStdString( enumNamesFromMouseButtons( binding.m_buttons ) ),
                    QString::fromStdString( enumNamesFromMouseButtons( Qt::NoButton ) ) );
                saveAttrib(
                    "modifiers",
                    QString::fromStdString( enumNamesFromKeyboardModifiers( binding.m_modifiers ) ),
                    QString::fromStdString( enumNamesFromKeyboardModifiers( Qt::NoModifier ) ) );
                saveAttrib( "key", "Key_" + QKeySequence( binding.m_key ).toString(), "Key_" );
                saveAttrib( "wheel", binding.m_wheel ? "true" : "false", "false" );

                stream.writeEndElement();
            }
        }
    }

    stream.writeEndElement();
}

void KeyMappingManager::loadConfigurationInternal( const QDomDocument& domDocument ) {
    ///\todo maybe find a better way to handle load and reload.
    /// -> do not clear m_contextNameToIndex m_actionNameToIndex so the keep their index values ...
    m_contextNameToIndex.clear();
    m_actionNameToIndex.clear();
    m_bindingToAction.clear();

    QDomElement domElement = domDocument.documentElement();

    if ( domElement.tagName() != "keymaps" ) {
        LOG( logWARNING ) << "No <keymaps> global bounding tag ! Maybe you set a different global "
                             "tag ? (Not a big deal)";
    }

    QDomNode node = domElement.firstChild();
    while ( !node.isNull() ) {
        if ( !node.isComment() ) {
            QDomElement nodeElement = node.toElement();
            loadConfigurationTagsInternal( nodeElement );
        }
        node = node.nextSibling();
    }
}

void KeyMappingManager::loadConfigurationTagsInternal( QDomElement& node ) {
    if ( node.tagName() == "keymap" ) {

        QDomElement e               = node.toElement();
        std::string keyString       = e.attribute( "key", "-1" ).toStdString();
        std::string modifiersString = e.attribute( "modifiers", "NoModifier" ).toStdString();
        std::string buttonsString   = e.attribute( "buttons", "NoButton" ).toStdString();
        std::string contextName     = e.attribute( "context", "AppContext" ).toStdString();
        std::string wheelString     = e.attribute( "wheel", "false" ).toStdString();
        std::string actionName      = e.attribute( "action" ).toStdString();

        loadConfigurationMappingInternal(
            contextName, keyString, modifiersString, buttonsString, wheelString, actionName );
    }
    else {
        LOG( logERROR ) << "Unrecognized XML key mapping configuration file tag \""
                        << qPrintable( node.tagName() ) << "\" !";
        LOG( logERROR ) << "Trying to load default configuration...";
        loadConfiguration();

        return;
    }
}

KeyMappingManager::KeyMappingAction KeyMappingManager::addAction( const Context& context,
                                                                  const std::string& actionName ) {
    Ra::Core::Utils::Index actionIndex;
    auto actionItr = m_actionNameToIndex[context].find( actionName );
    if ( actionItr == m_actionNameToIndex[context].end() ) {
        actionIndex                              = m_actionNameToIndex[context].size();
        m_actionNameToIndex[context][actionName] = actionIndex;
    }
    else { actionIndex = actionItr->second; }

    return actionIndex;
}

KeyMappingManager::KeyMappingAction KeyMappingManager::addAction( const Context& context,
                                                                  const EventBinding& binding,
                                                                  const std::string& actionName ) {
    auto actionIndex = addAction( context, actionName );
    setActionBinding( context, binding, actionIndex );
    return actionIndex;
}

int KeyMappingManager::getKeyCode( const std::string& keyString ) {
    auto metaEnumKey = QMetaEnum::fromType<Qt::Key>();
    return metaEnumKey.keyToValue( keyString.c_str() );
}

KeyMappingManager::KeyMappingAction
KeyMappingManager::loadConfigurationMappingInternal( const std::string& context,
                                                     const std::string& keyString,
                                                     const std::string& modifiersString,
                                                     const std::string& buttonsString,
                                                     const std::string& wheelString,
                                                     const std::string& actionName ) {
    auto contextIndex = addContext( context );
    auto actionIndex  = addAction( contextIndex, actionName );

    auto binding =
        createEventBindingFromStrings( buttonsString, modifiersString, keyString, wheelString );

    if ( binding.m_key == -1 && binding.m_buttons == Qt::NoButton && !binding.m_wheel ) {
        LOG( logERROR ) << "Invalid binding for action [" << actionName << "] with key ["
                        << keyString << "], buttons [" << buttonsString << "], wheel ["
                        << wheelString << "]";
    }
    else { setActionBinding( contextIndex, binding, actionIndex ); }
    return actionIndex;
}

Qt::KeyboardModifiers KeyMappingManager::getQtModifiersValue( const std::string& modifierString ) {
    Qt::KeyboardModifiers modifier = Qt::NoModifier;

    std::istringstream f( modifierString );
    std::string s;

    while ( getline( f, s, ',' ) ) {
        if ( s == "ShiftModifier" ) { modifier |= Qt::ShiftModifier; }
        else if ( s == "ControlModifier" ) { modifier |= Qt::ControlModifier; }
        else if ( s == "AltModifier" ) { modifier |= Qt::AltModifier; }
        else if ( s == "MetaModifier" ) { modifier |= Qt::MetaModifier; }
        else if ( s == "KeypadModifier" ) { modifier |= Qt::KeypadModifier; }
        else if ( s == "GroupSwitchModifier" ) { modifier |= Qt::GroupSwitchModifier; }
    }

    return modifier;
}

Qt::MouseButtons KeyMappingManager::getQtMouseButtonsValue( const std::string& keyString ) {
    Qt::MouseButtons key = Qt::NoButton;

    if ( keyString == "LeftButton" ) { key = Qt::LeftButton; }
    else if ( keyString == "RightButton" ) { key = Qt::RightButton; }
    else if ( keyString == "MidButton" || keyString == "MiddleButton" ) { key = Qt::MiddleButton; }
    else if ( keyString == "XButton1" ) { key = Qt::XButton1; }
    else if ( keyString == "XButton2" ) { key = Qt::XButton2; }
    ///\todo add other buttons support here
    return key;
}

void KeyMappingManager::reloadConfiguration() {
    QString filename = m_file->fileName();
    loadConfiguration( filename.toStdString().c_str() );
}

KeyMappingManager::~KeyMappingManager() {
    if ( m_file->isOpen() ) { m_file->close(); }
}

std::string KeyMappingManager::getHelpText() {
    std::ostringstream text;
    for ( const auto& context : m_contextNameToIndex ) {
        std::string contextName { context.first };
        auto end    = contextName.find( "Context" );
        contextName = contextName.substr( 0, end );
        text << "<h2>" << contextName << "</h2>\n";

        for ( const auto& action : m_bindingToAction[context.second] ) {
            const auto& binding     = action.first;
            const auto& actionIndex = action.second;
            const auto& actionNames = m_actionNameToIndex[context.second];
            const auto& actionName =
                std::find_if( actionNames.begin(),
                              actionNames.end(),
                              [&actionIndex]( const auto& a ) { return a.second == actionIndex; } );
            if ( actionName != actionNames.end() ) {
                text << "<tr><td>";
                text << "<b>" << actionName->first << "</b>";
                text << "</td><td>";
                if ( binding.m_buttons != Qt::NoButton )
                    text << "[" << enumNamesFromMouseButtons( binding.m_buttons ) << "] ";
                if ( binding.m_wheel ) text << " [Wheel] ";
                if ( binding.m_modifiers != Qt::NoModifier ) {

                    std::string modifierString { "Modifier" };
                    auto modifiers = enumNamesFromKeyboardModifiers( binding.m_modifiers );
                    auto found     = modifiers.find( modifierString );
                    while ( found != std::string::npos ) {
                        modifiers.erase( found, modifierString.length() );
                        found = modifiers.find( modifierString, found );
                    }
                    text << "[" << modifiers << "] ";
                }
                if ( binding.m_key != -1 )
                    text << "[key: " << QKeySequence( binding.m_key ).toString().toStdString()
                         << "] ";
                text << "</td></tr>\n";
            }
        }
        text << "</table>\n";
    }
    return text.str();
}

#define TEST_BUTTON_STRING( BUTTON ) \
    if ( buttons & Qt::BUTTON ) {    \
        returnText += sep + #BUTTON; \
        sep = ",";                   \
    }

std::string KeyMappingManager::enumNamesFromMouseButtons( const Qt::MouseButtons& buttons ) {
    std::string returnText;
    std::string sep;

    if ( buttons == Qt::NoButton ) return "NoButton";

    TEST_BUTTON_STRING( LeftButton );
    TEST_BUTTON_STRING( RightButton );
    TEST_BUTTON_STRING( MiddleButton );
    TEST_BUTTON_STRING( BackButton );
    TEST_BUTTON_STRING( ForwardButton );
    TEST_BUTTON_STRING( TaskButton );
    TEST_BUTTON_STRING( ExtraButton4 );
    TEST_BUTTON_STRING( ExtraButton5 );
    TEST_BUTTON_STRING( ExtraButton6 );
    TEST_BUTTON_STRING( ExtraButton7 );
    TEST_BUTTON_STRING( ExtraButton8 );
    TEST_BUTTON_STRING( ExtraButton9 );
    TEST_BUTTON_STRING( ExtraButton10 );
    TEST_BUTTON_STRING( ExtraButton11 );
    TEST_BUTTON_STRING( ExtraButton12 );
    TEST_BUTTON_STRING( ExtraButton13 );
    TEST_BUTTON_STRING( ExtraButton14 );
    TEST_BUTTON_STRING( ExtraButton15 );
    TEST_BUTTON_STRING( ExtraButton16 );
    TEST_BUTTON_STRING( ExtraButton17 );
    TEST_BUTTON_STRING( ExtraButton18 );
    TEST_BUTTON_STRING( ExtraButton19 );
    TEST_BUTTON_STRING( ExtraButton20 );
    TEST_BUTTON_STRING( ExtraButton21 );
    TEST_BUTTON_STRING( ExtraButton22 );
    TEST_BUTTON_STRING( ExtraButton23 );
    TEST_BUTTON_STRING( ExtraButton24 );

    return returnText;
}

std::string
KeyMappingManager::enumNamesFromKeyboardModifiers( const Qt::KeyboardModifiers& buttons ) {
    std::string returnText;
    std::string sep;

    if ( buttons == Qt::NoModifier ) return "NoModifier";

    TEST_BUTTON_STRING( ShiftModifier );
    TEST_BUTTON_STRING( ControlModifier );
    TEST_BUTTON_STRING( AltModifier );
    TEST_BUTTON_STRING( MetaModifier );
    TEST_BUTTON_STRING( KeypadModifier );
    TEST_BUTTON_STRING( GroupSwitchModifier );

    return returnText;
}

#undef TEST_BUTTON_STRING

KeyMappingManager::EventBinding
KeyMappingManager::createEventBindingFromStrings( const std::string& buttonsString,
                                                  const std::string& modifiersString,
                                                  const std::string& keyString,
                                                  const std::string& wheelString ) {
    return { getQtMouseButtonsValue( buttonsString ),
             getQtModifiersValue( modifiersString ),
             getKeyCode( keyString ),
             wheelString.compare( "true" ) == 0 };
}

RA_SINGLETON_IMPLEMENTATION( KeyMappingManager );
} // namespace Ra::Gui
