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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include "kscan.h"

#include <QtCore/QFile>

#include <klocale.h>
#include <kservicetypetrader.h>

class KScanDialog::KScanDialogPrivate
{
public:
    KScanDialogPrivate()
        : m_currentId( 1 )
    {}
    int m_currentId;
};

// static factory method
KScanDialog * KScanDialog::getScanDialog( QWidget *parent )
{
    return KServiceTypeTrader::createInstanceFromQuery<KScanDialog>( "KScan/KScanDialog", QString(), parent );
}


KScanDialog::KScanDialog( int dialogFace, int buttonMask,
			  QWidget *parent )
    : KPageDialog( parent ),
      d( new KScanDialogPrivate )
{
  setFaceType( (KPageDialog::FaceType)dialogFace );
  setCaption( i18n("Acquire Image") );
  setButtons( (KDialog::ButtonCodes)buttonMask );
  setDefaultButton( Close );
}

KScanDialog::~KScanDialog()
{
    delete d;
}

int KScanDialog::id() const
{
    return d->m_currentId;
}

int KScanDialog::nextId()
{
    return ++d->m_currentId;
}

bool KScanDialog::setup()
{
    return true;
}

///////////////////////////////////////////////////////////////////

class KOCRDialog::KOCRDialogPrivate
{
public:
    KOCRDialogPrivate()
        : m_currentId( 1 )
    {}
    int m_currentId;
};

// static factory method
KOCRDialog * KOCRDialog::getOCRDialog( QWidget *parent )
{
    return KServiceTypeTrader::createInstanceFromQuery<KOCRDialog>( "KScan/KOCRDialog", QString(), parent );
}


KOCRDialog::KOCRDialog( int dialogFace, int buttonMask,
			  QWidget *parent, bool modal )
    : KPageDialog( parent ),
      d( new KOCRDialogPrivate )
{
  setFaceType( (KPageDialog::FaceType)dialogFace );
  setCaption( i18n("OCR Image") );
  setButtons( (KDialog::ButtonCodes)buttonMask );
  setDefaultButton( Close );
  setModal( modal );
}

KOCRDialog::~KOCRDialog()
{
    delete d;
}

int KOCRDialog::id() const
{
    return d->m_currentId;
}

int KOCRDialog::nextId()
{
    return ++d->m_currentId;
}



#include "kscan.moc"
