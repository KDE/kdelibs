// $Id$
//
// $Log$
// Revision 1.14  1999/04/18 09:15:10  kulow
// taking out config.h from Header files. I don't know if I haven't noticed
// before, but this is even very dangerous
//
// Revision 1.13  1999/04/08 01:39:45  torben
// Torben: Corrected API and docu
//
// Revision 1.12  1999/03/01 23:33:23  kulow
// CVS_SILENT ported to Qt 2.0
//
// Revision 1.11.2.2  1999/02/14 02:05:45  granroth
// Converted a lot of 'const char*' to 'QString'.  This compiles... but
// it's entirely possible that nothing will run linked to it :-P
//
// Revision 1.11.2.1  1999/01/30 20:18:58  kulow
// start porting to Qt2.0 beta
//
// Revision 1.11  1998/10/07 06:49:24  kalle
// Correctly read double dollar signs (patch by Harri Porten)
// Dollar expansion can be turned off with setDollarExpansion( false ).
// \sa isDollarExpansion
//
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
// (C) 1996-1999 by Matthias Kalle Dalheimer

#ifndef _KCONFIGDATA_H
#define _KCONFIGDATA_H

#include <qdict.h> // QDict

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
  QCString aFile;
#endif
  
public:
  KConfigBaseData();
  KConfigBaseData( const QString& pGlobalAppFile, const QString& pLocalAppFile );
  
  KGroupIterator* groupIterator( void );
};

inline KConfigBaseData::KConfigBaseData() :
  aGroup("<default>"), bDirty(false),
  bLocaleInitialized(false), bReadOnly(false), bExpand( true ),
  aGroupDict( 37, false )
{
  aGroupDict.setAutoDelete( true );
}
  
inline KConfigBaseData::KConfigBaseData( const QString& pGlobalAppFile,
					 const QString& pLocalAppFile ) :
  aLocalAppFile(pLocalAppFile), aGlobalAppFile(pGlobalAppFile), 
  aGroup("<default>"), bDirty(false), 
  bLocaleInitialized(false), bReadOnly(false), aGroupDict( 37, false )
{
  aGroupDict.setAutoDelete( true );
}

inline KGroupIterator* KConfigBaseData::groupIterator(void)
{
  return new KGroupIterator(aGroupDict);
}


#endif
