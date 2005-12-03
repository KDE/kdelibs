/* This file is part of the KDE libraries
   Copyright (C) 2000 Daniel M. Duley <mosfet@kde.org>
   Copyright (C) 2002 Hamish Rodda <rodda@kde.org>

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

#include <qcursor.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qfontmetrics.h>
#include <QKeyEvent>
#include <QPointer>
#include <QMenuItem>
#include <QApplication>

#include "kmenu.h"

#include <kdebug.h>

class KMenu::KMenuPrivate
{
public:
    KMenuPrivate ()
        : noMatches(false)
        , shortcuts(false)
        , autoExec(false)
        , lastHitAction(0L)
        , mouseButtons(Qt::NoButton)
        , keyboardModifiers(Qt::NoModifier)
        , ctxMenu(0)
        , continueCtxMenuShow(true)
        , highlightedAction(0)
    {}

    ~KMenuPrivate ()
    {
        delete ctxMenu;
    }


    // variables for keyboard navigation
    QTimer clearTimer;

    bool noMatches : 1;
    bool shortcuts : 1;
    bool autoExec : 1;

    QString keySeq;
    QString originalText;

    QAction* lastHitAction;
    Qt::MouseButtons mouseButtons;
    Qt::KeyboardModifiers keyboardModifiers;

    // support for RMB menus on menus
    QMenu* ctxMenu;
    bool continueCtxMenuShow;
    QPointer<QAction> highlightedAction;
};

KMenu::KMenu(QWidget *parent)
    : QMenu(parent)
    , d(new KMenuPrivate())
{
    resetKeyboardVars();
    connect(&(d->clearTimer), SIGNAL(timeout()), SLOT(resetKeyboardVars()));
}

KMenu::KMenu( const QString & title, QWidget * parent )
    : QMenu(title, parent)
    , d(new KMenuPrivate())
{
    resetKeyboardVars();
    connect(&(d->clearTimer), SIGNAL(timeout()), SLOT(resetKeyboardVars()));
}

KMenu::~KMenu()
{
    delete d;
}

QAction* KMenu::addTitle(const QString &text, QAction* before)
{
    QAction* action = new QAction(text, this);
    action->setEnabled(false);
    QFont f = action->font();
    f.setBold(true);
    action->setFont(f);
    insertAction(before, action);
    return action;
}

QAction* KMenu::addTitle(const QIcon &icon, const QString &text, QAction* before)
{
    QAction* action = new QAction(icon, text, this);
    action->setEnabled(false);
    QFont f = action->font();
    f.setBold(true);
    action->setFont(f);
    insertAction(before, action);
    return action;
}

/**
 * This is re-implemented for keyboard navigation.
 */
void KMenu::closeEvent(QCloseEvent*e)
{
    if (d->shortcuts)
        resetKeyboardVars();
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
        return;
    }

    QAction* a = 0L;
    bool firstpass = true;
    QString keyString = e->text();

    // check for common commands dealt with by QPopup
    int key = e->key();
    if (key == Qt::Key_Escape || key == Qt::Key_Return || key == Qt::Key_Enter
            || key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_Left
            || key == Qt::Key_Right || key == Qt::Key_F1) {

        resetKeyboardVars();
        // continue event processing by Qpopup
        //e->ignore();
        d->keyboardModifiers = e->modifiers();
        QMenu::keyPressEvent(e);
        return;
    } else if ( key == Qt::Key_Shift || key == Qt::Key_Control || key == Qt::Key_Alt || key == Qt::Key_Meta )
        return QMenu::keyPressEvent(e);

    // check to see if the user wants to remove a key from the sequence (backspace)
    // or clear the sequence (delete)
    if (!d->keySeq.isNull()) {
        if (key == Qt::Key_Backspace) {

            if (d->keySeq.length() == 1) {
                resetKeyboardVars();
                return;
            }

            // keep the last sequence in keyString
            keyString = d->keySeq.left(d->keySeq.length() - 1);

            // allow sequence matching to be tried again
            resetKeyboardVars();

        } else if (key == Qt::Key_Delete) {
            resetKeyboardVars();

            // clear active item
            setActiveAction(0L);
            return;

        } else if (d->noMatches) {
            // clear if there are no matches
            resetKeyboardVars();

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
        resetKeyboardVars();
        return;
    }

    d->keySeq += keyString;
    int seqLen = d->keySeq.length();

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
        thisText = thisText.remove("&");

        // chop text to the search length
        thisText = thisText.left(seqLen);

        // do the search
        if (!thisText.find(d->keySeq, 0, false)) {

            if (firstpass) {
                // match
                setActiveAction(a);

                // check to see if we're underlining a different item
                if (d->lastHitAction != a)
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
                d->clearTimer.start(5000, true);

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
            resetKeyboardVars();

        } else if (d->lastHitAction && d->lastHitAction->menu()) {
            // only activate sub-menus
            d->lastHitAction->activate(QAction::Trigger);
            resetKeyboardVars();
        }

        return;
    }

    // no matches whatsoever, clean up
    resetKeyboardVars(true);
    //e->ignore();
    QMenu::keyPressEvent(e);
}

