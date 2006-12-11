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

#ifndef KPASTETEXTACTION_H
#define KPASTETEXTACTION_H

#include <kaction.h>

class KMenu;

/**
 * An action for pasting text from the clipboard.
 * It's useful for text handling applications as
 * when plugged into a toolbar it provides a menu
 * with the clipboard history if klipper is running.
 * If klipper is not running, the menu has only one
 * item: the current clipboard content.
 */
class KDEUI_EXPORT KPasteTextAction: public KAction
{
    Q_OBJECT
public:
    /**
     * Constructs an action in the specified KActionCollection.
     *
     * @param parent The action collection to contain this action.
     * @param name An internal name for this action.
     */
    KPasteTextAction(KActionCollection* parent, const QString& name);

    /**
     * Constructs an action with text; a shortcut may be specified by
     * the ampersand character (e.g. "&amp;Option" creates a shortcut with key \e O )
     *
     * This is the most common KAction used when you do not have a
     * corresponding icon (note that it won't appear in the current version
     * of the "Edit ToolBar" dialog, because an action needs an icon to be
     * plugged in a toolbar...).
     *
     * @param text The text that will be displayed.
     * @param parent The action collection to contain this action.
     * @param name An internal name for this action.
     */
    KPasteTextAction(const QString& text, KActionCollection* parent, const QString& name);

    /**
     * Constructs an action with text and an icon; a shortcut may be specified by
     * the ampersand character (e.g. "&amp;Option" creates a shortcut with key \e O )
     *
     * This is the other common KAction used.  Use it when you
     * \e do have a corresponding icon.
     *
     * @param icon The icon to display.
     * @param text The text that will be displayed.
     * @param parent The action collection to contain this action.
     * @param name An internal name for this action.
     */
    KPasteTextAction(const KIcon& icon, const QString& text, KActionCollection* parent, const QString& name);

    /**
     * \overload KPasteTextAction(const QIcon&, const QString&, KActionCollection*)
     *
     * This constructor differs from the above in that the icon is specified as
     * a icon name which can be loaded by KIconLoader.
     *
     * @param icon The name of the icon to load via KIconLoader.
     * @param text The text that will be displayed.
     * @param parent The action collection to contain this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KPasteTextAction(const QString& icon, const QString& text, KActionCollection* parent, const QString& name);

    /**
     * Create a KPasteTextAction, with a text, an icon, an accelerator,
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
    KDE_CONSTRUCTOR_DEPRECATED KPasteTextAction( const QString& text, const QString& icon, const KShortcut& cut,
                  const QObject* receiver, const char* slot,
                  KActionCollection* parent = 0, const QString& name = QString() );

    virtual ~KPasteTextAction();

    /**
    * Controls the behavior of the clipboard history menu popup.
    *
    * @param mode If false and the clipboard contains a non-text object
    *             the popup menu with the clipboard history will appear
    *             immediately as the user clicks the toolbar action; if
    *             true, the action works like the standard paste action
    *             even if the current clipboard object is not text.
    *             Default value is true.
    */
    void setMixedMode(bool mode);

protected Q_SLOTS:
    void menuAboutToShow();
    void slotTriggered(QAction* action);

private:
    void init();

    KMenu *m_popup;
    bool m_mixedMode;

    class KPasteTextActionPrivate;
    KPasteTextActionPrivate *d;
};

#endif
