/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <goffioul@imec.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef NETWORKSCANNER_H
#define NETWORKSCANNER_H

#include <qwidget.h>
#include <kdialogbase.h>
#include <qptrlist.h>

class NetworkScanner : public QWidget
{
	Q_OBJECT

public:
	NetworkScanner( int port = 9100, QWidget *parent = 0, const char *name = 0 );
	~NetworkScanner();

	struct SocketInfo
	{
		QString IP;
		QString Name;
		int Port;
	};

	const QPtrList<NetworkScanner::SocketInfo>* printerList();

	int timeout() const;
	void setTimeout( int to );
	QString subnet() const;
	void setSubnet( const QString& );
	int port() const;
	void setPort( int );
	bool checkPrinter( const QString&, int );

signals:
	void scanStarted();
	void scanFinished();

protected slots:
	void slotConnectionSuccess();
	void slotConnectionFailed( int );
	void slotTimeout();
	void slotScanClicked();
	void slotSettingsClicked();
	void slotNext();

protected:
	void next();
	void finish();
	void start();

private:
	class NetworkScannerPrivate;
	NetworkScannerPrivate *d;
};

class QLineEdit;
class QComboBox;

class NetworkScannerConfig : public KDialogBase
{
	Q_OBJECT

public:
	NetworkScannerConfig(NetworkScanner *scanner, const char *name = 0);
	~NetworkScannerConfig();

protected slots:
	void slotOk();

private:
	QLineEdit	*mask_, *tout_;
	QComboBox	*port_;
	NetworkScanner *scanner_;
};

#endif
