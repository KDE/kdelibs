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
#include <qtextstream.h>

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcharsets.h>

#include "kconfigbase.h"
#include "kconfigbackend.h"
#include "kdebug.h"
#include "kstandarddirs.h"
#undef Bool

static bool isUtf8(const char *buf) {
  int i, n;
  register char c;
  bool gotone = false;

#define F 0   /* character never appears in text */
#define T 1   /* character appears in plain ASCII text */
#define I 2   /* character appears in ISO-8859 text */
#define X 3   /* character appears in non-ISO extended ASCII (Mac, IBM PC) */

  static const unsigned char text_chars[256] = {
  /*                  BEL BS HT LF    FF CR    */
        F, F, F, F, F, F, F, T, T, T, T, F, T, T, F, F,  /* 0x0X */
        /*                              ESC          */
        F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F,  /* 0x1X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x2X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x3X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x4X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x5X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x6X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, F,  /* 0x7X */
        /*            NEL                            */
        X, X, X, X, X, T, X, X, X, X, X, X, X, X, X, X,  /* 0x8X */
        X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,  /* 0x9X */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xaX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xbX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xcX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xdX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xeX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I   /* 0xfX */
  };

  /* *ulen = 0; */
  for (i = 0; (c = buf[i]); i++) {
    if ((c & 0x80) == 0) {        /* 0xxxxxxx is plain ASCII */
      /*
       * Even if the whole file is valid UTF-8 sequences,
       * still reject it if it uses weird control characters.
       */

      if (text_chars[c] != T)
        return false;

    } else if ((c & 0x40) == 0) { /* 10xxxxxx never 1st byte */
      return false;
    } else {                           /* 11xxxxxx begins UTF-8 */
      int following;

    if ((c & 0x20) == 0) {             /* 110xxxxx */
      following = 1;
    } else if ((c & 0x10) == 0) {      /* 1110xxxx */
      following = 2;
    } else if ((c & 0x08) == 0) {      /* 11110xxx */
      following = 3;
    } else if ((c & 0x04) == 0) {      /* 111110xx */
      following = 4;
    } else if ((c & 0x02) == 0) {      /* 1111110x */
      following = 5;
    } else
      return false;

      for (n = 0; n < following; n++) {
        i++;
        if (!(c = buf[i]))
          goto done;

        if ((c & 0x80) == 0 || (c & 0x40))
          return false;
      }
      gotone = true;
    }
  }
done:
  return gotone;   /* don't claim it's UTF-8 if it's all 7-bit */
}

#undef F
#undef T
#undef I
#undef X


KConfigBase::KConfigBase()
  : backEnd(0L), bDirty(false), bLocaleInitialized(false),
    bReadOnly(false), bExpand(false)
{
    setGroup(QString::null);
}

KConfigBase::~KConfigBase()
{
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
  if ( group.isNull() )
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

  entryKey.c_key = key.utf8().data();
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
  entryKey.bLocal = true;
  aEntryData = lookupData(entryKey);
  if (!aEntryData.mValue.isNull()) {

    // for GNOME .desktop
    // const char *data = aEntryData.mValue.char();
    if ( isUtf8(aEntryData.mValue.data() ) )
      aValue = QString::fromUtf8( aEntryData.mValue.data() );
    else
      aValue = QString::fromLocal8Bit(aEntryData.mValue.data());
    expand = aEntryData.bExpand;

    // Ok this sucks. QString::fromUtf8("").isNull() is true,
    // but QString::fromLatin1("").isNull() returns false.
    if (aValue.isNull())
    {
      static const QString &emptyString = KGlobal::staticQString("");
      aValue = emptyString;
    }
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
	    if (isUtf8( pEnv ))
		aValue.replace( nDollarPos, nEndPos-nDollarPos, QString::fromUtf8(pEnv) );
	    else
		aValue.replace( nDollarPos, nEndPos-nDollarPos, QString::fromLocal8Bit(pEnv) );
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
  entryKey.c_key = pKey;
  KEntry aEntryData = lookupData(entryKey);
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
  QStringList list;
  if( !hasKey( pKey ) )
    return list;
  QString str_list = readEntry( pKey );
  if( str_list.isEmpty() )
    return list;
  QString value = "";
  int len = str_list.length();
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
          value += str_list[i];
          continue;
        }
      list.append( value );
      value.truncate(0);
    }
  if ( str_list[len-1] != sep || ( len > 1 && str_list[len-2] == '\\' ) )
    list.append( value );
  return list;
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


bool KConfigBase::readBoolEntry( const QString& pKey, const bool bDefault ) const
{
   return readBoolEntry(pKey.utf8().data(), bDefault);
}

bool KConfigBase::readBoolEntry( const char *pKey, const bool bDefault ) const
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

void KConfigBase::writePathEntry( const char *pKey, const QString & path,
                                  bool bPersistent, bool bGlobal,
                                  bool bNLS)
{
   QString value;
   if (!path.isEmpty())
   {
      value = KGlobal::dirs()->relativeLocation("home", path);
      if (value[0] != '/')
         value = "$HOME/"+value;
   }
   writeEntry(pKey, value, bPersistent, bGlobal, bNLS);
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
      if (isUtf8(it.current()))
        value = QString::fromUtf8(it.current());
      else
        value = QString::fromLocal8Bit(it.current());
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

KConfigGroup::KConfigGroup(KConfigBase *master, const QString &group)
{
  mMaster = master;
  backEnd = 0;
  bLocaleInitialized = true;
  bReadOnly = mMaster->bReadOnly;
  bExpand = false;
  bDirty = false; // Not used
  mGroup = group.utf8();
  aLocaleString = mMaster->aLocaleString;
}

KConfigGroup::KConfigGroup(KConfigBase *master, const QCString &group)
{
  mMaster = master;
  backEnd = 0;
  bLocaleInitialized = true;
  bReadOnly = mMaster->bReadOnly;
  bExpand = false;
  bDirty = false; // Not used
  mGroup = group;
  aLocaleString = mMaster->aLocaleString;
}

KConfigGroup::KConfigGroup(KConfigBase *master, const char * group)
{
  mMaster = master;
  backEnd = 0;
  bLocaleInitialized = true;
  bReadOnly = mMaster->bReadOnly;
  bExpand = false;
  bDirty = false; // Not used
  mGroup = group;
  aLocaleString = mMaster->aLocaleString;
}

void KConfigGroup::deleteGroup(bool bGlobal)
{
  mMaster->deleteGroup(KConfigBase::group(), false, bGlobal);
}

void KConfigGroup::setDirty(bool b)
{
  mMaster->setDirty(b);
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

#include "kconfigbase.moc"

