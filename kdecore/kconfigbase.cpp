// -*- c-basic-offset: 2 -*-
/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>

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

#include <stdlib.h>
#include <string.h>

#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcharsets.h>

#include "kconfigbase.h"
#include "kconfigbackend.h"
#include "kdebug.h"
#include "kstandarddirs.h"
#include "kstringhandler.h"

class KConfigBase::KConfigBasePrivate
{
public:
     KConfigBasePrivate() : readDefaults(false) { };

public:
     bool readDefaults;
};

KConfigBase::KConfigBase()
  : backEnd(0L), bDirty(false), bLocaleInitialized(false),
    bReadOnly(false), bExpand(false), d(0)
{
    setGroup(QString::null);
}

KConfigBase::~KConfigBase()
{
    delete d;
}

void KConfigBase::setLocale()
{
  bLocaleInitialized = true;

  if (KGlobal::locale())
    aLocaleString = KGlobal::locale()->language().utf8();
  else
    aLocaleString = KLocale::defaultLanguage().utf8();
  if (backEnd)
     backEnd->setLocaleString(aLocaleString);
}

QString KConfigBase::locale() const
{
  return QString::fromUtf8(aLocaleString);
}

void KConfigBase::setGroup( const QString& group )
{
  if ( group.isEmpty() )
    mGroup = "<default>";
  else
    mGroup = group.utf8();
}

void KConfigBase::setGroup( const char *pGroup )
{
  setGroup(QCString(pGroup));
}

void KConfigBase::setGroup( const QCString &group )
{
  if ( group.isEmpty() )
    mGroup = "<default>";
  else
    mGroup = group;
}

QString KConfigBase::group() const {
  return QString::fromUtf8(mGroup);
}

void KConfigBase::setDesktopGroup()
{
  mGroup = "Desktop Entry";
}

bool KConfigBase::hasKey(const QString &key) const
{
   return hasKey(key.utf8().data());
}

bool KConfigBase::hasKey(const char *pKey) const
{
  KEntryKey aEntryKey(mGroup, 0);
  aEntryKey.c_key = pKey;
  aEntryKey.bDefault = readDefaults();

  if (!locale().isNull()) {
    // try the localized key first
    aEntryKey.bLocal = true;
    KEntry entry = lookupData(aEntryKey);
    if (!entry.mValue.isNull())
       return true;
    aEntryKey.bLocal = false;
  }

  // try the non-localized version
  KEntry entry = lookupData(aEntryKey);
  return !entry.mValue.isNull();
}

bool KConfigBase::hasGroup(const QString &group) const
{
  return internalHasGroup( group.utf8());
}

bool KConfigBase::hasGroup(const char *_pGroup) const
{
  return internalHasGroup( QCString(_pGroup));
}

bool KConfigBase::hasGroup(const QCString &_pGroup) const
{
  return internalHasGroup( _pGroup);
}

bool KConfigBase::isImmutable() const
{
  return (getConfigState() != ReadWrite);
}

bool KConfigBase::groupIsImmutable(const QString &group) const
{
  if (getConfigState() != ReadWrite)
     return true;

  KEntryKey groupKey(group.utf8(), 0);
  KEntry entry = lookupData(groupKey);
  return entry.bImmutable;
}

bool KConfigBase::entryIsImmutable(const QString &key) const
{
  if (getConfigState() != ReadWrite)
     return true;

  KEntryKey entryKey(mGroup, 0);
  KEntry aEntryData = lookupData(entryKey); // Group
  if (aEntryData.bImmutable)
    return true;

  QCString utf8_key = key.utf8();
  entryKey.c_key = utf8_key.data();
  aEntryData = lookupData(entryKey); // Normal entry
  if (aEntryData.bImmutable)
    return true;

  entryKey.bLocal = true;
  aEntryData = lookupData(entryKey); // Localized entry
  return aEntryData.bImmutable;
}


QString KConfigBase::readEntryUntranslated( const QString& pKey,
                                const QString& aDefault ) const
{
   return KConfigBase::readEntryUntranslated(pKey.utf8().data(), aDefault);
}


QString KConfigBase::readEntryUntranslated( const char *pKey,
                                const QString& aDefault ) const
{
   QCString result = readEntryUtf8(pKey);
   if (result.isNull())
      return aDefault;
   return QString::fromUtf8(result);
}


QString KConfigBase::readEntry( const QString& pKey,
                                const QString& aDefault ) const
{
   return KConfigBase::readEntry(pKey.utf8().data(), aDefault);
}

QString KConfigBase::readEntry( const char *pKey,
                                const QString& aDefault ) const
{
  // we need to access _locale instead of the method locale()
  // because calling locale() will create a locale object if it
  // doesn't exist, which requires KConfig, which will create a infinite
  // loop, and nobody likes those.
  if (!bLocaleInitialized && KGlobal::_locale) {
    // get around const'ness.
    KConfigBase *that = const_cast<KConfigBase *>(this);
    that->setLocale();
  }

  QString aValue;

  bool expand = false;
  // construct a localized version of the key
  // try the localized key first
  KEntry aEntryData;
  KEntryKey entryKey(mGroup, 0);
  entryKey.c_key = pKey;
  entryKey.bDefault = readDefaults();
  entryKey.bLocal = true;
  aEntryData = lookupData(entryKey);
  if (!aEntryData.mValue.isNull()) {
    // for GNOME .desktop
    aValue = KStringHandler::from8Bit( aEntryData.mValue.data() );
    expand = aEntryData.bExpand;
  } else {
    entryKey.bLocal = false;
    aEntryData = lookupData(entryKey);
    if (!aEntryData.mValue.isNull()) {
      aValue = QString::fromUtf8(aEntryData.mValue.data());
      if (aValue.isNull())
      {
        static const QString &emptyString = KGlobal::staticQString("");
        aValue = emptyString;
      }
      expand = aEntryData.bExpand;
    } else {
      aValue = aDefault;
    }
  }

  // only do dollar expansion if so desired
  if( expand || bExpand )
    {
      // check for environment variables and make necessary translations
      int nDollarPos = aValue.find( '$' );

      while( nDollarPos != -1 && nDollarPos+1 < static_cast<int>(aValue.length())) {
        // there is at least one $
        if( (aValue)[nDollarPos+1] == '(' ) {
          uint nEndPos = nDollarPos+1;
          // the next character is no $
          while ( (nEndPos <= aValue.length()) && (aValue[nEndPos]!=')') )
              nEndPos++;
          nEndPos++;
          QString cmd = aValue.mid( nDollarPos+2, nEndPos-nDollarPos-3 );

          QString result;
          FILE *fs = popen(QFile::encodeName(cmd).data(), "r");
          if (fs)
          {
             QTextStream ts(fs, IO_ReadOnly);
             result = ts.read().stripWhiteSpace();
             pclose(fs);
          }
          aValue.replace( nDollarPos, nEndPos-nDollarPos, result );
        } else if( (aValue)[nDollarPos+1] != '$' ) {
          uint nEndPos = nDollarPos+1;
          // the next character is no $
          QString aVarName;
          if (aValue[nEndPos]=='{')
          {
            while ( (nEndPos <= aValue.length()) && (aValue[nEndPos]!='}') )
                nEndPos++;
            nEndPos++;
            aVarName = aValue.mid( nDollarPos+2, nEndPos-nDollarPos-3 );
          }
          else
          {
            while ( nEndPos <= aValue.length() && (aValue[nEndPos].isNumber()
                    || aValue[nEndPos].isLetter() || aValue[nEndPos]=='_' )  )
                nEndPos++;
            aVarName = aValue.mid( nDollarPos+1, nEndPos-nDollarPos-1 );
          }
          const char* pEnv = 0;
          if (!aVarName.isEmpty())
               pEnv = getenv( aVarName.ascii() );
          if( pEnv ) {
	    // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
	    // A environment variables may contain values in 8bit
	    // locale cpecified encoding or in UTF8 encoding.
	    aValue.replace( nDollarPos, nEndPos-nDollarPos, KStringHandler::from8Bit( pEnv ) );
          } else
            aValue.remove( nDollarPos, nEndPos-nDollarPos );
        } else {
          // remove one of the dollar signs
          aValue.remove( nDollarPos, 1 );
          nDollarPos++;
        }
        nDollarPos = aValue.find( '$', nDollarPos );
      }
    }

  return aValue;
}

