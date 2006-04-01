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

#include "kicon.h"

class KActionCollection;
class KGuiItem;
class KInstance;

/**
 * @short Class to encapsulate user-driven action or event
 *
 * The KAction class (and derived and super classes) extends QAction,
 * which provides a way to easily encapsulate a "real" user-selected 
 * action or event in your program.
 *
 * For instance, a user may want to @p paste the contents of
 * the clipboard, @p scroll @p down a document, or @p quit the
 * application.  These are all \b actions -- events that the
 * user causes to happen.  The KAction class allows the developer to
 * deal with these actions in an easy and intuitive manner, and conforms
 * to KDE's extended functionality requirements - including supporting
 * multiple user-configurable shortcuts, and KDE named icons.  Actions
 * also improve accessibility.
 *
 * Specifically, QAction (and thus KAction) encapsulates the various attributes
 * of an event/action.  For instance, an action might have an icon()
 * that provides a visual representation (a clipboard for a "paste" action or
 * scissors for a "cut" action).  The action should also be described by some text().
 * It will certainly be connected to a method that actually @p executes the action!
 * All these attributes are contained within the action object.
 *
 * The advantage of dealing with actions is that you can manipulate
 * the Action without regard to the GUI representation of it.  For
 * instance, in the "normal" way of dealing with actions like "cut",
 * you would manually insert a item for Cut into a menu and a button
 * into a toolbar.  If you want to disable the cut action for a moment
 * (maybe nothing is selected), you would have to hunt down the pointer
 * to the menu item and the toolbar button and disable both
 * individually.  Setting the menu item and toolbar item up uses very
 * similar code - but has to be done twice!
 *
 * With the action concept, you simply add the action to whatever
 * GUI element you want.  The KAction class will then take care of
 * correctly defining the menu item (with icons, accelerators, text,
 * etc), toolbar button, or other.  From then on, if you
 * manipulate the action at all, the effect will propogate through all
 * GUI representations of it.  Back to the "cut" example: if you want
 * to disable the Cut Action, you would simply call
 * 'cutAction->setEnabled(false)' and both the menuitem and button would
 * instantly be disabled!
 *
 * This is the biggest advantage to the action concept -- there is a
 * one-to-one relationship between the "real" action and @p all
 * GUI representations of it.
 *
 * KAction emits the hovered() signal on mouseover, and the triggered(bool checked)
 * signal on activation of a corresponding GUI element ( menu item, toolbar button, etc. )
 *
 * If you are in the situation of wanting to map the triggered()
 * signal of multiple action objects to one slot, with a special
 * argument bound to each action, you have several options:
 *
 * Using QActionGroup:
 * \li Create a QActionGroup and assign it to each of the actions with setActionGroup(), then
 * \li Connect the QActionGroup::triggered(QAction*) signal to your slot.
 *
 * Using QSignalMapper:
 * \code
 * QSignalMapper *desktopNumberMapper = new QSignalMapper( this );
 * connect( desktopNumberMapper, SIGNAL( mapped( int ) ),
 *          this, SLOT( moveWindowToDesktop( int ) ) );
 *
 * for ( uint i = 0; i < numberOfDesktops; ++i ) {
 *     KAction *desktopAction = new KAction( i18n( "Move Window to Desktop %i" ).arg( i ), ... );
 *     connect( desktopAction, SIGNAL( triggered(bool) ), desktopNumberMapper, SLOT( map() ) );
 *     desktopNumberMapper->setMapping( desktopAction, i );
 * }
 * \endcode
 *
 * \section kaction_general General Usage
 *
 * The steps to using actions are roughly as follows:
 *
 * @li Decide which attributes you want to associate with a given
 *     action (icons, text, keyboard shortcut, etc)
 * @li Create the action using KAction (or derived or super class).
 * @li Add the action into whatever GUI element you want.  Typically,
 *      this will be a menu or toolbar.
 *
 * \section kaction_example Detailed Example
 *
 * Here is an example of enabling a "New [document]" action
 * \code
 * KAction *newAct = new KAction("filenew", i18n("&New"), actionCollection(), "new");
 * newAct->setShortcut(KStdAccel::shortcut(KStdAccel::New));
 * connect(newAct, SIGNAL(triggered(bool)), SLOT(fileNew()));
 * \endcode
 *
 * This section creates our action.  It says that wherever this action is
 * displayed, it will use "&New" as the text, the standard icon, and
 * the standard shortcut.  It further says that whenever this action
 * is invoked, it will use the fileNew() slot to execute it.
 *
 * \code
 * QMenu *file = new QMenu;
 * file->addAction(newAct);
 * \endcode
 * That just inserted the action into the File menu.  The point is, it's not
 * important in which menu it is: all manipulation of the item is
 * done through the newAct object.
 *
 * \code
 * toolBar()->addAction(newAct);
 * \endcode
 * And this added the action into the main toolbar as a button.
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
 * Unlike with previous versions of KDE, the action can simply be deleted
 * when you have finished with it - the destructor takes care of all
 * of the cleanup.
 *
 * Note: if you are using a "standard" action like "new", "paste",
 * "quit", or any other action described in the KDE UI Standards,
 * please use the methods in the KStdAction class rather than
 * defining your own.
 *
 * \section kaction_xmlgui Usage Within the XML Framework
 *
 * If you are using KAction within the context of the XML menu and
 * toolbar building framework, you do not ever
 * have to add your actions to containers manually.  The framework
 * does that for you.
 *
 * @see KStdAction
 */
