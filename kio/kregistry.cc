 /*
  This file is or will be part of KDE desktop environment

  Copyright 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

  It is licensed under GPL version 2.
  
  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#include "kregistry.h"
#include "kdirwatch.h"

#include <string.h>
#include <string>

#include <qdir.h>
#include <kapp.h>

#include <iostream.h>
#include <assert.h>

/*******************************************************
 *
 * KRegistry
 *
 *******************************************************/

KRegistry *KRegistry::m_pSelf = 0L;

KRegistry::KRegistry()
{
  m_pSelf = this;

  m_bLoaded = false;
  m_bModified = false;
  
  m_lstEntries.setAutoDelete( true );

  m_pDirWatch = new KDirWatch;
 
  connect( m_pDirWatch, SIGNAL(dirty(const char *)),
	   this, SLOT(update(const char *)));
  connect( m_pDirWatch, SIGNAL(deleted(const char *)),
	   this, SLOT(dirDeleted(const char *))); 
}

KRegistry::~KRegistry()
{
}

void KRegistry::addFactory( KRegFactory *_factory )
{
  m_lstFactories.append( _factory );
  
  QStringList::ConstIterator it = _factory->pathList().begin();
  QStringList::ConstIterator end = _factory->pathList().end();
  for( ; it != end; ++it )
  {    
    m_lstToplevelDirs.append( *it );
    if ( m_bLoaded )
      readDirectory( *it, true );
  }
}

bool KRegistry::readDirectory( const char* _path, bool _init )
{
  kdebug( KDEBUG_INFO, 7011, "Reading directory %s", _path );
  
  QDir d( _path );                               // set QDir ...
  if ( !d.exists() )                            // exists&isdir?
    return false;                             // return false
  d.setSorting(QDir::Name);                  // just name

  QString path( _path );
  if ( path.right(1) != "/" )
    path += "/";
 
  QString file;

  //************************************************************************
  //                           Mark
  //************************************************************************
 
  QListIterator<KRegEntry> it( m_lstEntries );
  for( ; it.current(); ++it )
    it.current()->unmark();
   
  //************************************************************************
  //                           Setting dirs
  //************************************************************************
 
  if ( _init )                                  // first time?
    m_pDirWatch->addDir(path);          // add watch on this dir
  
  // Note: If some directory is gone, dirwatch will delete it from the list.
  
  //************************************************************************
  //                               Reading
  //************************************************************************
 
 unsigned int i;                           // counter and string length.
 unsigned int count = d.count();
 for( i = 0; i < count; i++ )                        // check all entries
 {
   if (strcmp(d[i],".")==0 || strcmp(d[i],"..")==0 || strcmp(d[i],"magic")==0)
    continue;                          // discard those ".", "..", "magic"...

   file = path.data();                          // set full path
   file += d[i];                          // and add the file name.
   if ( stat( file, &m_statbuff ) == -1 )           // get stat...
    continue;                                   // no such, continue.

   if ( S_ISDIR( m_statbuff.st_mode ) )               // isdir?
   {
     if ( _init )                                 // first time?
       readDirectory( file, _init );      // yes, dive into it.
     else if ( !m_pDirWatch->contains( file ) ) // New dir?
     {
       readDirectory( file, _init );      // yes, dive into it.
       m_pDirWatch->addDir( file );         // and add it to the list
     }
   }
   else                                         // no, not a dir/no recurse...
   {
     int i2 = -1;                                    // index
     // if ( !_init )                                 // first time?
       i2 = ( exists( file ) );                  // find it in list...
     if ( i2 != -1 )                              // got it?
     {                                         // Yeah!
       // kdebug( KDEBUG_INFO, 7011, "Updating %s", file.data() );
       KRegEntry *entry = m_lstEntries.at( i2 );
       entry->mark();
       KRegEntry *e = entry->update();                // update it (if needed)
       // Replace ?
       if ( e )
       {
	 m_lstEntries.insert( i2, e );
	 m_lstEntries.removeRef( entry );
	 m_bModified = true;
       }
     }
     else if ( file.right(1) != "~" )         // we don't have this one..
     {
       // Can we read the file ?
       if ( access( file, R_OK ) != -1 )
       {   
	 // Create a new entry
	 KRegEntry *entry = createEntry( file );
	 m_lstEntries.append( entry );
	 if ( !_init )
	   kdebug( KDEBUG_INFO, 7011, "KRegistry: New item %s", file.data() );
	 m_bModified = true;
       }
     }
   }                                        // that's all
 }

 // Now: what if file was removed from dir? We parsed only those found by
 // QDir, but we should kick out from the list deleted ones. This is what we
 // do:
 // We loop thro' the list to check if all items exist. And we remove those
 // that do not exist. Since this is recursive function, we use flag
 // 'inner'. If we're called recursively this flag is true.

 if ( _init )    // pointless if not top level or if first time
   return true;

 //************************************************************************
 //                        Removing deleted from list
 //                                  Sweep
 //************************************************************************

 KRegEntry *a = m_lstEntries.first();
 while( a )
 {
   if ( !a->isMarked() && a->isInDirectory( path ) )
   {
     kdebug( KDEBUG_INFO, 7011, "KRegistry: Deleted item %s", a->file());
     m_lstEntries.remove( m_lstEntries.at() );
     a = m_lstEntries.current();
     m_bModified = true;
     continue;
   }
   a = m_lstEntries.next();
 }

 return true;
}

