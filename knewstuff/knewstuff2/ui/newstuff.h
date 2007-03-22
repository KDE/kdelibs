/***************************************************************************
 *   Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>                  *
 *   Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KNEWSTUFF2_UI_NEWSTUFF_H
#define KNEWSTUFF2_UI_NEWSTUFF_H

#include <qdialog.h>

#include <knewstuff2/dxs/dxs.h>

using namespace KNS;

class AvailableItem;
class KJob;

class NewStuffDialog : public QDialog
{
    Q_OBJECT
    public:
        NewStuffDialog( QWidget * parent );
        ~NewStuffDialog();

	void setEngine(Dxs *engine);

        // show a message in the bottom bar
        enum MessageType { Normal, Info, Error };
        void displayMessage( const QString & msg,
            MessageType type = Normal, int timeOutMs = 3000 );

        // begin installing that item
        void installItem( AvailableItem * item );

        // remove an already installed item
        void removeItem( AvailableItem * item );

    private:
        // private storage class
        class NewStuffDialogPrivate * d;

	Dxs *m_dxs;
	QMap<QString, QString> m_categorymap;

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
        void slotDownloadItem( AvailableItem * );
        //void slotItemMessage( KJob *, const QString & );
        //void slotItemPercentage( KJob *, unsigned long );
        //void slotItemResult( KJob * );
};

#endif