class KDEUI_EXPORT KAction : public QAction
{
  Q_OBJECT

  // Needed for save/load of shortcut - as XMLGui sets properties from XML attributes
  // FIXME KAction port : replace (this hides QAction::shortcut)
  Q_PROPERTY( QString shortcut READ shortcutText WRITE setShortcutText )

  Q_PROPERTY( KShortcut defaultShortcut READ defaultShortcut WRITE setDefaultShortcut )
  Q_PROPERTY( bool shortcutConfigurable READ isShortcutConfigurable WRITE setShortcutConfigurable )
  Q_PROPERTY( KShortcut globalShortcut READ globalShortcut WRITE setGlobalShortcut )
  Q_PROPERTY( KShortcut defaultGlobalShortcut READ defaultGlobalShortcut WRITE setDefaultGlobalShortcut )
  Q_PROPERTY( bool globalShortcutAllowed READ globalShortcutAllowed WRITE setGlobalShortcutAllowed )

public:
    /**
     * A simple enumeration to define the type of shortcut, whether default or customised.
     * Used primarily so setShortcut() and setGlobalShortcut() can be made to also set the
     * default shortcut by default.
     */
    enum ShortcutType {
      /// The shortcut is a custom shortcut
      CustomShortcut = 0x1,
      /// The shortcut is a default shortcut
      DefaultShortcut = 0x2
    };
    Q_DECLARE_FLAGS(ShortcutTypes, ShortcutType)

    /**
     * Constructs an action in the specified KActionCollection.
     *
     * @param parent The action collection to contain this action.
     * @param name The internal name for this action.
     */
    KAction(KActionCollection* parent, const char* name);

    /**
     * Constructs an action with text; a shortcut may be specified by
     * the ampersand character (e.g. \"&amp;Option\" creates a shortcut with key \e O )
     *
     * This is the most common KAction constructor used when you do not have a
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
     * Constructs an action with text; a shortcut may be specified by
     * the ampersand character (e.g. \"&amp;Option\" creates a shortcut with key \e O )
     *
     * This is the other common KAction constructor used.  Use it when you
     * \e do have a corresponding icon.
     *
     * @param icon The icon to display.
     * @param text The text that will be displayed.
     * @param parent The action collection to contain this action.
     * @param name The internal name for this action.
     */
    KAction(const KIcon& icon, const QString& text, KActionCollection* parent, const char* name);

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
    KDE_CONSTRUCTOR_DEPRECATED KAction(const QString& icon, const QString& text, KActionCollection* parent, const char* name);

