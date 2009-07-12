/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*                                                               
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or   
*   (at your option) any later version.                                 
*                                                                       
*   This program is distributed in the hope that it will be useful,     
*   but WITHOUT ANY WARRANTY; without even the implied warranty of      
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       
*   GNU General Public License for more details.                        
*                                                                       
*   You should have received a copy of the GNU General Public License   
*   along with this program; if not, write to the                       
*   Free Software Foundation, Inc.,                                     
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .      
*/

#ifndef ACTION_H
#define ACTION_H

#include <QString>
#include <QVariant>
#include <QHash>

#include "ActionReply.h"
#include "ActionWatcher.h"

class ActionPrivate;

/**
 * @brief Class to access, authorize and execute actions.
 *
 * This is the main class of the kauth API. It provides the interface to
 * manipulate actions. Every action is identified by its name. Every instance
 * of the Action class with the same name refers to the same action.
 *
 * Once you have an action object you can tell the helper to execute it
 * (asking the user to authenticate if needed) with on the execute*() methods
 * The simplest thing to do is to execute a single action synchronously 
 * blocking for the reply, using the execute() method.
 *
 * For asynchronous calls, use the executeAsync() method. It sends the request
 * to the helper and returns immediatly. You can optionally provide an object
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
 */
class Action
{        
    ActionPrivate *d;
    
    public:
        /**
         * The three values returned by authorization methods
         */
        enum AuthStatus
        {
            Denied = 0, ///< The authorization has been denied by the authorization backend
            Authorized = 1, ///< The authorization has been granted by the authorization backend
            AuthRequired = 2 ///< The user could obtain the authorization after authentication
        };
        
        /** Copy constructor */
        Action(const Action &action);
        
        /**
         * This creates a new action object with this name
         * @param name The name of the new action
         */
        Action(const char *name);
        
        /**
         * This creates a new action object with this name
         * @param name The name of the new action
         */
        Action(const QString &name);
        
        /// Virtual destructor
        virtual ~Action();
        
        /// Assignment operator
        Action &operator=(const Action &action);
        
        /**
         * @brief Gets the action's name.
         *
         * This is the unique attribute that identifies
         * an action object. Two action objects with the same
         * name always refers to the same action.
         *
         * @return The action name
         */
        QString name();
        
        /**
         * @brief Sets the action's name.
         *
         * It's not common to change the action name
         * after it's creation. Usually you set the name
         * with the constructor (and you have to, because
         * there's no default constructor)
         */
        void setName(QString name);
        
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
        static QString helperID();
        
        /**
         * @brief Sets the default helper ID used for actions execution
         *
         * @param id The default helper ID.
         */
        static void setHelperID(const QString &id);
        
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
         * @brief The map object used to pass arguments to the helper.
         *
         * This method returns a reference to a variant map that the application
         * can use to pass arbitrary data to the helper when executing the action.
         *
         * The returned reference allows you to directly set the contents easily.
         * Something like:
         * @code 
         * action.arguments()["key"] = value;
         * @endcode
         * 
         * @return A reference to the arguments map that will be passed to the helper.
         */
        QVariantMap &arguments();
        
        /**
         * @brief Acquires authorization for an action without excuting it.
         *
         * This method acquires the authorization rights for the action, asking
         * the user to authenticate if needed.
         *
         * The result of this method is strictly related to the result of status().
         * If it returns Action::Denied or Action::Authorized, this method
         * will always return false or true, respectively. Instead, if the status()
         * result was Action::AuthRequired, the method would ask the user to authenticate.
         *
         * It's not so common to use this method directly, because it's already
         * called by any of the execute methods. Use it only if you need to acquire the
         * authorization long time before the execution, for example if you want to
         * enable some GUI elements after user authentication.
         *
         * @return @c true if the action is authorized,
         *         @c false otherwise
         */
        bool authorize();
        
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
         * @return @c Action::Denied if the user doesn't have the authorization to execute the action
         *         @c Action::Authorized if the action can be executed
         *         @c Action::AuthRequired if the user could acquire the authorization after authentication
         */
        AuthStatus status();
        
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
         * return value will be ActionReply::AuthorizationDenied. Due to policykit limitations, this currently
         * happens also if the user press Cancel in the authentication dialog.
         *
         * If the helper is busy executing another action (or action group) the reply will be ActionReply::HelperBusyReply
         *
         * If the request cannot be sent for bus errors, the method returns ActionReply::DBusErrorReply.
         *
         * @return The reply from the helper, or an error reply if something's wrong.
         */
        ActionReply execute();
        
        /**
         * @brief Synchronously executes the action with a specific helperID
         *
         * This method does the exact same thing as execute(), but it takes a specific helperID, useful
         * if you don't want to use the default one without changing it with setHelperID()
         *
         * @param helperID The helper ID to use for the execution of this action
         * @return The reply from the helper, or an error if something's wrong.
         */
        ActionReply execute(const QString &helperID);
        
        /**
         * @brief Asynchronously executes the action
         *
         * This method executes the action an returns immediatly. The return value is false if
         * a communication error occurred or if the authorization has been denied.
         *
         * To know when the action has been completed, and to get the reply data, 
         * you can connect to the actionPerformed() signal of the action watcher object.
         * You can do this also by passing the target object and the slot to the parameters of this method.
         * If on of the parameter is NULL, no connection is made.
         *
         * @param target The object to connect to the actionPerformed() signal
         * @param slot The slot to connect to the actionPerformed() signal
         * @return true if the execution is started successfully, false otherwise
         */
        bool executeAsync(QObject *target = NULL, const char *slot = NULL);
        
        /**
         * @brief Asynchronously executes the action with a specific helperID
         *
         * This method works exactly as the executeAsync(QObject*,const char*) method, but it lets you
         * specify a different helperID, if you don't want to change it with setHelperID()
         *
         * @param helperID The helper ID to use for this invocation
         * @param target The object to connect to the actionPerformed() signal
         * @param slot The slot to connect to the actionPerformed() signal
         * @return true if the execution is started successfully, false otherwise
         */
        bool executeAsync(const QString &helperID, QObject *target = NULL, const char *slot = NULL);
        
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
         * The return value will be false if communication errors occour. It will also be false if <b>all</b> the actions
         * in the list are denied.
         *
         * @param actions The list of actions to execute
         * @param deniedActions A pointer to a list to fill with the denied actions. Pass NULL if you don't need them.
         * @param helperID An optional helper ID, if you don't want to use the default one set with setHelperID(). Pass the empty string "" to use the default.
         */
        static bool executeActions(const QList<Action> &actions, QList<Action> *deniedActions = NULL, const QString &helperID = "");
        
        /**
         * @brief Ask the helper to stop executing an action
         *
         * This method sends a request to the helper asking to stop the execution of an action. It is only
         * useful for long-running actions, because short and fast actions won't obbey to this request most of the times.
         * Calling this method will make the HelperSupport::isStopped() method to return true the next time it's called.
         *
         * It's the helper's responsibility to regularly call it and exit if requested
         * The actionPerformed() signal is emitted regularly because, actually, the helper exists regularly. The return data
         * in this case is application-dependent.
         */
        void stop();
        
        /**
         * @brief Ask the helper to stop executing an action, using a specific helper ID
         *
         * This method works exactly as the stop() method, but it let's you specify an helper ID different from the
         * default one.
         *
         * It's important to understand that because the helper ID uniquely identifies an helper, to sto an action you
         * need to send the stop request to the helper that is executing that action. This of course means you have to
         * use the same helperID used for the execution call (either passed as a parameter or set as default with setHelperID() )
         */
        void stop(const QString &helperID);
};

#endif
