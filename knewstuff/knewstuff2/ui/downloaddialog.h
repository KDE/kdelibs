/***************************************************************************
 *   Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>                  *
 *   Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>           *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 ***************************************************************************/

#ifndef KNEWSTUFF2_UI_DOWNLOADDIALOG_H
#define KNEWSTUFF2_UI_DOWNLOADDIALOG_H

#include <QtGui/QDialog>

#include <knewstuff2/dxs/dxsengine.h>
#include <knewstuff2/core/category.h>

class KJob;

namespace KNS
{

/**
 * KNewStuff download dialog.
 *
 * The download dialog will present several categories of items to the user
 * for installation, deinstallation and updates.
 * Preview images as well as other meta information can be seen.
 * If the GHNS provider supports DXS, then the dialog will offer an extended
 * popup menu with collaborative interactions such as ratings, removal
 * requests, comments and translation suggestions.
 *
 * This class is used internally by the Engine class as part of the download
 * workflow.
 *
 * @internal
 */
class DownloadDialog : public QDialog
{
    Q_OBJECT
    public:
        DownloadDialog( QWidget * parent );
        ~DownloadDialog();

	void addEntry(Entry *entry, const Feed *feed, const Provider *provider);
	void refresh();

	void setEngine(DxsEngine *engine);

        // show a message in the bottom bar
        enum MessageType { Normal, Info, Error };
        void displayMessage( const QString & msg,
            MessageType type = Normal, int timeOutMs = 3000 );

        // begin installing that item
        void installItem( Entry * entry );

        // remove an already installed item
        void removeItem( Entry * entry );

    private:
        // private storage class
        class DownloadDialogPrivate * d;

	DxsEngine *m_engine;
	QMap<QString, QString> m_categorymap;

	//QList<Entry*> m_entries;
	QMap<const Feed*, KNS::Entry::List> m_entries;

    private Q_SLOTS:
        void slotResetMessageColors();
        void slotNetworkTimeout();
        void slotSortingSelected( int sortType );
	// DXS
        void slotLoadProvidersListDXS();
        void slotLoadProviderDXS(int index);
	void slotCategories(QList<KNS::Category*> categories);
	void slotEntries(QList<KNS::Entry*> entries);
	void slotFault();
	void slotError();
        // file downloading
        void slotDownloadItem( Entry * );
        //void slotItemMessage( KJob *, const QString & );
        //void slotItemPercentage( KJob *, unsigned long );
        //void slotItemResult( KJob * );
};

}

#endif
