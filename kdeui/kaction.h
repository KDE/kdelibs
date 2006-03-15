/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
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

#ifndef KACTION_H
#define KACTION_H

#include <QAction>

#include <kdelibs_export.h>
#include <kshortcut.h>
#include <kicontheme.h>
#include <kglobal.h>

#ifdef KDE3_SUPPORT
#include <qicon.h> // remove if iconSet() is removed
#endif

class KAccel;
class KActionCollection;
class KGuiItem;
class KInstance;

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
class KDEUI_EXPORT KAction : public QAction
{
  friend class KActionCollection;

  Q_OBJECT

  // Needed for save/load of shortcut - as XMLGui sets properties from XML attributes
  // FIXME KAction port : replace (this hides QAction::shortcut)
  Q_PROPERTY( QString shortcut READ shortcutText WRITE setShortcutText )

public:
    /**
     * Constructs an action in the specified KActionCollection.
     *
     * @param parent The action collection to contain this action.
     * @param name The internal name for this action.
     */
    KAction(KActionCollection* parent, const char* name);

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
     * @param name The internal name for this action.
     */
    KAction(const QString& text, KActionCollection* parent, const char* name);

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
     * @param name The internal name for this action.
     */
    KAction(const QIcon& icon, const QString& text, KActionCollection* parent, const char* name);

    /**
     * \overload KAction(const QIcon&, const QString&, KActionCollection*, const char*)
     *
     * This constructor differs from the above in that the icon is specified as
     * a icon name which can be loaded by KIconLoader.
     *
     * @param icon The name of the icon to load via KIconLoader.
     * @param text The text that will be displayed.
     * @param parent The action collection to contain this action.
     * @param name The internal name for this action.
     */
    KAction(const QString& icon, const QString& text, KActionCollection* parent, const char* name);

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
    KDE_DEPRECATED KAction( const QString& text, const KShortcut& cut,
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
    KDE_DEPRECATED KAction( const QString& text, const QIcon& pix, const KShortcut& cut,
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
    KDE_DEPRECATED KAction( const QString& text, const QString& pix, const KShortcut& cut,
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
    KDE_DEPRECATED KAction( const KGuiItem& item, const KShortcut& cut,
             const QObject* receiver, const char* slot,
             KActionCollection* parent, const char* name );

    /**
     * Standard destructor
     */
    virtual ~KAction();

    /**
     * Returns the action collection that owns this object.
     */
    KActionCollection* parentCollection() const;

    /**
     * Get the kde shortcut for this action.
     *
     * This is preferred over QAction::shortcut(), as it allows for multiple shortcuts
     * per action.
     */
    const KShortcut& shortcut() const;

    /**
     * Get the text version of the kde shortcut for this action.
     *
     * \sa shortcut()
     */
    QString shortcutText() const;

    /**
     * Set the shortcut for this action.
     *
     * This is preferred over QAction::setShortcut(), as it allows for multiple shortcuts
     * per action.
     */
    void setShortcut(const KShortcut& shortcut);

    /**
     * \overload void setShortcut(const KShortcut& shortcut)
     *
     * Allows for a KShortcut to be created from text before assignment to this action's shortcut.
     */
    void setShortcutText(const QString& shortcutText);

    /**
     * Get the default shortcut for this action.
     */
    const KShortcut& defaultShortcut() const;

    /**
     * Set the default shortcut for this action.
     */
    void setDefaultShortcut(const KShortcut& shortcut);

    /**
     * Returns true if this action's shortcut is configurable.
     */
    bool isShortcutConfigurable() const;

    /**
     * Indicate whether the user may configure the action's shortcut.
     *
     * \param configurable set to \e true if this shortcut may be configured by the user, otherwise \e false.
     */
    void setShortcutConfigurable(bool configurable);

    /**
     * Convenience function to determine if this action has an associated icon.
     */
    bool hasIcon() const;

    /**
     * Get the name which was passed to KIconLoader to retrieve this icon
     */
    const QString& iconName() const;

    /**
     * Set the icon for this action.
     *
     * \param icon the KDE icon name to pass to KIconLoader.
     * \param group the icon group
     * \param instance the KInstance from which to retrieve the icon loader.
     */
    void setIconName(const QString& icon, KIcon::Group group = KIcon::NoGroup, int size = -1, KInstance* instance = KGlobal::instance());

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
    KDE_DEPRECATED int plug( QWidget *widget, int index = -1 );

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
    KDE_DEPRECATED void unplug( QWidget *w );

    /**
     * "Unplug" or remove this action from all widgets.  You do not need
     * to call this function if the action is being deleted, it will be
     * taken care of automatically.
     */
    void unplugAll();

    /**
     * returns whether the action is plugged into any container widget or not.
     */
    bool isPlugged() const;

    /**
     * returns whether the action is plugged into the given container
     */
    bool isPlugged( QWidget *container ) const;

    /**
     * Returns how many widgets this action is added to.
     */
    KDE_DEPRECATED int containerCount() const;

    /**
     * Returns a widget which this action is added to.
     *
     * \param index index to the widget requested.
     */
    QWidget* container( int index ) const;

signals:
//#ifdef KDE3_SUPPORT
    /**
     * Emitted when this action is activated
     *
     * \todo KDE4: make KDE3_SUPPORT
     */
    void activated();
//#endif

    /**
     * Emitted when the action is triggered. Also provides the state of the
     * keyboard modifiers and mouse buttons at the time.
     */
    void triggered(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);

protected:
    virtual void virtual_hook( int id, void* data );

protected Q_SLOTS:
    /**
     * This function is connected to the QAction's triggered(bool) signal, and allows
     * KAction to emit the triggered signal with mouse and keyboard modifiers attached,
     * as well as the activation reason, if one was given.
     */
    virtual void slotTriggered();

private:
    void initPrivate(const char* name);
    // Compatability functions here only
    void initPrivate( const KShortcut& cut, const QObject* receiver, const char* slot, const char* name );

    // You're not supposed to change the action name throughout its life
    void setName ( const char * name );
    void setObjectName(const QString& name);

private:
    class KActionPrivate;
    KActionPrivate* const d;
};

#include <kactioncollection.h>
#include <kactionclasses.h>

#endif