QCString KConfigBase::readEntryUtf8( const char *pKey) const
{
  // We don't try the localized key
  KEntryKey entryKey(mGroup, 0);
  entryKey.bDefault = readDefaults();
  entryKey.c_key = pKey;
  KEntry aEntryData = lookupData(entryKey);
  if (aEntryData.bExpand)
  {
     // We need to do fancy, take the slow route.
     return readEntry(pKey, QString::null).utf8();
  }
  return aEntryData.mValue;
}

QVariant KConfigBase::readPropertyEntry( const QString& pKey,
                                          QVariant::Type type ) const
{
  return readPropertyEntry(pKey.utf8().data(), type);
}

QVariant KConfigBase::readPropertyEntry( const char *pKey,
                                          QVariant::Type type ) const
{
  QVariant va;
  if ( !hasKey( pKey ) ) return va;
  (void)va.cast(type);
  return readPropertyEntry(pKey, va);
}

QVariant KConfigBase::readPropertyEntry( const QString& pKey,
                                         const QVariant &aDefault ) const
{
  return readPropertyEntry(pKey.utf8().data(), aDefault);
}

QVariant KConfigBase::readPropertyEntry( const char *pKey,
                                         const QVariant &aDefault ) const
{
  if ( !hasKey( pKey ) ) return aDefault;

  QVariant tmp = aDefault;

  switch( aDefault.type() )
  {
      case QVariant::Invalid:
          return QVariant();
      case QVariant::String:
          return QVariant( readEntry( pKey, aDefault.toString() ) );
      case QVariant::StringList:
          return QVariant( readListEntry( pKey ) );
      case QVariant::List: {
          QStringList strList = readListEntry( pKey );
          QStringList::ConstIterator it = strList.begin();
          QStringList::ConstIterator end = strList.end();
          QValueList<QVariant> list;

          for (; it != end; ++it ) {
              tmp = *it;
              list.append( tmp );
          }
          return QVariant( list );
      }
      case QVariant::Font:
          return QVariant( readFontEntry( pKey, &tmp.asFont() ) );
      case QVariant::Point:
          return QVariant( readPointEntry( pKey, &tmp.asPoint() ) );
      case QVariant::Rect:
          return QVariant( readRectEntry( pKey, &tmp.asRect() ) );
      case QVariant::Size:
          return QVariant( readSizeEntry( pKey, &tmp.asSize() ) );
      case QVariant::Color:
          return QVariant( readColorEntry( pKey, &tmp.asColor() ) );
      case QVariant::Int:
          return QVariant( readNumEntry( pKey, aDefault.toInt() ) );
      case QVariant::UInt:
          return QVariant( readUnsignedNumEntry( pKey, aDefault.toUInt() ) );
      case QVariant::LongLong:
          return QVariant( readNum64Entry( pKey, aDefault.toLongLong() ) );
      case QVariant::ULongLong:
          return QVariant( readUnsignedNum64Entry( pKey, aDefault.toULongLong() ) );
      case QVariant::Bool:
          return QVariant( readBoolEntry( pKey, aDefault.toBool() ), 0 );
      case QVariant::Double:
          return QVariant( readDoubleNumEntry( pKey, aDefault.toDouble() ) );
      case QVariant::DateTime:
          return QVariant( readDateTimeEntry( pKey, &tmp.asDateTime() ) );
      case QVariant::Date:
          return QVariant(readDateTimeEntry( pKey, &tmp.asDateTime() ).date());

      case QVariant::Pixmap:
      case QVariant::Image:
      case QVariant::Brush:
      case QVariant::Palette:
      case QVariant::ColorGroup:
      case QVariant::Map:
      case QVariant::IconSet:
      case QVariant::CString:
      case QVariant::PointArray:
      case QVariant::Region:
      case QVariant::Bitmap:
      case QVariant::Cursor:
      case QVariant::SizePolicy:
      case QVariant::Time:
      case QVariant::ByteArray:
      case QVariant::BitArray:
      case QVariant::KeySequence:
      case QVariant::Pen:
          break;
  }

  Q_ASSERT( 0 );
  return QVariant();
}

int KConfigBase::readListEntry( const QString& pKey,
                                QStrList &list, char sep ) const
{
  return readListEntry(pKey.utf8().data(), list, sep);
}

