/*
    This file is part of libkabc and/or kaddressbook.
    Copyright (c) 2002 Klarälvdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>

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

#include "resourceimap.h"
#include <kglobal.h>
#include <klocale.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <kabc/formatfactory.h>
#include <ktempfile.h>
#include <kdebug.h>

using namespace KABC;

extern "C"
{
  Resource *resource( AddressBook *ab, const KConfig* )
  {
    KGlobal::locale()->insertCatalogue( "kabc_imap" );
    return new ResourceIMAP( ab );
  }
}

ResourceIMAP::ResourceIMAP( AddressBook *ab )
  : Resource( ab )
{
  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( "vcard" );
}

ResourceIMAP::~ResourceIMAP()
{
  delete mFormat;
}

bool ResourceIMAP::open()
{
  // Ensure that there is a kmail running
  return ( kapp->startServiceByDesktopName( "kmail" ) == 0 );
}

void ResourceIMAP::close()
{
  // Nothing to close
}

Ticket * ResourceIMAP::requestSaveTicket()
{
  DCOPClient* dcopClient = kapp->dcopClient();
  QByteArray returnData;
  QCString returnType;
  if ( !dcopClient->call( "kmail", "KMailIface", "lockContactsFolder()",
                          QByteArray(), returnType, returnData, true ) ) {
    return false;
  }

  Q_ASSERT( returnType == "bool" );
  QDataStream argIn( returnData, IO_ReadOnly );
  bool ok;
  argIn >> ok;

  if ( !ok )
    return 0;
  else
    return createTicket( this );
}

bool ResourceIMAP::load()
{
  kdDebug(5700) << "ResourceIMAP::load()" << endl;

  KTempFile tempFile( QString::null, ".vcf" );
  // For loading, send a DCOP call off to KMail
  DCOPClient* dcopClient = kapp->dcopClient();
  QByteArray outgoingData;
  QDataStream outgoingStream( outgoingData, IO_WriteOnly );
  outgoingStream << tempFile.name();
  QByteArray returnData;
  QCString returnType;

  // Important; we need the synchronous call, even though we don't
  // expect a return value.
  if ( !dcopClient->call( "kmail", "KMailIface",
                          "requestAddresses(QString)", outgoingData,
                           returnType, returnData, true ) ) {
    kdDebug(5700) << "DCOP call failed" << endl;
    return false;
  }

  // Now parse the vCards in that file
  QFile file( tempFile.name() );
  if ( !file.open( IO_ReadOnly ) ) {
    kdDebug(5700) << "Could not open temp file " << tempFile.name() << endl;
    return false;
  }

  kdDebug(5700) << "Opened temp file " << tempFile.name() << endl;

  mFormat->loadAll( addressBook(), this, &file );

  tempFile.unlink();

  Resource::load();

  return true;
}


bool ResourceIMAP::save( Ticket *ticket )
{
  // FormatPlugin only supports loading from a file, not from
  // memory, so we have to write to a temp file first. This is all
  // very uncool, but that's the price for reusing the vCard
  // parser. In the future, the FormatPlugin interface needs
  // changing big time.
  KTempFile tempFile( QString::null, ".vcf" );
  mFormat->saveAll( addressBook(), this, tempFile.file() );
  tempFile.close();

  DCOPClient* dcopClient = kapp->dcopClient();
  QCString returnType;
  QByteArray returnData;
  QByteArray paramData;
  QDataStream paramStream( paramData, IO_WriteOnly );
  paramStream << tempFile.name();
  paramStream << removedUIDs();
  if ( !dcopClient->call( "kmail", "KMailIface",
                          "storeAddresses(QString,QStringList)", paramData,
                          returnType, returnData, true ) )
    return false; // No need to continue in this case.

  Q_ASSERT( returnType == "bool" );
  QDataStream argIn( returnData, IO_ReadOnly );
  bool ok;
  argIn >> ok;
  tempFile.unlink();

  // Always try to unlock
  if ( !dcopClient->call( "kmail", "KMailIface",
                          "unlockContactsFolder()", QByteArray(),
                          returnType, returnData, true ) ) {
    return false;
  }

  Q_ASSERT( returnType == "bool" );
  QDataStream argIn2( returnData, IO_ReadOnly );
  bool ok2;
  argIn2 >> ok2;

  Resource::save( ticket );

  return ( ok2 && ok );
}

QString ResourceIMAP::identifier() const
{
  return "KMAIL-IMAP";
}

void ResourceIMAP::cleanUp()
{
  // We don't have anything to clean up here.
}
