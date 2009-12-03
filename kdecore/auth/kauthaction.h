/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*   Copyright (C) 2009 Dario Freddi <drf@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation; either version 2.1 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
*/

#ifndef ACTION_H
#define ACTION_H

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QHash>

#include <kdecore_export.h>

#include "kauthactionreply.h"

namespace KAuth
{

class ActionWatcher;

/**
 * @brief Class to access, authorize and execute actions.
 *
 * This is the main class of the kauth API. It provides the interface to
 * manipulate actions. Every action is identified by its name. Every instance
 * of the Action class with the same name refers to the same action.
 *
 * Once you have an action object you can tell the helper to execute it
 * (asking the user to authenticate if needed) with one of the execute*() methods.
 * The simplest thing to do is to execute a single action synchronously
 * blocking for the reply, using the execute() method.
 *
 * For asynchronous calls, use the executeAsync() method. It sends the request
 * to the helper and returns immediately. You can optionally provide an object
 * and a slot. This will be connected to the actionPerformed() signal of the
 * action's ActionWatcher object.
 * By calling the watcher() method, you obtain an object that emits some useful
 * signals that you can receive while the action is in progress. Those signals
 * are emitted also with the synchronous calls.
 * To execute a bunch of actions with a single call, you can use the executeActions()
 * static method. This is not the same as calling executeAsync() for each action,
 * because the actions are execute with a single request to the helper.
 * To use any of the execute*() methods you have to set the default helper's ID using
 * the setHelperID() static method. Alternatively, you can specify the helperID using
 * the overloaded version of the methods that takes it as a parameter.
 *
 * Each action object contains a QVariantMap object that is passed directly to the
 * helper when the action is executed. You can access this map using the arguments()
 * method. You can insert into it any kind of custom data you need to pass to the helper.
 *
 * @since 4.4
 */
class KDECORE_EXPORT Action
{
    class Private;
    Private * const d;

public:
    /**
     * The three values returned by authorization methods
     */
    enum AuthStatus {
        Denied, ///< The authorization has been denied by the authorization backend
        Error, ///< An error occurred
        Invalid, ///< An invalid action cannot be authorized
        Authorized, ///< The authorization has been granted by the authorization backend
        AuthRequired, ///< The user could obtain the authorization after authentication
        UserCancelled ///< The user pressed Cancel the authentication dialog. Currently used only on the mac
    };

    /**
     * @brief Default constructor
     *
     * This constructor sets the name to the empty string.
     * Such an action is invalid and cannot be authorized nor executed, so
     * you need to call setName() before you can use the object.
     */
    Action();

    /** Copy constructor */
    Action(const Action &action);

    /**
     * This creates a new action object with this name
     * @param name The name of the new action
     */
    Action(const QString &name);

    /**
     * This creates a new action object with this name and details
     * @param name The name of the new action
     * @param details The details of the action
     *
     * @see setDetails
     */
    Action(const QString &name, const QString &details);

    /// Virtual destructor
    ~Action();

    /// Assignment operator
    Action &operator=(const Action &action);

    /**
     * @brief Comparison operator
     *
     * This comparison operator compares the <b>names</b> of two
     * actions and returns whether they are the same. It does not
     * care about the arguments stored in the actions. However,
     * if two actions are invalid they'll match as equal, even
     * if the invalid names are different.
     *
     * @returns true if the two actions are the same or both invalid
     */
    bool operator==(const Action &action) const;

    /**
    * @brief Negated comparison operator
    *
    * Returns the negation of operator==
    *
    * @returns true if the two actions are different and not both invalid
    */
    bool operator!=(const Action &action) const;

    /**
     * @brief Gets the action's name.
     *
     * This is the unique attribute that identifies
     * an action object. Two action objects with the same
     * name always refer to the same action.
     *
     * @return The action name
     */
    QString name() const;

    /**
     * @brief Sets the action's name.
     *
     * It's not common to change the action name
     * after its creation. Usually you set the name
     * with the constructor (and you have to, because
     * there's no default constructor)
     */
    void setName(const QString &name);

    /**
     * @brief Sets the action's details
     *
     * You can use this function to provide the user more details
     * (if the backend supports it) on the action being authorized in
     * the authorization dialog
     */
    void setDetails(const QString &details);

