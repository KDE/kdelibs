/**
  * This file is part of the KDE project
  * Copyright (C) 2007, 2006 Rafael Fernández López <ereslibre@gmail.com>
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

#ifndef PROGRESSLISTMODEL_H
#define PROGRESSLISTMODEL_H

#include <QAbstractListModel>
#include <QTimer>

#include <kio/global.h>
#include <kio/jobclasses.h>
#include <kiconloader.h>

class QStyleOptionProgressBarV2;
class KWidgetJobTracker;

struct ActionInfo
{
    int actionId;                           ///< The number that identificates the action
    QString actionText;                     ///< The text that is shown on the button on the editor
};

struct JobInfo
{
    int jobId;                              ///< The number that is associated with the job
    QString applicationInternalName;        ///< The application name (konqueror, kopete...)
    QString applicationName;                ///< The friendly application name (Konqueror, Kopete...)
    QString icon;                           ///< The icon name
    qlonglong fileTotals;                   ///< The number of total files to be processed
    qlonglong filesProcessed;               ///< The number of processed files
    qlonglong dirTotals;                    ///< The number of total dirs to be processed
    qlonglong dirsProcessed;                ///< The number of processed dirs
    QString sizeTotals;                     ///< The total size of the operation
    QString sizeProcessed;                  ///< The processed size at the moment
    qlonglong timeElapsed;                  ///< The elapsed time
    qlonglong timeTotals;                   ///< The total time of the operation
    QString from;                           ///< From where are we performing the operation
    QString to;                             ///< To where are we performing the operation
    QString fromLabel;                      ///< The label to be shown on the delegate
    QString toLabel;                        ///< The label to be shown on the delegate
    QString speed;                          ///< The current speed of the operation (human readable, example, "3Mb/s")
    int percent;                            ///< The current percent of the progress
    QString message;                        ///< The information message to be shown
    QString progressMessage;                ///< The progress information message to be shown
    QList<ActionInfo> actionInfoList;       ///< The list of actions added to the job
    QStyleOptionProgressBarV2 *progressBar; ///< The progress bar to be shown
    KIconLoader *iconLoader;                ///< The icon loader for loading the icon on the delegate
    KWidgetJobTracker *defaultTracker;      ///< The default progress operation window
};

class ProgressListModel
    : public QAbstractItemModel
{
    Q_OBJECT

public:
    ProgressListModel(QObject *parent = 0);

    ~ProgressListModel();

    QModelIndex parent(const QModelIndex&) const;

    /**
      * Returns the data on @p index that @p role contains. The result is
      * a QVariant, so you may need to cast it to the type you want
      *
      * @param index    the index in which you are accessing
      * @param role     the role you want to retrieve
      * @return         the data in a QVariant class
      */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    /**
      * Returns what operations the model/delegate support on the given @p index
      *
      * @param index    the index in which you want to know the allowed operations
      * @return         the allowed operations on the model/delegate
      */
    Qt::ItemFlags flags(const QModelIndex &index) const;

    /**
      * Returns the index for the given @p row. Since it is a list, @p column should
      * be 0, but it will be ignored. @p parent will be ignored as well.
      *
      * @param row      the row you want to get the index
      * @param column   will be ignored
      * @param parent   will be ignored
      * @return         the index for the given @p row as a QModelIndex
      */
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;

    /**
      * Returns the index for the given @p jobId
      *
      * @param jobId    the jobId of which you want to get the index
      * @return         the index for the give @p jobId as a QModelIndex
      */
    QModelIndex indexForJob(uint jobId) const;

    /**
      * Returns the number of columns
      *
      * @param parent   will be ignored
      * @return         the number of columns. In this case is always 1
      */
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    /**
      * Returns the number of rows
      *
      * @param parent   will be ignored
      * @return         the number of rows in the model
      */
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());

    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    /**
      * Sets the data contained on @p value to the given @p index and @p role
      *
      * @param index    the index where the data contained on @p value will be stored
      * @param value    the data that is going to be stored
      * @param role     in what role we want to store the data at the given @p index
      * @return         whether the data was successfully stored or not
      */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    /**
      * Creates a new job on the model with the given @p jobId
      *
      * @param jobId            the identification number of the job that is going to be added to the model
      * @param internalAppName  the application name (konqueror, kopete...)
      * @param jobIcon          the icon name that will be shown for this job
      * @param appName          the friendly application name (Konqueror, Kopete...)
      * @param showProgress     whether the progress of this job is going to be shown or not
      */
    void newJob(uint jobId, const QString &internalAppName, const QString &jobIcon, const QString &appName, bool showProgress);

    /**
      * Removes from the model the data related to the job
      *
      * @param jobId the identification number of the job that is going to be removed
      */
    void finishJob(uint jobId);

    /**
      * Adds a new action to the job on the model
      *
      * @param jobId        the identification number of the job that will contain the new action
      * @param actionId     the identification number of the new action
      * @param actionText   the text that will be shown for this action
      */
    void newAction(uint jobId, uint actionId, const QString &actionText);

    /**
      * Edits an existing action
      *
      * @param jobId        the identification number of the job that contains the action to be modified
      * @param actionId     the identification number of the action to be modified
      * @param actionText   the new text to be shown related to this action
      */
    void editAction(int jobId, int actionId, const QString &actionText);

    /**
      * Returns all existing actions related to a given job identification number
      *
      * @param jobId    the job which we want to retrieve all actions
      * @return         the list of actions related to the identification job number @p jobId
      */
    const QList<ActionInfo> &actions(uint jobId) const;

    /**
      * Returns the progress bar for the given @p index
      *
      * @param index    the index we want to retrieve the progress bar
      * @return         the progress bar for the given @p index. Might return 0 if no progress was set
      */
    QStyleOptionProgressBarV2 *progressBar(const QModelIndex &index) const;

    /**
      * Returns the default progress window for the given @p row
      *
      * @param row  the row of which we want to retrieve the default progress window
      * @return     the default progress window for given @p row
      */
    KWidgetJobTracker *defaultTracker(int row) const;

    /**
      * Sets the default progress window for the given @p row to @p defaultProgress
      *
      * @param row              the row in which we want to set the default window progress
      * @param defaultTracker   the default window progress
      */
    void setDefaultTracker(int row, KWidgetJobTracker *defaultTracker);

Q_SIGNALS:
    /**
      * Called when an action has been modified at @p index
      *
      * @param index the index where the action has been modified
      */
    void actionModified(const QModelIndex &index);

private:
    /**
      * @internal
      */
    bool setData(int row, const QVariant &value, int role = Qt::EditRole);

    /**
      * @internal
      */
    KIconLoader *iconLoader(int row) const;

    /**
      * @internal
      */
    void setIconLoader(int row, KIconLoader *iconLoader);

    QList<JobInfo> jobInfoList; /// @internal
};

#endif // PROGRESSLISTMODEL_H
