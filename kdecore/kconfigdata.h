// $Id$
//
// $Log$
// Revision 1.10  1998/09/01 20:21:21  kulow
// I renamed all old qt header files to the new versions. I think, this looks
// nicer (and gives the change in configure a sense :)
//
// Revision 1.9  1998/08/22 20:02:39  kulow
// make kdecore have nicer output, when compiled with -Weffc++ :)
//
// Revision 1.8  1998/04/26 02:24:45  ssk
// Some classes marked internal.
//
// Revision 1.7  1998/01/18 14:38:53  kulow
// reverted the changes, Jacek commited.
// Only the RCS comments were affected, but to keep them consistent, I
// thought, it's better to revert them.
// I checked twice, that only comments are affected ;)
//
// Revision 1.5  1998/01/15 13:22:29  kalle
// Read-only mode for KSimpleConfig
//
// Revision 1.4  1997/12/12 14:46:04  denis
// Reverting to lower case true/false
//
// Revision 1.3  1997/12/12 14:33:48  denis
// replaced true by TRUE and false by FALSE
//
// Revision 1.2  1997/10/10 19:24:11  kulow
// removed mutable and replace const_cast with a more portable way.
//
// Revision 1.1  1997/10/04 19:51:06  kalle
// new KConfig
//
//
// (C) 1996-1998 by Matthias Kalle Dalheimer

#ifndef _KCONFIGDATA_H
#define _KCONFIGDATA_H

#include <qdict.h> // QDict
#include <qtextstream.h> // QTextStream

/**
* Entry-dictionary entry.
* @internal
*/
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
* @internal
*/
class KConfigBaseData
{
friend class KConfig;
friend class KConfigBase;
friend class KSimpleConfig;
private:
  QString aLocalAppFile;
  QString aGlobalAppFile;
  QString aGroup;
  QString aLocaleString; // locale code
  bool bDirty; // is there any entry that has to be written back to disk?
  bool bLocaleInitialized;
  bool bReadOnly; // currently only used by KSimpleConfig
	bool bExpand; // whether dollar expansion is used

  QDict<KEntryDict> aGroupDict;

#ifndef NDEBUG
  QString aFile;
#endif
  
public:
  KConfigBaseData();
  KConfigBaseData( const char* pGlobalAppFile, const char* pLocalAppFile );
  
  KGroupIterator* groupIterator( void );
};

inline KConfigBaseData::KConfigBaseData() :
    aLocalAppFile(0), aGlobalAppFile(0),
    aGroup("<default>"), aLocaleString(0), bDirty(false),
    bLocaleInitialized(false), bReadOnly(false), bExpand( true ),
	aGroupDict( 37, false )
#ifndef NDEBUG
  , aFile(0)
#endif
{
  aGroupDict.setAutoDelete( true );
}
  
inline KConfigBaseData::KConfigBaseData( const char* pGlobalAppFile,
					 const char* pLocalAppFile ) :
  aLocalAppFile(pLocalAppFile), aGlobalAppFile(pGlobalAppFile), 
  aGroup("<default>"), aLocaleString(0), bDirty(false), 
  bLocaleInitialized(false), bReadOnly(false), aGroupDict( 37, false )
#ifndef NDEBUG
  , aFile(0)
#endif
{
  aGroupDict.setAutoDelete( true );
}

inline KGroupIterator* KConfigBaseData::groupIterator(void)
{
  return new KGroupIterator(aGroupDict);
}


#endif
