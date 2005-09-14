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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include <qcursor.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qfontmetrics.h>
#include <QKeyEvent>
#include <QPointer>
#include <QMenuItem>

#include "kpopupmenu.h"

#include <kdebug.h>

class KPopupMenu::KPopupMenuPrivate
{
public:
    KPopupMenuPrivate ()
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

    ~KPopupMenuPrivate ()
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
    static KPopupMenu* s_contextedMenu;
};

QPointer<QAction> KPopupMenu::KPopupMenuPrivate::s_highlightedAction(0L);
int KPopupMenu::KPopupMenuPrivate::s_highlightedItem(-1);
KPopupMenu* KPopupMenu::KPopupMenuPrivate::s_contextedMenu(0);
bool KPopupMenu::KPopupMenuPrivate::s_continueCtxMenuShow(true);

KPopupMenu::KPopupMenu(QWidget *parent)
    : Q3PopupMenu(parent)
    , d(new KPopupMenuPrivate())
{
    resetKeyboardVars();
    connect(&(d->clearTimer), SIGNAL(timeout()), SLOT(resetKeyboardVars()));
}

KPopupMenu::~KPopupMenu()
{
    if (KPopupMenuPrivate::s_contextedMenu == this)
    {
        KPopupMenuPrivate::s_contextedMenu = 0;
        KPopupMenuPrivate::s_highlightedAction = 0L;
        KPopupMenuPrivate::s_highlightedItem = -1;
    }

    delete d;
}

QAction* KPopupMenu::addTitle(const QString &text, QAction* before)
{
    QAction* action = new QAction(text, this);
    action->setEnabled(false);
    QFont f = action->font();
    f.setBold(true);
    action->setFont(f);
    insertAction(before, action);
    return action;
}

QAction* KPopupMenu::addTitle(const QIcon &icon, const QString &text, QAction* before)
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
void KPopupMenu::closeEvent(QCloseEvent*e)
{
    if (d->shortcuts)
        resetKeyboardVars();
    Q3PopupMenu::closeEvent(e);
}

void KPopupMenu::activateItemAt(int index)
{
#ifdef QT3_SUPPORT
    d->state = Qt::NoButton;
#endif
    d->mouseButtons = Qt::NoButton;
    d->keyboardModifiers = Qt::NoModifier;
    Q3PopupMenu::activateItemAt(index);
}

#ifdef QT3_SUPPORT
Qt::ButtonState KPopupMenu::state() const
{
    return d->state;
}
#endif

Qt::MouseButtons KPopupMenu::mouseButtons() const
{
    return d->mouseButtons;
}

Qt::KeyboardModifiers KPopupMenu::keyboardModifiers() const
{
    return d->keyboardModifiers;
}

void KPopupMenu::keyPressEvent(QKeyEvent* e)
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

bool KPopupMenu::focusNextPrevChild( bool next )
{
    resetKeyboardVars();
    return Q3PopupMenu::focusNextPrevChild( next );
}

QString KPopupMenu::underlineText(const QString& text, uint length)
{
    QString ret = text;
    for (uint i = 0; i < length; i++) {
        if (ret[2*i] != '&')
            ret.insert(2*i, "&");
    }
    return ret;
}

void KPopupMenu::resetKeyboardVars(bool noMatches /* = false */)
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

void KPopupMenu::setKeyboardShortcutsEnabled(bool enable)
{
    d->shortcuts = enable;
}

void KPopupMenu::setKeyboardShortcutsExecute(bool enable)
{
    d->autoExec = enable;
}
/**
 * End keyboard navigation.
 */

/**
 * RMB menus on menus
 */

void KPopupMenu::mousePressEvent(QMouseEvent* e)
{
    if (d->m_ctxMenu && d->m_ctxMenu->isVisible())
    {
        // hide on a second context menu event
        d->m_ctxMenu->hide();
    }

    Q3PopupMenu::mousePressEvent(e);
}

void KPopupMenu::mouseReleaseEvent(QMouseEvent* e)
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

Q3PopupMenu* KPopupMenu::contextMenu()
{
    if (!d->m_ctxMenu)
    {
        d->m_ctxMenu = new Q3PopupMenu(this);
        connect(d->m_ctxMenu, SIGNAL(aboutToHide()), this, SLOT(ctxMenuHiding()));
    }

    return d->m_ctxMenu;
}

const Q3PopupMenu* KPopupMenu::contextMenu() const
{
    return const_cast< KPopupMenu* >( this )->contextMenu();
}

void KPopupMenu::hideContextMenu()
{
    KPopupMenuPrivate::s_continueCtxMenuShow = false;
}

QAction* KPopupMenu::contextMenuFocusAction()
{
    return KPopupMenuPrivate::s_highlightedAction;
}

KPopupMenu* KPopupMenu::contextMenuFocus()
{
    return KPopupMenuPrivate::s_contextedMenu;
}

