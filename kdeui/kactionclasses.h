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

#ifndef kactionclasses_h
#define kactionclasses_h

#include <QPointer>

#include <kguiitem.h>
#include <kstdaction.h>
#include <kicontheme.h>
#include <kaction.h>
#include <QToolButton>

class QMenuBar;
class QMenu;
class QComboBox;
class QPoint;
class QIcon;
class QString;
class KToolBar;

class KConfig;
class KConfigBase;
class KUrl;
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

public:
    /**
     * Constructs an action in the specified KActionCollection.
     *
     * @param parent The action collection to contain this action.
     */
    KToggleAction(KActionCollection* parent, const char* name, QActionGroup* exclusiveGroup = 0L);

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
     */
    KToggleAction(const QString& text, KActionCollection* parent, const char* name, QActionGroup* exclusiveGroup = 0L);

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
     */
    KToggleAction(const KIcon& icon, const QString& text, KActionCollection* parent, const char* name, QActionGroup* exclusiveGroup = 0L);

    /**
     * \overload KSelectAction(const QIcon&, const QString&, KActionCollection*)
     *
     * This constructor differs from the above in that the icon is specified as
     * a icon name which can be loaded by KIconLoader.
     *
     * @param icon The name of the icon to load via KIconLoader.
     * @param text The text that will be displayed.
     * @param parent The action collection to contain this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KToggleAction(const QString& icon, const QString& text, KActionCollection* parent, const char* name, QActionGroup* exclusiveGroup = 0L);

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
                   KActionCollection* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KToggleAction( const QString& text, const KShortcut& cut,
                   const QObject* receiver, const char* slot, KActionCollection* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KToggleAction( const QString& text, const QIcon& pix, const KShortcut& cut = KShortcut(),
             KActionCollection* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KToggleAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
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
    KDE_CONSTRUCTOR_DEPRECATED KToggleAction( const QString& text, const QIcon& pix, const KShortcut& cut,
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
    KDE_CONSTRUCTOR_DEPRECATED KToggleAction( const QString& text, const QString& pix, const KShortcut& cut,
                   const QObject* receiver, const char* slot,
                   KActionCollection* parent, const char* name = 0 );

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
     * @p checkedItem defines an icon explicitely. Same thing for tooltip and whatsthis.
     */
    void setCheckedState( const KGuiItem& checkedItem );

protected Q_SLOTS:
    virtual void slotToggled(bool checked);

private:
    void init();

    class KToggleActionPrivate;
    KToggleActionPrivate *d;
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
class KDEUI_EXPORT KSelectAction : public KAction, public QActionWidgetFactory
{
    Q_OBJECT
    Q_PROPERTY( QAction* currentAction READ currentAction WRITE setCurrentAction )
    Q_PROPERTY( bool editable READ isEditable WRITE setEditable )
    Q_PROPERTY( int comboWidth READ comboWidth WRITE setComboWidth )
    Q_PROPERTY( QString currentText READ currentText )
public:
    /**
     * Constructs an action in the specified KActionCollection.
     *
     * @param parent The action collection to contain this action.
     */
    KSelectAction(KActionCollection* parent, const char* name);

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
     */
    KSelectAction(const QString& text, KActionCollection* parent, const char* name);

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
     */
    KSelectAction(const KIcon& icon, const QString& text, KActionCollection* parent, const char* name);

    /**
     * \overload KSelectAction(const QIcon&, const QString&, KActionCollection*)
     *
     * This constructor differs from the above in that the icon is specified as
     * a icon name which can be loaded by KIconLoader.
     *
     * @param icon The name of the icon to load via KIconLoader.
     * @param text The text that will be displayed.
     * @param parent The action collection to contain this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KSelectAction(const QString& icon, const QString& text, KActionCollection* parent, const char* name);

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
    KDE_CONSTRUCTOR_DEPRECATED KSelectAction( const QString& text, const KShortcut& cut = KShortcut(), KActionCollection* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KSelectAction( const QString& text, const KShortcut& cut,
                   const QObject* receiver, const char* slot, KActionCollection* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KSelectAction( const QString& text, const QIcon& pix, const KShortcut& cut = KShortcut(),
             KActionCollection* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KSelectAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
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
    KDE_CONSTRUCTOR_DEPRECATED KSelectAction( const QString& text, const QIcon& pix, const KShortcut& cut,
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
    KDE_CONSTRUCTOR_DEPRECATED KSelectAction( const QString& text, const QString& pix, const KShortcut& cut,
                   const QObject* receiver, const char* slot,
                   KActionCollection* parent, const char* name = 0 );

    /**
     * Destructor
     */
    virtual ~KSelectAction();