int KConfigBase::readListEntry( const char *pKey,
                                QStrList &list, char sep ) const
{
  if( !hasKey( pKey ) )
    return 0;

  QCString str_list = readEntryUtf8( pKey );
  if (str_list.isEmpty())
    return 0;

  list.clear();
  QCString value = "";
  int len = str_list.length();

  for (int i = 0; i < len; i++) {
    if (str_list[i] != sep && str_list[i] != '\\') {
      value += str_list[i];
      continue;
    }
    if (str_list[i] == '\\') {
      i++;
      if ( i < len )
        value += str_list[i];
      continue;
    }
    // if we fell through to here, we are at a separator.  Append
    // contents of value to the list
    // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
    // A QStrList may contain values in 8bit locale cpecified
    // encoding
    list.append( value );
    value.truncate(0);
  }

  if ( str_list[len-1] != sep || ( len > 1 && str_list[len-2] == '\\' ) )
    list.append( value );
  return list.count();
}

QStringList KConfigBase::readListEntry( const QString& pKey, char sep ) const
{
  return readListEntry(pKey.utf8().data(), sep);
}

QStringList KConfigBase::readListEntry( const char *pKey, char sep ) const
{
  static const QString& emptyString = KGlobal::staticQString("");

  QStringList list;
  if( !hasKey( pKey ) )
    return list;
  QString str_list = readEntry( pKey );
  if( str_list.isEmpty() )
    return list;
  QString value(emptyString);
  int len = str_list.length();
 // obviously too big, but faster than letting each += resize the string.
  value.reserve( len );
  for( int i = 0; i < len; i++ )
    {
      if( str_list[i] != sep && str_list[i] != '\\' )
        {
          value += str_list[i];
          continue;
        }
      if( str_list[i] == '\\' )
        {
          i++;
          if ( i < len )
            value += str_list[i];
          continue;
        }
      QString finalvalue( value );
      finalvalue.squeeze();
      list.append( finalvalue );
      value.truncate( 0 );
    }
  if ( str_list[len-1] != sep || ( len > 1 && str_list[len-2] == '\\' ) )
  {
    value.squeeze();
    list.append( value );
  }
  return list;
}

QStringList KConfigBase::readListEntry( const char* pKey, const QStringList& aDefault,
		char sep ) const
{
	if ( !hasKey( pKey ) )
		return aDefault;
	else
		return readListEntry( pKey, sep );
}

QValueList<int> KConfigBase::readIntListEntry( const QString& pKey ) const
{
  return readIntListEntry(pKey.utf8().data());
}

QValueList<int> KConfigBase::readIntListEntry( const char *pKey ) const
{
  QStringList strlist = readListEntry(pKey);
  QValueList<int> list;
  for (QStringList::ConstIterator it = strlist.begin(); it != strlist.end(); it++)
    // I do not check if the toInt failed because I consider the number of items
    // more important than their value
    list << (*it).toInt();

  return list;
}

QString KConfigBase::readPathEntry( const QString& pKey, const QString& pDefault ) const
{
  return readPathEntry(pKey.utf8().data(), pDefault);
}

QString KConfigBase::readPathEntry( const char *pKey, const QString& pDefault ) const
{
  const bool bExpandSave = bExpand;
  bExpand = true;
  QString aValue = readEntry( pKey, pDefault );
  bExpand = bExpandSave;
  return aValue;
}

QStringList KConfigBase::readPathListEntry( const QString& pKey, char sep ) const
{
  return readPathListEntry(pKey.utf8().data(), sep);
}

QStringList KConfigBase::readPathListEntry( const char *pKey, char sep ) const
{
  const bool bExpandSave = bExpand;
  bExpand = true;
  QStringList aValue = readListEntry( pKey, sep );
  bExpand = bExpandSave;
  return aValue;
}

int KConfigBase::readNumEntry( const QString& pKey, int nDefault) const
{
  return readNumEntry(pKey.utf8().data(), nDefault);
}

int KConfigBase::readNumEntry( const char *pKey, int nDefault) const
{
  QCString aValue = readEntryUtf8( pKey );
  if( aValue.isNull() )
    return nDefault;
  else if( aValue == "true" || aValue == "on" || aValue == "yes" )
    return 1;
  else
    {
      bool ok;
      int rc = aValue.toInt( &ok );
      return( ok ? rc : nDefault );
    }
}


unsigned int KConfigBase::readUnsignedNumEntry( const QString& pKey, unsigned int nDefault) const
{
  return readUnsignedNumEntry(pKey.utf8().data(), nDefault);
}

unsigned int KConfigBase::readUnsignedNumEntry( const char *pKey, unsigned int nDefault) const
{
  QCString aValue = readEntryUtf8( pKey );
  if( aValue.isNull() )
    return nDefault;
  else
    {
      bool ok;
      unsigned int rc = aValue.toUInt( &ok );
      return( ok ? rc : nDefault );
    }
}


long KConfigBase::readLongNumEntry( const QString& pKey, long nDefault) const
{
  return readLongNumEntry(pKey.utf8().data(), nDefault);
}

long KConfigBase::readLongNumEntry( const char *pKey, long nDefault) const
{
  QCString aValue = readEntryUtf8( pKey );
  if( aValue.isNull() )
    return nDefault;
  else
    {
      bool ok;
      long rc = aValue.toLong( &ok );
      return( ok ? rc : nDefault );
    }
}


unsigned long KConfigBase::readUnsignedLongNumEntry( const QString& pKey, unsigned long nDefault) const
{
  return readUnsignedLongNumEntry(pKey.utf8().data(), nDefault);
}

unsigned long KConfigBase::readUnsignedLongNumEntry( const char *pKey, unsigned long nDefault) const
{
  QCString aValue = readEntryUtf8( pKey );
  if( aValue.isNull() )
    return nDefault;
  else
    {
      bool ok;
      unsigned long rc = aValue.toULong( &ok );
      return( ok ? rc : nDefault );
    }
}

Q_INT64 KConfigBase::readNum64Entry( const QString& pKey, Q_INT64 nDefault) const
{
  return readNum64Entry(pKey.utf8().data(), nDefault);
}

Q_INT64 KConfigBase::readNum64Entry( const char *pKey, Q_INT64 nDefault) const
{
  // Note that QCString::toLongLong() is missing, we muse use a QString instead.
  QString aValue = readEntry( pKey );
  if( aValue.isNull() )
    return nDefault;
  else
    {
      bool ok;
      Q_INT64 rc = aValue.toLongLong( &ok );
      return( ok ? rc : nDefault );
    }
}


Q_UINT64 KConfigBase::readUnsignedNum64Entry( const QString& pKey, Q_UINT64 nDefault) const
{
  return readUnsignedNum64Entry(pKey.utf8().data(), nDefault);
}