    /**
     * Constructs an action with text, potential keyboard
     * shortcut, and a SLOT to call when this action is invoked by
     * the user.
     *
     * If you do not want or have a keyboard shortcut,
     * set the @p cut param to 0.
     *
     * @param text The text that will be displayed.
     * @param cut The corresponding keyboard shortcut.
     * @param receiver The SLOT's parent.
     * @param slot The SLOT to invoke to execute this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     *
     * \deprecated This constructor was deprecated in line with the
     *  recommendation for constructors to be kept simple (Designing Qt-Style C++ APIs,
     *  "The Convenience Trap")
     */
    KDE_CONSTRUCTOR_DEPRECATED KAction( const QString& text, const KShortcut& cut,
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
     *
     * \deprecated This constructor was deprecated in line with the
     *  recommendation for constructors to be kept simple (Designing Qt-Style C++ APIs,
     *  "The Convenience Trap")
     */
    KDE_CONSTRUCTOR_DEPRECATED KAction( const QString& text, const QIcon& pix, const KShortcut& cut,
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
     *
     * \deprecated This constructor was deprecated in line with the
     *  recommendation for constructors to be kept simple (Designing Qt-Style C++ APIs,
     *  "The Convenience Trap")
     */
    KDE_CONSTRUCTOR_DEPRECATED KAction( const QString& text, const QString& pix, const KShortcut& cut,
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
     *
     * \deprecated This constructor was deprecated in line with the
     *  recommendation for constructors to be kept simple (Designing Qt-Style C++ APIs,
     *  "The Convenience Trap")
     */
    KDE_CONSTRUCTOR_DEPRECATED KAction( const KGuiItem& item, const KShortcut& cut,
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
     *
     * \param type the type of shortcut to return.  Should both be specified, only the 
     *             custom shortcut will be returned.  Defaults to the custom shortcut, if one exists.
     */
    const KShortcut& shortcut(ShortcutTypes types = CustomShortcut) const;

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
     *
     * \param shortcut shortcut(s) to use for this action in its specified shortcutContext()
     * \param type type of shortcut to be set, whether the custom shortcut, the default shortcut,
     *            or both (the default).
     */
    void setShortcut(const KShortcut& shortcut, ShortcutTypes type = static_cast<ShortcutType>(CustomShortcut | DefaultShortcut));

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
     * Get the global shortcut for this action, if one exists. Global shortcuts
     * allow your actions to respond to accellerators independently of the focused window.
     * Unlike regular shortcuts, the application's window does not need focus
     * for them to be activated.
     *
     * \param type the type of shortcut to be returned. Should both be specified, only the 
     *             custom shortcut will be returned.  Defaults to the custom shortcut,
     *             if one exists.
     *
     * \sa KGlobalAccel
     * \sa setGlobalShortcut()
     */
    const KShortcut& globalShortcut(ShortcutTypes type = CustomShortcut) const;

    /**
     * Assign a global shortcut for this action. Global shortcuts
     * allow your actions to respond to keys independently of the focused window.
     * Unlike regular shortcuts, the application's window does not need focus
     * for them to be activated.
     *
     * \param shortcut shortcut(s) to grab as global accelerators.
     * \param the type of shortcut to be set, whether the custom shortcut, the default shortcut,
     *            or both (the default).
     *
     * \note For convenience, passing a shortcut also sets the default (as this is by far
     *       the most common use case; mostly custom shortcuts are loaded from configuration
     *       files).  Pass \b false for \a isDefault to just set this
     *       shortcut.
     *
     * \sa KGlobalAccel
     * \sa globalShortcut()
     */
    void setGlobalShortcut(const KShortcut& shortcut, ShortcutTypes type = static_cast<ShortcutType>(CustomShortcut | DefaultShortcut));

    /**
     * Get the default global shortcut for this action, if one exists.
     *
     * \sa globalShortcut()
     */
    const KShortcut& defaultGlobalShortcut() const;

    /**
     * Set the default global shortcut for this action.
     *
     * \param shortcut default global shortcut(s).
     *
     * \sa defaultGlobalShortcut()
     */
    void setDefaultGlobalShortcut(const KShortcut& shortcut);

    /**
     * Returns true if this action is permitted to have a global shortcut.
     * Defaults to false.
     */
    bool globalShortcutAllowed() const;

    /**
     * Indicate whether the programmer and/or user may define a global shortcut for this action.
     * Defaults to false.
     *
     * \param allowed set to \e true if this action may have a global shortcut, otherwise \e false.
     */
    void setGlobalShortcutAllowed(bool allowed);

    /**
     * Convenience function to determine if this action has an associated icon.
     */
    bool hasIcon() const;

    /**
     * Return the icon for this action.
     *
     * This function hides QAction::icon() to be able to return a KIcon.
     */
    KIcon icon() const;

    /**
     * Set the icon for this action.
     *
     * This function hides QAction::setIcon(const QIcon&) to encourage programmers to pass
     * KIcons (which adhere to KDE style guidelines).  The QAction call can of course still
     * be accessed manually for the rare instances where KIcon does not make sense.
     *
     * \param icon the KIcon to assign to this action
     */
    void setIcon(const KIcon& icon);

    /**
     * Set the icon for this action.
     *
     * \param icon the KDE icon name to pass to KIconLoader.
     * \param group the icon group
     * \param instance the KInstance from which to retrieve the icon loader.
     *
     * \deprecated Use setIcon(KIcon("kdeiconname")) instead, or pass KIcon("kdeiconname") to the constructor.
     */
    KDE_DEPRECATED void setIconName(const QString& icon);

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
     *
     * \deprecated use QWidget::addAction() and QWidget::insertAction() instead.
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
     *
     * \deprecated use QWidget::removeAction() instead, or simply delete the action.
     */
    KDE_DEPRECATED void unplug( QWidget *w );

    /**
     * Convenience function to remove this action from all widgets.  This should
     * rarely be needed, as deleting the action takes care of this automatically.
     */
    void unplugAll();

    /**
     * Returns whether the action is plugged into any container widget or not.
     */
    bool isPlugged() const;

    /**
     * Returns whether the action is plugged into the given container
     */
    bool isPlugged( QWidget *container ) const;

    /**
     * Returns how many widgets this action is added to.
     *
     * \deprecated use associatedWidgets().count() instead.
     */
    KDE_DEPRECATED int containerCount() const;

    /**
     * Convenience function to return a widget which this action is added to.
     * Simply returns the widget specified in associatedWidgets(), if the
     * index is valid.
     *
     * \param index index to the widget requested.
     */
    QWidget* container( int index ) const;

signals:
#ifdef KDE3_SUPPORT
    /**
     * Emitted when this action is activated
     *
     * \deprecated use triggered(bool checked) instead.
     */
    QT_MOC_COMPAT void activated();
#endif

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
     * KAction to emit the triggered signal with mouse and keyboard modifiers attached.
     */
    virtual void slotTriggered();

private:
    // Core initialization, including Kiosk authorization checking
    void initPrivate(const char* name);
    // Compatability initialization functions here only
    void initPrivate( const KShortcut& cut, const QObject* receiver, const char* slot, const char* name );

    // You're not supposed to change the action name throughout its life - these methods are here to discourage you
    void setName ( const char * name );
    void setObjectName(const QString& name);

private:
    class KActionPrivate* const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KAction::ShortcutTypes)

#include <kactioncollection.h>
#include <kactionclasses.h>

#endif
