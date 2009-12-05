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

#include <kdeui_export.h>
#include <kguiitem.h>
#include <kshortcut.h>

#include <QtGui/QWidgetAction>

class KIcon;
class KShapeGesture;
class KRockerGesture;

namespace KAuth {
    class Action;
}

//TODO Reduce the word count. This is not very focused and takes too long to read.
//Keep in mind that QAction also has documentation that we don't need to repeat here.
/**
 * @short Class to encapsulate user-driven action or event
 * @extends QAction
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
 * manipulate the action at all, the effect will propagate through all
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
 * \section kaction_general The kinds of shortcuts
 *
 * Local shortcuts are active if their context has the focus, global shortcus
 * are active even if the program does not have the focus. If a global
 * shortcut and a local shortcut are ambiguous the global shortcut wins.
 *
 * @li Active shortcuts trigger a KAction if activated.
 * @li Default shortcuts are what the active shortcuts revert to if the user chooses
 * to reset shortcuts to default.
 *
 * \section kaction_example Detailed Example
 *
 * Here is an example of enabling a "New [document]" action
 * \code
 * KAction *newAct = actionCollection()->addAction(
 *              KStandardAction::New,   //< see KStandardAction
 *              this,                   //< Receiver
 *              SLOT(fileNew())  );     //< SLOT
 * \endcode
 *
 * This section creates our action. Text, Icon and Shortcut will be set from
 * KStandardAction. KStandardAction ensures your application complies to the
 * platform standards. When triggered the \c fileNew() slot will be called.
 *
 * @see KStandardAction for more information.
 *
 * If you want to create your own actions use
 * \code
 * KAction *newAct = actionCollection()->addAction("quick-connect");
 * newAct->setText(i18n("Quick Connect"))
 * newAct->setIcon(KIcon("quick-connect"));
 * newAct->setShortcut(Qt::Key_F6);
 * connect(newAct, SIGNAL(triggered()), this, SLOT(quickConnect()));
 * \endcode
 *
 * This section creates our action. It displays the text "Quick Connect",
 * uses the Icon "quick-connect" and pressing \c F6 will trigger it. When
 * invoked, the slot quickConnect() is called.
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
 * \warning calling QAction::setShortcut() on a KAction may lead to unexpected
 * behavior. There is nothing we can do about it because QAction::setShortcut()
 * is not virtual.
 *
 * \note if you are using a "standard" action like "new", "paste",
 * "quit", or any other action described in the KDE UI Standards,
 * please use the methods in the KStandardAction class rather than
 * defining your own.
 *
 * \section Using QActions
 *
 * Mixing QActions and KActions in an application is not a
 * good idea. KShortcutsEditor doesn't handle QActions at all.
 *
 * \section kaction_xmlgui Usage Within the XML Framework
 *
 * If you are using KAction within the context of the XML menu and
 * toolbar building framework, you do not ever
 * have to add your actions to containers manually.  The framework
 * does that for you.
 *
 * @see KStandardAction
 */
class KDEUI_EXPORT KAction : public QWidgetAction
{
  Q_OBJECT

  Q_PROPERTY( KShortcut shortcut READ shortcut WRITE setShortcut )
  Q_PROPERTY( bool shortcutConfigurable READ isShortcutConfigurable WRITE setShortcutConfigurable )
  Q_PROPERTY( KShortcut globalShortcut READ globalShortcut WRITE setGlobalShortcut )
  Q_PROPERTY( bool globalShortcutAllowed READ globalShortcutAllowed WRITE setGlobalShortcutAllowed )
  Q_PROPERTY( bool globalShortcutEnabled READ isGlobalShortcutEnabled )
  Q_FLAGS( ShortcutType )

public:
    /**
     * An enumeration about the two types of shortcuts in a KAction
     */
    enum ShortcutType {
      /// The shortcut will immediately become active but may be reset to "default".
      ActiveShortcut = 0x1,
      /// The shortcut is a default shortcut - it becomes active when somebody decides to
      /// reset shortcuts to default.
      DefaultShortcut = 0x2
    };
    Q_DECLARE_FLAGS(ShortcutTypes, ShortcutType)

    /**
     * An enum about global shortcut setter semantics
     */
    //This enum will be ORed with ShortcutType in calls to KGlobalAccel, so it must not contain
    //any value equal to a value in ShortcutType.
    enum GlobalShortcutLoading {
      /// Look up the action in global settings (using its main component's name and text())
      /// and set the shortcut as saved there.
      /// @see setGlobalShortcut()
      Autoloading = 0x0,
      /// Prevent autoloading of saved global shortcut for action
      NoAutoloading = 0x4
    };
    /**
     * Constructs an action.
     */
    explicit KAction(QObject *parent);

    /**
     * Constructs an action with the specified parent and visible text.
     *
     * @param text The visible text for this action.
     * @param parent The parent for this action.
     */
    KAction(const QString& text, QObject *parent);

