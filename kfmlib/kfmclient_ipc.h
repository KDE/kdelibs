/* This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#ifndef KfmIpc_h
#define KfmIpc_h

#include <ctype.h>
#include <ksock.h>
#include <qobject.h>
#include "kfmipc.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

class KfmIpc : public QObject
{
    Q_OBJECT
public:
    KfmIpc( int _port );
    ~KfmIpc();

    bool isConnected();

public slots:
	void refreshDesktop();
public slots:
	void refreshDirectory(const char* _url);
public slots:
	void openURL(const char* _url);
public slots:
	void openProperties(const char* _url);
public slots:
	void list(const char* _url);
public slots:
	void exec(const char* _url, const char* _binding);
public slots:
	void copy(const char* _src, const char* _dest);
public slots:
	void move(const char* _src, const char* _dest);
public slots:
	void moveClient(const char* _src, const char* _dest);
public slots:
	void copyClient(const char* _src, const char* _dest);
public slots:
	void sortDesktop();
public slots:
	void auth(const char* _password);
public slots:
	void selectRootIcons(int _x, int _y, int _w, int _h, bool _add);
signals:
	void finished();
private:
	void parse_finished( char *_data, int _len );
signals:
	void error(int _kerror, const char* _text);
private:
	void parse_error( char *_data, int _len );
signals:
	void dirEntry(const char* _name, const char* _access, const char* _owner, const char* _group, const char* _date, int _size);
private:
	void parse_dirEntry( char *_data, int _len );
public slots:
    void readEvent( KSocket * );
    void closeEvent( KSocket * );
private:
    void parse( char *_data, int _len );

    int port;
    KSocket *sock;
    bool connected;
    char headerBuffer[11];
    int cHeader;
    bool bHeader;
    char *pBody;
    int cBody;
    int bodyLen;
};

#endif
