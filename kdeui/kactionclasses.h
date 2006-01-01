/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>

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

#ifndef kactionclasses_h
#define kactionclasses_h

#include <qkeysequence.h>
#include <qobject.h>
#include <qpointer.h>

#include <kguiitem.h>
#include <kshortcut.h>
#include <kstdaction.h>
#include <kicontheme.h>
#include <ktoolbar.h>
#include <kaction.h>

class QMenuBar;
class QMenu;
class QComboBox;
class QPoint;
class QIcon;
class QString;
class KToolBar;

class KAccel;
class KAccelActions;
class KConfig;
class KConfigBase;
class KURL;
class KInstance;
class KActionCollection;
class KMenu;
class KMainWindow;

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
    Q_PROPERTY( bool checked READ isChecked WRITE setChecked )
    Q_PROPERTY( QString exclusiveGroup READ exclusiveGroup WRITE setExclusiveGroup )
public:

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
    KToggleAction( const QString& text, const KShortcut& cut = KShortcut(), KActionCollection* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( const QString& text, const KShortcut& cut,
                   const QObject* receiver, const char* slot, KActionCollection* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( const QString& text, const QIcon& pix, const KShortcut& cut = KShortcut(),
             KActionCollection* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
                   KActionCollection* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( const QString& text, const QIcon& pix, const KShortcut& cut,
                   const QObject* receiver, const char* slot, KActionCollection* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( const QString& text, const QString& pix, const KShortcut& cut,
                   const QObject* receiver, const char* slot,
                   KActionCollection* parent, const char* name = 0 );

    /**
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( KActionCollection* parent = 0, const char* name = 0 );

    /**
     * Destructor
     */
    virtual ~KToggleAction();

    /**
     *  "Plug" or insert this action into a given widget.
     *
     *  This will typically be a menu or a toolbar.  From this point
     *  on, you will never need to directly manipulate the item in the
     *  menu or toolbar.  You do all enabling/disabling/manipulation
     *  directly with your KToggleAction object.
     *
     *  @param widget The GUI element to display this action.
     *  @param index  The index of the item.
     */
    virtual int plug( QWidget* widget, int index = -1 );

    /**
     *  Returns the actual state of the action.
     */
    bool isChecked() const;

    /**
     * @return which "exclusive group" this action is part of.
     * @see setExclusiveGroup
     */
    QString exclusiveGroup() const;

    /**
     * Defines which "exclusive group" this action is part of.
     * In a given exclusive group, only one toggle action can be checked
     * at a any moment. Checking an action unchecks the other actions
     * of the group.
     */
    virtual void setExclusiveGroup( const QString& name );

    /**
     * Defines the text (and icon, tooltip, whatsthis) that should be displayed
     * instead of the normal text, when the action is checked.
     * This feature replaces the checkmark that usually appears in front of the text, in menus.
     * It is useful when the text is mainly a verb: e.g. "Show <foo>"
     * should turn into "Hide <foo>" when activated.
     *
     * If hasIcon(), the icon is kept for the 'checked state', unless
     * @p checkedItem defines an icon explicitely. Same thing for tooltip and whatsthis.
     * @since 3.3
     */
    void setCheckedState( const KGuiItem& checkedItem );

    /// Reimplemented for internal reasons
    virtual QString toolTip() const;

public slots:
    /**
     *  Sets the state of the action.
     */
    virtual void setChecked( bool );

protected slots:
    virtual void slotActivated();

protected:
    virtual void updateChecked( int id );

signals:
    void toggled( bool );

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KToggleActionPrivate;
    KToggleActionPrivate *d;
};

/**
 *  @short Radiobox like action.
 *
 * An action that operates like a radio button. At any given time
 * only a single action from the group will be active.
 */
class KDEUI_EXPORT KRadioAction : public KToggleAction
{
  Q_OBJECT
public:
    /**
     * Constructs a radio action with text and potential keyboard
     * accelerator but nothing else. Use this only if you really
     * know what you are doing.
     *
     * @param text The text that will be displayed.
     * @param cut The corresponding keyboard accelerator (shortcut).
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KRadioAction( const QString& text, const KShortcut& cut = KShortcut(), KActionCollection* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( const QString& text, const KShortcut& cut,
                  const QObject* receiver, const char* slot, KActionCollection* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( const QString& text, const QIcon& pix, const KShortcut& cut = KShortcut(),
                  KActionCollection* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
                  KActionCollection* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( const QString& text, const QIcon& pix, const KShortcut& cut,
                  const QObject* receiver, const char* slot, KActionCollection* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( const QString& text, const QString& pix, const KShortcut& cut,
                  const QObject* receiver, const char* slot,
                  KActionCollection* parent, const char* name = 0 );

    /**
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( KActionCollection* parent = 0, const char* name = 0 );

protected:
    virtual void slotActivated();

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KRadioActionPrivate;
    KRadioActionPrivate *d;
};

/**
 *  @short Action for selecting one of several items
 *
 *  Action for selecting one of several items.
 *
 *  This action shows up a submenu with a list of items.
 *  One of them can be checked. If the user clicks on an item
 *  this item will automatically be checked,
 *  the formerly checked item becomes unchecked.
 *  There can be only one item checked at a time.
 */
class KDEUI_EXPORT KSelectAction : public KAction
{
    Q_OBJECT
    Q_PROPERTY( int currentItem READ currentItem WRITE setCurrentItem )
    Q_PROPERTY( QStringList items READ items WRITE setItems )
    Q_PROPERTY( bool editable READ isEditable WRITE setEditable )
    Q_PROPERTY( int comboWidth READ comboWidth WRITE setComboWidth )
    Q_PROPERTY( QString currentText READ currentText )
    Q_PROPERTY( bool menuAccelsEnabled READ menuAccelsEnabled WRITE setMenuAccelsEnabled )
public:

    /**
     * Constructs a select action with text and potential keyboard
     * accelerator but nothing else. Use this only if you really
     * know what you are doing.
     *
     * @param text The text that will be displayed.
     * @param cut The corresponding keyboard accelerator (shortcut).
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KSelectAction( const QString& text, const KShortcut& cut = KShortcut(), KActionCollection* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( const QString& text, const KShortcut& cut,
                   const QObject* receiver, const char* slot, KActionCollection* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( const QString& text, const QIcon& pix, const KShortcut& cut = KShortcut(),
             KActionCollection* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
                   KActionCollection* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( const QString& text, const QIcon& pix, const KShortcut& cut,
                   const QObject* receiver, const char* slot, KActionCollection* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( const QString& text, const QString& pix, const KShortcut& cut,
                   const QObject* receiver, const char* slot,
                   KActionCollection* parent, const char* name = 0 );

    /**
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( KActionCollection* parent = 0, const char* name = 0 );

    /**
     * Destructor
     */
    virtual ~KSelectAction();

    /**
     *  "Plug" or insert this action into a given widget.
     *
     *  This will typically be a menu or a toolbar.
     *  From this point on, you will never need to directly
     *  manipulate the item in the menu or toolbar.
     *  You do all enabling/disabling/manipulation directly with your KSelectAction object.
     *
     *  @param widget The GUI element to display this action.
     *  @param index  The index of the item.
     */
    virtual int plug( QWidget* widget, int index = -1 );

    /**
     * When this action is plugged into a toolbar, it creates a combobox.
     * @return true if the combo editable.
     */
    virtual bool isEditable() const;

    /**
     * @return the items that can be selected with this action.
     * Use setItems to set them.
     */
    virtual QStringList items() const;

    /**
     * Changes the text of item @param index to @param text .
     */
    virtual void changeItem( int index, const QString& text );

    /**
     * Returns the text of the currently selected item.
     */
    virtual QString currentText() const;

    /**
     * Returns the index of the current item.
     * @see setCurrentItem
     */
    virtual int currentItem() const;

    /**
     * When this action is plugged into a toolbar, it creates a combobox.
     * This returns the maximum width set by setComboWidth
     */
    virtual int comboWidth() const;

    /**
     * Sets the maximum items that are visible at once if the action
     * is a combobox, that is the number of items in the combobox's viewport
     * Only works before the action is plugged
     * @since 3.5
     */
    void setMaxComboViewCount( int n );

    /**
     * Returns a pointer to the popup menu used by this action.
     */
    QMenu* popupMenu() const;

    /**
     * @deprecated See setMenuAccelsEnabled .
     * @since 3.1
     */
    void setRemoveAmpersandsInCombo( bool b ) KDE_DEPRECATED;
    /// @since 3.1
    bool removeAmpersandsInCombo() const;

    /**
     * Sets whether any occurrence of the ampersand character ( &amp; ) in items
     * should be interpreted as keyboard accelerator for items displayed in a
     * menu or not.
     * @since 3.1
     */
    void setMenuAccelsEnabled( bool b );
    /// @since 3.1
    bool menuAccelsEnabled() const;

    virtual bool isShortcutConfigurable() const { return false; }

public slots:
    /**
     *  Sets the currently checked item.
     *
     *  @param index Index of the item (remember the first item is zero).
     */
    virtual void setCurrentItem( int index );

    /**
     * Sets the items to be displayed in this action
     * You need to call this.
     */
    virtual void setItems( const QStringList &lst );

    /**
     * Clears up all the items in this action
     */
    virtual void clear();

    /**
     * When this action is plugged into a toolbar, it creates a combobox.
     * This makes the combo editable or read-only.
     */
    virtual void setEditable( bool );

    /**
     * When this action is plugged into a toolbar, it creates a combobox.
     * This gives a _maximum_ size to the combobox.
     * The minimum size is automatically given by the contents (the items).
     */
    virtual void setComboWidth( int width );

protected:
    virtual void changeItem( int id, int index, const QString& text );

    /**
     * Depending on the menuAccelsEnabled property this method will return the
     * actions items in a way for inclusion in a combobox with the ampersand
     * character removed from all items or not.
     * @since 3.1
     */
    QStringList comboItems() const;

protected slots:
    virtual void slotActivated( int id );
    virtual void slotActivated( const QString &text );
    virtual void slotActivated();

signals:
    /**
     * This signal is emitted when an item is selected; @param index indicated
     * the item selected.
     */
    void activated( int index );
    /**
     * This signal is emitted when an item is selected; @param text indicates
     * the item selected.
     */
    void activated( const QString& text );

protected:
    virtual void updateCurrentItem( int id );

    virtual void updateComboWidth( int id );

    virtual void updateItems( int id );

    virtual void updateClear( int id );

protected:
    virtual void virtual_hook( int id, void* data );
private:
    void setupMenu() const;
    class KSelectActionPrivate;
    KSelectActionPrivate *d;

};

/// Remove this class in KDE-4.0. It doesn't add _anything_ to KSelectAction
/**
 * @deprecated Use KSelectAction instead.
 */
class KDEUI_EXPORT_DEPRECATED KListAction : public KSelectAction
{
    Q_OBJECT
public:
    /**
     * Constructs a list action with text and potential keyboard
     * accelerator but nothing else. Use this only if you really
     * know what you are doing.
     *
     * @param text The text that will be displayed.
     * @param cut The corresponding keyboard accelerator (shortcut).
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KListAction( const QString& text, const KShortcut& cut = KShortcut(), KActionCollection* parent = 0,
                  const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KListAction( const QString& text, const KShortcut& cut, const QObject* receiver,
                  const char* slot, KActionCollection* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KListAction( const QString& text, const QIcon& pix, const KShortcut& cut = KShortcut(),
                      KActionCollection* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KListAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
                      KActionCollection* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KListAction( const QString& text, const QIcon& pix, const KShortcut& cut,
                          const QObject* receiver, const char* slot, KActionCollection* parent,
                  const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KListAction( const QString& text, const QString& pix, const KShortcut& cut,
                 const QObject* receiver, const char* slot, KActionCollection* parent,
                 const char* name = 0 );

    /**
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KListAction( KActionCollection* parent = 0, const char* name = 0 );

    /**
     * Destructor
     */
    virtual ~KListAction();


    virtual QString currentText() const;
    virtual int currentItem() const;


public slots:
    /**
     *  Sets the currently checked item.
     *
     *  @param index Index of the item (remember the first item is zero).
     */
    virtual void setCurrentItem( int index );

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KListActionPrivate;
    KListActionPrivate *d;
};

/**
 *  @short Recent files action
 *
 *  This class is an action to handle a recent files submenu.
 *  The best way to create the action is to use KStdAction::openRecent.
 *  Then you simply need to call loadEntries on startup, saveEntries
 *  on shutdown, addURL when your application loads/saves a file.
 *
 *  @author Michael Koch
 */
class KDEUI_EXPORT KRecentFilesAction : public KListAction  // TODO public KSelectAction
{
  Q_OBJECT
  Q_PROPERTY( int maxItems READ maxItems WRITE setMaxItems )
public:
  /**
   *  @param text The text that will be displayed.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( const QString& text, const KShortcut& cut,
                      KActionCollection* parent, const char* name = 0,
                      int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param receiver The SLOT's parent.
   *  @param slot The SLOT to invoke when a URL is selected.
   *  Its signature is of the form slotURLSelected( const KURL & ).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( const QString& text, const KShortcut& cut,
                      const QObject* receiver, const char* slot,
                      KActionCollection* parent, const char* name = 0,
                      int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The icons that go with this action.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( const QString& text, const QIcon& pix, const KShortcut& cut,
                      KActionCollection* parent, const char* name = 0,
                      int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The dynamically loaded icon that goes with this action.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( const QString& text, const QString& pix, const KShortcut& cut,
                      KActionCollection* parent, const char* name = 0,
                      int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The icons that go with this action.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param receiver The SLOT's parent.
   *  @param slot The SLOT to invoke when a URL is selected.
   *  Its signature is of the form slotURLSelected( const KURL & ).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( const QString& text, const QIcon& pix, const KShortcut& cut,
                      const QObject* receiver, const char* slot,
                      KActionCollection* parent, const char* name = 0,
                      int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The dynamically loaded icon that goes with this action.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param receiver The SLOT's parent.
   *  @param slot The SLOT to invoke when a URL is selected.
   *  Its signature is of the form slotURLSelected( const KURL & ).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( const QString& text, const QString& pix, const KShortcut& cut,
                      const QObject* receiver, const char* slot,
                      KActionCollection* parent, const char* name = 0,
                      int maxItems = 10 );

  /**
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( KActionCollection* parent = 0, const char* name = 0,
                      int maxItems = 10 );

  /**
   *  Destructor.
   */
  virtual ~KRecentFilesAction();

  virtual int plug( QWidget *widget, int index = -1 );

  /**
   *  Returns the maximum of items in the recent files list.
   */
  int maxItems() const;

public slots:
  /**
   *  Sets the maximum of items in the recent files list.
   *  The default for this value is 10 set in the constructor.
   *
   *  If this value is lesser than the number of items currently
   *  in the recent files list the last items are deleted until
   *  the number of items are equal to the new maximum.
   */
  void setMaxItems( int maxItems );

  /**
   *  Loads the recent files entries from a given KConfig object.
   *  You can provide the name of the group used to load the entries.
   *  If the groupname is empty, entries are load from a group called 'RecentFiles'
   *
   *  This method does not effect the active group of KConfig.
   */
  void loadEntries( KConfig* config, const QString &groupname=QString() );

  /**
   *  Saves the current recent files entries to a given KConfig object.
   *  You can provide the name of the group used to load the entries.
   *  If the groupname is empty, entries are saved to a group called 'RecentFiles'
   *
   *  This method does not effect the active group of KConfig.
   */
  void saveEntries( KConfig* config, const QString &groupname=QString() );

  /**
   *  Add URL to recent files list.
   *
   *  @param url The URL of the file
   *  @param name The user visible pretty name that appears before the URL
   *  @since 3.5
   */
  void addURL( const KURL& url, const QString& name = QString() ); 


  /**
   *  Remove an URL from the recent files list.
   *
   *  @param url The URL of the file
   */
  void removeURL( const KURL& url );

  /**
   *  Removes all entries from the recent files list.
   */
  void clearURLList();

signals:

  /**
   *  This signal gets emited when the user selects an URL.
   *
   *  @param url The URL thats the user selected.
   */
  void urlSelected( const KURL& url );

protected slots:
  void itemSelected( const QString& string );
  void menuAboutToShow();
  void menuItemActivated( int id );
  void slotClicked();
  virtual void slotActivated(int);
  virtual void slotActivated(const QString& );
  virtual void slotActivated();

protected:
  virtual void virtual_hook( int id, void* data );

private:
  void init();

  class KRecentFilesActionPrivate;
  KRecentFilesActionPrivate *d;
};

class KDEUI_EXPORT KFontAction : public KSelectAction
{
    Q_OBJECT
    Q_PROPERTY( QString font READ font WRITE setFont )
public:
    KFontAction( const QString& text, const KShortcut& cut = KShortcut(), KActionCollection* parent = 0,
                 const char* name = 0 );
    KFontAction( const QString& text, const KShortcut& cut,
                 const QObject* receiver, const char* slot, KActionCollection* parent,
                 const char* name = 0 );
    KFontAction( const QString& text, const QIcon& pix, const KShortcut& cut = KShortcut(),
                 KActionCollection* parent = 0, const char* name = 0 );
    KFontAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
                 KActionCollection* parent = 0, const char* name = 0 );
    KFontAction( const QString& text, const QIcon& pix, const KShortcut& cut,
                 const QObject* receiver, const char* slot, KActionCollection* parent,
                 const char* name = 0 );
    KFontAction( const QString& text, const QString& pix, const KShortcut& cut,
                 const QObject* receiver, const char* slot, KActionCollection* parent,
                 const char* name = 0 );

// The ctors with fontListCriteria were added after 3.3-beta1.
// This define is used in koffice. Remove when koffice has a dependency on kdelibs-3.3 or more.
#define KFONTACTION_HAS_CRITERIA_ARG
    KFontAction( uint fontListCriteria, const QString& text,
                 const KShortcut& cut = KShortcut(), KActionCollection* parent = 0,
                 const char* name = 0 );
    KFontAction( uint fontListCriteria, const QString& text, const QString& pix,
                 const KShortcut& cut = KShortcut(),
                 KActionCollection* parent = 0, const char* name = 0 );

    KFontAction( KActionCollection* parent = 0, const char* name = 0 );
    ~KFontAction();

    QString font() const {
        return currentText();
    }

    int plug( QWidget*, int index = -1 );

public slots:
    void setFont( const QString &family );

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KFontActionPrivate;
    KFontActionPrivate *d;
};

class KDEUI_EXPORT KFontSizeAction : public KSelectAction
{
    Q_OBJECT
    Q_PROPERTY( int fontSize READ fontSize WRITE setFontSize )
public:
    KFontSizeAction( const QString& text, const KShortcut& cut = KShortcut(), KActionCollection* parent = 0,
                     const char* name = 0 );
    KFontSizeAction( const QString& text, const KShortcut& cut, const QObject* receiver,
                     const char* slot, KActionCollection* parent, const char* name = 0 );
    KFontSizeAction( const QString& text, const QIcon& pix, const KShortcut& cut = KShortcut(),
                     KActionCollection* parent = 0, const char* name = 0 );
    KFontSizeAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
                     KActionCollection* parent = 0, const char* name = 0 );
    KFontSizeAction( const QString& text, const QIcon& pix, const KShortcut& cut,
                     const QObject* receiver, const char* slot,
                     KActionCollection* parent, const char* name = 0 );
    KFontSizeAction( const QString& text, const QString& pix, const KShortcut& cut,
                     const QObject* receiver, const char* slot,
                     KActionCollection* parent, const char* name = 0 );
    KFontSizeAction( KActionCollection* parent = 0, const char* name = 0 );

    virtual ~KFontSizeAction();

    virtual int fontSize() const;

public slots:
    virtual void setFontSize( int size );

protected slots:
    virtual void slotActivated( int );
    virtual void slotActivated( const QString& );
    virtual void slotActivated() { KAction::slotActivated(); }

signals:
    void fontSizeChanged( int );

private:
    void init();


protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KFontSizeActionPrivate;
    KFontSizeActionPrivate *d;
};


/**
 * A KActionMenu is an action that holds a sub-menu of other actions.
 * insert() and remove() allow to insert and remove actions into this action-menu.
 * Plugged in a popupmenu, it will create a submenu.
 * Plugged in a toolbar, it will create a button with a popup menu.
 *
 * This is the action used by the XMLGUI since it holds other actions.
 * If you want a submenu for selecting one tool among many (without icons), see KSelectAction.
 * See also setDelayed about the main action.
 */
class KDEUI_EXPORT KActionMenu : public KAction
{
  Q_OBJECT
  Q_PROPERTY( bool delayed READ delayed WRITE setDelayed )
  Q_PROPERTY( bool stickyMenu READ stickyMenu WRITE setStickyMenu )

public:
    KActionMenu( const QString& text, KActionCollection* parent = 0,
                 const char* name = 0 );
    KActionMenu( const QString& text, const QIcon& icon,
                 KActionCollection* parent = 0, const char* name = 0 );
    KActionMenu( const QString& text, const QString& icon,
                 KActionCollection* parent = 0, const char* name = 0 );
    KActionMenu( KActionCollection* parent = 0, const char* name = 0 );
    virtual ~KActionMenu();

    virtual void insert( KAction*, int index = -1 );
    virtual void remove( KAction* );

    KMenu* popupMenu() const;
    void popup( const QPoint& global );

    /**
     * Returns true if this action creates a delayed popup menu
     * when plugged in a KToolbar.
     */
    bool delayed() const;
    /**
     * If set to true, this action will create a delayed popup menu
     * when plugged in a KToolbar. Otherwise it creates a normal popup.
     * Default: delayed
     *
     * Remember that if the "main" action (the toolbar button itself)
     * cannot be clicked, then you should call setDelayed(false).
     *
     * On the opposite, if the main action can be clicked, it can only happen
     * in a toolbar: in a menu, the parent of a submenu can't be activated.
     * To get a "normal" menu item when plugged a menu (and no submenu)
     * use KToolBarPopupAction.
     */
    void setDelayed(bool _delayed);

    /**
     * Returns true if this action creates a sticky popup menu.
     * @see setStickyMenu().
     */
    bool stickyMenu() const;
    /**
     * If set to true, this action will create a sticky popup menu
     * when plugged in a KToolbar.
     * "Sticky", means it's visible until a selection is made or the mouse is
     * clicked elsewhere. This feature allows you to make a selection without
     * having to press and hold down the mouse while making a selection.
     * Default: sticky.
     */
    void setStickyMenu(bool sticky);

    virtual int plug( QWidget* widget, int index = -1 );

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KActionMenuPrivate;
    KActionMenuPrivate *d;
};

/**
 * This action is a normal action everywhere, except in a toolbar
 * where it also has a popupmenu (optionnally delayed). This action is designed
 * for history actions (back/forward, undo/redo) and for any other action
 * that has more detail in a toolbar than in a menu (e.g. tool chooser
 * with "Other" leading to a dialog...).
 */
class KDEUI_EXPORT KToolBarPopupAction : public KAction
{
  Q_OBJECT
  Q_PROPERTY( bool delayed READ delayed WRITE setDelayed )
  Q_PROPERTY( bool stickyMenu READ stickyMenu WRITE setStickyMenu )

public:
    //Not all constructors - because we need an icon, since this action only makes
    // sense when being plugged at least in a toolbar.
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
    KToolBarPopupAction( const QString& text, const QString& icon, const KShortcut& cut = KShortcut(),
                         KActionCollection* parent = 0, const char* name = 0 );

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
    KToolBarPopupAction( const QString& text, const QString& icon, const KShortcut& cut,
                         const QObject* receiver, const char* slot,
                         KActionCollection* parent = 0, const char* name = 0 );

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
    KToolBarPopupAction( const KGuiItem& item, const KShortcut& cut,
                         const QObject* receiver, const char* slot,
                         KActionCollection* parent, const char* name );

    virtual ~KToolBarPopupAction();

    virtual int plug( QWidget *widget, int index = -1 );

    /**
     * The popup menu that is shown when clicking (some time) on the toolbar
     * button. You may want to plug items into it on creation, or connect to
     * aboutToShow for a more dynamic menu.
     */
    KMenu *popupMenu() const;

    /**
     * Returns true if this action creates a delayed popup menu
     * when plugged in a KToolbar.
     */
    bool delayed() const;
    /**
     * If set to true, this action will create a delayed popup menu
     * when plugged in a KToolbar. Otherwise it creates a normal popup.
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
     * when plugged in a KToolbar.
     * "Sticky", means it's visible until a selection is made or the mouse is
     * clicked elsewhere. This feature allows you to make a selection without
     * having to press and hold down the mouse while making a selection.
     * Only available if delayed() is true.
     * Default: sticky.
     */
    void setStickyMenu(bool sticky);

private:
    KMenu *m_popup;
    bool m_delayed:1;
    bool m_stickyMenu:1;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KToolBarPopupActionPrivate;
    KToolBarPopupActionPrivate *d;
};

/**
 * An action that takes care of everything associated with
 * showing or hiding a toolbar by a menu action. It will
 * show or hide the toolbar with the given name when
 * activated, and check or uncheck itself if the toolbar
 * is manually shown or hidden.
 *
 * If you need to perfom some additional action when the
 * toolbar is shown or hidden, connect to the toggled(bool)
 * signal. It will be emitted after the toolbar's
 * visibility has changed, whenever it changes.
 * @since 3.1
 */
class KDEUI_EXPORT KToggleToolBarAction : public KToggleAction
{
    Q_OBJECT
public:
    /**
     * Create a KToggleToolbarAction that manages the toolbar
     * named toolBarName. This can be either the name of a
     * toolbar in an xml ui file, or a toolbar programmatically
     * created with that name.
     */
    KToggleToolBarAction( const char* toolBarName, const QString& text,
                          KActionCollection* parent, const char* name );
    KToggleToolBarAction( KToolBar *toolBar, const QString &text,
                          KActionCollection *parent, const char *name );
    virtual ~KToggleToolBarAction();

    virtual int plug( QWidget * widget, int index = -1 );

    KToolBar *toolBar() { return (KToolBar*)m_toolBar; }

public slots:
    virtual void setChecked( bool );

private:
    QByteArray               m_toolBarName;
    QPointer<KToolBar>  m_toolBar;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KToggleToolBarActionPrivate;
    KToggleToolBarActionPrivate *d;
};

/**
 * An action for switching between to/from full screen mode. Note that
 * QWidget::isFullScreen() may reflect the new or the old state
 * depending on how the action was triggered (by the application or
 * from the window manager). Also don't try to track the window state
 * yourself. Rely on this action's state (isChecked()) instead.
 *
 * Important: If you need to set/change the fullscreen state manually,
 * use the relevant QWidget function (showFullScreen etc.), do not
 * call directly the slot connected to the toggled() signal. The slot
 * still needs to explicitly set the window state though.
 * @since 3.2
 */
class KDEUI_EXPORT KToggleFullScreenAction : public KToggleAction
{
    Q_OBJECT
public:
    /**
     * Create a KToggleFullScreenAction
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param window the window that will switch to/from full screen mode
     *  @param name An internal name for this action.
     */
    KToggleFullScreenAction( const KShortcut &cut,
                             const QObject* receiver, const char* slot,
                             KActionCollection* parent, QWidget* window,
                             const char* name );
    virtual ~KToggleFullScreenAction();

    /**
     * Sets the window that will be related to this action.
     */
    void setWindow( QWidget* window );
public slots:
    virtual void setChecked( bool );
protected:
    /**
     * @internal
     */
    virtual bool eventFilter( QObject* o, QEvent* e );
private:
    QWidget* window;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KToggleFullScreenActionPrivate;
    KToggleFullScreenActionPrivate *d;
};


/**
 * An action that automatically embeds a widget into a
 * toolbar.
 */
class KDEUI_EXPORT KWidgetAction : public KAction
{
    Q_OBJECT
public:
    /**
     * Create an action that will embed widget into a toolbar
     * when plugged. This action may only be plugged into
     * a toolbar.
     */
    KWidgetAction( QWidget* widget, const QString& text,
                   const KShortcut& cut,
                   const QObject* receiver, const char* slot,
                   KActionCollection* parent, const char* name );
    virtual ~KWidgetAction();

    /**
     * Returns the widget associated with this action.
     */
    QWidget* widget() { return (QWidget*)m_widget; }

    void setAutoSized( bool );

    /**
     * Plug the action. The widget passed to the constructor
     * will be reparented to w, which must inherit KToolBar.
     */
    virtual int plug( QWidget* widget, int index = -1 );
    /**
     * Unplug the action. Ensures that the action is not
     * destroyed. It will be hidden and reparented to 0L instead.
     */
    virtual void unplug( QWidget *w );
protected slots:
    void slotToolbarDestroyed();
private:
    QPointer<QWidget> m_widget;
    bool                 m_autoSized;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KWidgetActionPrivate;
    KWidgetActionPrivate *d;
};

class KDEUI_EXPORT KActionSeparator : public KAction
{
    Q_OBJECT
public:
    KActionSeparator( KActionCollection* parent = 0, const char* name = 0 );
    virtual ~KActionSeparator();

    virtual int plug( QWidget *widget, int index = -1 );

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KActionSeparatorPrivate;
    KActionSeparatorPrivate *d;
};

/**
 * An action for pasting text from the clipboard.
 * It's useful for text handling applications as
 * when plugged into a toolbar it provides a menu
 * with the clipboard history if klipper is running.
 * If klipper is not running, the menu has only one
 * item: the current clipboard content.
 *
 * @since 3.2
 */
class KDEUI_EXPORT KPasteTextAction: public KAction
{
    Q_OBJECT
public:
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
    KPasteTextAction( const QString& text, const QString& icon, const KShortcut& cut,
                  const QObject* receiver, const char* slot,
                  KActionCollection* parent = 0, const char* name = 0 );

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

    virtual int plug( QWidget *widget, int index = -1 );

protected slots:
    void menuAboutToShow();
    void menuItemActivated( int id);
    virtual void slotActivated();

protected:
    virtual void virtual_hook( int id, void* data );

private:
    KMenu *m_popup;
    bool m_mixedMode;
    class KPasteTextActionPrivate;
    KPasteTextActionPrivate *d;
};

#endif
