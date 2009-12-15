/*
    knewstuff3/ui/downloaddialog.cpp.
    Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
    Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2007-2009 Jeremy Whiting <jeremy@scitools.com>
    Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNEWSTUFF3_UI_DOWNLOADDIALOG_P_H
#define KNEWSTUFF3_UI_DOWNLOADDIALOG_P_H


#include "downloaddialog.h"

#include <QtGui/QSortFilterProxyModel>
#include <QtCore/QTimer>
#include <QtGui/QScrollBar>

#include <kmessagebox.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <ktitlewidget.h>

#include "ui/itemsmodel.h"
#include "ui/itemsviewdelegate.h"

#include "ui_downloaddialog.h"


namespace KNS3 {

class DownloadDialogPrivate :public QObject {

    Q_OBJECT

public:
    Ui::DownloadDialog ui;
    // The engine that does all the work
    Engine *engine;
    // Model to show the entries
    ItemsModel* model;
    // sort items according to sort combo
    QSortFilterProxyModel * sortingProxyModel;
    // Timeout for messge display
    QTimer* messageTimer;

    ItemsViewDelegate * delegate;

    QString searchTerm;
    QSet<EntryInternal> changedEntries;

    QSet<QString> categories;
    QSet<QString> providers;

    DownloadDialogPrivate();
    ~DownloadDialogPrivate();
    void init(const QString& configFile);
    void displayMessage(const QString & msg, KTitleWidget::MessageType type, int timeOutMs = 0);

public Q_SLOTS:
    void slotEntriesLoaded(const KNS3::EntryInternal::List& entries);
    void slotEntryChanged(const KNS3::EntryInternal& entry);

    void slotPayloadFailed(const EntryInternal& entry);
    void slotPayloadLoaded(KUrl url);

    void slotResetMessage();
    void slotNetworkTimeout();
    void sortingChanged();
    void slotSearchTextChanged();
    void slotUpdateSearch();
    void slotCategoryChanged(int);

    void slotInfo(QString provider, QString server, QString version);
    void slotError(const QString& message);
    void scrollbarValueChanged(int value);

Q_SIGNALS:
    void signalRequestMoreData();
};

}

#endif
