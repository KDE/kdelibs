/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
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

#ifndef ACTION_WATCHER_H
#define ACTION_WATCHER_H

#include <QtCore/QObject>
#include <QtCore/QString>

#include <kdecore_export.h>

#include "kauthactionreply.h"
#include "kauthaction.h"

namespace KAuth
{

/**
 * @brief Class used to receive notifications about the status of an action execution.
 *
 * The ActionWatcher class provides some signals useful to track the execution of an action.
 * The Action class is designed to be very ligthweight, so it's not the case to make it
 * a QObject subclass. This means the action object can't expose signals. This is the reason
 * why every action (not every Action object) used by the app has an associated ActionWatcher.
 *
 * You don't create watchers directly. Instead, you should get one from the Action::watcher() method,
 * if you have an action object, or with the ActionWatcher::watcher() static method, which takes the
 * action name string.
 *
 * See the documentation of single signals for more details about them.
 *
 * @since 4.4
 */
class KDECORE_EXPORT ActionWatcher : public QObject
{
    Q_OBJECT

    class Private;
    Private * const d;

    ActionWatcher();
    ActionWatcher(const QString &action);

    Q_PRIVATE_SLOT(d, void actionStartedSlot(const QString &action))
    Q_PRIVATE_SLOT(d, void actionPerformedSlot(const QString &action, const ActionReply &reply))
    Q_PRIVATE_SLOT(d, void progressStepSlot(const QString &action, int i))
    Q_PRIVATE_SLOT(d, void progressStepSlot(const QString &action, const QVariantMap &data))
    Q_PRIVATE_SLOT(d, void statusChangedSlot(const QString &action, Action::AuthStatus status))

public:
    /**
     * @brief Factory method to get watchers
     *
     * This method allows you to obtain (and create if needed) an
     * action watcher from the action string identifier.
     * It's more common to obtain a watcher using Action::watcher(),
     * which actually calls this method.
     *
     * Every signal of this class
     * is emitted whichever method you used to execute the action.
     * This means you could connect to the signal actionPerformed()
     * even if you're using the execute() method (which already returns the reply)
     * and you'll get the same reply.
     *
     * @param action The action string identifier for the creation of the watcher
     * @return The action watcher associated with the given action
     */
    static ActionWatcher *watcher(const QString &action);

    /// Virtual destructor
    virtual ~ActionWatcher();

    /// Returns the action name associated with this watcher
    QString action() const;

Q_SIGNALS:
    /**
     * @brief Signal emitted when an action starts the execution
     *
     * This signal is emitted whe In case of
     * execute() and executeAsync(), the signal is emitted about
     * immediately, because the request is very fast.
     *
     * If you execute a group of actions using Action::executeActions(),
     * this signal is emitted when the single action is actually about
     * to be executed, not when the whole group starts executing.
     * This means you can use this signal to start some kind of timeout
     * to handle helper crashes, if you feel the need.
     */
    void actionStarted();

    /**
     * @brief Signal emitted when an action completed the execution
     *
     * This signal provides the only way to obtain the reply from the helper
     * in case of asynchronous calls. The reply object is the same returned
     * by the helper, or an error reply from the library if something went
     * wrong.
     *
     * @param reply The reply coming from the helper
     */
    void actionPerformed(const ActionReply &reply);

    /**
     * @brief Signal emitted by the helper to notify the action's progress
     *
     * This signal is emitted every time the helper's code calls the
     * HelperSupport::progressStep(int) method. This is useful to let the
     * helper notify the execution status of a long action.
     * The meaning of the integer passed here is totally application-dependent.
     * If you need to be more expressive, you can use the other signal that
     * pass a QVariantMap.
     *
     * @param progress The progress indicator from the helper
     */
    void progressStep(int progress);

    /**
    * @brief Signal emitted by the helper to notify the action's progress
    *
    * This signal is emitted every time the helper's code calls the
    * HelperSupport::progressStep(QVariantMap) method. This is useful to let the
    * helper notify the execution status of a long action, also providing
    * some data, for example if you want to achieve some sort of progressive loading.
    * The meaning of the data passed here is totally application-dependent.
    * If you only need to pass some percentage, you can use the other signal that
    * pass an int.
    *
    * @param data The progress data from the helper
    */
    void progressStep(const QVariantMap &data);

    void statusChanged(int status);
};

} // namespace Auth

#endif