Q_UINT64 KConfigBase::readUnsignedNum64Entry( const char *pKey, Q_UINT64 nDefault) const
{
  // Note that QCString::toULongLong() is missing, we muse use a QString instead.
  QString aValue = readEntry( pKey );
  if( aValue.isNull() )
    return nDefault;
  else
    {
      bool ok;
      Q_UINT64 rc = aValue.toULongLong( &ok );
      return( ok ? rc : nDefault );
    }
}

double KConfigBase::readDoubleNumEntry( const QString& pKey, double nDefault) const
{
  return readDoubleNumEntry(pKey.utf8().data(), nDefault);
}

double KConfigBase::readDoubleNumEntry( const char *pKey, double nDefault) const
{
  QCString aValue = readEntryUtf8( pKey );
  if( aValue.isNull() )
    return nDefault;
  else
    {
      bool ok;
      double rc = aValue.toDouble( &ok );
      return( ok ? rc : nDefault );
    }
}


bool KConfigBase::readBoolEntry( const QString& pKey, bool bDefault ) const
{
   return readBoolEntry(pKey.utf8().data(), bDefault);
}

bool KConfigBase::readBoolEntry( const char *pKey, bool bDefault ) const
{
  QCString aValue = readEntryUtf8( pKey );

  if( aValue.isNull() )
    return bDefault;
  else
    {
      if( aValue == "true" || aValue == "on" || aValue == "yes" || aValue == "1" )
        return true;
      else
        {
          bool bOK;
          int val = aValue.toInt( &bOK );
          if( bOK && val != 0 )
            return true;
          else
            return false;
        }
    }
}

QFont KConfigBase::readFontEntry( const QString& pKey, const QFont* pDefault ) const
{
  return readFontEntry(pKey.utf8().data(), pDefault);
}

QFont KConfigBase::readFontEntry( const char *pKey, const QFont* pDefault ) const
{
  QFont aRetFont;

  QString aValue = readEntry( pKey );
  if( !aValue.isNull() ) {
    if ( aValue.contains( ',' ) > 5 ) {
      // KDE3 and upwards entry
      if ( !aRetFont.fromString( aValue ) && pDefault )
        aRetFont = *pDefault;
    }
    else {
      // backward compatibility with older font formats
      // ### remove KDE 3.1 ?
      // find first part (font family)
      int nIndex = aValue.find( ',' );
      if( nIndex == -1 ){
        if( pDefault )
          aRetFont = *pDefault;
        return aRetFont;
      }
      aRetFont.setFamily( aValue.left( nIndex ) );

      // find second part (point size)
      int nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );
      if( nIndex == -1 ){
        if( pDefault )
          aRetFont = *pDefault;
        return aRetFont;
      }

      aRetFont.setPointSize( aValue.mid( nOldIndex+1,
                                         nIndex-nOldIndex-1 ).toInt() );

      // find third part (style hint)
      nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );

      if( nIndex == -1 ){
        if( pDefault )
          aRetFont = *pDefault;
        return aRetFont;
      }

      aRetFont.setStyleHint( (QFont::StyleHint)aValue.mid( nOldIndex+1, nIndex-nOldIndex-1 ).toUInt() );

      // find fourth part (char set)
      nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );

      if( nIndex == -1 ){
        if( pDefault )
          aRetFont = *pDefault;
        return aRetFont;
      }

      QString chStr=aValue.mid( nOldIndex+1,
                                nIndex-nOldIndex-1 );
      // find fifth part (weight)
      nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );

      if( nIndex == -1 ){
        if( pDefault )
          aRetFont = *pDefault;
        return aRetFont;
      }

      aRetFont.setWeight( aValue.mid( nOldIndex+1,
                                      nIndex-nOldIndex-1 ).toUInt() );

      // find sixth part (font bits)
      uint nFontBits = aValue.right( aValue.length()-nIndex-1 ).toUInt();

      aRetFont.setItalic( nFontBits & 0x01 );
      aRetFont.setUnderline( nFontBits & 0x02 );
      aRetFont.setStrikeOut( nFontBits & 0x04 );
      aRetFont.setFixedPitch( nFontBits & 0x08 );
      aRetFont.setRawMode( nFontBits & 0x20 );
    }
  }
  else
    {
      if( pDefault )
        aRetFont = *pDefault;
    }

  return aRetFont;
}


QRect KConfigBase::readRectEntry( const QString& pKey, const QRect* pDefault ) const
{
  return readRectEntry(pKey.utf8().data(), pDefault);
}

QRect KConfigBase::readRectEntry( const char *pKey, const QRect* pDefault ) const
{
  QCString aValue = readEntryUtf8(pKey);

  if (!aValue.isEmpty())
  {
    int left, top, width, height;

    if (sscanf(aValue.data(), "%d,%d,%d,%d", &left, &top, &width, &height) == 4)
    {
       return QRect(left, top, width, height);
    }
  }
  if (pDefault)
    return *pDefault;
  return QRect();
}


QPoint KConfigBase::readPointEntry( const QString& pKey,
                                    const QPoint* pDefault ) const
{
  return readPointEntry(pKey.utf8().data(), pDefault);
}

QPoint KConfigBase::readPointEntry( const char *pKey,
                                    const QPoint* pDefault ) const
{
  QCString aValue = readEntryUtf8(pKey);

  if (!aValue.isEmpty())
  {
    int x,y;

    if (sscanf(aValue.data(), "%d,%d", &x, &y) == 2)
    {
       return QPoint(x,y);
    }
  }
  if (pDefault)
    return *pDefault;
  return QPoint();
}

QSize KConfigBase::readSizeEntry( const QString& pKey,
                                  const QSize* pDefault ) const
{
  return readSizeEntry(pKey.utf8().data(), pDefault);
}

QSize KConfigBase::readSizeEntry( const char *pKey,
                                  const QSize* pDefault ) const
{
  QCString aValue = readEntryUtf8(pKey);

  if (!aValue.isEmpty())
  {
    int width,height;

    if (sscanf(aValue.data(), "%d,%d", &width, &height) == 2)
    {
       return QSize(width, height);
    }
  }
  if (pDefault)
    return *pDefault;
  return QSize();
}


QColor KConfigBase::readColorEntry( const QString& pKey,
                                    const QColor* pDefault ) const
{
  return readColorEntry(pKey.utf8().data(), pDefault);
}

