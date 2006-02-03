/*
*  Copyright (C) 2001-2003, Richard J. Moore <rich@kde.org>
*  Copyright (C) 2005, Ian Reinhart Geiser <geiseri@kde.org>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
*  Boston, MA 02111-1307, USA.
*/

#include "global.h"
#include "jseventmapper.h"
namespace KJSEmbed {

JSEventMapper *JSEventMapper::m_inst = 0;

/** Used internally for the event handler table. */
struct EventType
{
    EventType( KJS::Identifier _id, QEvent::Type _type ) :
            id(_id), type(_type) {;}

    const KJS::Identifier id;
    const QEvent::Type type;
};
/*
Qt4 events as of Qt 4.0
        None = 0,                               // invalid event
        Timer = 1,                              // timer event
        MouseButtonPress = 2,                   // mouse button pressed
        MouseButtonRelease = 3,                 // mouse button released
        MouseButtonDblClick = 4,                // mouse button double click
        MouseMove = 5,                          // mouse move
        KeyPress = 6,                           // key pressed
        KeyRelease = 7,                         // key released
        FocusIn = 8,                            // keyboard focus received
        FocusOut = 9,                           // keyboard focus lost
        Enter = 10,                             // mouse enters widget
        Leave = 11,                             // mouse leaves widget
        Paint = 12,                             // paint widget
        Move = 13,                              // move widget
        Resize = 14,                            // resize widget
        Create = 15,                            // after widget creation
        Destroy = 16,                           // during widget destruction
        Show = 17,                              // widget is shown
        Hide = 18,                              // widget is hidden
        Close = 19,                             // request to close widget
        Quit = 20,                              // request to quit application
        ParentChange = 21,                      // widget has been reparented
        ParentAboutToChange = 131,              // sent just before the parent change is done
        ThreadChange = 22,                      // object has changed threads
        WindowActivate = 24,                    // window was activated
        WindowDeactivate = 25,                  // window was deactivated
        ShowToParent = 26,                      // widget is shown to parent
        HideToParent = 27,                      // widget is hidden to parent
        Wheel = 31,                             // wheel event
        WindowTitleChange = 33,                 // window title changed
        WindowIconChange = 34,                  // icon changed
        ApplicationWindowIconChange = 35,       // application icon changed
        ApplicationFontChange = 36,             // application font changed
        ApplicationLayoutDirectionChange = 37,  // application layout direction changed
        ApplicationPaletteChange = 38,          // application palette changed
        PaletteChange = 39,                     // widget palette changed
        Clipboard = 40,                         // internal clipboard event
        Speech = 42,                            // reserved for speech input
        MetaCall =  43,                         // meta call event
        SockAct = 50,                           // socket activation
        WinEventAct = 123,                      // win event activation
        DeferredDelete = 52,                    // deferred delete event
        DragEnter = 60,                         // drag moves into widget
        DragMove = 61,                          // drag moves in widget
        DragLeave = 62,                         // drag leaves or is cancelled
        Drop = 63,                              // actual drop
        DragResponse = 64,                      // drag accepted/rejected
        ChildAdded = 68,                        // new child widget
        ChildPolished = 69,                     // polished child widget
        ChildRemoved = 71,                      // deleted child widget
        ShowWindowRequest = 73,                 // widget's window should be mapped
        PolishRequest = 74,                     // widget should be polished
        Polish = 75,                            // widget is polished
        LayoutRequest = 76,                     // widget should be relayouted
        UpdateRequest = 77,                     // widget should be repainted
        EmbeddingControl = 79,                  // ActiveX embedding
        ActivateControl = 80,                   // ActiveX activation
        DeactivateControl = 81,                 // ActiveX deactivation
        ContextMenu = 82,                       // context popup menu
        InputMethod = 83,                       // input method
        AccessibilityPrepare = 86,              // accessibility information is requested
        TabletMove = 87,                        // Wacom tablet event
        LocaleChange = 88,                      // the system locale changed
        LanguageChange = 89,                    // the application language changed
        LayoutDirectionChange = 90,             // the layout direction changed
        Style = 91,                             // internal style event
        TabletPress = 92,                       // tablet press
        TabletRelease = 93,                     // tablet release
        OkRequest = 94,                         // CE (Ok) button pressed
        HelpRequest = 95,                       // CE (?)  button pressed
        IconDrag = 96,                          // proxy icon dragged
        FontChange = 97,                        // font has changed
        EnabledChange = 98,                     // enabled state has changed
        ActivationChange = 99,                  // window activation has changed
        StyleChange = 100,                      // style has changed
        IconTextChange = 101,                   // icon text has changed
        ModifiedChange = 102,                   // modified state has changed
        MouseTrackingChange = 109,              // mouse tracking state has changed
        WindowBlocked = 103,                    // window is about to be blocked modally
        WindowUnblocked = 104,                  // windows modal blocking has ended
        WindowStateChange = 105,
        ToolTip = 110,
        WhatsThis = 111,
        StatusTip = 112,
        ActionChanged = 113,
        ActionAdded = 114,
        ActionRemoved = 115,
        FileOpen = 116,                         // file open request
        Shortcut = 117,                         // shortcut triggered
        ShortcutOverride = 51,                  // shortcut override request
        WhatsThisClicked = 118,
        ToolBarChange = 120,                    // toolbar visibility toggled
        ApplicationActivated = 121,             // application has been changed to active
        ApplicationDeactivated = 122,           // application has been changed to inactive
        QueryWhatsThis = 123,                   // query what's this widget help
        EnterWhatsThisMode = 124,
        LeaveWhatsThisMode = 125,
        ZOrderChange = 126,                     // child widget has had its z-order changed
        HoverEnter = 127,                       // mouse cursor enters a hover widget
        HoverLeave = 128,                       // mouse cursor leaves a hover widget
        HoverMove = 129,                        // mouse cursor move inside a hover widget
        AccessibilityHelp = 119,                // accessibility help text request
        AccessibilityDescription = 130
*/
static EventType events[] = {
    EventType( KJS::Identifier("onTimerEvent"), QEvent::Timer ),
    EventType( KJS::Identifier("onMouseButtonPressEvent"), QEvent::MouseButtonPress ),
    EventType( KJS::Identifier("onMouseButtonReleaseEvent"), QEvent::MouseButtonRelease ),
    EventType( KJS::Identifier("onMouseButtonDblClickEvent"), QEvent::MouseButtonDblClick ),
    EventType( KJS::Identifier("onMouseMoveEvent"), QEvent::MouseMove ),
    EventType( KJS::Identifier("onKeyPressEvent"), QEvent::KeyPress ),
    EventType( KJS::Identifier("onKeyReleaseEvent"), QEvent::KeyRelease ),
    EventType( KJS::Identifier("onFocusInEvent"), QEvent::FocusIn ),
    EventType( KJS::Identifier("onFocusOutEvent"), QEvent::FocusOut ),
    EventType( KJS::Identifier("onEnterEvent"), QEvent::Enter ),
    EventType( KJS::Identifier("onLeaveEvent"), QEvent::Leave ),
    EventType( KJS::Identifier("onPaintEvent"), QEvent::Paint ),
    EventType( KJS::Identifier("onMoveEvent"), QEvent::Move ),
    EventType( KJS::Identifier("onResizeEvent"), QEvent::Resize ),
    EventType( KJS::Identifier("onCreateEvent"), QEvent::Create ),
    EventType( KJS::Identifier("onDestroyEvent"), QEvent::Destroy ),
    EventType( KJS::Identifier("onShowEvent"), QEvent::Show ),
    EventType( KJS::Identifier("onHideEvent"), QEvent::Hide ),
    EventType( KJS::Identifier("onCloseEvent"), QEvent::Close ),
    EventType( KJS::Identifier("onQuitEvent"), QEvent::Quit ),
    EventType( KJS::Identifier("onParentChangeEvent"), QEvent::ParentChange ),
    EventType( KJS::Identifier("onParentAboutToChangeEvent"), QEvent::ParentAboutToChange ),
    EventType( KJS::Identifier("onThreadChangeEvent"), QEvent::ThreadChange ),
    EventType( KJS::Identifier("onWindowActivateEvent"), QEvent::WindowActivate ),
    EventType( KJS::Identifier("onWindowDeactivateEvent"), QEvent::WindowDeactivate ),
    EventType( KJS::Identifier("onShowToParentEvent"), QEvent::ShowToParent ),
    EventType( KJS::Identifier("onHideToParentEvent"), QEvent::HideToParent ),
    EventType( KJS::Identifier("onWheelEvent"), QEvent::Wheel ),
    EventType( KJS::Identifier("onWindowTitleChangeEvent"), QEvent::WindowTitleChange ),
    EventType( KJS::Identifier("onWindowIconChangeEvent"), QEvent::WindowIconChange ),
    EventType( KJS::Identifier("onApplicationWindowIconChangeEvent"), QEvent::ApplicationWindowIconChange ),
    EventType( KJS::Identifier("onApplicationFontChangeEvent"), QEvent::ApplicationFontChange ),
    EventType( KJS::Identifier("onApplicationLayoutDirectionChangeEvent"), QEvent::ApplicationLayoutDirectionChange ),
    EventType( KJS::Identifier("onApplicationPaletteChangeEvent"), QEvent::ApplicationPaletteChange ),
    EventType( KJS::Identifier("onPaletteChangeEvent"), QEvent::PaletteChange ),
    EventType( KJS::Identifier("onClipboardEvent"), QEvent::Clipboard ),
    EventType( KJS::Identifier("onSpeechEvent"), QEvent::Speech ),
    EventType( KJS::Identifier("onMetaCallEvent"), QEvent::MetaCall ),
    EventType( KJS::Identifier("onSockActEvent"), QEvent::SockAct ),
    EventType( KJS::Identifier("onWinEventActEvent"), QEvent::WinEventAct ),
    EventType( KJS::Identifier("onDeferredDeleteEvent"), QEvent::DeferredDelete ),
    EventType( KJS::Identifier("onDragEnterEvent"), QEvent::DragEnter ),
    EventType( KJS::Identifier("onDragMoveEvent"), QEvent::DragMove ),
    EventType( KJS::Identifier("onDragLeaveEvent"), QEvent::DragLeave ),
    EventType( KJS::Identifier("onDropEvent"), QEvent::Drop ),
    EventType( KJS::Identifier("onDragResponseEvent"), QEvent::DragResponse ),
    EventType( KJS::Identifier("onChildAddedEvent"), QEvent::ChildAdded ),
    EventType( KJS::Identifier("onChildPolishedEvent"), QEvent::ChildRemoved ),
    EventType( KJS::Identifier("onShowWindowRequestEvent"), QEvent::ShowWindowRequest ),
    EventType( KJS::Identifier("onPolishRequestEvent"), QEvent::PolishRequest ),
    EventType( KJS::Identifier("onPolishEvent"), QEvent::Polish ),
    EventType( KJS::Identifier("onLayoutRequestEvent"), QEvent::LayoutRequest ),
    EventType( KJS::Identifier("onUpdateRequestEvent"), QEvent::UpdateRequest ),
    EventType( KJS::Identifier("onEmbeddingControlEvent"), QEvent::EmbeddingControl ),
    EventType( KJS::Identifier("onActivateControlEvent"), QEvent::ActivateControl ),
    EventType( KJS::Identifier("onDeactivateControlEvent"), QEvent::DeactivateControl ),
    EventType( KJS::Identifier("onContextMenuEvent"), QEvent::ContextMenu ),
    EventType( KJS::Identifier("onInputMethodEvent"), QEvent::InputMethod ),
    EventType( KJS::Identifier("onAccessibilityPrepareEvent"), QEvent::AccessibilityPrepare ),
    EventType( KJS::Identifier("onTabletMoveEvent"), QEvent::TabletMove ),
    EventType( KJS::Identifier("onLocaleChangeEvent"), QEvent::LocaleChange ),
    EventType( KJS::Identifier("onLanguageChangeEvent"), QEvent::LanguageChange ),
    EventType( KJS::Identifier("onLayoutDirectionChangeEvent"), QEvent::LayoutDirectionChange ),
    EventType( KJS::Identifier("onStyleEvent"), QEvent::Style ),
    EventType( KJS::Identifier("onTabletPressEvent"), QEvent::TabletPress ),
    EventType( KJS::Identifier("onTabletReleaseEvent"), QEvent::TabletRelease ),
    EventType( KJS::Identifier("onOkRequestEvent"), QEvent::OkRequest ),
    EventType( KJS::Identifier("onHelpRequestEvent"), QEvent::HelpRequest ),
    EventType( KJS::Identifier("onIconDragEvent"), QEvent::IconDrag ),
    EventType( KJS::Identifier("onFontChangeEvent"), QEvent::FontChange ),
    EventType( KJS::Identifier("onEnabledChangeEvent"), QEvent::EnabledChange ),
    EventType( KJS::Identifier("onActivationChangeEvent"), QEvent::ActivationChange ),
    EventType( KJS::Identifier("onStyleChangeEvent"), QEvent::StyleChange ),
    EventType( KJS::Identifier("onIconTextChangeEvent"), QEvent::IconTextChange ),
    EventType( KJS::Identifier("onModifiedChangeEvent"), QEvent::ModifiedChange ),
    EventType( KJS::Identifier("onMouseTrackingChangeEvent"), QEvent::MouseTrackingChange ),
    EventType( KJS::Identifier("onWindowBlockedEvent"), QEvent::WindowBlocked ),
    EventType( KJS::Identifier("onWindowUnblockedEvent"), QEvent::WindowUnblocked ),
    EventType( KJS::Identifier("onWindowStateChangeEvent"), QEvent::WindowStateChange ),
    EventType( KJS::Identifier("onToolTipEvent"), QEvent::ToolTip ),
    EventType( KJS::Identifier("onWhatsThisEvent"), QEvent::WhatsThis ),
    EventType( KJS::Identifier("onStatusTipEvent"), QEvent::StatusTip ),
    EventType( KJS::Identifier("onActionChangedEvent"), QEvent::ActionChanged ),
    EventType( KJS::Identifier("onActionAddedEvent"), QEvent::ActionAdded ),
    EventType( KJS::Identifier("onActionRemovedEvent"), QEvent::ActionRemoved ),
    EventType( KJS::Identifier("onFileOpenEvent"), QEvent::FileOpen ),
    EventType( KJS::Identifier("onShortcutEvent"), QEvent::Shortcut ),
    EventType( KJS::Identifier("onShortcutOverrideEvent"), QEvent::ShortcutOverride ),
    EventType( KJS::Identifier("onWhatsThisClickedEvent"), QEvent::WhatsThisClicked ),
    EventType( KJS::Identifier("onToolBarChangeEvent"), QEvent::ToolBarChange ),
    EventType( KJS::Identifier("onApplicationActivatedEvent"), QEvent::ApplicationActivated ),
    EventType( KJS::Identifier("onApplicationDeactivatedEvent"), QEvent::ApplicationDeactivated ),
    EventType( KJS::Identifier("onQueryWhatsThisEvent"), QEvent::QueryWhatsThis ),
    EventType( KJS::Identifier("onEnterWhatsThisModeEvent"), QEvent::EnterWhatsThisMode ),
    EventType( KJS::Identifier("onLeaveWhatsThisModeEvent"), QEvent::LeaveWhatsThisMode ),
    EventType( KJS::Identifier("onZOrderChangeEvent"), QEvent::ZOrderChange ),
    EventType( KJS::Identifier("onHoverEnterEvent"), QEvent::HoverEnter ),
    EventType( KJS::Identifier("onHoverLeaveEvent"), QEvent::HoverLeave ),
    EventType( KJS::Identifier("onHoverMoveEvent"), QEvent::HoverMove ),
    EventType( KJS::Identifier("onAccessibilityHelpEvent"), QEvent::AccessibilityHelp ),
    EventType( KJS::Identifier("onAccessibilityDescriptionEvent"), QEvent::AccessibilityDescription ),
    EventType(KJS::Identifier(), QEvent::None )
};

JSEventMapper::JSEventMapper()
{
    int i = 0;
    do {
        addEvent( events[i].id, events[i].type );
        i++;
    } while( events[i].type != QEvent::None );
}

JSEventMapper::~JSEventMapper()
{
}

void JSEventMapper::addEvent( const KJS::Identifier &name, QEvent::Type t )
{
    m_handlerToEvent[ name.qstring() ] = t;
    m_eventToHandler[ t ] = name;
}

QEvent::Type JSEventMapper::findEventType( const KJS::Identifier &name ) const
{
    return  m_handlerToEvent[ name.qstring() ];
}

bool JSEventMapper::isEventHandler( const KJS::Identifier &name ) const
{
    return m_handlerToEvent.find( name.qstring() ) ? true : false;
}

KJS::Identifier JSEventMapper::findEventHandler( QEvent::Type t ) const
{
    return m_eventToHandler[t];
}

JSEventMapper *JSEventMapper::mapper()
{
    if ( m_inst )
        return m_inst;
    m_inst = new JSEventMapper();
    return m_inst;
}


} // namespace KJSEmbed

// Local Variables:
// c-basic-offset: 4
// End:
