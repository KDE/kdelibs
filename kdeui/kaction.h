/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>

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
//$Id$

#ifndef __kaction_h__
#define __kaction_h__

#include <qkeysequence.h>
#include <qobject.h>
#include <qvaluelist.h>
#include <qguardedptr.h>
#include <kguiitem.h>
#include <kshortcut.h>
#include <kstdaction.h>
#include <kicontheme.h>

class QMenuBar;
class QPopupMenu;
class QComboBox;
class QPoint;
class QIconSet;
class QString;
class KToolBar;

class KAccel;
class KAccelActions;
class KConfig;
class KConfigBase;
class KURL;
class KInstance;
class KToolBar;
class KActionCollection;
class KPopupMenu;
class KMainWindow;

/**
 * The KAction class (and derived and super classes) provides a way to
 * easily encapsulate a "real" user-selected action or event in your
 * program.
 *
 * For instance, a user may want to @p paste the contents of
 * the clipboard or @p scroll @p down a document or @p quit the
 * application.  These are all @p actions -- events that the
 * user causes to happen.  The KAction class allows the developer to
 * deal with these actions in an easy and intuitive manner.
 *
 * Specifically, the KAction class encapsulated the various attributes
 * to an event/action.  For instance, an action might have an icon
 * that goes along with it (a clipboard for a "paste" action or
 * scissors for a "cut" action).  The action might have some text to
 * describe the action.  It will certainly have a method or function
 * that actually @p executes the action!  All these attributes
 * are contained within the KAction object.
 *
 * The advantage of dealing with Actions is that you can manipulate
 * the Action without regard to the GUI representation of it.  For
 * instance, in the "normal" way of dealing with actions like "cut",
 * you would manually insert a item for Cut into a menu and a button
 * into a toolbar.  If you want to disable the cut action for a moment
 * (maybe nothing is selected), you woud have to hunt down the pointer
 * to the menu item and the toolbar button and disable both
 * individually.  Setting the menu item and toolbar item up uses very
 * similar code - but has to be done twice!
 *
 * With the Action concept, you simply "plug" the Action into whatever
 * GUI element you want.  The KAction class will then take care of
 * correctly defining the menu item (with icons, accelerators, text,
 * etc) or toolbar button.. or whatever.  From then on, if you
 * manipulate the Action at all, the effect will propogate through all
 * GUI representations of it.  Back to the "cut" example: if you want
 * to disable the Cut Action, you would simply do
 * 'cutAction->setEnabled(false)' and the menuitem and button would
 * instantly be disabled!
 *
 * This is the biggest advantage to the Action concept -- there is a
 * one-to-one relationship between the "real" action and @p all
 * GUI representations of it.
 *
 * KAction emits the activated() signal if the user activated the 
 * corresponding GUI element ( menu item, toolbar button, etc. )
 *
 * If you are in the situation of wanting to map the activated() 
 * signal of multiple action objects to one slot, with a special 
 * argument bound to each action, then you might consider using 
 * @ref QSignalMapper . A tiny example:
 *
 * <PRE>
 * QSignalMapper *desktopNumberMapper = new QSignalMapper( this );
 * connect( desktopNumberMapper, SIGNAL( mapped( int ) ),
 *          this, SLOT( moveWindowToDesktop( int ) ) );
 *
 * for ( uint i = 0; i < numberOfDesktops; ++i ) {
 *     KAction *desktopAction = new KAction( i18n( "Move Window To Desktop %i" ).arg( i ), ... );
 *     connect( desktopAction, SIGNAL( activated() ), desktopNumberMapper, SLOT( map() ) );
 *     desktopNumberMapper->setMapping( desktopAction, i );
 * }
 * </PRE>
 *
 * @sect General Usage:
 *
 * The steps to using actions are roughly as follows
 *
 * @li Decide which attributes you want to associate with a given
 *     action (icons, text, keyboard shortcut, etc)
 * @li Create the action using KAction (or derived or super class).
 * @li "Plug" the Action into whatever GUI element you want.  Typically,
 *      this will be a menu or toolbar.
 *
 * @sect Detailed Example:
 *
 * Here is an example of enabling a "New [document]" action
 * <PRE>
 * KAction *newAct = new KAction(i18n("&New"), "filenew",
 *                               KStdAccel::shortcut(KStdAccel::New),
 *                               this, SLOT(fileNew()),
 *                               actionCollection(), "new");
 * </PRE>
 * This line creates our action.  It says that wherever this action is
 * displayed, it will use "&New" as the text, the standard icon, and
 * the standard shortcut.  It further says that whenever this action
 * is invoked, it will use the fileNew() slot to execute it.
 *
 * <PRE>
 * QPopupMenu *file = new QPopupMenu;
 * newAct->plug(file);
 * </PRE>
 * That just inserted the action into the File menu.  The point is, it's not
 * important in which menu it is: all manipulation of the item is
 * done through the newAct object.
 *
 * <PRE>
 * newAct->plug(toolBar());
 * </PRE>
 * And this inserted the Action into the main toolbar as a button.
 *
 * That's it!
 *
 * If you want to disable that action sometime later, you can do so
 * with
 * <PRE>
 * newAct->setEnabled(false)
 * </PRE>
 * and both the menuitem in File and the toolbar button will instantly
 * be disabled.
 *
 * Note: if you are using a "standard" action like "new", "paste",
 * "quit", or any other action described in the KDE UI Standards,
 * please use the methods in the @ref KStdAction class rather than
 * defining your own.
 *
 * @sect Usage Within the XML Framework:
 *
 * If you are using KAction within the context of the XML menu and
 * toolbar building framework, then there are a few tiny changes.  The
 * first is that you must insert your new action into an action
 * collection.  The action collection (a @ref KActionCollection) is,
 * logically enough, a central collection of all of the actions
 * defined in your application.  The XML UI framework code in KXMLGUI
 * classes needs access to this collection in order to build up the
 * GUI (it's how the builder code knows which actions are valid and
 * which aren't).
 *
 * Also, if you use the XML builder framework, then you do not ever
 * have to plug your actions into containers manually.  The framework
 * does that for you.
 *
 * @see KStdAction
 * @short Class to encapsulate user-driven action or event
 */