    enum ToolBarMode {
      /// Creates a button which pops up a menu when interacted with, as defined by toolButtonPopupMode().
      MenuMode,
      /// Creates a combo box which contains the actions.
      /// This is the default.
      ComboBoxMode
    };

    /**
     * Returns which type of widget (combo box or button with drop-down menu) will be inserted
     * in a toolbar.
     */
    ToolBarMode toolBarMode() const;

    /**
     * Set the type of widget to be inserted in a toolbar to \a mode.
     */
    void setToolBarMode(ToolBarMode mode);

    /**
     * Returns the style for the list of actions, when this action is plugged
     * into a KToolBar.
     *
     * \sa QToolButton::setPopupMode()
     */
    QToolButton::ToolButtonPopupMode toolButtonPopupMode() const;

    /**
     * Set how this list of actions should behave when in popup mode and plugged into a toolbar.
     */
    void setToolButtonPopupMode(QToolButton::ToolButtonPopupMode mode);

    /**
     * The action group used to create exclusivity between the actions associated with this action.
     */
    QActionGroup* selectableActionGroup() const;

    /**
     * Returns the current QAction.
     * @see setCurrentAction
     */
    QAction* currentAction() const;

    /**
     * \overload currentAction()
     *
     * Returns the index of the current item.
     */
    int currentItem() const;

    /**
     * \overload currentAction()
     *
     * Returns the text of the currently selected item.
     */
    QString currentText() const;

    /**
     * Returns the list of selectable actions
     */
    QList<QAction*> actions() const;

    /**
     * Returns the action at \a index, if one exists.
     */
    QAction* action(int index) const;

