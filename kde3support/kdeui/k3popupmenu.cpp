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

#include "k3popupmenu.h"

#include <kdebug.h>

class K3PopupMenu::K3PopupMenuPrivate
{
public:
    K3PopupMenuPrivate ()
        : noMatches(false)
        , shortcuts(false)
        , autoExec(false)
        , lastHitAction(0L)
#ifdef QT3_SUPPORT
        , state(Qt::NoButton)
#endif
        , mouseButtons(Qt::NoButton)
        , keyboardModifiers(Qt::NoModifier)
        , m_ctxMenu(0)
    {}

    ~K3PopupMenuPrivate ()
    {
        delete m_ctxMenu;
    }

    QString m_lastTitle;

    // variables for keyboard navigation
    QTimer clearTimer;

    bool noMatches : 1;
    bool shortcuts : 1;
    bool autoExec : 1;

    QString keySeq;
    QString originalText;

    QAction* lastHitAction;
#ifdef QT3_SUPPORT
    Qt::ButtonState state;
    Qt::MouseButtons mouseButtons;
    Qt::KeyboardModifiers keyboardModifiers;
#endif

    // support for RMB menus on menus
    Q3PopupMenu* m_ctxMenu;
    static bool s_continueCtxMenuShow;
    static QPointer<QAction> s_highlightedAction;
    // KDE4: deprecated
    static int s_highlightedItem;
    static K3PopupMenu* s_contextedMenu;
};

QPointer<QAction> K3PopupMenu::K3PopupMenuPrivate::s_highlightedAction(0L);
int K3PopupMenu::K3PopupMenuPrivate::s_highlightedItem(-1);
K3PopupMenu* K3PopupMenu::K3PopupMenuPrivate::s_contextedMenu(0);
bool K3PopupMenu::K3PopupMenuPrivate::s_continueCtxMenuShow(true);

K3PopupMenu::K3PopupMenu(QWidget *parent)
    : Q3PopupMenu(parent)
    , d(new K3PopupMenuPrivate())
{
    resetKeyboardVars();
    connect(&(d->clearTimer), SIGNAL(timeout()), SLOT(resetKeyboardVars()));
}

K3PopupMenu::~K3PopupMenu()
{
    if (K3PopupMenuPrivate::s_contextedMenu == this)
    {
        K3PopupMenuPrivate::s_contextedMenu = 0;
        K3PopupMenuPrivate::s_highlightedAction = 0L;
        K3PopupMenuPrivate::s_highlightedItem = -1;
    }

    delete d;
}

QAction* K3PopupMenu::addTitle(const QString &text, QAction* before)
{
    QAction* action = new QAction(text, this);
    action->setEnabled(false);
    QFont f = action->font();
    f.setBold(true);
    action->setFont(f);
    insertAction(before, action);
    return action;
}

QAction* K3PopupMenu::addTitle(const QIcon &icon, const QString &text, QAction* before)
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
void K3PopupMenu::closeEvent(QCloseEvent*e)
{
    if (d->shortcuts)
        resetKeyboardVars();
    Q3PopupMenu::closeEvent(e);
}

void K3PopupMenu::activateItemAt(int index)
{
#ifdef QT3_SUPPORT
    d->state = Qt::NoButton;
#endif
    d->mouseButtons = Qt::NoButton;
    d->keyboardModifiers = Qt::NoModifier;
    Q3PopupMenu::activateItemAt(index);
}

#ifdef QT3_SUPPORT
Qt::ButtonState K3PopupMenu::state() const
{
    return d->state;
}
#endif

Qt::MouseButtons K3PopupMenu::mouseButtons() const
{
    return d->mouseButtons;
}

Qt::KeyboardModifiers K3PopupMenu::keyboardModifiers() const
{
    return d->keyboardModifiers;
}

void K3PopupMenu::keyPressEvent(QKeyEvent* e)
{
#ifdef QT3_SUPPORT
    d->state = Qt::NoButton;
#endif
    d->mouseButtons = Qt::NoButton;
    d->keyboardModifiers = Qt::NoModifier;
    if (!d->shortcuts) {
        // continue event processing by Qpopup
        //e->ignore();
#ifdef QT3_SUPPORT
        d->state = e->state();
#endif
        d->keyboardModifiers = e->modifiers();
        Q3PopupMenu::keyPressEvent(e);
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
#ifdef QT3_SUPPORT
        d->state = e->state();
#endif
        d->keyboardModifiers = e->modifiers();
        Q3PopupMenu::keyPressEvent(e);
        return;
    } else if ( key == Qt::Key_Shift || key == Qt::Key_Control || key == Qt::Key_Alt || key == Qt::Key_Meta )
        return Q3PopupMenu::keyPressEvent(e);

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
    Q3PopupMenu::keyPressEvent(e);
}

