/* This file is part of the KDE libraries
   Copyright (C) 2000 Daniel M. Duley <mosfet@kde.org>
   Copyright (C) 2002,2006 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2006 Olivier Goffart <ogoffart@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kmenu.h"
#include "khbox.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QCursor>
#include <QtGui/QFontMetrics>
#include <QtGui/QHBoxLayout>
#include <QtGui/QKeyEvent>
#include <QtGui/QMenuItem>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QToolButton>
#include <QtGui/QWidgetAction>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kacceleratormanager.h>

static const char KMENU_TITLE[] = "kmenu_title";

class KMenu::KMenuPrivate
{
public:
    KMenuPrivate (KMenu *_parent);
    ~KMenuPrivate ();

    void resetKeyboardVars(bool noMatches = false);
    void actionHovered(QAction* action);
    void showCtxMenu(const QPoint &pos);
    void skipTitles(QKeyEvent *event);

    KMenu *parent;

    // variables for keyboard navigation
    QTimer clearTimer;

    bool noMatches : 1;
    bool shortcuts : 1;
    bool autoExec : 1;

    QString keySeq;
    QString originalText;

    QAction* lastHitAction;
    QAction* lastHoveredAction;
    Qt::MouseButtons mouseButtons;
    Qt::KeyboardModifiers keyboardModifiers;

    // support for RMB menus on menus
    QMenu* ctxMenu;
    QPointer<QAction> highlightedAction;

    class EventSniffer;
    EventSniffer *eventSniffer;
};

/**
  * @internal
  *
  * This event sniffer is an event filter which will be installed
  * on the title of the menu, which is a QToolButton. This will
  * prevent clicks (what would change down and focus properties on
  * the title) on the title of the menu.
  *
  * @author Rafael Fernández López <ereslibre@kde.org>
  */
class KMenu::KMenuPrivate::EventSniffer
    : public QObject
{
public:
    EventSniffer(QObject *parent = 0)
        : QObject(parent) { }

    ~EventSniffer() { }

    bool eventFilter(QObject *object, QEvent *event)
    {
        Q_UNUSED(object);

        if (event->type() == QEvent::Paint ||
            event->type() == QEvent::KeyPress ||
            event->type() == QEvent::KeyRelease) {
            return false;
        }

        event->accept();
        return true;
    }
};

KMenu::KMenuPrivate::KMenuPrivate (KMenu *_parent)
    : parent(_parent)
    , noMatches(false)
    , shortcuts(false)
    , autoExec(false)
    , lastHitAction(0L)
    , lastHoveredAction(0L)
    , mouseButtons(Qt::NoButton)
    , keyboardModifiers(Qt::NoModifier)
    , ctxMenu(0)
    , highlightedAction(0)
    , eventSniffer(new EventSniffer)
{
    resetKeyboardVars();
    KAcceleratorManager::manage(parent);
}

KMenu::KMenuPrivate::~KMenuPrivate ()
{
    delete ctxMenu;
    delete eventSniffer;
}


/**
 * custom variant type for QAction::data of kmenu context menus
 * @author Joseph Wenninger <jowenn@kde.org>
 */
class KMenuContext {
public:
    KMenuContext();
    KMenuContext(const KMenuContext& o);
    KMenuContext(QPointer<KMenu> menu,QPointer<QAction> action);

    inline QPointer<KMenu> menu() const { return m_menu; }
    inline QPointer<QAction> action() const { return m_action; }

private:
    QPointer<KMenu> m_menu;
    QPointer<QAction> m_action;
};


Q_DECLARE_METATYPE(KMenuContext)



KMenu::KMenu(QWidget *parent)
    : QMenu(parent)
    , d(new KMenuPrivate(this))
{
    connect(&(d->clearTimer), SIGNAL(timeout()), SLOT(resetKeyboardVars()));
}

KMenu::KMenu( const QString & title, QWidget * parent )
    : QMenu(title, parent)
    , d(new KMenuPrivate(this))
{
    connect(&(d->clearTimer), SIGNAL(timeout()), SLOT(resetKeyboardVars()));
}

KMenu::~KMenu()
{
    delete d;
}

