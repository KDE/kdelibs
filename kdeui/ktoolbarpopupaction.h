/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

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

#ifndef KTOOLBARPOPUPACTION_H
#define KTOOLBARPOPUPACTION_H

#include <kaction.h>

class KMenu;

/**
 * This action is a normal action everywhere, except in a toolbar
 * where it also has a popupmenu (optionally delayed). This action is designed
 * for history actions (back/forward, undo/redo) and for any other action
 * that has more detail in a toolbar than in a menu (e.g. tool chooser
 * with "Other" leading to a dialog...).
 *
 * In contrast to KActionMenu, this action is a \e simple menuitem when plugged 
 * into a menu, and has a popup only in a toolbar.
 *
 * Use cases include Back/Forward, and Undo/Redo. Simple click is what's most commonly
 * used, and enough for menus, but in toolbars there is \e also an optional popup
 * to go back N steps or undo N steps.
 */
class KDEUI_EXPORT KToolBarPopupAction : public KAction, public QActionWidgetFactory
{
  Q_OBJECT
  Q_PROPERTY( bool delayed READ delayed WRITE setDelayed )
  Q_PROPERTY( bool stickyMenu READ stickyMenu WRITE setStickyMenu )

public:
    //Not all constructors - because we need an icon, since this action only makes
    // sense when being plugged at least in a toolbar.
    /**
     * Create a KToolBarPopupAction, with a text, an icon, a
     * parent and a name.
     *
     * @param icon The icon to display.
     * @param text The text that will be displayed.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KToolBarPopupAction( const KIcon& icon, const QString& text, KActionCollection* parent = 0, const QString& name = QString() );

    /**
     * Create a KToolBarPopupAction, with a text, an icon, an optional accelerator,
     * parent and name.
     *
     * @param text The text that will be displayed.
     * @param icon The icon to display.
     * @param cut The corresponding keyboard accelerator (shortcut).
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KToolBarPopupAction( const QString& text, const QString& icon, const KShortcut& cut = KShortcut(),
                         KActionCollection* parent = 0, const QString& name = QString() );

    /**
     * Create a KToolBarPopupAction, with a text, an icon, an accelerator,
     * a slot connected to the action, parent and name.
     *
     * If you do not want or have a keyboard accelerator, set the
     * @p cut param to 0.
     *
     * @param text The text that will be displayed.
     * @param icon The icon to display.
     * @param cut The corresponding keyboard accelerator (shortcut).
     * @param receiver The SLOT's owner.
     * @param slot The SLOT to invoke to execute this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KToolBarPopupAction( const QString& text, const QString& icon, const KShortcut& cut,
                         const QObject* receiver, const char* slot,
                         KActionCollection* parent = 0, const QString& name = QString() );

    /**
     * Create a KToolBarPopupAction, with a KGuiItem, an accelerator,
     * a slot connected to the action, parent and name. The text and the
     * icon are taken from the KGuiItem.
     *
     * If you do not want or have a keyboard accelerator, set the
     * @p cut param to 0.
     *
     * @param item The text and icon that will be displayed.
     * @param cut The corresponding keyboard accelerator (shortcut).
     * @param receiver The SLOT's owner.
     * @param slot The SLOT to invoke to execute this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KToolBarPopupAction( const KGuiItem& item, const KShortcut& cut,
                         const QObject* receiver, const char* slot,
                         KActionCollection* parent, const QString& name );

    virtual ~KToolBarPopupAction();

    /**
     * The popup menu that is shown when clicking (some time) on the toolbar
     * button. You may want to plug items into it on creation, or connect to
     * aboutToShow for a more dynamic menu.
     *
     * \deprecated use menu() instead
     */
    KDE_DEPRECATED KMenu *popupMenu() const;

    /**
     * Returns true if this action creates a delayed popup menu
     * when plugged in a KToolBar.
     */
    bool delayed() const;
    /**
     * If set to true, this action will create a delayed popup menu
     * when plugged in a KToolBar. Otherwise it creates a normal popup.
     * Default: delayed.
     */
    void setDelayed(bool delayed);
    /**
     * Returns true if this action creates a sticky popup menu.
     * @see setStickyMenu().
     */
    bool stickyMenu() const;
    /**
     * If set to true, this action will create a sticky popup menu
     * when plugged in a KToolBar.
     * "Sticky", means it's visible until a selection is made or the mouse is
     * clicked elsewhere. This feature allows you to make a selection without
     * having to press and hold down the mouse while making a selection.
     * Only available if delayed() is true.
     * Default: sticky.
     */
    void setStickyMenu(bool sticky);

    virtual QWidget* createToolBarWidget(QToolBar* parent);

private:
    bool m_delayed:1;
    bool m_stickyMenu:1;
private:
    class KToolBarPopupActionPrivate;
    KToolBarPopupActionPrivate *d;
};

#endif