int KRegistry::exists( const char *_file )
{
  int pos = 0;
  QListIterator<KRegEntry> it( m_lstEntries );
  for( ; it.current(); ++it )
  {
    if ( strcmp( it.current()->file(), _file ) == 0 )
      return pos;
    pos++;
  }

  return -1;
}

void KRegistry::load( const char *_dbfile )
{
  if ( _dbfile )
  {    
    QFile file( _dbfile );
    if ( file.open( IO_ReadOnly ) )
    {
      QDataStream str( &file );
      
      while( !str.eof() )
      {
	QString type;
	QString file;
	str >> type >> file;
      
	kdebug( KDEBUG_INFO, 7011, "STORE: %s of type %s", file.data(), type.data());
	
	KRegEntry *entry = createEntry( str, file, type );
	if ( entry )
	  m_lstEntries.append( entry );
      }

      file.close();
    }
  } 
  const char *s;
  for( s = m_lstToplevelDirs.first(); s != 0L; s = m_lstToplevelDirs.next() )
  {    
    kdebug( KDEBUG_INFO, 7011, "========== SCANNING %s ==============", s );
    readDirectory( s, true );
  }
  
  m_bLoaded = true;
}

bool KRegistry::save( const char *_dbfile )
{
  assert( _dbfile != 0L );
  
  QFile file( _dbfile );
  if ( file.open( IO_WriteOnly ) )
  {
    QDataStream str( &file );
    
    KRegEntry* e;
    for( e = m_lstEntries.first(); e != 0L; e = m_lstEntries.next() )
    {
      QString type = e->type();
      QString file = e->file();
      str << type << file;
      e->save( str );
    }
  }
  
  file.close();

  return true;
}

void KRegistry::update( const char *_path )
{
  if ( _path )
    readDirectory( _path );
  else
  {
    const char *s;
    for( s = m_lstToplevelDirs.first(); s != 0L; s = m_lstToplevelDirs.next() )
      readDirectory( s );
  }
}

void KRegistry::dirDeleted( const char *_path )
{
  kdebug( KDEBUG_INFO, 7011, "KRegistry: Dir deleted %s", _path );
  
  KRegEntry *a = m_lstEntries.first();
  while( a )
  {
    if ( a->isInDirectory( _path, true ) )
    {
      kdebug( KDEBUG_INFO, 7011, "KRegistry: Deleted item %s", a->file() );
      m_lstEntries.remove( m_lstEntries.at() );
      a = m_lstEntries.current();
      continue;
    }
    a = m_lstEntries.next();
  }
}

KRegEntry* KRegistry::createEntry( QDataStream& _str, const char *_file, const char *_type )
{
  // Just a backup file ?
  if ( strcmp( _type, "Dummy" ) == 0L )
  {    
    KRegEntry *e = new KRegDummy( this, _file );
    e->load( _str );
    KRegEntry *e2 = e->update();
    if ( e2 )
      delete e;
    else
      e2 = e;
    return e;
  }
    
  KRegFactory *f;
  for( f = m_lstFactories.first(); f != 0L; f = m_lstFactories.next() )
  {
    if ( strcmp( _type, f->type() ) == 0L && f->matchFile( _file ) )
    {
      KRegEntry *e = f->create( this, _file, _str );
      if ( e )
	return e;
      break;
    }
  }
  
  return 0L;
}