bool K3PopupMenu::focusNextPrevChild( bool next )
{
    resetKeyboardVars();
    return Q3PopupMenu::focusNextPrevChild( next );
}

QString K3PopupMenu::underlineText(const QString& text, uint length)
{
    QString ret = text;
    for (uint i = 0; i < length; i++) {
        if (ret[2*i] != '&')
            ret.insert(2*i, "&");
    }
    return ret;
}

void K3PopupMenu::resetKeyboardVars(bool noMatches /* = false */)
{
    // Clean up keyboard variables
    if (d->lastHitAction) {
        d->lastHitAction->setText(d->originalText);
        d->lastHitAction = 0L;
    }

    if (!noMatches) {
        d->keySeq.clear();
    }

    d->noMatches = noMatches;
}

void K3PopupMenu::setKeyboardShortcutsEnabled(bool enable)
{
    d->shortcuts = enable;
}

void K3PopupMenu::setKeyboardShortcutsExecute(bool enable)
{
    d->autoExec = enable;
}
/**
 * End keyboard navigation.
 */

/**
 * RMB menus on menus
 */

void K3PopupMenu::mousePressEvent(QMouseEvent* e)
{
    if (d->m_ctxMenu && d->m_ctxMenu->isVisible())
    {
        // hide on a second context menu event
        d->m_ctxMenu->hide();
    }

    Q3PopupMenu::mousePressEvent(e);
}

void K3PopupMenu::mouseReleaseEvent(QMouseEvent* e)
{
#ifdef QT3_SUPPORT
    // Save the button, and the modifiers from state()
    d->state = Qt::ButtonState(e->button() | (e->state() & Qt::KeyboardModifierMask));
#endif
    // Save the button, and the modifiers
    d->keyboardModifiers = e->modifiers();
    d->mouseButtons = e->buttons();

    if ( !d->m_ctxMenu || !d->m_ctxMenu->isVisible() )
	Q3PopupMenu::mouseReleaseEvent(e);
}

Q3PopupMenu* K3PopupMenu::contextMenu()
{
    if (!d->m_ctxMenu)
    {
        d->m_ctxMenu = new Q3PopupMenu(this);
        connect(d->m_ctxMenu, SIGNAL(aboutToHide()), this, SLOT(ctxMenuHiding()));
    }

    return d->m_ctxMenu;
}

const Q3PopupMenu* K3PopupMenu::contextMenu() const
{
    return const_cast< K3PopupMenu* >( this )->contextMenu();
}

void K3PopupMenu::hideContextMenu()
{
    K3PopupMenuPrivate::s_continueCtxMenuShow = false;
}

QAction* K3PopupMenu::contextMenuFocusAction()
{
    return K3PopupMenuPrivate::s_highlightedAction;
}

K3PopupMenu* K3PopupMenu::contextMenuFocus()
{
    return K3PopupMenuPrivate::s_contextedMenu;
}

void K3PopupMenu::actionHovered(QAction* action)
{
    if (!d->m_ctxMenu || !d->m_ctxMenu->isVisible())
    {
        return;
    }

    d->m_ctxMenu->hide();
    showCtxMenu(actionGeometry(action).center());
}

void K3PopupMenu::showCtxMenu(const QPoint &pos)
{
    if (K3PopupMenuPrivate::s_highlightedAction)
        if (QMenu* subMenu = K3PopupMenuPrivate::s_highlightedAction->menu())
            disconnect(subMenu, SIGNAL(aboutToShow()), this, SLOT(ctxMenuHideShowingMenu()));

    K3PopupMenuPrivate::s_highlightedAction = activeAction();
    K3PopupMenuPrivate::s_highlightedItem = itemAtPos(pos);

    if (!K3PopupMenuPrivate::s_highlightedAction)
    {
        K3PopupMenuPrivate::s_contextedMenu = 0;
        return;
    }

    emit aboutToShowContextMenu(this, K3PopupMenuPrivate::s_highlightedAction, d->m_ctxMenu);
    emit aboutToShowContextMenu(this, K3PopupMenuPrivate::s_highlightedItem, d->m_ctxMenu);

    if (QMenu* subMenu = K3PopupMenuPrivate::s_highlightedAction->menu())
    {
        connect(subMenu, SIGNAL(aboutToShow()), SLOT(ctxMenuHideShowingMenu()));
        QTimer::singleShot(100, subMenu, SLOT(hide()));
    }

    if (!K3PopupMenuPrivate::s_continueCtxMenuShow)
    {
        K3PopupMenuPrivate::s_continueCtxMenuShow = true;
        return;
    }

    K3PopupMenuPrivate::s_contextedMenu = this;
    d->m_ctxMenu->exec(this->mapToGlobal(pos));
    connect(this, SIGNAL(hovered(QAction*)), SLOT(actionHovered(QAction*)));
}