class KAction : public QObject
{
  friend class KActionCollection;
  Q_OBJECT
  Q_PROPERTY( int containerCount READ containerCount )
  Q_PROPERTY( QString plainText READ plainText )
  Q_PROPERTY( QString text READ text WRITE setText )
  Q_PROPERTY( QString shortcut READ shortcutText WRITE setShortcutText )
  Q_PROPERTY( bool enabled READ isEnabled WRITE setEnabled )
  Q_PROPERTY( QString group READ group WRITE setGroup )
  Q_PROPERTY( QString whatsThis READ whatsThis WRITE setWhatsThis )
  Q_PROPERTY( QString toolTip READ toolTip WRITE setToolTip )
  Q_PROPERTY( QString icon READ icon WRITE setIcon )
public:
    /// @since 3.1
    enum Scope { ScopeUnspecified, ScopeWidget, ScopeBuilder, ScopeGlobal };

    /**
     * Constructs an action with text, potential keyboard
     * shortcut, and a SLOT to call when this action is invoked by
     * the user.
     *
     * If you do not want or have a keyboard shortcut,
     * set the @p cut param to 0.
     *
     * This is the most common KAction used when you do not have a
     * corresponding icon (note that it won't appear in the current version
     * of the "Edit ToolBar" dialog, because an action needs an icon to be
     * plugged in a toolbar...).
     *
     * @param text The text that will be displayed.
     * @param cut The corresponding keyboard shortcut.
     * @param receiver The SLOT's parent.
     * @param slot The SLOT to invoke to execute this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KAction( const QString& text, const KShortcut& cut,
             const QObject* receiver, const char* slot,
             KActionCollection* parent, const char* name );

    /**
     * Constructs an action with text, icon, potential keyboard
     * shortcut, and a SLOT to call when this action is invoked by
     * the user.
     *
     * If you do not want or have a keyboard shortcut, set the
     * @p cut param to 0.
     *
     * This is the other common KAction used.  Use it when you
     * @p do have a corresponding icon.
     *
     * @param text The text that will be displayed.
     * @param pix The icon to display.
     * @param cut The corresponding keyboard shortcut.
     * @param receiver The SLOT's parent.
     * @param slot The SLOT to invoke to execute this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KAction( const QString& text, const QIconSet& pix, const KShortcut& cut,
             const QObject* receiver, const char* slot,
             KActionCollection* parent, const char* name );

    /**
     * Constructs an action with text, icon, potential keyboard
     * shortcut, and a SLOT to call when this action is invoked by
     * the user.  The icon is loaded on demand later based on where it
     * is plugged in.
     *
     * If you do not want or have a keyboard shortcut, set the
     * @p cut param to 0.
     *
     * This is the other common KAction used.  Use it when you
     * @p do have a corresponding icon.
     *
     * @param text The text that will be displayed.
     * @param pix The icon to display.
     * @param cut The corresponding keyboard shortcut (shortcut).
     * @param receiver The SLOT's parent.
     * @param slot The SLOT to invoke to execute this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KAction( const QString& text, const QString& pix, const KShortcut& cut,
             const QObject* receiver, const char* slot,
             KActionCollection* parent, const char* name );

    /**
     * The same as the above constructor, but with a KGuiItem providing
     * the text and icon.
     *
     * @param item The KGuiItem with the label and (optional) icon.
     */
    KAction( const KGuiItem& item, const KShortcut& cut,
             const QObject* receiver, const char* slot,
             KActionCollection* parent, const char* name );