QAction* KMenu::addTitle(const QString &text, QAction* before)
{
    return addTitle(QIcon(), text, before);
}

QAction* KMenu::addTitle(const QIcon &icon, const QString &text, QAction* before)
{
    QAction *buttonAction = new QAction(this);
    QFont font = buttonAction->font();
    font.setBold(true);
    buttonAction->setFont(font);
    buttonAction->setText(text);
    buttonAction->setIcon(icon);

    QWidgetAction *action = new QWidgetAction(this);
    action->setObjectName(KMENU_TITLE);
    QToolButton *titleButton = new QToolButton(this);
    titleButton->installEventFilter(d->eventSniffer); // prevent clicks on the title of the menu
    titleButton->setDefaultAction(buttonAction);
    titleButton->setDown(true); // prevent hover style changes in some styles
    titleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    action->setDefaultWidget(titleButton);

    insertAction(before, action);
    return action;
}

/**
 * This is re-implemented for keyboard navigation.
 */
void KMenu::closeEvent(QCloseEvent*e)
{
    if (d->shortcuts)
        d->resetKeyboardVars();
    QMenu::closeEvent(e);
}

Qt::MouseButtons KMenu::mouseButtons() const
{
    return d->mouseButtons;
}

Qt::KeyboardModifiers KMenu::keyboardModifiers() const
{
    return d->keyboardModifiers;
}

void KMenu::keyPressEvent(QKeyEvent* e)
{
    d->mouseButtons = Qt::NoButton;
    d->keyboardModifiers = Qt::NoModifier;

    if (!d->shortcuts) {
        d->keyboardModifiers = e->modifiers();
        QMenu::keyPressEvent(e);

        if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
            d->skipTitles(e);
        }

        return;
    }

    QAction* a = 0L;
    bool firstpass = true;
    QString keyString = e->text();

    // check for common commands dealt with by QMenu
    int key = e->key();
    if (key == Qt::Key_Escape || key == Qt::Key_Return || key == Qt::Key_Enter
            || key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_Left
            || key == Qt::Key_Right || key == Qt::Key_F1 || key == Qt::Key_PageUp
            || key == Qt::Key_PageDown || key == Qt::Key_Back || key == Qt::Key_Select) {

        d->resetKeyboardVars();
        // continue event processing by QMenu
        //e->ignore();
        d->keyboardModifiers = e->modifiers();
        QMenu::keyPressEvent(e);

        if (key == Qt::Key_Up || key == Qt::Key_Down) {
            d->skipTitles(e);
        }
        return;
    } else if ( key == Qt::Key_Shift || key == Qt::Key_Control || key == Qt::Key_Alt || key == Qt::Key_Meta )
        return QMenu::keyPressEvent(e);

    // check to see if the user wants to remove a key from the sequence (backspace)
    // or clear the sequence (delete)
    if (!d->keySeq.isNull()) {
        if (key == Qt::Key_Backspace) {

            if (d->keySeq.length() == 1) {
                d->resetKeyboardVars();
                return;
            }

            // keep the last sequence in keyString
            keyString = d->keySeq.left(d->keySeq.length() - 1);

            // allow sequence matching to be tried again
            d->resetKeyboardVars();

        } else if (key == Qt::Key_Delete) {
            d->resetKeyboardVars();

            // clear active item
            setActiveAction(0L);
            return;

        } else if (d->noMatches) {
            // clear if there are no matches
            d->resetKeyboardVars();

            // clear active item
            setActiveAction(0L);

        } else {
            // the key sequence is not a null string
            // therefore the lastHitAction is valid
            a = d->lastHitAction;
        }

    } else if (key == Qt::Key_Backspace && menuAction()) {
        // backspace with no chars in the buffer... go back a menu.
        hide();
        d->resetKeyboardVars();
        return;
    }

    d->keySeq += keyString;
    const int seqLen = d->keySeq.length();

    foreach (a, actions()) {
        // don't search disabled entries
        if (!a->isEnabled())
            continue;

        QString thisText;

        // retrieve the right text
        // (the last selected item one may have additional ampersands)
        if (a == d->lastHitAction)
            thisText = d->originalText;
        else
            thisText = a->text();

        // if there is an accelerator present, remove it
        thisText = KGlobal::locale()->removeAcceleratorMarker(thisText);

        // chop text to the search length
        thisText = thisText.left(seqLen);

        // do the search
        if (!thisText.indexOf(d->keySeq, 0, Qt::CaseInsensitive)) {

            if (firstpass) {
                // match
                setActiveAction(a);

                // check to see if we're underlining a different item
                if (d->lastHitAction && d->lastHitAction != a)
                    // yes; revert the underlining
                    d->lastHitAction->setText(d->originalText);

                // set the original text if it's a different item
                if (d->lastHitAction != a || d->lastHitAction == 0L)
                    d->originalText = a->text();

                // underline the currently selected item
                a->setText(underlineText(d->originalText, d->keySeq.length()));

                // remember what's going on
                d->lastHitAction = a;

                // start/restart the clear timer
                d->clearTimer.setSingleShot(true);
                d->clearTimer.start(5000);

                // go around for another try, to see if we can execute
                firstpass = false;
            } else {
                // don't allow execution
                return;
            }
        }

        // fall through to allow execution
    }

    if (!firstpass) {
        if (d->autoExec) {
            // activate anything
            d->lastHitAction->activate(QAction::Trigger);
            d->resetKeyboardVars();

        } else if (d->lastHitAction && d->lastHitAction->menu()) {
            // only activate sub-menus
            d->lastHitAction->activate(QAction::Trigger);
            d->resetKeyboardVars();
        }

        return;
    }

    // no matches whatsoever, clean up
    d->resetKeyboardVars(true);
    //e->ignore();
    QMenu::keyPressEvent(e);
}

