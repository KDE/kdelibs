/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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

#include "kservicetypefactory.h"
#include "ksycoca.h"
#include "ksycocatype.h"
#include "ksycocadict.h"
#include "kservicetype.h"
#include "kmimetype.h"

#include <klocale.h>
#include <kapp.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessageboxwrapper.h>
#include <kdebug.h>
#include <assert.h>

KServiceTypeFactory::KServiceTypeFactory()
 : KSycocaFactory( KST_KServiceTypeFactory )
{
   _self = this;
}

KSycocaEntry *
KServiceTypeFactory::createEntry(const QString &file)
{
  //debug("KServiceTypeFactory::createEntry(%s)",file.ascii());
  // Just a backup file ?
  if ( file.right(1) == "~" || file.right(4) == ".bak" || ( file[0] == '%' && file.right(1) == "%" ) )
      return 0;

  KSimpleConfig cfg( file, true);
  cfg.setDesktopGroup();

  QString mime = cfg.readEntry( "MimeType" );
  QString service = cfg.readEntry( "X-KDE-ServiceType" );

  if ( mime.isEmpty() && service.isEmpty() )
  {
    QString tmp = i18n( "The service/mime type config file\n%1\n"
			"does not contain a ServiceType=...\nor MimeType=... entry").arg( file );
    KMessageBoxWrapper::error( 0L, tmp);
    return 0;
  }
  
  KServiceType* e;
  if ( mime == "inode/directory" )
    e = new KFolderType( cfg );
  else if ( mime == "application/x-desktop" )
    e = new KDEDesktopMimeType( cfg );
  else if ( mime == "application/x-executable" || mime == "application/x-shellscript" )
    e = new KExecMimeType( cfg );
  else if ( !mime.isEmpty() )
    e = new KMimeType( cfg );
  else
    e = new KServiceType( cfg );

  if ( !(e->isValid()) )
  {
    kdebug( KDEBUG_WARN, 7012, "Invalid ServiceType : %s", file.ascii() );
    delete e;
    return 0;
  }

  return e;
}

KServiceTypeFactory::~KServiceTypeFactory()
{
}

KServiceTypeFactory * KServiceTypeFactory::self()
{
  if (!_self)
    _self = new KServiceTypeFactory();
  return _self;
}

KServiceType * KServiceTypeFactory::findServiceTypeByName(const QString &_name)
{
   if (!m_sycocaDict) return 0L; // Error!
   assert (!KSycoca::self()->isBuilding());
   int offset = m_sycocaDict->find_string( _name );
   if (!offset) return 0; // Not found
   KServiceType * newServiceType = createServiceType(offset);
   
   // Check whether the dictionary was right.
   if (newServiceType && (newServiceType->name() != _name))
   {
     // No it wasn't...
     delete newServiceType;
     newServiceType = 0; // Not found
   }
   return newServiceType;
}

// We can't use Qt's usctrcmp, declared 'static' in qstring.cpp :((
// Submitted to qt-bugs@troll.no
int ucstrcmp( const QString &as, const QString &bs )
{
    const QChar *a = as.unicode();
    const QChar *b = bs.unicode();
    if ( a == b )
        return 0;
    if ( a == 0 )
        return 1;
    if ( b == 0 )
        return -1;
    int l=QMIN(as.length(),bs.length());
    while ( l-- && *a == *b )
        a++,b++;
    if ( l==-1 )
        return ( as.length()-bs.length() );
    return a->unicode() - b->unicode();
}   

bool KServiceTypeFactory::matchFilename( const QString& _filename, const QString& _pattern  ) const
{
  //kdebug(KDEBUG_INFO, 7011, QString("matchFilename filename='%1' pattern='%2'")
  //                          .arg(_filename).arg(_pattern));
  int len = _filename.length();
  const char* s = _pattern.ascii();
  int pattern_len = _pattern.length();
  if (!pattern_len)
     return false;
  
  // Patterns like "Makefile*"
  if ( s[ pattern_len - 1 ] == '*' && len + 1 >= pattern_len )
     if ( strncasecmp( _filename.ascii(), s, pattern_len - 1 ) == 0 )
	return true;
  
  // Patterns like "*~", "*.extension"
  if ( s[ 0 ] == '*' && len + 1 >= pattern_len )
  {
     if ( strncasecmp( _filename.ascii() + len - pattern_len + 1, s + 1, pattern_len - 1 ) == 0 )
	return true;
     // TODO : Patterns like "*.*pk"
  }

  // Patterns like "Makefile"
  return (strcasecmp( _filename.ascii(), s ) == 0);
}