void KPopupMenu::actionHovered(QAction* action)
{
    if (!d->m_ctxMenu || !d->m_ctxMenu->isVisible())
    {
        return;
    }

    d->m_ctxMenu->hide();
    showCtxMenu(actionGeometry(action).center());
}

void KPopupMenu::showCtxMenu(const QPoint &pos)
{
    if (KPopupMenuPrivate::s_highlightedAction)
        if (QMenu* subMenu = KPopupMenuPrivate::s_highlightedAction->menu())
            disconnect(subMenu, SIGNAL(aboutToShow()), this, SLOT(ctxMenuHideShowingMenu()));

    KPopupMenuPrivate::s_highlightedAction = activeAction();
    KPopupMenuPrivate::s_highlightedItem = itemAtPos(pos);

    if (!KPopupMenuPrivate::s_highlightedAction)
    {
        KPopupMenuPrivate::s_contextedMenu = 0;
        return;
    }

    emit aboutToShowContextMenu(this, KPopupMenuPrivate::s_highlightedAction, d->m_ctxMenu);
    emit aboutToShowContextMenu(this, KPopupMenuPrivate::s_highlightedItem, d->m_ctxMenu);

    if (QMenu* subMenu = KPopupMenuPrivate::s_highlightedAction->menu())
    {
        connect(subMenu, SIGNAL(aboutToShow()), SLOT(ctxMenuHideShowingMenu()));
        QTimer::singleShot(100, subMenu, SLOT(hide()));
    }

    if (!KPopupMenuPrivate::s_continueCtxMenuShow)
    {
        KPopupMenuPrivate::s_continueCtxMenuShow = true;
        return;
    }

    KPopupMenuPrivate::s_contextedMenu = this;
    d->m_ctxMenu->exec(this->mapToGlobal(pos));
    connect(this, SIGNAL(hovered(QAction*)), SLOT(actionHovered(QAction*)));
}

/*
 * this method helps prevent submenus popping up while we have a context menu
 * showing
 */
void KPopupMenu::ctxMenuHideShowingMenu()
{
    if (KPopupMenuPrivate::s_highlightedAction)
        if (QMenu* subMenu = KPopupMenuPrivate::s_highlightedAction->menu())
            QTimer::singleShot(0, subMenu, SLOT(hide()));
}

void KPopupMenu::ctxMenuHiding()
{
    if (KPopupMenuPrivate::s_highlightedAction)
        if (QMenu* subMenu = KPopupMenuPrivate::s_highlightedAction->menu())
            disconnect(subMenu, SIGNAL(aboutToShow()), this, SLOT(ctxMenuHideShowingMenu()));

    connect(this, SIGNAL(hovered(QAction*)), this, SLOT(actionHovered(QAction*)));
    KPopupMenuPrivate::s_continueCtxMenuShow = true;
}

void KPopupMenu::contextMenuEvent(QContextMenuEvent* e)
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

void KPopupMenu::hideEvent(QHideEvent *e)
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

void KPopupMenu::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


KPopupMenu::KPopupMenu(const QString &title, QWidget *parent)
    : Q3PopupMenu(parent)
    , d(new KPopupMenuPrivate())
{
    resetKeyboardVars();
    connect(&(d->clearTimer), SIGNAL(timeout()), SLOT(resetKeyboardVars()));
    addAction(title);
}

#ifdef QT3_SUPPORT
int KPopupMenu::insertTitle(const QString &text, int id, int index)
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

int KPopupMenu::insertTitle(const QPixmap &icon, const QString &text, int id, int index)
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

void KPopupMenu::changeTitle(int id, const QString &text)
{
    QMenuItem* menuItem = findItem(id);
    Q_ASSERT(menuItem);
    if (!menuItem)
        return;
    menuItem->setText(text);
    menuItem->setIcon(QIcon());
    return;
}

void KPopupMenu::changeTitle(int id, const QPixmap &icon, const QString &text)
{
    QMenuItem* menuItem = findItem(id);
    Q_ASSERT(menuItem);
    if (!menuItem)
        return;
    menuItem->setText(text);
    menuItem->setIcon(icon);
    return;
}

QString KPopupMenu::title(int id) const
{
    QMenuItem* menuItem = findItem(id);
    Q_ASSERT(menuItem);
    if (!menuItem)
        return QString::null;
    return menuItem->text();
}

QPixmap KPopupMenu::titlePixmap(int id) const
{
    QMenuItem* menuItem = findItem(id);
    Q_ASSERT(menuItem);
    if (!menuItem)
        return QPixmap();
    return menuItem->icon().pixmap();
}

void KPopupMenu::setTitle(const QString &title)
{
    addAction(title);
}

int KPopupMenu::contextMenuFocusItem()
{
    return KPopupMenuPrivate::s_highlightedItem;
}

#endif // END compat methods

#include "kpopupmenu.moc"