    /**
     * Searches for an action with the specified \a text, using a search whose
     * case sensitivity is defined by \a cs.
     */
    QAction* action(const QString& text, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    /**
     *  Sets the currently checked item.
     *
     *  @param item the QAction to become the currently checked item.
     */
    void setCurrentAction(QAction* action);

    /**
     * \overload setCurrentAction(QAction*)
     *
     * Convenience function to set the currently checked action to be the action
     * at index \p index.
     *
     * If there is no action at that index, the currently checked action (if any) will
     * be deselected.
     *
     * \return \e true if a corresponding action was found and thus set to the current action, otherwise \e false
     */
    bool setCurrentItem(int index);

    /**
     * \overload setCurrentAction(QAction*)
     *
     * Convenience function to set the currently checked action to be the action
     * which has \p text as its text().
     *
     * If there is no action at that index, the currently checked action (if any) will
     * be deselected.
     *
     * \return \e true if a corresponding action was found, otherwise \e false
     */
    bool setCurrentAction(const QString& text, Qt::CaseSensitivity cs = Qt::CaseSensitive);

    /**
     * Add \a action to the list of selectable actions.
     */
    virtual void addAction(QAction* action);

    /**
     * \overload addAction(QAction* action)
     *
     * Convenience function which creates an action from \a text and inserts it into
     * the list of selectable actions.
     */
    QAction* addAction(const QString& text);

    /**
     * \overload addAction(QAction* action)
     *
     * Convenience function which creates an action from \a text and \a icon and inserts it into
     * the list of selectable actions.
     */
    QAction* addAction(const QIcon& icon, const QString& text);

    /**
     * Remove the specified \a action from this action selector.
     *
     * You take ownership here, so save or delete it in order to not leak the action.
     */
    virtual QAction* removeAction(QAction* action);

    /**
     * Convenience function to create the list of selectable items.
     * Any previously existing items will be cleared.
     */
    void setItems( const QStringList &lst );

    /**
     * Convenience function which returns the items that can be selected with this action.
     * It is the same as iterating selectableActionGroup()->actions() and looking at each
     * action's text().
     */
    QStringList items() const;

    /**
     * When this action is plugged into a toolbar, it creates a combobox.
     * @return true if the combo editable.
     */
    bool isEditable() const;

    /**
     * When this action is plugged into a toolbar, it creates a combobox.
     * This makes the combo editable or read-only.
     */
    void setEditable( bool );

    /**
     * When this action is plugged into a toolbar, it creates a combobox.
     * This returns the maximum width set by setComboWidth
     */
    int comboWidth() const;

    /**
     * When this action is plugged into a toolbar, it creates a combobox.
     * This gives a _maximum_ size to the combobox.
     * The minimum size is automatically given by the contents (the items).
     */
    void setComboWidth( int width );

    /**
     * Sets the maximum items that are visible at once if the action
     * is a combobox, that is the number of items in the combobox's viewport
     */
    void setMaxComboViewCount( int n );

    /**
     * Clears up all the items in this action.
     * \warning The actions will be deleted for backwards compatibility with KDE3.
     *          If you just want to remove all actions, use removeAllActions()
     */
    void clear();

    void removeAllActions();

    /**
     * Sets whether any occurrence of the ampersand character ( &amp; ) in items
     * should be interpreted as keyboard accelerator for items displayed in a
     * menu or not.  Only applies to (overloaded) methods dealing with QStrings,
     * not those dealing with QActions.
     *
     * Defaults to true.
     *
     * \param b true if ampersands indicate a keyboard accelerator, otherwise false.
     */
    void setMenuAccelsEnabled( bool b );

    /**
     * Returns whether ampersands passed to methods using QStrings are interpreted
     * as keyboard accelerator indicators or as literal ampersands.
     */
    bool menuAccelsEnabled() const;

    /**
     * Changes the text of item @param index to @param text .
     */
    void changeItem( int index, const QString& text );

    virtual QWidget* createToolBarWidget(QToolBar* parent);

Q_SIGNALS:
    /**
     * This signal is emitted when an item is selected; @param index indicated
     * the item selected.
     */
    void triggered( QAction* action );

    /**
     * This signal is emitted when an item is selected; @param index indicated
     * the item selected.
     */
    void triggered( int index );

    /**
     * This signal is emitted when an item is selected; @param text indicates
     * the item selected.
     */
    void triggered( const QString& text );

protected Q_SLOTS:
    /**
     * This function is called whenever an action from the selections is triggered.
     */
    virtual void actionTriggered(QAction* action);

private Q_SLOTS:
    void comboBoxDeleted(QObject* object);

private:
    void init();

    class KSelectActionPrivate;
    KSelectActionPrivate* const d;
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
class KDEUI_EXPORT KRecentFilesAction : public KSelectAction
{
  Q_OBJECT
  Q_PROPERTY( int maxItems READ maxItems WRITE setMaxItems )

public:
    /**
     * Constructs an action in the specified KActionCollection.
     *
     * @param parent The action collection to contain this action.
     */
    KRecentFilesAction(KActionCollection* parent, const char* name);

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
     */
    KRecentFilesAction(const QString& text, KActionCollection* parent, const char* name);

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
     */
    KRecentFilesAction(const KIcon& icon, const QString& text, KActionCollection* parent, const char* name);

    /**
     * \overload KRecentFilesAction(const QIcon&, const QString&, KActionCollection*)
     *
     * This constructor differs from the above in that the icon is specified as
     * a icon name which can be loaded by KIconLoader.
     *
     * @param icon The name of the icon to load via KIconLoader.
     * @param text The text that will be displayed.
     * @param parent The action collection to contain this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KRecentFilesAction(const QString& icon, const QString& text, KActionCollection* parent, const char* name);

  /**
   *  @param text The text that will be displayed.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KDE_CONSTRUCTOR_DEPRECATED KRecentFilesAction( const QString& text, const KShortcut& cut,
                      KActionCollection* parent, const char* name = 0,
                      int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param receiver The SLOT's parent.
   *  @param slot The SLOT to invoke when a URL is selected.
   *  Its signature is of the form slotURLSelected( const KUrl & ).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KDE_CONSTRUCTOR_DEPRECATED KRecentFilesAction( const QString& text, const KShortcut& cut,
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
  KDE_CONSTRUCTOR_DEPRECATED KRecentFilesAction( const QString& text, const QIcon& pix, const KShortcut& cut,
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
  KDE_CONSTRUCTOR_DEPRECATED KRecentFilesAction( const QString& text, const QString& pix, const KShortcut& cut,
                      KActionCollection* parent, const char* name = 0,
                      int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The icons that go with this action.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param receiver The SLOT's parent.
   *  @param slot The SLOT to invoke when a URL is selected.
   *  Its signature is of the form slotURLSelected( const KUrl & ).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KDE_CONSTRUCTOR_DEPRECATED KRecentFilesAction( const QString& text, const QIcon& pix, const KShortcut& cut,
                      const QObject* receiver, const char* slot,
                      KActionCollection* parent, const char* name = 0,
                      int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The dynamically loaded icon that goes with this action.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param receiver The SLOT's parent.
   *  @param slot The SLOT to invoke when a URL is selected.
   *  Its signature is of the form slotURLSelected( const KUrl & ).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KDE_CONSTRUCTOR_DEPRECATED KRecentFilesAction( const QString& text, const QString& pix, const KShortcut& cut,
                      const QObject* receiver, const char* slot,
                      KActionCollection* parent, const char* name = 0,
                      int maxItems = 10 );

  /**
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KDE_CONSTRUCTOR_DEPRECATED KRecentFilesAction( KActionCollection* parent = 0, const char* name = 0,
                      int maxItems = 10 );

  /**
   *  Destructor.
   */
  virtual ~KRecentFilesAction();

  /**
   * Adds \a action to the list of URLs, with \a url and title \a name.
   *
   * Do not use addAction(QAction*), as no url will be associated, and
   * consequently urlSelected() will not be emitted when \a action is selected.
   */
  void addAction(QAction* action, const KUrl& url, const QString& name);

  /**
   * Reimplemented for internal reasons.
   */
  virtual QAction* removeAction(QAction* action);

  /**
   * Reimplemented for internal reasons.
   */
  virtual void clear();

  /**
   *  Returns the maximum of items in the recent files list.
   */
  int maxItems() const;

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
   */
  void addUrl( const KUrl& url, const QString& name = QString() );

  /**
   *  Remove an URL from the recent files list.
   *
   *  @param url The URL of the file
   */
  void removeUrl( const KUrl& url );

Q_SIGNALS:
  /**
   *  This signal gets emited when the user selects an URL.
   *
   *  @param url The URL thats the user selected.
   */
  void urlSelected( const KUrl& url );

private Q_SLOTS:
  void urlSelected( QAction* action );

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
    KFontAction(uint fontListCriteria, KActionCollection* parent, const char* name);
    KFontAction(KActionCollection* parent, const char* name);
    KFontAction(const QString& text, KActionCollection* parent, const char* name);
    KFontAction(const KIcon& icon, const QString& text, KActionCollection* parent, const char* name);
    KDE_CONSTRUCTOR_DEPRECATED KFontAction(const QString& icon, const QString& text, KActionCollection* parent, const char* name);

    KDE_CONSTRUCTOR_DEPRECATED KFontAction( const QString& text, const KShortcut& cut = KShortcut(), KActionCollection* parent = 0,
                 const char* name = 0 );
    KDE_CONSTRUCTOR_DEPRECATED KFontAction( const QString& text, const KShortcut& cut,
                 const QObject* receiver, const char* slot, KActionCollection* parent,
                 const char* name = 0 );
    KDE_CONSTRUCTOR_DEPRECATED KFontAction( const QString& text, const QIcon& pix, const KShortcut& cut = KShortcut(),
                 KActionCollection* parent = 0, const char* name = 0 );
    KDE_CONSTRUCTOR_DEPRECATED KFontAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
                 KActionCollection* parent = 0, const char* name = 0 );
    KDE_CONSTRUCTOR_DEPRECATED KFontAction( const QString& text, const QIcon& pix, const KShortcut& cut,
                 const QObject* receiver, const char* slot, KActionCollection* parent,
                 const char* name = 0 );
    KDE_CONSTRUCTOR_DEPRECATED KFontAction( const QString& text, const QString& pix, const KShortcut& cut,
                 const QObject* receiver, const char* slot, KActionCollection* parent,
                 const char* name = 0 );
    KDE_CONSTRUCTOR_DEPRECATED KFontAction( uint fontListCriteria, const QString& text,
                 const KShortcut& cut = KShortcut(), KActionCollection* parent = 0,
                 const char* name = 0 );
    KDE_CONSTRUCTOR_DEPRECATED KFontAction( uint fontListCriteria, const QString& text, const QString& pix,
                 const KShortcut& cut = KShortcut(),
                 KActionCollection* parent = 0, const char* name = 0 );

    virtual ~KFontAction();

    QString font() const {
        return currentText();
    }

    void setFont( const QString &family );

    virtual QWidget* createToolBarWidget(QToolBar* parent);

private:
    class KFontActionPrivate;
    KFontActionPrivate *d;
};

class KDEUI_EXPORT KFontSizeAction : public KSelectAction
{
    Q_OBJECT
    Q_PROPERTY( int fontSize READ fontSize WRITE setFontSize )

public:
    KFontSizeAction(KActionCollection* parent, const char* name);
    KFontSizeAction(const QString& text, KActionCollection* parent, const char* name);
    KFontSizeAction(const KIcon& icon, const QString& text, KActionCollection* parent, const char* name);
    KDE_CONSTRUCTOR_DEPRECATED KFontSizeAction(const QString& icon, const QString& text, KActionCollection* parent, const char* name);