/*
 * this method helps prevent submenus popping up while we have a context menu
 * showing
 */
void K3PopupMenu::ctxMenuHideShowingMenu()
{
    if (K3PopupMenuPrivate::s_highlightedAction)
        if (QMenu* subMenu = K3PopupMenuPrivate::s_highlightedAction->menu())
            QTimer::singleShot(0, subMenu, SLOT(hide()));
}

void K3PopupMenu::ctxMenuHiding()
{
    if (K3PopupMenuPrivate::s_highlightedAction)
        if (QMenu* subMenu = K3PopupMenuPrivate::s_highlightedAction->menu())
            disconnect(subMenu, SIGNAL(aboutToShow()), this, SLOT(ctxMenuHideShowingMenu()));

    connect(this, SIGNAL(hovered(QAction*)), this, SLOT(actionHovered(QAction*)));
    K3PopupMenuPrivate::s_continueCtxMenuShow = true;
}

void K3PopupMenu::contextMenuEvent(QContextMenuEvent* e)
{
    if (d->m_ctxMenu)
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

    Q3PopupMenu::contextMenuEvent(e);
}

void K3PopupMenu::hideEvent(QHideEvent *e)
{
    if (d->m_ctxMenu && d->m_ctxMenu->isVisible())
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
        d->m_ctxMenu->hide();
        blockSignals(false);
    }
    Q3PopupMenu::hideEvent(e);
}
/**
 * end of RMB menus on menus support
 */

void K3PopupMenu::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


K3PopupMenu::K3PopupMenu(const QString &title, QWidget *parent)
    : Q3PopupMenu(parent)
    , d(new K3PopupMenuPrivate())
{
    resetKeyboardVars();
    connect(&(d->clearTimer), SIGNAL(timeout()), SLOT(resetKeyboardVars()));
    addAction(title);
}

#ifdef QT3_SUPPORT
int K3PopupMenu::insertTitle(const QString &text, int id, int index)
{
    int newid = insertItem(text, id, index);
    QMenuItem* menuItem = findItem(newid);
    Q_ASSERT(menuItem);
    menuItem->setEnabled(false);
    QFont f = menuItem->font();
    f.setBold(true);
    menuItem->setFont(f);
    return newid;
}

int K3PopupMenu::insertTitle(const QPixmap &icon, const QString &text, int id, int index)
{
    int newid = insertItem(text, id, index);
    QMenuItem* menuItem = findItem(newid);
    Q_ASSERT(menuItem);
    menuItem->setEnabled(false);
    menuItem->setIcon(icon);
    QFont f = menuItem->font();
    f.setBold(true);
    menuItem->setFont(f);
    return newid;
}

void K3PopupMenu::changeTitle(int id, const QString &text)
{
    QMenuItem* menuItem = findItem(id);
    Q_ASSERT(menuItem);
    if (!menuItem)
        return;
    menuItem->setText(text);
    menuItem->setIcon(QIcon());
    return;
}

void K3PopupMenu::changeTitle(int id, const QPixmap &icon, const QString &text)
{
    QMenuItem* menuItem = findItem(id);
    Q_ASSERT(menuItem);
    if (!menuItem)
        return;
    menuItem->setText(text);
    menuItem->setIcon(icon);
    return;
}

QString K3PopupMenu::title(int id) const
{
    QMenuItem* menuItem = findItem(id);
    Q_ASSERT(menuItem);
    if (!menuItem)
        return QString();
    return menuItem->text();
}

QPixmap K3PopupMenu::titlePixmap(int id) const
{
    QMenuItem* menuItem = findItem(id);
    Q_ASSERT(menuItem);
    if (!menuItem)
        return QPixmap();
    return menuItem->icon().pixmap();
}

void K3PopupMenu::setTitle(const QString &title)
{
    addAction(title);
}

int K3PopupMenu::contextMenuFocusItem()
{
    return K3PopupMenuPrivate::s_highlightedItem;
}

#endif // END compat methods

#include "k3popupmenu.moc"