	/**
	 * @obsolete
	 */
	KAction( const QString& text, const KShortcut& cut = KShortcut(), QObject* parent = 0, const char* name = 0 );
	/**
	 * @obsolete
	 */
	KAction( const QString& text, const KShortcut& cut,
		const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
	/**
	 * @obsolete
	 */
	KAction( const QString& text, const QIconSet& pix, const KShortcut& cut = KShortcut(),
		QObject* parent = 0, const char* name = 0 );
	/**
	 * @obsolete
	 */
	KAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
		QObject* parent = 0, const char* name = 0 );
	/**
	 * @obsolete
	 */
	KAction( const QString& text, const QIconSet& pix, const KShortcut& cut,
		const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
	/**
	 * @obsolete
	 */
	KAction( const QString& text, const QString& pix, const KShortcut& cut,
		const QObject* receiver, const char* slot, QObject* parent,
		const char* name = 0 );
	/**
	 * @obsolete
	 */
	KAction( QObject* parent = 0, const char* name = 0 );

    /**
     * Standard destructor
     */
    virtual ~KAction();

    /**
     * "Plug" or insert this action into a given widget.
     *
     * This will
     * typically be a menu or a toolbar.  From this point on, you will
     * never need to directly manipulate the item in the menu or
     * toolbar.  You do all enabling/disabling/manipulation directly
     * with your KAction object.
     *
     * @param w The GUI element to display this action
     */
    virtual int plug( QWidget *w, int index = -1 );

    /**
     * @deprecated.  Shouldn't be used.  No substitute available.
     *
     * "Plug" or insert this action into a given KAccel.
     *
     * @param accel The KAccel collection which holds this accel
     * @param configurable If the shortcut is configurable via
     * the KAccel configuration dialog (this is somehow deprecated since
     * there is now a KAction key configuration dialog).
     */
    virtual void plugAccel(KAccel *accel, bool configurable = true);

    /**
     * "Unplug" or remove this action from a given widget.
     *
     * This will typically be a menu or a toolbar.  This is rarely
     * used in "normal" application.  Typically, it would be used if
     * your application has several views or modes, each with a
     * completely different menu structure.  If you simply want to
     * disable an action for a given period, use @ref setEnabled()
     * instead.
     *
     * @param w Remove the action from this GUI element.
     */
    virtual void unplug( QWidget *w );

    /**
     * @deprecated.  Complement method to plugAccel().
     * Disconnect this action from the KAccel.
     */
    virtual void unplugAccel();

    /**
     * returns whether the action is plugged into any container widget or not.
     * @since 3.1
     */
    virtual bool isPlugged() const;

    /**
     * returns whether the action is plugged into the given container
     */
    bool isPlugged( const QWidget *container ) const;

    /**
     * returns whether the action is plugged into the given container with the given, container specific, id (often
     * menu or toolbar id ) .
     */
    virtual bool isPlugged( const QWidget *container, int id ) const;

    /**
     * returns whether the action is plugged into the given container with the given, container specific, representative
     * container widget item.
     */
    virtual bool isPlugged( const QWidget *container, const QWidget *_representative ) const;

    QWidget* container( int index ) const;
    int itemId( int index ) const;
    QWidget* representative( int index ) const;
    int containerCount() const;
    /// @since 3.1
    uint kaccelCount() const;

    virtual bool hasIcon() const;
#ifndef KDE_NO_COMPAT
    bool hasIconSet() const { return hasIcon(); }
#endif
    virtual QString plainText() const;

    /**
     * Get the text associated with this action.
     */
    virtual QString text() const;

    /**
     * Get the keyboard shortcut associated with this action.
     */
    virtual const KShortcut& shortcut() const;
    /**
     * Get the default shortcut for this action.
     */
    virtual const KShortcut& shortcutDefault() const;

    // These two methods are for Q_PROPERTY
    QString shortcutText() const;
    void setShortcutText( const QString& );

    /**
     * Returns true if this action is enabled.
     */
    virtual bool isEnabled() const;

    /**
     * Returns true if this action's shortcut is configurable.
     */
    virtual bool isShortcutConfigurable() const;

    virtual QString group() const;

    /**
     * Get the What's this text for the action.
     */
    virtual QString whatsThis() const;

    /**
     * Get the tooltip text for the action.
     */
    virtual QString toolTip() const;

    /**
     * Get the QIconSet from which the icons used to display this action will
     * be chosen.
     */
    virtual QIconSet iconSet( KIcon::Group group, int size=0 ) const;
#ifndef KDE_NO_COMPAT
    QIconSet iconSet() const { return iconSet( KIcon::Small ); }
#endif

    virtual QString icon() const;

    KActionCollection *parentCollection() const;

    /**
     * @internal
     * Generate a toolbar button id. Made public for reimplementations.
     */
    static int getToolButtonID();


    void unplugAll();

public slots:
    /**
     * Sets the text associated with this action. The text is used for menu
     * and toolbar labels etc.
     */
    virtual void setText(const QString &text);

    /**
     * Sets the keyboard shortcut associated with this action.
     */
    virtual bool setShortcut( const KShortcut& );

    virtual void setGroup( const QString& );

    /**
     * Sets the What's this text for the action. This text will be displayed when
     * a widget that has been created by plugging this action into a container
     * is clicked on in What's this mode.
     *
     * The What's this text can include QML markup as well as raw text.
     */
    virtual void setWhatsThis( const QString& text );

    /**
     * Sets the tooltip text for the action.
     * This will be used as a tooltip for a toolbar button, as a
     * statusbar help-text for a menu item, and it also appears
     * in the toolbar editor, to describe the action.
     */
    virtual void setToolTip( const QString& );

    /**
     * Sets the QIconSet from which the icons used to display this action will
     * be chosen.
     */
    virtual void setIconSet( const QIconSet &iconSet );

    virtual void setIcon( const QString& icon );

    /**
     * Enables or disables this action. All uses of this action (eg. in menus
     * or toolbars) will be updated to reflect the state of the action.
     */
    virtual void setEnabled(bool enable);

    /**
     * Indicate whether the user may configure the action's shortcut.
     */
    virtual void setShortcutConfigurable( bool );

    /**
     * Emulate user's interaction programmatically, by activating the action.
     * The implementation simply emits activated().
     */
    virtual void activate();

protected slots:
    virtual void slotDestroyed();
    virtual void slotKeycodeChanged();
    virtual void slotActivated();

protected:
    KToolBar* toolBar( int index ) const;
    QPopupMenu* popupMenu( int index ) const;
    void removeContainer( int index );
    int findContainer( const QWidget* widget ) const;
    void plugMainWindowAccel( QWidget *w );

    void addContainer( QWidget* parent, int id );
    void addContainer( QWidget* parent, QWidget* representative );

    virtual void updateShortcut( int i );
    virtual void updateShortcut( QPopupMenu* menu, int id );
    virtual void updateGroup( int id );
    virtual void updateText(int i );
    virtual void updateEnabled(int i);
    virtual void updateIconSet(int i);
    virtual void updateIcon( int i);
    virtual void updateToolTip( int id );
    virtual void updateWhatsThis( int i );

    KActionCollection *m_parentCollection;

signals:
    void activated();
    /// @since 3.1
    void activated( int );
    void enabled( bool );

private:
    void initPrivate( const QString& text, const KShortcut& cut,
                  const QObject* receiver, const char* slot );
    KAccel* kaccelCurrent();
    bool initShortcut( const KShortcut& );
    void plugShortcut();
    bool updateKAccelShortcut( KAccel* kaccel );
    void insertKAccel( KAccel* );
    /** @internal To be used exclusively by KActionCollection::removeWidget(). */
    void removeKAccel( KAccel* );

    QString whatsThisWithIcon() const;

#ifndef KDE_NO_COMPAT
public:
    /**
     * @deprecated.  Use shortcut().
     * Get the keyboard accelerator associated with this action.
     */
    int accel() const;

    QString statusText() const
        { return toolTip(); }

    /**
     * @deprecated.  Use setShortcut().
     * Sets the keyboard accelerator associated with this action.
     */
    void setAccel( int key );

    /**
     * @deprecated. Use setToolTip instead (they do the same thing now).
     */
    void setStatusText( const QString &text )
         { setToolTip( text ); }

    /**
     * @deprecated. for backwards compatibility.
     */
    int menuId( int i ) { return itemId( i ); }
#endif // !KDE_NO_COMPAT

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KActionPrivate;
    KActionPrivate *d;
};

/**
 *  Checkbox like action.
 *
 *  This action provides two states: checked or not.
 *
 *  @short Checkbox like action.
 */
class KToggleAction : public KAction
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
    KToggleAction( const QString& text, const KShortcut& cut = KShortcut(), QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( const QString& text, const KShortcut& cut,
                   const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( const QString& text, const QIconSet& pix, const KShortcut& cut = KShortcut(),
             QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
                   QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( const QString& text, const QIconSet& pix, const KShortcut& cut,
                   const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

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
                   QObject* parent, const char* name = 0 );

    /**
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KToggleAction( QObject* parent = 0, const char* name = 0 );

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
 * An action that operates like a radio button. At any given time
 * only a single action from the group will be active.
 */
class KRadioAction : public KToggleAction
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
    KRadioAction( const QString& text, const KShortcut& cut = KShortcut(), QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( const QString& text, const KShortcut& cut,
                  const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( const QString& text, const QIconSet& pix, const KShortcut& cut = KShortcut(),
                  QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
                  QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( const QString& text, const QIconSet& pix, const KShortcut& cut,
                  const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

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
                  QObject* parent, const char* name = 0 );

    /**
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KRadioAction( QObject* parent = 0, const char* name = 0 );

protected:
    virtual void slotActivated();

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KRadioActionPrivate;
    KRadioActionPrivate *d;
};

/**
 *  Action for selecting one of several items.
 *
 *  This action shows up a submenu with a list of items.
 *  One of them can be checked. If the user clicks on an item
 *  this item will automatically be checked,
 *  the formerly checked item becomes unchecked.
 *  There can be only one item checked at a time.
 *
 *  @short Action for selecting one of several items
 */
class KSelectAction : public KAction
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
    KSelectAction( const QString& text, const KShortcut& cut = KShortcut(), QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( const QString& text, const KShortcut& cut,
                   const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( const QString& text, const QIconSet& pix, const KShortcut& cut = KShortcut(),
             QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
                   QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( const QString& text, const QIconSet& pix, const KShortcut& cut,
                   const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

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
                   QObject* parent, const char* name = 0 );

    /**
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KSelectAction( QObject* parent = 0, const char* name = 0 );

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

    virtual void changeItem( int index, const QString& text );

    virtual QString currentText() const;

    virtual int currentItem() const;

    /**
     * When this action is plugged into a toolbar, it creates a combobox.
     * This returns the maximum width set by setComboWidth
     */
    virtual int comboWidth() const;

    QPopupMenu* popupMenu() const;

    /**
     * Deprecated. See @ref setMenuAccelsEnabled .
     * @since 3.1
     */
    void setRemoveAmpersandsInCombo( bool b );
    /// @since 3.1
    bool removeAmpersandsInCombo() const;

    /**
     * Sets whether any occurence of the ampersand character ( &amp; ) in items
     * should be interpreted as keyboard accelerator for items displayed in a
     * menu or not.
     * @since 3.1
     */
    void setMenuAccelsEnabled( bool b );
    /// @since 3.1
    bool menuAccelsEnabled() const;

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
    void activated( int index );
    void activated( const QString& text );

protected:
    virtual void updateCurrentItem( int id );

    virtual void updateComboWidth( int id );

    virtual void updateItems( int id );

    virtual void updateClear( int id );

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KSelectActionPrivate;
    KSelectActionPrivate *d;

};

/// Remove this class in KDE-4.0. It doesn't add _anything_ to KSelectAction
/**
 * @deprecated Use KSelectAction instead.
 */
class KListAction : public KSelectAction
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
    KListAction( const QString& text, const KShortcut& cut = KShortcut(), QObject* parent = 0,
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
                  const char* slot, QObject* parent, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KListAction( const QString& text, const QIconSet& pix, const KShortcut& cut = KShortcut(),
                      QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The dynamically loaded icon that goes with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KListAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
                      QObject* parent = 0, const char* name = 0 );

    /**
     *  @param text The text that will be displayed.
     *  @param pix The icons that go with this action.
     *  @param cut The corresponding keyboard accelerator (shortcut).
     *  @param receiver The SLOT's parent.
     *  @param slot The SLOT to invoke to execute this action.
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KListAction( const QString& text, const QIconSet& pix, const KShortcut& cut,
                          const QObject* receiver, const char* slot, QObject* parent,
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
                 const QObject* receiver, const char* slot, QObject* parent,
                 const char* name = 0 );

    /**
     *  @param parent This action's parent.
     *  @param name An internal name for this action.
     */
    KListAction( QObject* parent = 0, const char* name = 0 );

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
 *  This class is an action to handle a recent files submenu.
 *  The best way to create the action is to use KStdAction::openRecent.
 *  Then you simply need to call @ref loadEntries on startup, @ref saveEntries
 *  on shutdown, @ref addURL when your application loads/saves a file.
 *
 *  @author Michael Koch
 *  @short Recent files action
 */
class KRecentFilesAction : public KListAction  // TODO public KSelectAction
{
  Q_OBJECT
  Q_PROPERTY( uint maxItems READ maxItems WRITE setMaxItems )
public:
  /**
   *  @param text The text that will be displayed.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( const QString& text, const KShortcut& cut,
                      QObject* parent, const char* name = 0,
                      uint maxItems = 10 );

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
                      QObject* parent, const char* name = 0,
                      uint maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The icons that go with this action.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( const QString& text, const QIconSet& pix, const KShortcut& cut,
                      QObject* parent, const char* name = 0,
                      uint maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The dynamically loaded icon that goes with this action.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( const QString& text, const QString& pix, const KShortcut& cut,
                      QObject* parent, const char* name = 0,
                      uint maxItems = 10 );

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
  KRecentFilesAction( const QString& text, const QIconSet& pix, const KShortcut& cut,
                      const QObject* receiver, const char* slot,
                      QObject* parent, const char* name = 0,
                      uint maxItems = 10 );

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
                      QObject* parent, const char* name = 0,
                      uint maxItems = 10 );

  /**
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KRecentFilesAction( QObject* parent = 0, const char* name = 0,
                      uint maxItems = 10 );

  /**
   *  Destructor.
   */
  virtual ~KRecentFilesAction();

  /**
   *  Returns the maximum of items in the recent files list.
   */
  uint maxItems() const;

public slots:
  /**
   *  Sets the maximum of items in the recent files list.
   *  The default for this value is 10 set in the constructor.
   *
   *  If this value is lesser than the number of items currently
   *  in the recent files list the last items are deleted until
   *  the number of items are equal to the new maximum.
   */
  void setMaxItems( uint maxItems );

  /**
   *  Loads the recent files entries from a given KConfig object.
   *  You can provide the name of the group used to load the entries.
   *  If the groupname is empty, entries are load from a group called 'RecentFiles'
   *
   *  This method does not effect the active group of KConfig.
   */
  void loadEntries( KConfig* config, QString groupname=QString::null );

  /**
   *  Saves the current recent files entries to a given KConfig object.
   *  You can provide the name of the group used to load the entries.
   *  If the groupname is empty, entries are saved to a group called 'RecentFiles'
   *
   *  This method does not effect the active group of KConfig.
   */
  void saveEntries( KConfig* config, QString groupname=QString::null );

public slots:
  /**
   *  Add URL to recent files list.
   *
   *  @param url The URL of the file
   */
  void addURL( const KURL& url );

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
  /**
   *
   */
  void itemSelected( const QString& string );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  void init();

  class KRecentFilesActionPrivate;
  KRecentFilesActionPrivate *d;
};

class KFontAction : public KSelectAction
{
    Q_OBJECT
    Q_PROPERTY( QString font READ font WRITE setFont )
public:
    KFontAction( const QString& text, const KShortcut& cut = KShortcut(), QObject* parent = 0,
                 const char* name = 0 );
    KFontAction( const QString& text, const KShortcut& cut,
                 const QObject* receiver, const char* slot, QObject* parent,
                 const char* name = 0 );
    KFontAction( const QString& text, const QIconSet& pix, const KShortcut& cut = KShortcut(),
                 QObject* parent = 0, const char* name = 0 );
    KFontAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
                 QObject* parent = 0, const char* name = 0 );
    KFontAction( const QString& text, const QIconSet& pix, const KShortcut& cut,
                 const QObject* receiver, const char* slot, QObject* parent,
                 const char* name = 0 );
    KFontAction( const QString& text, const QString& pix, const KShortcut& cut,
                 const QObject* receiver, const char* slot, QObject* parent,
                 const char* name = 0 );

    KFontAction( QObject* parent = 0, const char* name = 0 );
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

class KFontSizeAction : public KSelectAction
{
    Q_OBJECT
    Q_PROPERTY( int fontSize READ fontSize WRITE setFontSize )
public:
    KFontSizeAction( const QString& text, const KShortcut& cut = KShortcut(), QObject* parent = 0,
                     const char* name = 0 );
    KFontSizeAction( const QString& text, const KShortcut& cut, const QObject* receiver,
                     const char* slot, QObject* parent, const char* name = 0 );
    KFontSizeAction( const QString& text, const QIconSet& pix, const KShortcut& cut = KShortcut(),
                     QObject* parent = 0, const char* name = 0 );
    KFontSizeAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
                     QObject* parent = 0, const char* name = 0 );
    KFontSizeAction( const QString& text, const QIconSet& pix, const KShortcut& cut,
                     const QObject* receiver, const char* slot,
                     QObject* parent, const char* name = 0 );
    KFontSizeAction( const QString& text, const QString& pix, const KShortcut& cut,
                     const QObject* receiver, const char* slot,
                     QObject* parent, const char* name = 0 );
    KFontSizeAction( QObject* parent = 0, const char* name = 0 );

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
class KActionMenu : public KAction
{
  Q_OBJECT
  Q_PROPERTY( bool delayed READ delayed WRITE setDelayed )
  Q_PROPERTY( bool stickyMenu READ stickyMenu WRITE setStickyMenu )

public:
    KActionMenu( const QString& text, QObject* parent = 0,
                 const char* name = 0 );
    KActionMenu( const QString& text, const QIconSet& icon,
                 QObject* parent = 0, const char* name = 0 );
    KActionMenu( const QString& text, const QString& icon,
                 QObject* parent = 0, const char* name = 0 );
    KActionMenu( QObject* parent = 0, const char* name = 0 );
    virtual ~KActionMenu();

    virtual void insert( KAction*, int index = -1 );
    virtual void remove( KAction* );

    KPopupMenu* popupMenu() const;
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
     * See @ref setStickyMenu.
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
class KToolBarPopupAction : public KAction
{
  Q_OBJECT
  Q_PROPERTY( bool delayed READ delayed WRITE setDelayed )
  Q_PROPERTY( bool stickyMenu READ stickyMenu WRITE setStickyMenu )

public:
    //Not all constructors - because we need an icon, since this action only makes
    // sense when being plugged at least in a toolbar.
    /**
     * Create a KToolBarPopupAction, with a text, an icon, an optionnal accelerator,
     * parent and name.
     *
     * @param text The text that will be displayed.
     * @param icon The icon to display.
     * @param cut The corresponding keyboard accelerator (shortcut).
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KToolBarPopupAction( const QString& text, const QString& icon, const KShortcut& cut = KShortcut(),
                         QObject* parent = 0, const char* name = 0 );

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
                         QObject* parent = 0, const char* name = 0 );

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
    KPopupMenu *popupMenu() const;

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
     * See @ref setStickyMenu.
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
    KPopupMenu *m_popup;
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
class KToggleToolBarAction : public KToggleAction
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

    virtual int plug( QWidget*, int index = -1 );

    KToolBar *toolBar() { return m_toolBar; }

public slots:
    virtual void setChecked( bool );

private:
    QCString               m_toolBarName;
    QGuardedPtr<KToolBar>  m_toolBar;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KToggleToolBarActionPrivate;
    KToggleToolBarActionPrivate *d;
};

/**
 * An action that automatically embeds a widget into a
 * toolbar.
 */
class KWidgetAction : public KAction
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
    QWidget* widget() { return m_widget; }

    void setAutoSized( bool );

    /**
     * Plug the action. The widget passed to the constructor
     * will be reparented to w, which must inherit KToolBar.
     */
    virtual int plug( QWidget* w, int index = -1 );
    /**
     * Unplug the action. Ensures that the action is not
     * destroyed. It will be hidden and reparented to 0L instead.
     */
    virtual void unplug( QWidget *w );
private:
    QGuardedPtr<QWidget> m_widget;
    bool                 m_autoSized;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KWidgetActionPrivate;
    KWidgetActionPrivate *d;
};

class KActionSeparator : public KAction
{
    Q_OBJECT
public:
    KActionSeparator( QObject* parent = 0, const char* name = 0 );
    virtual ~KActionSeparator();

    virtual int plug( QWidget*, int index = -1 );

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KActionSeparatorPrivate;
    KActionSeparatorPrivate *d;
};

typedef QValueList<KAction *> KActionPtrList;

/**
 * A managed set of KAction objects.
 */
class KActionCollection : public QObject
{
  friend class KAction;
  friend class KXMLGUIClient;

  Q_OBJECT
public:
  KActionCollection( QWidget *parent, const char *name = 0, KInstance *instance = 0 );
  /**
   * Use this constructor if you want the collection's actions to restrict
   * their accelerator keys to @p watch rather than the @p parent.  If
   * you don't require shortcuts, you can pass a null to the @p watch parameter.
   */
  KActionCollection( QWidget *watch, QObject* parent, const char *name = 0, KInstance *instance = 0 );
  KActionCollection( const KActionCollection &copy );
  virtual ~KActionCollection();

  /**
   * This sets the widget to which the keyboard shortcuts should be attached.
   * You only need to call this if a null pointer was passed in the constructor.
   */
  virtual void setWidget( QWidget *widget );

  /**
   * This indicates whether new actions which are created in this collection
   * should have their keyboard shortcuts automatically connected on
   * construction.  Set to 'false' if you will be loading XML-based settings.
   * This is automatically done by KParts.  The default is 'true'.
   */
  void setAutoConnectShortcuts( bool );

  bool isAutoConnectShortcuts();

  /**
   * This sets the default shortcut scope for new actions created in this
   * collection.  The default is ScopeUnspecified.  Ideally the default
   * would have been ScopeWidget, but that would cause some backwards
   * compatibility problems.
   */
  void setDefaultScope( KAction::Scope );

  /**
   * Doc/View model.  This lets you add the action collection of a document
   * to a view's action collection.
   */
  bool addDocCollection( KActionCollection* pDoc );

  /** Returns the number of widgets which this collection is associated with. */
  //uint widgetCount() const;

  /**
   * Returns true if the collection has its own KAccel object.  This will be
   * the case if it was constructed with a valid widget ptr or if setWidget()
   * was called.
   */
  //bool ownsKAccel() const;

  /** @deprecated  Deprecated because of ambiguous name.  Use kaccel() */
  virtual KAccel* accel();
  virtual const KAccel* accel() const;

  /** Returns the KAccel object of the most recently set widget. */
  KAccel* kaccel();
  const KAccel* kaccel() const;

  /** @internal, for KAction::kaccelCurrent() */
  KAccel* builderKAccel() const;
  /** Returns the KAccel object associated with widget #. */
  //KAccel* widgetKAccel( uint i );
  //const KAccel* widgetKAccel( uint i ) const;

  /* Returns the number of actions in the collection **/
  virtual uint count() const;
  bool isEmpty() const { return count() == 0; }
  virtual KAction* action( int index ) const;
  virtual KAction* action( const char* name, const char* classname = 0 ) const;

  virtual QStringList groups() const;
  virtual KActionPtrList actions( const QString& group ) const;
  virtual KActionPtrList actions() const;

  /**
   * Used for reading shortcut configuration from a non-XML rc file.
   */
  bool readShortcutSettings( const QString& sConfigGroup = QString::null, KConfigBase* pConfig = 0 );
  /**
   * Used for writing shortcut configuration to a non-XML rc file.
   */
  bool writeShortcutSettings( const QString& sConfigGroup = QString::null, KConfigBase* pConfig = 0 ) const;

  void setInstance( KInstance *instance );
  KInstance *instance() const;

  /**
   * Use this to tell the KActionCollection what rc file its configuration
   * is stored in.
   */
  void setXMLFile( const QString& );
  const QString& xmlFile() const;

  void setHighlightingEnabled( bool enable );
  bool highlightingEnabled() const;

  void connectHighlight( QWidget *container, KAction *action );
  void disconnectHighlight( QWidget *container, KAction *action );

signals:
  void inserted( KAction* );
  void removed( KAction* );

  void actionHighlighted( KAction *action );
  void actionHighlighted( KAction *action, bool highlight );

  void actionStatusText( const QString &text );
  void clearStatusText();

private:
  /**
   * @internal Only to be called by KXMLGUIFactory::addClient().
   * When actions are being connected, KAction needs to know what
   * widget it should connect widget-scope actions to, and what
   * main window it should connect
   */
  void beginXMLPlug( QWidget *widget );
  void endXMLPlug();
  /** @internal.  Only to be called by KXMLGUIFactory::removeClient() */
  void prepareXMLUnplug();
  void unplugShortcuts( KAccel* kaccel );

  void _clear();
  void _insert( KAction* );
  void _remove( KAction* );
  KAction* _take( KAction* );

private slots:
   void slotMenuItemHighlighted( int id );
   void slotToolBarButtonHighlighted( int id, bool highlight );
   void slotMenuAboutToHide();
   void slotDestroyed();

private:
   KAction *findAction( QWidget *container, int id );

#ifndef KDE_NO_COMPAT
public:
  KActionCollection( QObject *parent, const char *name = 0, KInstance *instance = 0 );

  void insert( KAction* );
  void remove( KAction* );
  KAction* take( KAction* );

  KActionCollection operator+ ( const KActionCollection& ) const;
  KActionCollection& operator= ( const KActionCollection& );
  KActionCollection& operator+= ( const KActionCollection& );

public slots:
  /**
   * Clears the entire actionCollection, deleting all actions.
   * @see #remove
   */
  void clear();
#endif // !KDE_NO_COMPAT
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KActionCollectionPrivate;
    KActionCollectionPrivate *d;
};

#endif