QColor KConfigBase::readColorEntry( const char *pKey,
                                    const QColor* pDefault ) const
{
  QColor aRetColor;
  int nRed = 0, nGreen = 0, nBlue = 0;

  QString aValue = readEntry( pKey );
  if( !aValue.isEmpty() )
    {
      if ( aValue.at(0) == '#' )
        {
          aRetColor.setNamedColor(aValue);
        }
      else
        {

          bool bOK;

          // find first part (red)
          int nIndex = aValue.find( ',' );

          if( nIndex == -1 ){
            // return a sensible default -- Bernd
            if( pDefault )
              aRetColor = *pDefault;
            return aRetColor;
          }

          nRed = aValue.left( nIndex ).toInt( &bOK );

          // find second part (green)
          int nOldIndex = nIndex;
          nIndex = aValue.find( ',', nOldIndex+1 );

          if( nIndex == -1 ){
            // return a sensible default -- Bernd
            if( pDefault )
              aRetColor = *pDefault;
            return aRetColor;
          }
          nGreen = aValue.mid( nOldIndex+1,
                               nIndex-nOldIndex-1 ).toInt( &bOK );

          // find third part (blue)
          nBlue = aValue.right( aValue.length()-nIndex-1 ).toInt( &bOK );

          aRetColor.setRgb( nRed, nGreen, nBlue );
        }
    }
  else {

    if( pDefault )
      aRetColor = *pDefault;
  }

  return aRetColor;
}


QDateTime KConfigBase::readDateTimeEntry( const QString& pKey,
                                          const QDateTime* pDefault ) const
{
  return readDateTimeEntry(pKey.utf8().data(), pDefault);
}

// ### currentDateTime() as fallback ? (Harri)
QDateTime KConfigBase::readDateTimeEntry( const char *pKey,
                                          const QDateTime* pDefault ) const
{
  if( !hasKey( pKey ) )
    {
      if( pDefault )
        return *pDefault;
      else
        return QDateTime::currentDateTime();
    }

  QStrList list;
  int count = readListEntry( pKey, list, ',' );
  if( count == 6 ) {
    QDate date( atoi( list.at( 0 ) ), atoi( list.at( 1 ) ),
                atoi( list.at( 2 ) ) );
    QTime time( atoi( list.at( 3 ) ), atoi( list.at( 4 ) ),
                atoi( list.at( 5 ) ) );

    return QDateTime( date, time );
  }

  return QDateTime::currentDateTime();
}

void KConfigBase::writeEntry( const QString& pKey, const QString& value,
                                 bool bPersistent,
                                 bool bGlobal,
                                 bool bNLS )
{
   writeEntry(pKey.utf8().data(), value, bPersistent,  bGlobal, bNLS);
}

void KConfigBase::writeEntry( const char *pKey, const QString& value,
                                 bool bPersistent,
                                 bool bGlobal,
                                 bool bNLS )
{
  // the KConfig object is dirty now
  // set this before any IO takes place so that if any derivative
  // classes do caching, they won't try and flush the cache out
  // from under us before we read. A race condition is still
  // possible but minimized.
  if( bPersistent )
    setDirty(true);

  if (!bLocaleInitialized && KGlobal::locale())
    setLocale();

  KEntryKey entryKey(mGroup, pKey);
  entryKey.bLocal = bNLS;

  KEntry aEntryData;
  aEntryData.mValue = value.utf8();  // set new value
  aEntryData.bGlobal = bGlobal;
  aEntryData.bNLS = bNLS;

  if (bPersistent)
    aEntryData.bDirty = true;

  // rewrite the new value
  putData(entryKey, aEntryData, true);
}

void KConfigBase::writePathEntry( const QString& pKey, const QString & path,
                                  bool bPersistent, bool bGlobal,
                                  bool bNLS)
{
   writePathEntry(pKey.utf8().data(), path, bPersistent, bGlobal, bNLS);
}


static bool cleanHomeDirPath( QString &path, const QString &homeDir )
{
   if (!path.startsWith(homeDir))
        return false;

   unsigned int len = homeDir.length();
   // replace by "$HOME" if possible
   if (path.length() == len || path[len] == '/') {
        path.replace(0, len, QString::fromLatin1("$HOME"));
        return true;
   } else
        return false;
}

static QString translatePath( QString path )
{
   if (path.isEmpty())
       return path;

   // only "our" $HOME should be interpreted
   path.replace('$', "$$");

   bool startsWithFile = path.startsWith("file:", false);

   // return original path, if it refers to another type of URL (e.g. http:/), or
   // if the path is already relative to another directory
   if (!startsWithFile && path[0] != '/' ||
        startsWithFile && path[5] != '/')
	return path;

   if (startsWithFile)
        path.remove(0,5); // strip leading "file:/" off the string

   // we can not use KGlobal::dirs()->relativeLocation("home", path) here,
   // since it would not recognize paths without a trailing '/'.
   // All of the 3 following functions to return the user's home directory
   // can return different paths. We have to test all them.
   QString homeDir0 = QFile::decodeName(getenv("HOME"));
   QString homeDir1 = QDir::homeDirPath();
   QString homeDir2 = QDir(homeDir1).canonicalPath();
   if (cleanHomeDirPath(path, homeDir0) ||
       cleanHomeDirPath(path, homeDir1) ||
       cleanHomeDirPath(path, homeDir2) ) {
     // kdDebug() << "Path was replaced\n";
   }

   if (startsWithFile)
      path.prepend( "file:" );

   return path;
}

void KConfigBase::writePathEntry( const char *pKey, const QString & path,
                                  bool bPersistent, bool bGlobal,
                                  bool bNLS)
{
   writeEntry(pKey, translatePath(path), bPersistent, bGlobal, bNLS);
}

void KConfigBase::writePathEntry ( const QString& pKey, const QStringList &list,
                               char sep , bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  writePathEntry(pKey.utf8().data(), list, sep, bPersistent, bGlobal, bNLS);
}

void KConfigBase::writePathEntry ( const char *pKey, const QStringList &list,
                               char sep , bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  if( list.isEmpty() )
    {
      writeEntry( pKey, QString::fromLatin1(""), bPersistent );
      return;
    }
  QStringList new_list;
  QStringList::ConstIterator it = list.begin();
  for( ; it != list.end(); ++it )
    {
      QString value = *it;
      new_list.append( translatePath(value) );
    }
  writeEntry( pKey, new_list, sep, bPersistent, bGlobal, bNLS );
}

void KConfigBase::deleteEntry( const QString& pKey,
                                 bool bNLS,
                                 bool bGlobal)
{
   deleteEntry(pKey.utf8().data(), bNLS, bGlobal);
}