bool KMenu::focusNextPrevChild( bool next )
{
    d->resetKeyboardVars();
    return QMenu::focusNextPrevChild( next );
}

QString KMenu::underlineText(const QString& text, uint length)
{
    QString ret = text;
    for (uint i = 0; i < length; i++) {
        if (ret[2*i] != '&')
            ret.insert(2*i, '&');
    }
    return ret;
}

void KMenu::KMenuPrivate::resetKeyboardVars(bool _noMatches)
{
    // Clean up keyboard variables
    if (lastHitAction) {
        lastHitAction->setText(originalText);
        lastHitAction = 0L;
    }

    if (!noMatches) {
        keySeq.clear();
    }

    noMatches = _noMatches;
}

void KMenu::setKeyboardShortcutsEnabled(bool enable)
{
    d->shortcuts = enable;
}

void KMenu::setKeyboardShortcutsExecute(bool enable)
{
    d->autoExec = enable;
}
/**
 * End keyboard navigation.
 */

/**
 * RMB menus on menus
 */

void KMenu::mousePressEvent(QMouseEvent* e)
{
    if (d->ctxMenu && d->ctxMenu->isVisible())
    {
        // hide on a second context menu event
        d->ctxMenu->hide();
    }

    if( e->button() == Qt::MidButton)
      return;

    QMenu::mousePressEvent(e);
}

void KMenu::mouseReleaseEvent(QMouseEvent* e)
{
    // Save the button, and the modifiers
    d->keyboardModifiers = e->modifiers();
    d->mouseButtons = e->buttons();

    if ( e->button() == Qt::MidButton) {
      if(activeAction() ) {
        QMetaObject::invokeMethod(activeAction(), "triggered", Qt::DirectConnection,
               Q_ARG(Qt::MouseButtons, e->button()),
              Q_ARG(Qt::KeyboardModifiers, QApplication::keyboardModifiers() ));
      }
      return;
    }

    if ( !d->ctxMenu || !d->ctxMenu->isVisible() )
        QMenu::mouseReleaseEvent(e);
}

QMenu* KMenu::contextMenu()
{
    if (!d->ctxMenu)
    {
        d->ctxMenu = new QMenu(this);
        connect(this, SIGNAL(hovered(QAction*)), SLOT(actionHovered(QAction*)));
    }

    return d->ctxMenu;
}