    /**
     * @brief Gets the action's details
     *
     * The details that will be shown in the authorization dialog, if the
     * backend supports it.
     *
     * @return The action's details
     */
    QString details() const;

    /**
     * @brief Returns if the object represents a valid action
     *
     * Action names have to respect a simple syntax.
     * They have to be all in lowercase characters, separated
     * by dots. Dots can't appear at the beginning and at the end of
     * the name.
     *
     * In other words, the action name has to match this perl-like
     * regular expression:
     * @verbatim
     * /^[a-z]+(\.[a-z]+)*$/
     * @endverbatim
     *
     * This method returns false if the action name doesn't match the
     * valid syntax.
     *
     * Invalid actions cannot be authorized nor executed.
     * The empty string is not a valid action name, so the default
     * constructor returns an invalid action.
     */
    bool isValid() const;

    /**
     * @brief Gets the default helper ID used for actions execution
     *
     * The helper ID is the string that uniquely identifies the helper in
     * the system. It is the string passed to the KDE4_AUTH_HELPER() macro
     * in the helper source. Because one could have different helpers,
     * you need to specify an helper ID for each execution, or set a default
     * ID by calling setHelperID(). This method returns the current default
     * value.
     *
     * @return The default helper ID.
     */
    QString helperID() const;

    /**
     * @brief Sets the default helper ID used for actions execution
     *
     * @param id The default helper ID.
     */
    void setHelperID(const QString &id);

    /**
     * @brief Gets the ActionWatcher object for this action
     *
     * ActionWatcher objects are used to get notifications about the action
     * execution status. Every action watcher is tied to an action and
     * every action has a watcher. This means that if you call this method
     * on two different Action objects with the same name, you'll get the
     * same watcher object.
     *
     * @return The action watcher for this action
     */
    ActionWatcher *watcher();

    /**
     * @brief Sets the map object used to pass arguments to the helper.
     *
     * This method sets the variant map that the application
     * can use to pass arbitrary data to the helper when executing the action.
     *
     * @param arguments The new arguments map
     */
    void setArguments(const QVariantMap &arguments);

    /**
     * @brief Returns map object used to pass arguments to the helper.
     *
     * This method returns the variant map that the application
     * can use to pass arbitrary data to the helper when executing the action.
     *
     * @return The arguments map that will be passed to the helper.
     */
    QVariantMap arguments() const;

    /**
    * @brief Convenience method to add an argument.
    *
    * This method adds the pair @c key/value to the QVariantMap used to
    * send custom data to the helper.
    *
    * Use this method if you don't want to create a new QVariantMap only to
    * add a new entry.
    *
    * @param key The new entry's key
    * @param value The value of the new entry
    */
    void addArgument(const QString &key, const QVariant &value);

    /**
     * @brief Acquires authorization for an action without excuting it.
     *
     * This method acquires the authorization rights for the action, asking
     * the user to authenticate if needed.
     *
     * The result of this method is strictly related to the result of status().
     * If it returns Action::Denied or Action::Authorized, this method
     * will always return the same. Instead, if the status()
     * result was Action::AuthRequired, the method would ask the user to authenticate.
     * The Action::UserCancelled value is intended to be returned when the authentication
     * fails because the user purposely cancelled it. Unfortunately, this isn't currently
     * supported by policykit, so instead you'll get a Denied result in this case.
     * The Mac OS X backend will return UserCancelled when appropriate.
     *
     * It's not so common to use this method directly, because it's already
     * called by any of the execute methods. Use it only if you need to acquire the
     * authorization long time before the execution, for example if you want to
     * enable some GUI elements after user authentication.
     *
     * @return The result of the authorization process
     */
    AuthStatus authorize() const;

    /**
     * @brief Gets information about the authorization status of an action
     *
     * This methods query the authorization backend to know if the user can try
     * to acquire the authorization for this action. If the result is Action::AuthRequired,
     * the user can try to acquire the authorization by authenticating.
     *
     * It should not be needed to call this method directly, because the execution methods
     * already take care of all the authorization stuff.
     *
     * @return @c Action::Denied if the user doesn't have the authorization to execute the action,
     *         @c Action::Authorized if the action can be executed,
     *         @c Action::AuthRequired if the user could acquire the authorization after authentication,
     *         @c Action::UserCancelled if the user cancels the authentication dialog. Not currently supported by the Polkit backend
     */
    AuthStatus status() const;