void KConfigBase::deleteEntry( const char *pKey,
                                 bool bNLS,
                                 bool bGlobal)
{
  // the KConfig object is dirty now
  // set this before any IO takes place so that if any derivative
  // classes do caching, they won't try and flush the cache out
  // from under us before we read. A race condition is still
  // possible but minimized.
  setDirty(true);

  if (!bLocaleInitialized && KGlobal::locale())
    setLocale();

  KEntryKey entryKey(mGroup, pKey);
  KEntry aEntryData;

  aEntryData.bGlobal = bGlobal;
  aEntryData.bNLS = bNLS;
  aEntryData.bDirty = true;
  aEntryData.bDeleted = true;

  // rewrite the new value
  putData(entryKey, aEntryData, true);
}

bool KConfigBase::deleteGroup( const QString& group, bool bDeep, bool bGlobal )
{
  KEntryMap aEntryMap = internalEntryMap(group);

  if (!bDeep) {
    // Check if it empty
    return aEntryMap.isEmpty();
  }

  bool dirty = false;
  bool checkGroup = true;
  // we want to remove all entries in the group
  KEntryMapIterator aIt;
  for (aIt = aEntryMap.begin(); aIt != aEntryMap.end(); ++aIt)
  {
    if (!aIt.key().mKey.isEmpty() && !aIt.key().bDefault && !(*aIt).bDeleted)
    {
      (*aIt).bDeleted = true;
      (*aIt).bDirty = true;
      (*aIt).bGlobal = bGlobal;
      (*aIt).mValue = 0;
      putData(aIt.key(), *aIt, checkGroup);
      checkGroup = false;
      dirty = true;
    }
  }
  if (dirty)
     setDirty(true);
  return true;
}

void KConfigBase::writeEntry ( const QString& pKey, const QVariant &prop,
                               bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  writeEntry(pKey.utf8().data(), prop, bPersistent, bGlobal, bNLS);
}

void KConfigBase::writeEntry ( const char *pKey, const QVariant &prop,
                               bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  switch( prop.type() )
    {
    case QVariant::Invalid:
      writeEntry( pKey, "", bPersistent, bGlobal, bNLS );
      return;
    case QVariant::String:
      writeEntry( pKey, prop.toString(), bPersistent, bGlobal, bNLS );
      return;
    case QVariant::StringList:
      writeEntry( pKey, prop.toStringList(), ',', bPersistent, bGlobal, bNLS );
      return;
    case QVariant::List: {
        QValueList<QVariant> list = prop.toList();
        QValueList<QVariant>::ConstIterator it = list.begin();
        QValueList<QVariant>::ConstIterator end = list.end();
        QStringList strList;

        for (; it != end; ++it )
            strList.append( (*it).toString() );

        writeEntry( pKey, strList, ',', bPersistent, bGlobal, bNLS );

        return;
    }
    case QVariant::Font:
      writeEntry( pKey, prop.toFont(), bPersistent, bGlobal, bNLS );
      return;
    case QVariant::Point:
      writeEntry( pKey, prop.toPoint(), bPersistent, bGlobal, bNLS );
      return;
    case QVariant::Rect:
      writeEntry( pKey, prop.toRect(), bPersistent, bGlobal, bNLS );
      return;
    case QVariant::Size:
      writeEntry( pKey, prop.toSize(), bPersistent, bGlobal, bNLS );
      return;
    case QVariant::Color:
      writeEntry( pKey, prop.toColor(), bPersistent, bGlobal, bNLS );
      return;
    case QVariant::Int:
      writeEntry( pKey, prop.toInt(), bPersistent, bGlobal, bNLS );
      return;
    case QVariant::UInt:
      writeEntry( pKey, prop.toUInt(), bPersistent, bGlobal, bNLS );
      return;
    case QVariant::LongLong:
      writeEntry( pKey, prop.toLongLong(), bPersistent, bGlobal, bNLS );
      return;
    case QVariant::ULongLong:
      writeEntry( pKey, prop.toULongLong(), bPersistent, bGlobal, bNLS );
      return;
    case QVariant::Bool:
      writeEntry( pKey, prop.toBool(), bPersistent, bGlobal, bNLS );
      return;
    case QVariant::Double:
      writeEntry( pKey, prop.toDouble(), bPersistent, bGlobal, 'g', 6, bNLS );
      return;
    case QVariant::DateTime:
      writeEntry( pKey, prop.toDateTime(), bPersistent, bGlobal, bNLS);
      return;
    case QVariant::Date:
      writeEntry( pKey, QDateTime(prop.toDate()), bPersistent, bGlobal, bNLS);
      return;

    case QVariant::Pixmap:
    case QVariant::Image:
    case QVariant::Brush:
    case QVariant::Palette:
    case QVariant::ColorGroup:
    case QVariant::Map:
    case QVariant::IconSet:
    case QVariant::CString:
    case QVariant::PointArray:
    case QVariant::Region:
    case QVariant::Bitmap:
    case QVariant::Cursor:
    case QVariant::SizePolicy:
    case QVariant::Time:
    case QVariant::ByteArray:
    case QVariant::BitArray:
    case QVariant::KeySequence:
    case QVariant::Pen:
        break;
    }

  Q_ASSERT( 0 );
}

void KConfigBase::writeEntry ( const QString& pKey, const QStrList &list,
                               char sep , bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  writeEntry(pKey.utf8().data(), list, sep, bPersistent, bGlobal, bNLS);
}

void KConfigBase::writeEntry ( const char *pKey, const QStrList &list,
                               char sep , bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  if( list.isEmpty() )
    {
      writeEntry( pKey, QString::fromLatin1(""), bPersistent );
      return;
    }
  QString str_list;
  QStrListIterator it( list );
  for( ; it.current(); ++it )
    {
      uint i;
      QString value;
      // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
      // A QStrList may contain values in 8bit locale cpecified
      // encoding or in UTF8 encoding.
      value = KStringHandler::from8Bit(it.current());
      for( i = 0; i < value.length(); i++ )
        {
          if( value[i] == sep || value[i] == '\\' )
            str_list += '\\';
          str_list += value[i];
        }
      str_list += sep;
    }
  if( str_list.at(str_list.length() - 1) == sep )
    str_list.truncate( str_list.length() -1 );
  writeEntry( pKey, str_list, bPersistent, bGlobal, bNLS );
}

void KConfigBase::writeEntry ( const QString& pKey, const QStringList &list,
                               char sep , bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  writeEntry(pKey.utf8().data(), list, sep, bPersistent, bGlobal, bNLS);
}