KRegEntry* KRegistry::createEntry( const char *_file )
{
  QString file = _file;
  // Just a backup file ?
  if ( file.right(1) == "~" || file.right(4) == ".bak" || ( file[0] == '%' && file.right(1) == "%" ) )
      return new KRegDummy( this, _file );

  KSimpleConfig cfg( _file, true );
  cfg.setGroup( "KDE Desktop Entry" );
  QString type = cfg.readEntry( "Type" );
  
  KRegFactory *f;
  for( f = m_lstFactories.first(); f != 0L; f = m_lstFactories.next() )
  {
    if ( type == f->type() && f->matchFile( _file ) )
    {
      KRegEntry *e = f->create( this, _file, cfg );
      if ( e )
	return e;
      break;
    }
  }
  
  return new KRegDummy( this, _file );
}

/***************************************************
 *
 * KRegEntry
 *
 ***************************************************/

KRegEntry::KRegEntry( KRegistry* _reg, const char* _file )
{
  m_bMarked = true;
  m_strFile = _file;
  m_pRegistry = _reg;
  
  struct stat statbuff;
  if ( stat( m_strFile, &statbuff ) == -1 )
  {
    kdebug( KDEBUG_ERROR, 7011, "Oooops %s", m_strFile.data() );
  }

  m_ctime = statbuff.st_ctime;
}

bool KRegEntry::isInDirectory( const char *_path, bool _allow_subdir )
{
  if ( _allow_subdir )
  {
    if ( strncmp( m_strFile.data(), _path, strlen( _path ) ) == 0 )
      return true;
    return false;
  }
  
  unsigned int pos = m_strFile.findRev( '/' );
  if ( pos != strlen( _path ) )
    return false;
  
  if ( strncmp( _path, m_strFile.data(), pos + 1 ) == 0 )
    return true;
  
  return false;
}

KRegEntry* KRegEntry::update()
{
  kdebug( KDEBUG_INFO, 7011, "Checking %s", m_strFile.data() );

  struct stat statbuff;
  if (stat( m_strFile, &statbuff) == -1)
  {
    kdebug( KDEBUG_INFO, 7011, "Removing us" );
    // We are going to be deleted now
    unmark();
    return 0L;
  }

  if ( access( m_strFile, R_OK ) == -1 )
  {
    kdebug( KDEBUG_INFO, 7011, "We are no longer readable" );
    unmark();
    return 0L;
  }
 
  if ( statbuff.st_ctime == m_ctime )
    return 0L; // nothing happened 

  kdebug( KDEBUG_INFO, 7011, "OUTDATED %s %d old was %d", m_strFile.data(), statbuff.st_ctime, m_ctime );
  
  if ( !updateIntern() )
    return m_pRegistry->createEntry( m_strFile );
    
  // Means that we indeed updated
  return 0L;
}

void KRegEntry::load( QDataStream& _str )
{
  Q_INT32 t;
  _str >> t;
  m_ctime = (time_t)t;
}

void KRegEntry::save( QDataStream& _str )
{
  Q_INT32 t = (Q_INT32)m_ctime;
  _str << t;
}

/*******************************************************
 *
 * KRegFactory
 *
 *******************************************************/

bool KRegFactory::matchFile( const char *_file )
{
  QString file = _file;
  int i = file.findRev( '/' );
  if ( i == -1 )
    return false;
  file = file.left( i + 1 );

  QStringList::ConstIterator it = pathList().begin();
  QStringList::ConstIterator end = pathList().end();
  for( ; it != end ; ++it )
  {    
    QString p = *it;
    if ( p.right(1) != "/" )
      p += "/";
  
    if ( strncmp( p.data(), file.data(), p.length() ) == 0 && p.length() <= file.length() )
      return true;
  }
  
  return false;
}

#include "kregistry.moc"
