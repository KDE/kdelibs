/***************************************************************************
 *   Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>                  *
 *   Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KNEWSTUFF2_UI_DOWNLOADDIALOG_H
#define KNEWSTUFF2_UI_DOWNLOADDIALOG_H

#include <qdialog.h>

#include <knewstuff2/dxs/dxsengine.h>
#include <knewstuff2/core/category.h>

class KJob;

namespace KNS
{

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

    private slots:
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