void KConfigBase::writeEntry ( const char *pKey, const QStringList &list,
                               char sep , bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  if( list.isEmpty() )
    {
      writeEntry( pKey, QString::fromLatin1(""), bPersistent );
      return;
    }
  QString str_list;
  str_list.reserve( 4096 );
  QStringList::ConstIterator it = list.begin();
  for( ; it != list.end(); ++it )
    {
      QString value = *it;
      uint i;
      for( i = 0; i < value.length(); i++ )
        {
          if( value[i] == sep || value[i] == '\\' )
            str_list += '\\';
          str_list += value[i];
        }
      str_list += sep;
    }
  if( str_list.at(str_list.length() - 1) == sep )
    str_list.truncate( str_list.length() -1 );
  writeEntry( pKey, str_list, bPersistent, bGlobal, bNLS );
}

void KConfigBase::writeEntry ( const QString& pKey, const QValueList<int> &list,
                               bool bPersistent, bool bGlobal, bool bNLS )
{
  writeEntry(pKey.utf8().data(), list, bPersistent, bGlobal, bNLS);
}

void KConfigBase::writeEntry ( const char *pKey, const QValueList<int> &list,
                               bool bPersistent, bool bGlobal, bool bNLS )
{
    QStringList strlist;
    QValueList<int>::ConstIterator end = list.end();
    for (QValueList<int>::ConstIterator it = list.begin(); it != end; it++)
        strlist << QString::number(*it);
    writeEntry(pKey, strlist, ',', bPersistent, bGlobal, bNLS );
}

void KConfigBase::writeEntry( const QString& pKey, int nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, QString::number(nValue), bPersistent, bGlobal, bNLS );
}

void KConfigBase::writeEntry( const char *pKey, int nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, QString::number(nValue), bPersistent, bGlobal, bNLS );
}


void KConfigBase::writeEntry( const QString& pKey, unsigned int nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, QString::number(nValue), bPersistent, bGlobal, bNLS );
}

void KConfigBase::writeEntry( const char *pKey, unsigned int nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, QString::number(nValue), bPersistent, bGlobal, bNLS );
}


void KConfigBase::writeEntry( const QString& pKey, long nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, QString::number(nValue), bPersistent, bGlobal, bNLS );
}

void KConfigBase::writeEntry( const char *pKey, long nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, QString::number(nValue), bPersistent, bGlobal, bNLS );
}


void KConfigBase::writeEntry( const QString& pKey, unsigned long nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, QString::number(nValue), bPersistent, bGlobal, bNLS );
}

void KConfigBase::writeEntry( const char *pKey, unsigned long nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, QString::number(nValue), bPersistent, bGlobal, bNLS );
}

void KConfigBase::writeEntry( const QString& pKey, Q_INT64 nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, QString::number(nValue), bPersistent, bGlobal, bNLS );
}

void KConfigBase::writeEntry( const char *pKey, Q_INT64 nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, QString::number(nValue), bPersistent, bGlobal, bNLS );
}


void KConfigBase::writeEntry( const QString& pKey, Q_UINT64 nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, QString::number(nValue), bPersistent, bGlobal, bNLS );
}

void KConfigBase::writeEntry( const char *pKey, Q_UINT64 nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, QString::number(nValue), bPersistent, bGlobal, bNLS );
}

void KConfigBase::writeEntry( const QString& pKey, double nValue,
                                 bool bPersistent, bool bGlobal,
                                 char format, int precision,
                                 bool bNLS )
{
  writeEntry( pKey, QString::number(nValue, format, precision),
                     bPersistent, bGlobal, bNLS );
}

void KConfigBase::writeEntry( const char *pKey, double nValue,
                                 bool bPersistent, bool bGlobal,
                                 char format, int precision,
                                 bool bNLS )
{
  writeEntry( pKey, QString::number(nValue, format, precision),
                     bPersistent, bGlobal, bNLS );
}


void KConfigBase::writeEntry( const QString& pKey, bool bValue,
                                 bool bPersistent,
                                 bool bGlobal,
                                 bool bNLS )
{
  writeEntry(pKey.utf8().data(), bValue, bPersistent, bGlobal, bNLS);
}

void KConfigBase::writeEntry( const char *pKey, bool bValue,
                                 bool bPersistent,
                                 bool bGlobal,
                                 bool bNLS )
{
  QString aValue;

  if( bValue )
    aValue = "true";
  else
    aValue = "false";

  writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}


void KConfigBase::writeEntry( const QString& pKey, const QFont& rFont,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry(pKey.utf8().data(), rFont, bPersistent, bGlobal, bNLS);
}

void KConfigBase::writeEntry( const char *pKey, const QFont& rFont,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, rFont.toString(), bPersistent, bGlobal, bNLS );
}


void KConfigBase::writeEntry( const QString& pKey, const QRect& rRect,
                              bool bPersistent, bool bGlobal,
                              bool bNLS )
{
  writeEntry(pKey.utf8().data(), rRect, bPersistent, bGlobal, bNLS);
}

void KConfigBase::writeEntry( const char *pKey, const QRect& rRect,
                              bool bPersistent, bool bGlobal,
                              bool bNLS )
{
  QStrList list;
  QCString tempstr;
  list.insert( 0, tempstr.setNum( rRect.left() ) );
  list.insert( 1, tempstr.setNum( rRect.top() ) );
  list.insert( 2, tempstr.setNum( rRect.width() ) );
  list.insert( 3, tempstr.setNum( rRect.height() ) );

  writeEntry( pKey, list, ',', bPersistent, bGlobal, bNLS );
}


void KConfigBase::writeEntry( const QString& pKey, const QPoint& rPoint,
                              bool bPersistent, bool bGlobal,
                              bool bNLS )
{
  writeEntry(pKey.utf8().data(), rPoint, bPersistent, bGlobal, bNLS);
}

void KConfigBase::writeEntry( const char *pKey, const QPoint& rPoint,
                              bool bPersistent, bool bGlobal,
                              bool bNLS )
{
  QStrList list;
  QCString tempstr;
  list.insert( 0, tempstr.setNum( rPoint.x() ) );
  list.insert( 1, tempstr.setNum( rPoint.y() ) );

  writeEntry( pKey, list, ',', bPersistent, bGlobal, bNLS );
}


void KConfigBase::writeEntry( const QString& pKey, const QSize& rSize,
                              bool bPersistent, bool bGlobal,
                              bool bNLS )
{
  writeEntry(pKey.utf8().data(), rSize, bPersistent, bGlobal, bNLS);
}

