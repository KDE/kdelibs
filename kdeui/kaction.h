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

#ifndef __kaction_h__
#define __kaction_h__

#include <qicon.h> // remove if iconSet() is removed
#include <qobject.h>

#include <kicontheme.h>
#include <kshortcut.h>

class QMenu;
class QIcon;
class QString;

class KAccel;
class KActionCollection;
class KGuiItem;
class KToolBar;

/**
 * @short Class to encapsulate user-driven action or event
 *
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
 * (maybe nothing is selected), you would have to hunt down the pointer
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
 * QSignalMapper . A tiny example:
 *
 * \code
 * QSignalMapper *desktopNumberMapper = new QSignalMapper( this );
 * connect( desktopNumberMapper, SIGNAL( mapped( int ) ),
 *          this, SLOT( moveWindowToDesktop( int ) ) );
 *
 * for ( uint i = 0; i < numberOfDesktops; ++i ) {
 *     KAction *desktopAction = new KAction( i18n( "Move Window to Desktop %i" ).arg( i ), ... );
 *     connect( desktopAction, SIGNAL( activated() ), desktopNumberMapper, SLOT( map() ) );
 *     desktopNumberMapper->setMapping( desktopAction, i );
 * }
 * \endcode
 *
 * <b>General Usage:</b>\n
 *
 * The steps to using actions are roughly as follows
 *
 * @li Decide which attributes you want to associate with a given
 *     action (icons, text, keyboard shortcut, etc)
 * @li Create the action using KAction (or derived or super class).
 * @li "Plug" the Action into whatever GUI element you want.  Typically,
 *      this will be a menu or toolbar.
 *
 * <b>Detailed Example:</b>\n
 *
 * Here is an example of enabling a "New [document]" action
 * \code
 * KAction *newAct = new KAction(i18n("&New"), "filenew",
 *                               KStdAccel::shortcut(KStdAccel::New),
 *                               this, SLOT(fileNew()),
 *                               actionCollection(), "new");
 * \endcode
 * This line creates our action.  It says that wherever this action is
 * displayed, it will use "&New" as the text, the standard icon, and
 * the standard shortcut.  It further says that whenever this action
 * is invoked, it will use the fileNew() slot to execute it.
 *
 * \code
 * QMenu *file = new QMenu;
 * newAct->plug(file);
 * \endcode
 * That just inserted the action into the File menu.  The point is, it's not
 * important in which menu it is: all manipulation of the item is
 * done through the newAct object.
 *
 * \code
 * newAct->plug(toolBar());
 * \endcode
 * And this inserted the Action into the main toolbar as a button.
 *
 * That's it!
 *
 * If you want to disable that action sometime later, you can do so
 * with
 * \code
 * newAct->setEnabled(false)
 * \endcode
 * and both the menuitem in File and the toolbar button will instantly
 * be disabled.
 *
 * Do not delete a KAction object without unplugging it from all its
 * containers. The simplest way to do that is to use the unplugAll()
 * as in the following example:
 * \code
 * newAct->unplugAll();
 * delete newAct;
 * \endcode
 * Normally you will not need to do this as KActionCollection manages
 * everything for you.
 *
 * Note: if you are using a "standard" action like "new", "paste",
 * "quit", or any other action described in the KDE UI Standards,
 * please use the methods in the KStdAction class rather than
 * defining your own.
 *
 * <b>Usage Within the XML Framework:</b>\n
 *
 * If you are using KAction within the context of the XML menu and
 * toolbar building framework, then there are a few tiny changes.  The
 * first is that you must insert your new action into an action
 * collection.  The action collection (a KActionCollection) is,
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
 */