    KDE_CONSTRUCTOR_DEPRECATED KFontSizeAction( const QString& text, const KShortcut& cut = KShortcut(), KActionCollection* parent = 0,
                     const char* name = 0 );
    KDE_CONSTRUCTOR_DEPRECATED KFontSizeAction( const QString& text, const KShortcut& cut, const QObject* receiver,
                     const char* slot, KActionCollection* parent, const char* name = 0 );
    KDE_CONSTRUCTOR_DEPRECATED KFontSizeAction( const QString& text, const QIcon& pix, const KShortcut& cut = KShortcut(),
                     KActionCollection* parent = 0, const char* name = 0 );
    KDE_CONSTRUCTOR_DEPRECATED KFontSizeAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
                     KActionCollection* parent = 0, const char* name = 0 );
    KDE_CONSTRUCTOR_DEPRECATED KFontSizeAction( const QString& text, const QIcon& pix, const KShortcut& cut,
                     const QObject* receiver, const char* slot,
                     KActionCollection* parent, const char* name = 0 );
    KDE_CONSTRUCTOR_DEPRECATED KFontSizeAction( const QString& text, const QString& pix, const KShortcut& cut,
                     const QObject* receiver, const char* slot,
                     KActionCollection* parent, const char* name = 0 );

    virtual ~KFontSizeAction();