void KConfigBase::writeEntry( const char *pKey, const QSize& rSize,
                              bool bPersistent, bool bGlobal,
                              bool bNLS )
{
  QStrList list;
  QCString tempstr;
  list.insert( 0, tempstr.setNum( rSize.width() ) );
  list.insert( 1, tempstr.setNum( rSize.height() ) );

  writeEntry( pKey, list, ',', bPersistent, bGlobal, bNLS );
}

void KConfigBase::writeEntry( const QString& pKey, const QColor& rColor,
                              bool bPersistent,
                              bool bGlobal,
                              bool bNLS  )
{
  writeEntry( pKey.utf8().data(), rColor, bPersistent, bGlobal, bNLS);
}

void KConfigBase::writeEntry( const char *pKey, const QColor& rColor,
                              bool bPersistent,
                              bool bGlobal,
                              bool bNLS  )
{
  QString aValue;
  if (rColor.isValid())
      aValue.sprintf( "%d,%d,%d", rColor.red(), rColor.green(), rColor.blue() );
  else
      aValue = "invalid";

  writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}

void KConfigBase::writeEntry( const QString& pKey, const QDateTime& rDateTime,
                              bool bPersistent, bool bGlobal,
                              bool bNLS )
{
  writeEntry(pKey.utf8().data(), rDateTime, bPersistent, bGlobal, bNLS);
}

void KConfigBase::writeEntry( const char *pKey, const QDateTime& rDateTime,
                              bool bPersistent, bool bGlobal,
                              bool bNLS )
{
  QStrList list;
  QCString tempstr;

  QTime time = rDateTime.time();
  QDate date = rDateTime.date();

  list.insert( 0, tempstr.setNum( date.year() ) );
  list.insert( 1, tempstr.setNum( date.month() ) );
  list.insert( 2, tempstr.setNum( date.day() ) );

  list.insert( 3, tempstr.setNum( time.hour() ) );
  list.insert( 4, tempstr.setNum( time.minute() ) );
  list.insert( 5, tempstr.setNum( time.second() ) );

  writeEntry( pKey, list, ',', bPersistent, bGlobal, bNLS );
}

void KConfigBase::parseConfigFiles()
{
  if (!bLocaleInitialized && KGlobal::_locale) {
    setLocale();
  }
  if (backEnd)
  {
     backEnd->parseConfigFiles();
     bReadOnly = (backEnd->getConfigState() == ReadOnly);
  }
}

void KConfigBase::sync()
{
  if (isReadOnly())
    return;

  if (backEnd)
     backEnd->sync();
  if (bDirty)
    rollback();
}

KConfigBase::ConfigState KConfigBase::getConfigState() const {
    if (backEnd)
       return backEnd->getConfigState();
    return ReadOnly;
}

void KConfigBase::rollback( bool /*bDeep = true*/ )
{
  bDirty = false;
}


void KConfigBase::setReadDefaults(bool b)
{
  if (!d)
  {
     if (!b) return;
     d = new KConfigBasePrivate();
  }

  d->readDefaults = b;
}

bool KConfigBase::readDefaults() const
{
  return (d && d->readDefaults);
}

void KConfigBase::revertToDefault(const QString &key)
{
  setDirty(true);

  KEntryKey aEntryKey(mGroup, key.utf8());
  aEntryKey.bDefault = true;

  if (!locale().isNull()) {
    // try the localized key first
    aEntryKey.bLocal = true;
    KEntry entry = lookupData(aEntryKey);
    if (entry.mValue.isNull())
        entry.bDeleted = true;

    entry.bDirty = true;
    putData(aEntryKey, entry, true); // Revert
    aEntryKey.bLocal = false;
  }

  // try the non-localized version
  KEntry entry = lookupData(aEntryKey);
  if (entry.mValue.isNull())
     entry.bDeleted = true;
  entry.bDirty = true;
  putData(aEntryKey, entry, true); // Revert
}

bool KConfigBase::hasDefault(const QString &key) const
{
  KEntryKey aEntryKey(mGroup, key.utf8());
  aEntryKey.bDefault = true;

  if (!locale().isNull()) {
    // try the localized key first
    aEntryKey.bLocal = true;
    KEntry entry = lookupData(aEntryKey);
    if (!entry.mValue.isNull())
        return true;

    aEntryKey.bLocal = false;
  }

  // try the non-localized version
  KEntry entry = lookupData(aEntryKey);
  if (!entry.mValue.isNull())
     return true;

  return false;
}



KConfigGroup::KConfigGroup(KConfigBase *master, const QString &group)
{
  mMaster = master;
  backEnd = mMaster->backEnd; // Needed for getConfigState()
  bLocaleInitialized = true;
  bReadOnly = mMaster->bReadOnly;
  bExpand = false;
  bDirty = false; // Not used
  mGroup = group.utf8();
  aLocaleString = mMaster->aLocaleString;
  setReadDefaults(mMaster->readDefaults());
}

KConfigGroup::KConfigGroup(KConfigBase *master, const QCString &group)
{
  mMaster = master;
  backEnd = mMaster->backEnd; // Needed for getConfigState()
  bLocaleInitialized = true;
  bReadOnly = mMaster->bReadOnly;
  bExpand = false;
  bDirty = false; // Not used
  mGroup = group;
  aLocaleString = mMaster->aLocaleString;
  setReadDefaults(mMaster->readDefaults());
}

KConfigGroup::KConfigGroup(KConfigBase *master, const char * group)
{
  mMaster = master;
  backEnd = mMaster->backEnd; // Needed for getConfigState()
  bLocaleInitialized = true;
  bReadOnly = mMaster->bReadOnly;
  bExpand = false;
  bDirty = false; // Not used
  mGroup = group;
  aLocaleString = mMaster->aLocaleString;
  setReadDefaults(mMaster->readDefaults());
}

void KConfigGroup::deleteGroup(bool bGlobal)
{
  mMaster->deleteGroup(KConfigBase::group(), true, bGlobal);
}

void KConfigGroup::setDirty(bool _bDirty)
{
  mMaster->setDirty(_bDirty);
}

void KConfigGroup::putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup)
{
  mMaster->putData(_key, _data, _checkGroup);
}

KEntry KConfigGroup::lookupData(const KEntryKey &_key) const
{
  return mMaster->lookupData(_key);
}

void KConfigGroup::sync()
{
  mMaster->sync();
}

void KConfigBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KConfigGroup::virtual_hook( int id, void* data )
{ KConfigBase::virtual_hook( id, data ); }

bool KConfigBase::checkConfigFilesWritable(bool warnUser)
{
  if (backEnd)
    return backEnd->checkConfigFilesWritable(warnUser);
  else
    return false;
}

#include "kconfigbase.moc"
