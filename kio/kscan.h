/* This file is part of the KDE libraries
    Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef KSCAN_H
#define KSCAN_H

#include <kdialogbase.h>
#include <kinstance.h>
#include <klibloader.h>

class QImage;

// baseclass for scan-dialogs
class KScanDialog : public KDialogBase
{
    Q_OBJECT

public:
    static KScanDialog * getScanDialog( QWidget *parent=0L,
					const char *name=0, bool modal=false );
    ~KScanDialog();

protected:
    KScanDialog( int dialogFace=Tabbed, int buttonMask = Close|Help,
		 QWidget *parent=0L, const char *name=0, bool modal=false );

    int id() const { return m_currentId; }
    int nextId() { return ++m_currentId; }

signals:
    // we need an id so that applications can distinguish between new
    // scans and redone scans!
    void preview( const QImage&, int id );

    void finalImage( const QImage&, int id );

    void textRecognized( const QString&, int id );

private:
    int m_currentId;

};


class KScanDialogFactory : public KLibFactory
{
public:
    virtual ~KScanDialogFactory();

    /**
     * Your library should reimplement this method to return your KScanDialog
     * derived dialog.
     */
    virtual KScanDialog * createDialog( QWidget *parent=0, const char *name=0,
					bool modal=false ) = 0;

protected:
    KScanDialogFactory( QObject *parent=0, const char *name=0 );

    virtual QObject* createObject( QObject* parent = 0, const char* name = 0,
                                   const char* classname = "QObject",
                                   const QStringList &args = QStringList() );


    void setName( const QCString& instanceName ) {
	delete m_instance;
	m_instance = new KInstance( instanceName );
    }

    KInstance *instance() const { return m_instance; }

private:
    KInstance *m_instance;

};

#endif // KSCAN_H