    int fontSize() const;

    void setFontSize( int size );

Q_SIGNALS:
    void fontSizeChanged( int );

private slots:
    void slotTriggered();

private:
    void init();

    class KFontSizeActionPrivate;
    KFontSizeActionPrivate *d;
};


/**
 * A KActionMenu is an action that has several properties specific to holding a
 * sub-menu of other actions.
 *
 * Any QAction can be used to create a submenu.
 *
 * Plugged in a popupmenu, it will create a submenu.
 * Plugged in a toolbar, it will create a button with a popup menu.
 *
 * This is the action used by the XMLGUI since it holds other actions.
 * If you want a submenu for selecting one tool among many (without icons), see KSelectAction.
 * See also setDelayed about the main action.
 */
class KDEUI_EXPORT KActionMenu : public KAction, public QActionWidgetFactory
{
  Q_OBJECT
  Q_PROPERTY( bool delayed READ delayed WRITE setDelayed )
  Q_PROPERTY( bool stickyMenu READ stickyMenu WRITE setStickyMenu )

public:
    KActionMenu(KActionCollection* parent, const char* name);
    KActionMenu(const QString& text, KActionCollection* parent, const char* name);
    KActionMenu(const KIcon& icon, const QString& text, KActionCollection* parent, const char* name);
    virtual ~KActionMenu();

    KDE_DEPRECATED void insert( KAction*, QAction* before = 0L );
    KDE_DEPRECATED void remove( KAction* );

    void addAction(QAction* action);
    QAction* addSeparator();
    void insertAction(QAction* before, QAction* action);
    QAction* insertSeparator(QAction* before);
    void removeAction(QAction* action);

    /**
     * Returns this actions' menu as a KMenu, if it is one.
     * If none exists, one will be created.
     */
    inline KDE_DEPRECATED KMenu* popupMenu() { return kMenu(); }

    /**
     * Returns this actions' menu as a KMenu, if it is one.
     * If none exists, one will be created.
     */
    KMenu* kMenu();

    /**
     * Returns true if this action creates a delayed popup menu
     * when plugged in a KToolBar.
     */
    bool delayed() const;