    /**
     * @brief Synchronously executes the action
     *
     * This is the simpler of all the action execution methods. It sends an execution request to the
     * caller, and returns the reply directly to the caller. The ActionReply object will contain the
     * custom data coming from the helper.
     *
     * The method blocks the execution, and will
     * return only when the action has been completed (or failed). Take note, however, that with the dbus
     * helper proxy (currently the only one implemented on all the supported platforms), the request is
     * sent using the QDBus::BlockWithGui flag.
     *
     * This means the method will enter a local eventloop to wait
     * for the reply. This allows the application GUI to stay responsive, but you have to be prepared to
     * receive other events in the meantime.
     *
     * All the signals from the ActionWatcher class are emitted also with this method (although they're more
     * useful with the asynchronous calls)
     *
     * The method checks for authorization before to execute the action. If the user is not authorized, the
     * return value will be ActionReply::AuthorizationDeniedReply.
     * If the user cancels the authentication, the return value should be ActionReply::UserCancelledReply.
     * Due to policykit limitations, this currently only with the Mac OS X backend.
     *
     * If the helper is busy executing another action (or action group) the reply will be ActionReply::HelperBusyReply
     *
     * If the request cannot be sent for bus errors, the method returns ActionReply::DBusErrorReply.
     *
     * @return The reply from the helper, or an error reply if something's wrong.
     */
    ActionReply execute() const;

    /**
     * @brief Synchronously executes the action with a specific helperID
     *
     * This method does the exact same thing as execute(), but it takes a specific helperID, useful
     * if you don't want to use the default one without changing it with setHelperID()
     *
     * @param helperID The helper ID to use for the execution of this action
     * @return The reply from the helper, or an error if something's wrong.
     */
    ActionReply execute(const QString &helperID) const;

    void setExecutesAsync(bool async);
    bool executesAsync() const;

    /**
     * @brief Asynchronously executes a group of actions with a single request
     *
     * This method executes each action in the list. It checks for authorization of each action, and put the
     * denied actions, if any, in the list pointed by the deniedActions parameter, if not NULL.
     *
     * Please note that with the dbus helper proxy (currently the only one implemented), the execution of a group
     * of actions is very different from executing in sequence each action using, for example, executeAsync().
     * Currently, the helper can execute only one request at the time. For this reason, if you have to call
     * different actions in sequence, you can't call executeAsync() like this:
     * @code
     * action1.executeAsync();
     * action2.executeAsync();
     * @endcode
     * because the second call will almost certainly return ActionReply::HelperBusy. You would have to execute the second
     * action in the slot connected to the first action's actionPerformed() signal. This is not so good. This method
     * allows the application to send a request with a list of actions. With this method, the code above becomes:
     * @code
     * QList<Action> list;
     * list << action1 << action2;
     * Action::executeActions(list);
     * @endcode
     * The return value will be false if communication errors occur. It will also be false if <b>all</b> the actions
     * in the list are denied.
     *
     * @param actions The list of actions to execute
     * @param deniedActions A pointer to a list to fill with the denied actions. Pass NULL if you don't need them.
     * @param helperId The helper ID to execute the actions on.
     */
    static bool executeActions(const QList<Action> &actions, QList<Action> *deniedActions, const QString &helperId);

    /**
     * @brief Ask the helper to stop executing an action
     *
     * This method sends a request to the helper asking to stop the execution of an action. It is only
     * useful for long-running actions, because short and fast actions won't obbey to this request most of the times.
     * Calling this method will make the HelperSupport::isStopped() method to return true the next time it's called.
     *
     * It's the helper's responsibility to regularly call it and exit if requested
     * The actionPerformed() signal is emitted normally because, actually, the helper exists regularly. The return data
     * in this case is application-dependent.
     */
    void stop();

    /**
     * @brief Ask the helper to stop executing an action, using a specific helper ID
     *
     * This method works exactly as the stop() method, but it lets you specify an helper ID different from the
     * default one.
     *
     * To stop an action you need to send the stop request to the helper that is executing that action. This of course means you have to
     * use the same helperID used for the execution call (either passed as a parameter or set as default with setHelperID() )
     */
    void stop(const QString &helperID);
};

} // namespace Auth

#endif
