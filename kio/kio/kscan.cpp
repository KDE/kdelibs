/* This file is part of the KDE libraries
    Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qfile.h>

#include <klocale.h>
#include <ktrader.h>

#include "kscan.h"

// static factory method
KScanDialog * KScanDialog::getScanDialog( QWidget *parent, const char *name,
					  bool modal )
{
    KTrader::OfferList offers = KTrader::self()->query("KScan/KScanDialog");
    if ( offers.isEmpty() )
	return 0L;
	
    KService::Ptr ptr = *(offers.begin());
    KLibFactory *factory = KLibLoader::self()->factory( QFile::encodeName(ptr->library()) );

    if ( !factory )
        return 0;

    QStringList args;
    args << QString::number( (int)modal );

    QObject *res = factory->create( parent, name, "KScanDialog", args );

    return dynamic_cast<KScanDialog *>( res );
}


KScanDialog::KScanDialog( int dialogFace, int buttonMask,
			  QWidget *parent, const char *name, bool modal )
    : KDialogBase( dialogFace, i18n("Acquire Image"), buttonMask, Close,
		   parent, name, modal, true ),
      m_currentId( 1 )
{
}

KScanDialog::~KScanDialog()
{
}

bool KScanDialog::setup()
{
    return true;
}

///////////////////////////////////////////////////////////////////


// static factory method
KOCRDialog * KOCRDialog::getOCRDialog( QWidget *parent, const char *name,
					  bool modal )
{
    KTrader::OfferList offers = KTrader::self()->query("KScan/KOCRDialog");
    if ( offers.isEmpty() )
	return 0L;
	
    KService::Ptr ptr = *(offers.begin());
    KLibFactory *factory = KLibLoader::self()->factory( QFile::encodeName(ptr->library()) );

    if ( !factory )
        return 0;

    QStringList args;
    args << QString::number( (int)modal );

    QObject *res = factory->create( parent, name, "KOCRDialog", args );

    return dynamic_cast<KOCRDialog *>( res );
}


KOCRDialog::KOCRDialog( int dialogFace, int buttonMask,
			  QWidget *parent, const char *name, bool modal )
    : KDialogBase( dialogFace, i18n("OCR Image"), buttonMask, Close,
		   parent, name, modal, true ),
      m_currentId( 1 )
{

}

KOCRDialog::~KOCRDialog()
{
}


///////////////////////////////////////////////////////////////////


KScanDialogFactory::KScanDialogFactory( QObject *parent, const char *name )
    : KLibFactory( parent, name ),
      m_instance( 0L )
{
}

KScanDialogFactory::~KScanDialogFactory()
{
    delete m_instance;
}

QObject *KScanDialogFactory::createObject( QObject *parent, const char *name,
                                           const char *classname,
                                           const QStringList &args )
{
    if ( strcmp( classname, "KScanDialog" ) != 0 )
        return 0;

    if ( parent && !parent->isWidgetType() )
       return 0;

    bool modal = false;

    if ( args.count() == 1 )
        modal = (bool)args[ 0 ].toInt();

    return createDialog( static_cast<QWidget *>( parent ), name, modal );
}


///////////////////////////////////////////////////////////////////


KOCRDialogFactory::KOCRDialogFactory( QObject *parent, const char *name )
    : KLibFactory( parent, name ),
      m_instance( 0L )
{
}

KOCRDialogFactory::~KOCRDialogFactory()
{
    delete m_instance;
}

QObject *KOCRDialogFactory::createObject( QObject *parent, const char *name,
                                           const char *classname,
                                           const QStringList &args )
{
    if ( strcmp( classname, "KOCRDialog" ) != 0 )
        return 0;

    if ( parent && !parent->isWidgetType() )
       return 0;

    bool modal = false;

    if ( args.count() == 1 )
        modal = (bool)args[ 0 ].toInt();

    return createDialog( static_cast<QWidget *>( parent ), name, modal );
}

void KScanDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

void KScanDialogFactory::virtual_hook( int id, void* data )
{ KLibFactory::virtual_hook( id, data ); }

void KOCRDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

void KOCRDialogFactory::virtual_hook( int id, void* data )
{ KLibFactory::virtual_hook( id, data ); }


#include "kscan.moc"
