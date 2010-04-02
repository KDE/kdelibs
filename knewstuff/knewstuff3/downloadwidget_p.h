/*
    knewstuff3/ui/downloaddialog.cpp.
    Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
    Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2007-2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009-2010 Frederik Gladhorn <gladhorn@kde.org>
    Copyright (C) 2010 Reza Fatahilah Shah <rshah0385@kireihana.com>
    
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

#ifndef KNEWSTUFF3_UI_DOWNLOADWIDGET_P_H
#define KNEWSTUFF3_UI_DOWNLOADWIDGET_P_H

#include <QtGui/QSortFilterProxyModel>
#include <QtCore/QTimer>
#include <QtGui/QScrollBar>
#include <QListView>
#include <kmessagebox.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>

#include "ui/itemsmodel.h"
#include "ui/itemsviewbasedelegate.h"
#include "ui/entrydetailsdialog.h"

#include "ui_downloadwidget.h"

namespace KNS3 {
class DownloadWidget;

class DownloadWidgetPrivate
{
public:
    DownloadWidget* q;
    EntryDetails* details;

    // The engine that does all the work
    Engine *engine;
    Ui::DownloadWidget ui;
    // Model to show the entries
    ItemsModel* model;
    // Timeout for messge display
    QTimer* messageTimer;

    ItemsViewBaseDelegate * delegate;

    QString searchTerm;
    QSet<EntryInternal> changedEntries;

    QSet<QString> categories;
    QSet<QString> providers;

    QString titleText;
    QString m_configFile;
    bool dialogMode;

    DownloadWidgetPrivate(DownloadWidget* q);
    ~DownloadWidgetPrivate();

    void init(const QString& configFile);
    void displayMessage(const QString & msg, KTitleWidget::MessageType type, int timeOutMs = 0);

    void slotProvidersLoaded();
    void slotEntriesLoaded(const KNS3::EntryInternal::List& entries);
    void slotEntryChanged(const KNS3::EntryInternal& entry);
    
    void slotShowDetails(const KNS3::EntryInternal& entry);
    void slotShowOverview();

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

    void slotUpload();
    void slotListViewListMode();
    void slotListViewIconMode();
    void setListViewMode(QListView::ViewMode mode);
};

}

#endif