    /**
     * If set to true, this action will create a delayed popup menu
     * when plugged in a KToolBar. Otherwise it creates a normal popup.
     * Default: delayed
     *
     * Remember that if the "main" action (the toolbar button itself)
     * cannot be clicked, then you should call setDelayed(false).
     *
     * On the opposite, if the main action can be clicked, it can only happen
     * in a toolbar: in a menu, the parent of a submenu can't be activated.
     * To get a "normal" menu item when plugged a menu (and no submenu)
     * use KToolBarMenuAction.
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
     * Default: sticky.
     */
    void setStickyMenu(bool sticky);

    virtual QWidget* createToolBarWidget(QToolBar* parent);

private:
    class KActionMenuPrivate* const d;
};

/**
 * This action is a normal action everywhere, except in a toolbar
 * where it also has a popupmenu (optionnally delayed). This action is designed
 * for history actions (back/forward, undo/redo) and for any other action
 * that has more detail in a toolbar than in a menu (e.g. tool chooser
 * with "Other" leading to a dialog...).
 *
 * FIXME KAction port - what does this add over KActionMenu?
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
    KToolBarPopupAction( const KIcon& icon, const QString& text, KActionCollection* parent = 0, const char* name = 0 );

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
    KDE_CONSTRUCTOR_DEPRECATED KToolBarPopupAction( const QString& text, const QString& icon, const KShortcut& cut,
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
    KDE_CONSTRUCTOR_DEPRECATED KToolBarPopupAction( const KGuiItem& item, const KShortcut& cut,
                         const QObject* receiver, const char* slot,
                         KActionCollection* parent, const char* name );

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

    KToolBar *toolBar();

    virtual bool eventFilter(QObject* watched, QEvent* event);

private Q_SLOTS:
    virtual void slotToggled(bool checked);

private:
    QByteArray          m_toolBarName;
    QPointer<KToolBar>  m_toolBar;
    bool                m_beingToggled;

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

 */
class KDEUI_EXPORT KToggleFullScreenAction : public KToggleAction
{
    Q_OBJECT
public:
    KToggleFullScreenAction( KActionCollection* parent, const char* name );
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

protected:
    bool eventFilter( QObject* o, QEvent* e );

private slots:
    virtual void slotToggled(bool checked);

private:
    QWidget* m_window;

    class KToggleFullScreenActionPrivate;
    KToggleFullScreenActionPrivate *d;
};


/**
 * An action that automatically embeds a widget into a
 * toolbar.
 *
 * \deprecated This action can only be plugged into one toolbar at
 *             a time.  Instead, create your own subclass of KAction,
 *             and provide a QActionWidgetFactory to create widgets
 *             on request.
 */
class KDEUI_EXPORT KWidgetAction : public KAction, public QActionWidgetFactory
{
    Q_OBJECT
public:
    /**
     * Create an action that will embed widget into a toolbar
     * when plugged. This action may only be plugged into
     * ONE toolbar.
     */
    KDE_CONSTRUCTOR_DEPRECATED KWidgetAction( QWidget* widget, const QString& text,
                   const KShortcut& cut,
                   const QObject* receiver, const char* slot,
                   KActionCollection* parent, const char* name );
    virtual ~KWidgetAction();

    /**
     * Returns the widget associated with this action.
     */
    KDE_DEPRECATED QWidget* widget() const { return m_widget; }

    virtual KDE_DEPRECATED QWidget* createToolBarWidget(QToolBar* parent);
    virtual KDE_DEPRECATED void destroyToolBarWidget(QWidget* widget);

protected:
    virtual bool event(QEvent* event);

private:
    QPointer<QWidget> m_widget;

    class KWidgetActionPrivate;
    KWidgetActionPrivate *d;
};

class KDEUI_EXPORT KActionSeparator : public KAction
{
    Q_OBJECT
public:
    KActionSeparator( KActionCollection* parent = 0, const char* name = 0 );

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
 */
class KDEUI_EXPORT KPasteTextAction: public KAction
{
    Q_OBJECT
public:
    /**
     * Constructs an action in the specified KActionCollection.
     *
     * @param parent The action collection to contain this action.
     */
    KPasteTextAction(KActionCollection* parent, const char* name);

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
     */
    KPasteTextAction(const QString& text, KActionCollection* parent, const char* name);

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
     */
    KPasteTextAction(const KIcon& icon, const QString& text, KActionCollection* parent, const char* name);

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
    KDE_CONSTRUCTOR_DEPRECATED KPasteTextAction(const QString& icon, const QString& text, KActionCollection* parent, const char* name);

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