class KDEUI_EXPORT KAction : public QObject
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
    KAction( const QString& text, const QIcon& pix, const KShortcut& cut,
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
     * @param cut The corresponding keyboard shortcut (shortcut).
     * @param receiver The SLOT's parent.
     * @param slot The SLOT to invoke to execute this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KAction( const KGuiItem& item, const KShortcut& cut,
             const QObject* receiver, const char* slot,
             KActionCollection* parent, const char* name );

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
     * @param widget The GUI element to display this action
     * @param index The position into which the action is plugged. If
     * this is negative, the action is inserted at the end.
     */
    virtual int plug( QWidget *widget, int index = -1 );

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
    virtual void plugAccel(KAccel *accel, bool configurable = true) KDE_DEPRECATED;

    /**
     * "Unplug" or remove this action from a given widget.
     *
     * This will typically be a menu or a toolbar.  This is rarely
     * used in "normal" application.  Typically, it would be used if
     * your application has several views or modes, each with a
     * completely different menu structure.  If you simply want to
     * disable an action for a given period, use setEnabled()
     * instead.
     *
     * @param w Remove the action from this GUI element.
     */
    virtual void unplug( QWidget *w );

    /**
     * @deprecated.  Complement method to plugAccel().
     * Disconnect this action from the KAccel.
     */
    virtual void unplugAccel() KDE_DEPRECATED;

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
     *
     * In KDE4 set group default to KIcon::Small while removing the other
     * iconSet() function.
     */
    virtual QIcon iconSet( KIcon::Group group, int size=0 ) const;
    /**
     * Remove in KDE4
     */
    QIcon iconSet() const { return iconSet( KIcon::Small ); }

    virtual QString icon() const;

    KActionCollection *parentCollection() const;

    /**
     * @internal
     * Generate a toolbar button id. Made public for reimplementations.
     */
    static int getToolButtonID();


    void unplugAll();

    /**
    * @since 3.4
    */
    enum ActivationReason { UnknownActivation, EmulatedActivation, AccelActivation, PopupMenuActivation, ToolBarActivation };

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
     *
     * For the tooltip to show up on the statusbar you will need to connect
     * a couple of the actionclass signals to the toolbar.
     * The easiest way of doing this is in your main window class, when you create
     * a statusbar.  See the KActionCollection class for more details.
     *
     * @see KActionCollection
     *
     */
    virtual void setToolTip( const QString& );

    /**
     * Sets the QIconSet from which the icons used to display this action will
     * be chosen.
     */
    virtual void setIcon( const QIcon &iconSet );

    virtual void setIcon( const QString& icon );

    /**
     * Enables or disables this action. All uses of this action (eg. in menus
     * or toolbars) will be updated to reflect the state of the action.
     */
    virtual void setEnabled(bool enable);

    /**
     * Calls setEnabled( !disable ).
     * @since 3.5
     */
    void setDisabled(bool disable) { return setEnabled(!disable); }

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
    /// @since 3.4
    virtual void slotPopupActivated();
    /// @since 3.4
    virtual void slotButtonClicked( int, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers );

protected:
    KToolBar* toolBar( int index ) const;
    QMenu* menu( int index ) const;
    void removeContainer( int index );
    int findContainer( const QWidget* widget ) const;
    int findContainer( int id ) const;
    void plugMainWindowAccel( QWidget *w );

    void addContainer( QWidget* parent, int id );
    void addContainer( QWidget* parent, QWidget* representative );

    virtual void updateShortcut( int i );
    virtual void updateShortcut( QMenu* menu, int id );
    virtual void updateGroup( int id );
    virtual void updateText(int i );
    virtual void updateEnabled(int i);
    virtual void updateIconSet(int i);
    virtual void updateIcon( int i);
    virtual void updateToolTip( int id );
    virtual void updateWhatsThis( int i );

    KActionCollection *m_parentCollection;
    QString whatsThisWithIcon() const;
    /**
     * Return the underlying KGuiItem
     * @since 3.3
     */
    const KGuiItem& guiItem() const;

signals:
    /**
     * Emitted when this action is activated
     */
    void activated();
    /**
     * This signal allows to know the reason why an action was activated:
     * whether it was due to a toolbar button, popupmenu, keyboard accel, or programmatically.
     * In the first two cases, it also allows to know which mouse button was
     * used (Left or Middle), and whether keyboard modifiers were pressed (e.g. CTRL).
     *
     * Note that this signal is emitted before the normal activated() signal.
     * Yes, BOTH signals are always emitted, so that connecting to activated() still works.
     * Applications which care about reason and mouse/keyboard state can either ignore the activated()
     * signal for a given action and react to this one instead, or store the
     * reason and mouse/keyboard state until the activated() signal is emitted.
     *
     * @since 3.4
     */
    void activated( KAction::ActivationReason reason, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers );

//#ifdef QT3_SUPPORT TODO
    /// @deprecated, use the signal
    /// void activated( KAction::ActivationReason reason, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers );
    QT_MOC_COMPAT void activated( KAction::ActivationReason reason, Qt::ButtonState state );
//#endif
    void enabled( bool );

private:
    void initPrivate( const QString& text, const KShortcut& cut,
                  const QObject* receiver, const char* slot, const char* name );
    KAccel* kaccelCurrent();
    bool initShortcut( const KShortcut& );
    void plugShortcut();
    bool updateKAccelShortcut( KAccel* kaccel );
    void insertKAccel( KAccel* );
    /** @internal To be used exclusively by KActionCollection::removeWidget(). */
    void removeKAccel( KAccel* );

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KActionPrivate;
    KActionPrivate* const d;
};

#include <kactioncollection.h>
#include <kactionclasses.h>

#endif
