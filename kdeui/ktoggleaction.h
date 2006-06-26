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

#ifndef KTOGGLEACTION_H
#define KTOGGLEACTION_H

#include <kaction.h>

class KGuiItem;

/**
 *  @short Checkbox like action.
 *
 *  Checkbox like action.
 *
 *  This action provides two states: checked or not.
 *
 */
class KDEUI_EXPORT KToggleAction : public KAction
{
    Q_OBJECT

  public:
    /**
     * Constructs an action in the specified KActionCollection.
     *
     * @param parent The action collection to contain this action.
     * @param name An internal name for this action.
     * @param exclusiveGroup the group of actions that this action is part of, for exclusive action groups
     */
    KToggleAction( KActionCollection* parent, const QString& name, QActionGroup* exclusiveGroup = 0L );

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
     * @param exclusiveGroup the group of actions that this action is part of, for exclusive action groups
     */
    KToggleAction( const QString& text, KActionCollection* parent,
                   const QString& name, QActionGroup* exclusiveGroup = 0L );

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
     * @param exclusiveGroup the group of actions that this action is part of, for exclusive action groups
     */
    KToggleAction( const KIcon& icon, const QString& text, KActionCollection* parent,
                   const QString& name, QActionGroup* exclusiveGroup = 0L );

    /**
     * \overload KSelectAction(const QIcon&, const QString&, KActionCollection*)
     *
     * This constructor differs from the above in that the icon is specified as
     * a icon name which can be loaded by KIconLoader.
     *
     * @param icon The name of the icon to load via KIconLoader.
     * @param text The text that will be displayed.
     * @param parent The action collection to contain this action.
     * @param name An internal name for this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KToggleAction( const QString& icon, const QString& text, KActionCollection* parent,
                                              const QString& name, QActionGroup* exclusiveGroup = 0L );

    /**
     * Constructs a toggle action with text and potential keyboard
     * accelerator but nothing else. Use this only if you really
     * know what you are doing.
     *
     * @param text The text that will be displayed.
     * @param cut The corresponding keyboard accelerator (shortcut).
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KToggleAction( const QString& text, const KShortcut& cut = KShortcut(),
                   KActionCollection* parent = 0, const QString& name = QString() );

    /**
     *  @param text The text that will be displayed.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KToggleAction( const QString& text, const KShortcut& cut,
                   const QObject* receiver, const char* slot, KActionCollection* parent, const QString& name = QString() );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KToggleAction( const QString& text, const QIcon& pix, const KShortcut& cut = KShortcut(),
             KActionCollection* parent = 0, const QString& name = QString() );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KToggleAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
                   KActionCollection* parent = 0, const QString& name = QString() );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KToggleAction( const QString& text, const QIcon& pix, const KShortcut& cut,
                   const QObject* receiver, const char* slot, KActionCollection* parent, const QString& name = QString() );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KToggleAction( const QString& text, const QString& pix, const KShortcut& cut,
                   const QObject* receiver, const char* slot,
                   KActionCollection* parent, const QString& name = QString() );

    /**
     * Destructor
     */
    virtual ~KToggleAction();

    /**
     * Defines the text (and icon, tooltip, whatsthis) that should be displayed
     * instead of the normal text, when the action is checked.
     * This feature replaces the checkmark that usually appears in front of the text, in menus.
     * It is useful when the text is mainly a verb: e.g. "Show <foo>"
     * should turn into "Hide <foo>" when activated.
     *
     * If hasIcon(), the icon is kept for the 'checked state', unless
     * @p checkedItem defines an icon explicitly. Same thing for tooltip and whatsthis.
     */
    void setCheckedState( const KGuiItem& checkedItem );

  protected Q_SLOTS:
    virtual void slotToggled( bool checked );

  private:
    class Private;
    Private* const d;

    Q_DISABLE_COPY( KToggleAction )
};

#endif