    /**
     * Constructs an action with text and icon; a shortcut may be specified by
     * the ampersand character (e.g. \"&amp;Option\" creates a shortcut with key \e O )
     *
     * This is the other common KAction constructor used.  Use it when you
     * \e do have a corresponding icon.
     *
     * @param icon The icon to display.
     * @param text The text that will be displayed.
     * @param parent The parent for this action.
     */
    KAction(const KIcon& icon, const QString& text, QObject *parent);

    /**
     * Standard destructor
     */
    virtual ~KAction();

    /**
     * Sets the help text for the action.
     * This help text will be set for all help mechanisms:
     *  - the status-bar help text
     *  - the tooltip (for toolbar buttons)
     *  - the "WhatsThis" help text (unless one was already set)
     *
     * This is more convenient than calling all three methods with the
     * same text, and this level of abstraction can allow to change
     * the default implementation of help one day more easily.
     * Of course you can also call setStatusTip, setToolTip and setWhatsThis
     * separately for more flexibility.
     *
     * This method is also the easiest way to port from KDE3's KAction::setToolTip.
     *
     * @since 4.3
     */
    void setHelpText(const QString& text);

    /**
     * Get the shortcut for this action.
     *
     * This is preferred over QAction::shortcut(), as it allows for multiple shortcuts
     * per action. The first and second shortcut as reported by shortcuts() will be the
     * primary and alternate shortcut of the shortcut returned.
     *
     * \param types the type of shortcut to return. Should both be specified, only the
     *             active shortcut will be returned. Defaults to the active shortcut, if one exists.
     * \sa shortcuts()
     */
    KShortcut shortcut(ShortcutTypes types = ActiveShortcut) const;

    /**
     * Set the shortcut for this action.
     *
     * This is preferred over QAction::setShortcut(), as it allows for multiple shortcuts
     * per action.
     *
     * \param shortcut shortcut(s) to use for this action in its specified shortcutContext()
     * \param type type of shortcut to be set: active shortcut,
     *  default shortcut, or both (the default).
     */
    void setShortcut(const KShortcut& shortcut, ShortcutTypes type = ShortcutTypes(ActiveShortcut | DefaultShortcut));

    /**
     * \overload void setShortcut(const KShortcut& shortcut)
     *
     * Set the primary shortcut only for this action.
     *
     * This function is there to explicitly override QAction::setShortcut(const QKeySequence&).
     * QAction::setShortcut() will bypass everything in KAction and may lead to unexpected behavior.
     *
     * \param shortcut shortcut(s) to use for this action in its specified shortcutContext()
     * \param type type of shortcut to be set: active shortcut,
     *  default shortcut, or both (default argument value).
     */
    void setShortcut(const QKeySequence& shortcut, ShortcutTypes type = ShortcutTypes(ActiveShortcut | DefaultShortcut));

    /**
     * \overload void setShortcuts(const QList\<QKeySequence\>& shortcuts).
     *
     * Set the shortcuts for this action.
     *
     * This function is there to explicitly override QAction::setShortcut(const QList\<QKeySequence\>&).
     * QAction::setShortcuts() will bypass everything in KAction and may lead to unexpected behavior.
     *
     * \param shortcut shortcut(s) to use for this action in its specified shortcutContext()
     * \param type type of shortcut to be set: active shortcut,
     *  default shortcut, or both (default argument value).
     */
    void setShortcuts(const QList<QKeySequence>& shortcuts, ShortcutTypes type = ShortcutTypes(ActiveShortcut | DefaultShortcut));

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
     *             active shortcut will be returned.  Defaults to the active shortcut,
     *             if one exists.
     *
     * \sa KGlobalAccel
     * \sa setGlobalShortcut()
     */
    const KShortcut& globalShortcut(ShortcutTypes type = ActiveShortcut) const;

    /**
     * Assign a global shortcut for this action. Global shortcuts
     * allow an action to respond to key shortcuts independently of the focused window,
     * i.e. the action will trigger if the keys were pressed no matter where in the X session.
     *
     * The action must have a per main component unique
     * objectName() to enable cross-application bookeeping. If the objectName() is empty this method will
     * do nothing, otherwise the isGlobalShortcutEnabled() property will be set to true and the
     * shortcut will be enabled.
     * It is mandatory that the objectName() doesn't change once isGlobalshortcutEnabled()
     * has become true.
     *
     * \note KActionCollection::insert(name, action) will set action's objectName to name so you often
     * don't have to set an objectName explicitly.
     *
     * When an action, identified by main component name and objectName(), is assigned
     * a global shortcut for the first time on a KDE installation the assignment will
     * be saved. The shortcut will then be restored every time setGlobalShortcut() is
     * called with @p loading == Autoloading.
     *
     * If you actually want to change the global shortcut you have to set
     * @p loading to NoAutoloading. The new shortcut will be automatically saved again.
     *
     * \param shortcut global shortcut(s) to assign. Will be ignored unless \p loading is set to NoAutoloading or this is the first time ever you call this method (see above).
     * \param type the type of shortcut to be set, whether the active shortcut, the default shortcut,
     *             or both (the default).
     * \param loading if Autoloading, assign the global shortcut this action has previously had if any.
     *                   That way user preferences and changes made to avoid clashes will be conserved.
     *                if NoAutoloading the given shortcut will be assigned without looking up old values.
     *                   You should only do this if the user wants to change the shortcut or if you have
     *                   another very good reason. Key combinations that clash with other shortcuts will be
     *                   dropped.
     *
     * \note the default shortcut will never be influenced by autoloading - it will be set as given.
     * \sa globalShortcut()
     */
    void setGlobalShortcut(const KShortcut& shortcut, ShortcutTypes type =
                           ShortcutTypes(ActiveShortcut | DefaultShortcut),
                           GlobalShortcutLoading loading = Autoloading);

