// $Id$
//
// $Log$
// Revision 1.5  1997/09/10 11:55:38  kdecvs
// Kalle:
// - new features in KConfig as announced on Monday
// - new KIconLoader from Christoph
//
// Revision 1.4  1997/09/04 19:44:03  kdecvs
// Coolo: corrected a comment. the language code is not two letters only
//
// Revision 1.3  1997/08/31 15:56:12  kdecvs
// Kalle:
// - Internationalized Config Entries
// - Default-String in readEntry changed to const char*
//
// Revision 1.2  1997/07/18 05:49:14  ssk
// Taj: All kdecore doc now in javadoc format (hopefully).
//
// Revision 1.1.1.1  1997/04/13 14:42:41  cvsuser
// Source imported
//
// Revision 1.1.1.1  1997/04/09 00:28:07  cvsuser
// Sources imported
//
// Revision 1.6  1997/03/16 22:47:58  kalle
// do not include kstream.h
//
// Revision 1.5  1997/03/09 17:29:00  kalle
// KTextStream -> QTextStream
//
// Revision 1.4  1996/11/23 21:17:30  kalle
// KTextStream instead of QTextStream
//
// Revision 1.3  1996/11/20 20:16:07  kalle
// bDirty in KEntryDictEntry and KConfig added (for write access)
//
// Revision 1.2  1996/11/17 09:51:28  kalle
// new KEntryDictEntry structure instead of simple strings
//
// Revision 1.1  1996/11/10 18:31:27  kalle
// Initial revision
//
// Revision 1.4  1996/11/03 21:02:17  kalle
// Change to chached config entries
//
// Revision 1.3  1996/10/26 19:10:29  kalle
// new: aFile (debug version only)
//
// Revision 1.2  1996/10/26 18:09:37  kalle
// initialize aGroup to empty string (not _null_ string)
//
// Revision 1.1  1996/10/25 17:15:39  kalle
// Initial revision
//
// Revision 1.1  1996/10/20 19:55:19  kalle
// Initial revision
//

// KConfig: configuration options for the Kool Desktop Environment
//
// KConfig.cpp
//
// (C) 1996 by Matthias Kalle Dalheimer

// KConfig::KConfig(): create a Config object, only system files
// are searched 

#ifndef _KCONFIGDATA_H
#define _KCONFIGDATA_H

#include <qdict.h> // QDict
#include <qtstream.h> // QTextStream

struct KEntryDictEntry
{
  QString aValue;
  bool    bDirty; // must the entry be written back to disk?
  bool    bGlobal; // entry should be written to the global config file
  bool    bNLS;    // entry should be written with locale tag
};

typedef QDict<KEntryDictEntry> KEntryDict;
typedef QDict<KEntryDict> KGroupDict;
typedef QDictIterator<KEntryDict> KGroupIterator;
typedef QDictIterator<KEntryDictEntry> KEntryIterator;

/**
* Configuration data manager, used internally by KConfig.
* @short Configuration data manager, used internally by KConfig.
* @version $Id$
* @author Matthias Kalle Dalheimer (kalle@kde.org)
*/
class KConfigData
{
friend class KConfig;
private:
  QTextStream* pAppStream;
  QString aGroup;
  QString aLocaleString; // locale code
  bool bDirty; // is there any entry that has to be written back to disk?

  QDict<KEntryDict> aGroupDict;

#ifndef NDEBUG
  QString aFile;
#endif
  
public:
  KConfigData();
  KConfigData( QTextStream* );
  
  KGroupIterator* groupIterator( void );
};

inline KConfigData::KConfigData() :
    aGroupDict( 37, FALSE )
{
  aGroupDict.setAutoDelete( true );
  pAppStream = NULL;
  aGroup = "<default>";
  bDirty = false;
}
  
inline KConfigData::KConfigData( QTextStream* pStream ) :
    aGroupDict( 37, FALSE )
{
  aGroupDict.setAutoDelete( true );
  pAppStream = pStream;
  aGroup = "<default>";
  bDirty = false;
}

inline KGroupIterator* KConfigData::groupIterator(void)
{
  return new KGroupIterator(aGroupDict);
}


#endif