const QMenu* KMenu::contextMenu() const
{
    return const_cast< KMenu* >( this )->contextMenu();
}

void KMenu::hideContextMenu()
{
    if (!d->ctxMenu || !d->ctxMenu->isVisible())
    {
        return;
    }

    d->ctxMenu->hide();
}

void KMenu::KMenuPrivate::actionHovered(QAction* action)
{
    lastHoveredAction = action;
    parent->hideContextMenu();
}

static void KMenuSetActionData(QMenu *menu,KMenu* contextedMenu, QAction* contextedAction) {
    const QList<QAction*> actions=menu->actions();
    QVariant v;
    v.setValue(KMenuContext(contextedMenu,contextedAction));
    for(int i=0;i<actions.count();i++) {
        actions[i]->setData(v);
    }
}

void KMenu::KMenuPrivate::showCtxMenu(const QPoint &pos)
{
    highlightedAction = parent->activeAction();

    if (!highlightedAction)
    {
        KMenuSetActionData(parent,0,0);
        return;
    }

    emit parent->aboutToShowContextMenu(parent, highlightedAction, ctxMenu);
    KMenuSetActionData(parent,parent,highlightedAction);


    if (QMenu* subMenu = highlightedAction->menu())
    {
        QTimer::singleShot(100, subMenu, SLOT(hide()));
    }


    ctxMenu->popup(parent->mapToGlobal(pos));
}

void KMenu::KMenuPrivate::skipTitles(QKeyEvent *event)
{
    QWidgetAction *action = qobject_cast<QWidgetAction*>(parent->activeAction());
    QWidgetAction *firstAction = action;
    while (action && action->objectName() == KMENU_TITLE)
    {
        parent->keyPressEvent(event);
        action = qobject_cast<QWidgetAction*>(parent->activeAction());
        if (firstAction == action) { // we looped and only found titles
            parent->setActiveAction(0);
            break;
        }
    }
}

KMenu * KMenu::contextMenuFocus( )
{
  return qobject_cast<KMenu*>(QApplication::activePopupWidget());
}

QAction * KMenu::contextMenuFocusAction( )
{
  if (KMenu* menu = qobject_cast<KMenu*>(QApplication::activePopupWidget())) {
    if (!menu->d->lastHoveredAction) {
      return 0;
    }
    QVariant var = menu->d->lastHoveredAction->data();
    KMenuContext ctx = var.value<KMenuContext>();
    Q_ASSERT(ctx.menu() == menu);
    return ctx.action();
  }

  return 0L;
}

void KMenu::contextMenuEvent(QContextMenuEvent* e)
{
    if (d->ctxMenu)
    {
        if (e->reason() == QContextMenuEvent::Mouse)
        {
            d->showCtxMenu(e->pos());
        }
        else if (activeAction())
        {
            d->showCtxMenu(actionGeometry(activeAction()).center());
        }

        e->accept();
        return;
    }

    QMenu::contextMenuEvent(e);
}

void KMenu::hideEvent(QHideEvent *e)
{
    if (d->ctxMenu && d->ctxMenu->isVisible())
    {
        // we need to block signals here when the ctxMenu is showing
        // to prevent the QPopupMenu::activated(int) signal from emitting
        // when hiding with a context menu, the user doesn't expect the
        // menu to actually do anything.
        // since hideEvent gets called very late in the process of hiding
        // (deep within QWidget::hide) the activated(int) signal is the
        // last signal to be emitted, even after things like aboutToHide()
        // AJS
        bool blocked = blockSignals(true);
        d->ctxMenu->hide();
        blockSignals(blocked);
    }
    QMenu::hideEvent(e);
}
/**
 * end of RMB menus on menus support
 */





KMenuContext::KMenuContext( )
  : m_menu(0L)
  , m_action(0L)
{
}

KMenuContext::KMenuContext( const KMenuContext & o )
  : m_menu(o.m_menu)
  , m_action(o.m_action)
{
}

KMenuContext::KMenuContext(QPointer<KMenu> menu,QPointer<QAction> action)
  : m_menu(menu)
  , m_action(action)
{
}

#include "kmenu.moc"