    /**
     * Returns true if this action is permitted to have a global shortcut.
     * Defaults to false.
     * Use isGlobalShortcutEnabled() instead.
     */
    KDE_DEPRECATED bool globalShortcutAllowed() const;

    /**
     * Indicate whether the programmer and/or user may define a global shortcut for this action.
     * Defaults to false. Note that calling setGlobalShortcut() turns this on automatically.
     *
     * \param allowed set to \e true if this action may have a global shortcut, otherwise \e false.
     * \param loading if Autoloading, assign to this action the global shortcut it has previously had
     *                if any.
     */
    KDE_DEPRECATED void setGlobalShortcutAllowed(bool allowed, GlobalShortcutLoading loading = Autoloading);

    /**
     * Returns true if this action is enabled to have a global shortcut.
     * This will be respected by \class KGlobalShortcutsEditor.
     * Defaults to false.
     */
    bool isGlobalShortcutEnabled() const;

    /**
     * Sets the globalShortcutEnabled property to false and sets the global shortcut to an
     * empty shortcut.
     * This will also wipe out knowlegde about the existence of this action's global shortcut
     * so it will not be considered anymore for shortcut conflict resolution. It will also not be
     * visible anymore in the shortcuts KControl module.
     * This method should not be used unless these effects are explicitly desired.
     * @since 4.1
     */
    void forgetGlobalShortcut();

    KShapeGesture shapeGesture(ShortcutTypes type = ActiveShortcut) const;
    KRockerGesture rockerGesture(ShortcutTypes type = ActiveShortcut) const;

    void setShapeGesture(const KShapeGesture& gest, ShortcutTypes type = ShortcutTypes(ActiveShortcut | DefaultShortcut));
    void setRockerGesture(const KRockerGesture& gest, ShortcutTypes type = ShortcutTypes(ActiveShortcut | DefaultShortcut));

    /**
     * Returns the action object associated with this action, or 0 if it does not have one
     *
     * @returns the KAuth::Action associated with this action.
     */
     KAuth::Action *authAction() const;

    /**
     * Sets the action object associated with this action
     *
     * By setting a KAuth::Action, this action will become associated with it, and
     * whenever it gets clicked, it will trigger the authorization and execution process
     * for the action. The signal activated will also be emitted whenever the action gets
     * clicked and the action gets authorized. Pass 0 to this function to disassociate the action
     *
     * @param action the KAuth::Action to associate with this action.
     */
     void setAuthAction(KAuth::Action *action);

     /**
     * Sets the action object associated with this action
     *
     * Overloaded member to allow creating the action by name
     *
     * @param actionName the name of the action to associate
     */
     void setAuthAction(const QString &actionName);

    /**
     * @reimp
     */
    bool event(QEvent*);


Q_SIGNALS:
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

    /**
     * Signal emitted when the action is triggered and authorized
     *
     * If the action needs authorization, when the user triggers the action,
     * the authorization process automatically begins.
     * If it succeeds, this signal is emitted. The KAuth::Action object is provided for convenience
     * if you have multiple KAuthorizedAction objects, but of course it's always the same set with
     * setAuthAction().
     *
     * WARNING: If your action needs authorization you should connect eventual slots processing
     * stuff to this signal, and NOT triggered. Triggered will be emitted even if the user has not
     * been authorized
     *
     * @param action The object set with setAuthAction()
     */
    void authorized(KAuth::Action *action);

    /**
     * Emitted when the global shortcut is changed. A global shortcut is
     * subject to be changed by the global shortcuts kcm.
     */
    void globalShortcutChanged(const QKeySequence&);

private:
    friend class KGlobalAccelPrivate; // Needs access to the component
    friend class KActionCollectionPrivate; // Needs access to the component
    friend class KShortcutsEditorDelegate; // Needs access to the component
    Q_PRIVATE_SLOT(d, void slotTriggered())
    Q_PRIVATE_SLOT(d, void authStatusChanged(int))
    class KActionPrivate* const d;
    friend class KActionPrivate;
    friend class KGlobalShortcutTest;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KAction::ShortcutTypes)

#endif
