/*
 * Copyright (c) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef ACTIVITY_CONTROLLER_H
#define ACTIVITY_CONTROLLER_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QStringList>

#include "kactivityconsumer.h"

#include <kurl.h>
#include <kdemacros.h>

/**
 * This class provides methods for controlling and managing
 * the activities.
 *
 * @see KActivityConsumer for info about activities
 *
 * @since 4.5
 */
class KDE_EXPORT KActivityController: public KActivityConsumer
{
    Q_OBJECT

    Q_PROPERTY(QString currentActivity READ currentActivity WRITE setCurrentActivity)

public:
    explicit KActivityController(QObject * parent = 0);

    ~KActivityController();

    /**
     * Sets the name of the specified activity
     * @param id id of the activity
     * @param name name to be set
     */
    void setActivityName(const QString & id, const QString & name);

    /**
     * Sets the icon of the specified activity
     * @param id id of the activity
     * @param icon icon to be set - freedesktop.org name or file path
     */
    void setActivityIcon(const QString & id, const QString & icon);

    /**
     * Sets the current activity
     * @param id id of the activity to make current
     * @returns true if successful
     */
    bool setCurrentActivity(const QString & id);

    /**
     * Adds a new activity
     * @param name name of the activity
     * @returns id of the newly created activity
     */
    QString addActivity(const QString & name);

    /**
     * Removes the specified activity
     * @param id id of the activity to delete
     */
    void removeActivity(const QString & id);

    /**
     * Stops the activity
     * @param id id of the activity to stop
     */
    void stopActivity(const QString & id);

    /**
     * Starts the activity
     * @param id id of the activity to start
     */
    void startActivity(const QString & id);

Q_SIGNALS:
    /**
     * This signal is emitted when the global
     * activity is changed
     * @param id id of the new current activity
     */
    void activityAdded(const QString & id);

    /**
     * This signal is emitted when the activity
     * is removed
     * @param id id of the removed activity
     */
    void activityRemoved(const QString & id);

private:
    class Private;
    Private * const d;
};

#endif // ACTIVITY_CONTROLLER_H