bool KMenu::focusNextPrevChild( bool next )
{
    resetKeyboardVars();
    return QMenu::focusNextPrevChild( next );
}

QString KMenu::underlineText(const QString& text, uint length)
{
    QString ret = text;
    for (uint i = 0; i < length; i++) {
        if (ret[2*i] != '&')
            ret.insert(2*i, "&");
    }
    return ret;
}

void KMenu::resetKeyboardVars(bool noMatches /* = false */)
{
    // Clean up keyboard variables
    if (d->lastHitAction) {
        d->lastHitAction->setText(d->originalText);
        d->lastHitAction = 0L;
    }

    if (!noMatches) {
        d->keySeq = QString::null;
    }

    d->noMatches = noMatches;
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

    QMenu::mousePressEvent(e);
}

void KMenu::mouseReleaseEvent(QMouseEvent* e)
{
    // Save the button, and the modifiers
    d->keyboardModifiers = e->modifiers();
    d->mouseButtons = e->buttons();

    if ( !d->ctxMenu || !d->ctxMenu->isVisible() )
        QMenu::mouseReleaseEvent(e);
}

QMenu* KMenu::contextMenu()
{
    if (!d->ctxMenu)
    {
        d->ctxMenu = new QMenu(this);
        connect(d->ctxMenu, SIGNAL(aboutToHide()), this, SLOT(ctxMenuHiding()));
    }

    return d->ctxMenu;
}

const QMenu* KMenu::contextMenu() const
{
    return const_cast< KMenu* >( this )->contextMenu();
}

void KMenu::hideContextMenu()
{
    d->continueCtxMenuShow = false;
}

void KMenu::actionHovered(QAction* action)
{
    if (!d->ctxMenu || !d->ctxMenu->isVisible())
    {
        return;
    }

    d->ctxMenu->hide();
    showCtxMenu(actionGeometry(action).center());
}

Q_DECLARE_METATYPE(KMenuContext)

static void KMenuSetActionData(QMenu *menu,KMenu* contextedMenu, QAction* contextedAction) {
    QList<QAction*> actions=menu->actions();
    QVariant v;
    v.setValue(KMenuContext(contextedMenu,contextedAction));
    for(int i=0;i<actions.count();i++) {
        actions[i]->setData(v);
    }
}

void KMenu::showCtxMenu(const QPoint &pos)
{
    if (d->highlightedAction)
        if (QMenu* subMenu = d->highlightedAction->menu())
            disconnect(subMenu, SIGNAL(aboutToShow()), this, SLOT(ctxMenuHideShowingMenu()));

    d->highlightedAction = activeAction();

    if (d->highlightedAction)
    {
        KMenuSetActionData(this,0,0);
        return;
    }

    emit aboutToShowContextMenu(this, d->highlightedAction, d->ctxMenu);
    KMenuSetActionData(this,this,d->highlightedAction);

    if (QMenu* subMenu = d->highlightedAction->menu())
    {
        connect(subMenu, SIGNAL(aboutToShow()), SLOT(ctxMenuHideShowingMenu()));
        QTimer::singleShot(100, subMenu, SLOT(hide()));
    }

    if (!d->continueCtxMenuShow)
    {
        d->continueCtxMenuShow = true;
        return;
    }

    d->ctxMenu->exec(this->mapToGlobal(pos));
    connect(this, SIGNAL(hovered(QAction*)), SLOT(actionHovered(QAction*)));
}

KMenu * KMenu::contextMenuFocus( )
{
  return qobject_cast<KMenu*>(QApplication::activePopupWidget());
}

QAction * KMenu::contextMenuFocusAction( )
{
  if (KMenu* menu = qobject_cast<KMenu*>(QApplication::activePopupWidget())) {
    QAction* action = menu->menuAction();
    QVariant var = action->data();
    KMenuContext ctx = var.value<KMenuContext>();
    Q_ASSERT(ctx.menu() == menu);
    return ctx.action();
  }

  return 0L;
}

/*
 * this method helps prevent submenus popping up while we have a context menu
 * showing
 */
void KMenu::ctxMenuHideShowingMenu()
{
    if (d->highlightedAction)
        if (QMenu* subMenu = d->highlightedAction->menu())
            QTimer::singleShot(0, subMenu, SLOT(hide()));
}

void KMenu::ctxMenuHiding()
{
    if (d->highlightedAction)
        if (QMenu* subMenu = d->highlightedAction->menu())
            disconnect(subMenu, SIGNAL(aboutToShow()), this, SLOT(ctxMenuHideShowingMenu()));

    connect(this, SIGNAL(hovered(QAction*)), this, SLOT(actionHovered(QAction*)));
    d->continueCtxMenuShow = true;
}

void KMenu::contextMenuEvent(QContextMenuEvent* e)
{
    if (d->ctxMenu)
    {
        if (e->reason() == QContextMenuEvent::Mouse)
        {
            showCtxMenu(e->pos());
        }
        else if (activeAction())
        {
            showCtxMenu(actionGeometry(activeAction()).center());
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
        blockSignals(true);
        d->ctxMenu->hide();
        blockSignals(false);
    }
    QMenu::hideEvent(e);
}
/**
 * end of RMB menus on menus support
 */

void KMenu::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

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