KMimeType * KServiceTypeFactory::findFromPattern(const QString &_filename)
{
   // Assume we're NOT building a database
   // Get stream to the header
   QDataStream *str = KSycoca::self()->findHeader();
   Q_INT32 offerListOffset;
   (*str) >> offerListOffset;
   Q_INT32 fastOffset;
   (*str) >> fastOffset;
   Q_INT32 otherOffset;
   (*str) >> otherOffset;
   Q_INT32 entrySize;
   (*str) >> entrySize;

   //kdebug(KDEBUG_INFO, 7011, QString("fastOffset : %1").arg(fastOffset,8,16));
   //kdebug(KDEBUG_INFO, 7011, QString("otherOffset : %1").arg(otherOffset,8,16));
   //kdebug(KDEBUG_INFO, 7011, QString("entrySize : %1").arg(entrySize));

   QString pattern;
   Q_INT32 mimetypeOffset;

   // Let's go for a binary search in the "fast" pattern index
   Q_INT32 left = 0;
   Q_INT32 right = (otherOffset-fastOffset) / entrySize - 1;
   Q_INT32 middle;
   // Extract extension
   int lastDot = _filename.findRev('.');
   if (lastDot != -1) // if no '.', skip the extension lookup
   {
      QString extension = _filename.right( _filename.length() - _filename.findRev('.') - 1 );
      extension = extension.leftJustify(4);
      //kdebug(KDEBUG_INFO, 7011, QString("extension is '%1'").arg(extension));
      
      while (left <= right) {
         middle = (left + right) / 2;
         //kdebug(KDEBUG_INFO, 7011, QString("the situation is left=%1 middle=%2 right=%3")
         //       .arg(left).arg(middle).arg(right));
         // read pattern at position "middle"
         str->device()->at( middle * entrySize + fastOffset );
         (*str) >> pattern;
         //kdebug(KDEBUG_INFO, 7011, QString("testing extension '%1'").arg(pattern));
         int cmp = ucstrcmp( pattern, extension );
         if (cmp < 0)
            left = middle + 1;
         else if (cmp == 0) // found
         {
            (*str) >> mimetypeOffset;
            KServiceType * newServiceType = createServiceType(mimetypeOffset);
            assert (newServiceType && newServiceType->isType( KST_KMimeType ));
            return (KMimeType *) newServiceType;
         }
         else
            right = middle - 1;
      }
   }
      
   // Not found or no extension, try the "other" offset
   str->device()->at( otherOffset );

   while (true)
   {
      (*str) >> pattern;
      if (pattern.isEmpty()) // end of list
         return 0L;
      (*str) >> mimetypeOffset;
      if ( matchFilename( _filename, pattern ) )
      {
         KServiceType * newServiceType = createServiceType(mimetypeOffset);
         assert (newServiceType && newServiceType->isType( KST_KMimeType ));
         return (KMimeType *) newServiceType;
      }
   }
}

KMimeType::List KServiceTypeFactory::allMimeTypes()
{
   kdebug(KDEBUG_INFO, 7011, "KServiceTypeFactory::allMimeTypes()");
   KMimeType::List list;
   // Assume we're NOT building a database
   // Get stream to factory start
   QDataStream *str = KSycoca::self()->findFactory( factoryId() );
   // Read the dict offset - will serve as an end point for the list of entries
   Q_INT32 sycocaDictOffset;
   (*str) >> sycocaDictOffset;

   int offset = str->device()->at();
   KServiceType *newServiceType;
   while ( offset < sycocaDictOffset )
   {
      newServiceType = createServiceType(offset);
      // We don't want service types, but we have to build them
      // anyway, to skip their info
      if (newServiceType && newServiceType->isType( KST_KMimeType ))
      {
         KMimeType * mimeType = (KMimeType *) newServiceType;
         list.append( KMimeType::Ptr( mimeType ) );
      }

      offset = str->device()->at();
   }
   return list;
}

KServiceType::List KServiceTypeFactory::allServiceTypes()
{
   kdebug(KDEBUG_INFO, 7011, "KServiceTypeFactory::allServiceTypes()");
   KServiceType::List list;
   // Assume we're NOT building a database
   // Get stream to factory start
   QDataStream *str = KSycoca::self()->findFactory( factoryId() );
   // Read the dict offset - will serve as an end point for the list of entries
   Q_INT32 sycocaDictOffset;
   (*str) >> sycocaDictOffset;

   int offset = str->device()->at();
   KServiceType *newServiceType;
   while ( offset < sycocaDictOffset )
   {
      newServiceType = createServiceType(offset);
      if (newServiceType)
         list.append( KServiceType::Ptr( newServiceType ) );

      offset = str->device()->at();
   }
   return list;
}

bool KServiceTypeFactory::checkMimeTypes()
{
   QDataStream *str = KSycoca::self()->findFactory( factoryId() );
   // Read the dict offset - will serve as an end point for the list of entries
   Q_INT32 sycocaDictOffset;
   (*str) >> sycocaDictOffset;
   // There are mimetypes/servicetypes if the dict offset is
   // not right now in the file
   return (str->device()->at() < sycocaDictOffset);
}

KServiceType * KServiceTypeFactory::createServiceType(int offset)
{
   KServiceType *newEntry = 0;
   KSycocaType type; 
   QDataStream *str = KSycoca::self()->findEntry(offset, type);
   switch(type)
   {
     case KST_KServiceType:
        newEntry = new KServiceType(*str, offset);
        break;
     case KST_KMimeType:
        newEntry = new KMimeType(*str, offset);
        break;
     case KST_KFolderType:
        newEntry = new KFolderType(*str, offset);
        break;
     case KST_KDEDesktopMimeType:
        newEntry = new KDEDesktopMimeType(*str, offset);
        break;
     case KST_KExecMimeType:
        newEntry = new KExecMimeType(*str, offset);
        break;

     default:
        kdebug( KDEBUG_ERROR, 7011, QString("KServiceTypeFactory: unexpected object entry in KSycoca database (type = %1)").arg((int)type) );
        break;
   } 
   if (!newEntry->isValid())
   {
      kdebug( KDEBUG_ERROR, 7011, "KServiceTypeFactory: corrupt object in KSycoca database!\n");
      delete newEntry;
      newEntry = 0;
   }   
   return newEntry;
}

KServiceTypeFactory *KServiceTypeFactory::_self = 0;
