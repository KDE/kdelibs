/*
    knewstuff3/ui/downloaddialog.h.
    Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
    Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2007-2009 Jeremy Whiting <jeremy@scitools.com>

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

#ifndef KNEWSTUFF3_UI_DOWNLOADDIALOG_H
#define KNEWSTUFF3_UI_DOWNLOADDIALOG_H

#include <QMutex>

#include <kdialog.h>
#include <ktitlewidget.h>

#include <knewstuff3/engine.h>
#include <knewstuff3/core/category.h>

#include "ui_DownloadDialog.h"

class KJob;
class KLineEdit;
class QComboBox;
class QHideEvent;
class QLabel;
class ProgressIndicator;
class QSortFilterProxyModel;

namespace KNS3
{

class DownloadDialog;
class ItemsModel;
class ItemsViewDelegate;

/**
 * KNewStuff download dialog.
 *
 * The download dialog will present several categories of items to the user
 * for installation, uninstallation and updates.
 * Preview images as well as other meta information can be seen.
 * If the GHNS provider supports DXS, then the dialog will offer an extended
 * popup menu with collaborative interactions such as ratings, removal
 * requests, comments and translation suggestions.
 *
 * Please remember when changing this class that it has to render
 * untrusted data. Do not let scripts run, make sure that data is properly escaped etc.
 *
 * This class is used internally by the Engine class as part of the download
 * workflow.
 *
 * @internal
 */
class DownloadDialog : public KDialog, public Ui::DownloadDialog
{
    Q_OBJECT

public:
    DownloadDialog(Engine* engine, QWidget * parent);
    ~DownloadDialog();

    enum EntryAction {
        kViewInfo,
        kComments,
        kChanges,
        kContactEmail,
        kContactJabber,
        kCollabTranslate,
        kCollabRemoval,
        kCollabSubscribe,
        kUninstall,
        kInstall,
        kCollabComment,
        kCollabRate
    };

    // show a message in the bottom bar
    void displayMessage(const QString & msg,
                        KTitleWidget::MessageType type = KTitleWidget::PlainMessage,
                        int timeOutMs = 0);

private Q_SLOTS:
    void slotEntryChanged(const KNS3::Entry& entry);
    
    void slotPayloadFailed(const Entry& entry);
    void slotPayloadLoaded(KUrl url);

    void slotResetMessage();
    void slotNetworkTimeout();
    void slotSortingSelected(int sortType);
    void slotSearchTextChanged();
    void slotUpdateSearch();

    void slotInfo(QString provider, QString server, QString version);
    void slotComments(QStringList comments);
    
    void slotCategories(QList<Category*> categories);

    void slotError(const QString& message);

    // file downloading
    //void slotItemMessage( KJob *, const QString & );
    //void slotItemPercentage( KJob *, unsigned long );
    //void slotItemResult( KJob * );
    void slotProgress(const QString & text, int percentage);

    void slotPerformAction(DownloadDialog::EntryAction action, KNS3::Entry entry);
    void slotCollabAction(QAction * action);

    void slotListIndexChanged(const QModelIndex &index, const QModelIndex &old);

    void scrollbar(int value);
Q_SIGNALS:
    void signalRequestMoreData();
    
protected:
    virtual void hideEvent(QHideEvent * event);

private:

    void populateSortCombo(const Provider * provider);

    class Private;
    Private *const d;
};

}

#endif
