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

KRegistry* KRegistry::self()
{
  if ( !m_pSelf )
    (void)new KRegistry();
  return m_pSelf;
}

KRegistry::KRegistry()
{
  m_pSelf = this;

  m_bLoaded = false;
  m_bModified = false;
  
  m_lstEntries.setAutoDelete( true );

  m_pDirWatch = new KDirWatch;
 
  connect( m_pDirWatch, SIGNAL(dirty(const QString&)),
	   this, SLOT(update(const QString&)));
  connect( m_pDirWatch, SIGNAL(deleted(const QString&)),
	   this, SLOT(dirDeleted(const QString&))); 
}

KRegistry::~KRegistry()
{
}

void KRegistry::addFactory( KRegFactory *_factory )
{
  m_lstFactories.append( _factory );
  
  QStringList::ConstIterator it = _factory->pathList().begin();
  // QStringList::ConstIterator end = _factory->pathList().end(); doesn't work ??? (David)
  for( ; it != _factory->pathList().end(); ++it )
  {    
    kdebug( KDEBUG_INFO, 7011, "addFactory : adding path %s", (*it).ascii() );
    m_lstToplevelDirs.append( *it );
    if ( m_bLoaded )
      readDirectory( *it, true );
  }
}

bool KRegistry::readDirectory( const QString& _path, bool _init )
{
  kdebug( KDEBUG_INFO, 7011, "Reading directory %s", _path.ascii() );
  
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
	  i2 = exists( file );                  // find it in list...
	  if ( i2 != -1 )                              // got it?
	    {                                         // Yeah!
	      // kdebug( KDEBUG_INFO, 7011, "Updating %s", file.data() );
	      KRegEntry *entry = m_lstEntries.at( i2 );
	      entry->mark();
	      if ( !entry->update() )                // update it (if needed)
		{
		  m_bModified = true;
		  m_lstEntries.removeRef( entry );
		  m_lstEntries.append( createEntry( file ) );
		}
	    }
	  else if ( file.right(1) != "~" )         // we don't have this one..
	    {
	      // Can we read the file ?
	      if ( access( file, R_OK ) != -1 )
		{   
		  // Create a new entry
		  m_lstEntries.append( createEntry( file ) );
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
	  kdebug( KDEBUG_INFO, 7011, "KRegistry: Deleted item %s", a->file().ascii());
	  m_lstEntries.remove( m_lstEntries.at() );
	  a = m_lstEntries.current();
	  m_bModified = true;
	  continue;
	}
      a = m_lstEntries.next();
    }
  
  return true;
}

int KRegistry::exists( const QString& _file ) const
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

void KRegistry::load( const QString& _dbfile )
{
  if ( !_dbfile.isEmpty() )
  {    
    QFile file( _dbfile );
    if ( file.open( IO_ReadOnly ) )
    {
      QDataStream str( &file );
      
      while( !str.eof() )
      {
	QString file;
	str >> file;
      
	kdebug( KDEBUG_INFO, 7011, "STORE: %s", file.ascii());
	
	KRegEntry *entry = 0;
	if ( file == "//Dummy//" )
	{
	  str >> file;
	  entry = new KRegDummy( this, file );
	  entry->load( str );
	}
	else
	  entry = createEntry( str, file );
	if ( entry )
	  m_lstEntries.append( entry );
      }

      file.close();
    }
  } 

  m_bModified = false;
  
  QStringList::Iterator it = m_lstToplevelDirs.begin();
  for( ; it != m_lstToplevelDirs.end(); ++it )
  {    
    kdebug( KDEBUG_INFO, 7011, "========== SCANNING %s ==============", it->ascii() );
    readDirectory( *it, true );
  }
  
  m_bLoaded = true;
}

bool KRegistry::save( const QString& _dbfile ) const
{
  QString f;
  if ( _dbfile.isEmpty() )
    f = kapp->localconfigdir() + "/kregistry";
  else
    f = _dbfile;
  
  QFile file( f );
  if ( file.open( IO_WriteOnly ) )
  {
    QDataStream str( &file );

    QListIterator<KRegEntry> it( m_lstEntries );
    for( ; it.current(); ++it )
    {
      if ( it.current()->isDummy() )
	str << "//Dummy//";
      str << it.current()->file();
      it.current()->save( str );
    }
  }
  else
    return false;
  
  file.close();

  return true;
}

void KRegistry::update( const QString& _path )
{
  if ( _path )
    readDirectory( _path );
  else
  {
    QStringList::Iterator it = m_lstToplevelDirs.begin();
    for( ; it != m_lstToplevelDirs.end(); ++it )
      readDirectory( *it );
  }
}

void KRegistry::dirDeleted( const QString& _path )
{
  kdebug( KDEBUG_INFO, 7011, "KRegistry: Dir deleted %s", _path.ascii() );
  
  KRegEntry *a = m_lstEntries.first();
  while( a )
  {
    if ( a->isInDirectory( _path, true ) )
    {
      kdebug( KDEBUG_INFO, 7011, "KRegistry: Deleted item %s", a->file().ascii() );
      m_lstEntries.remove( m_lstEntries.at() );
      a = m_lstEntries.current();
      continue;
    }
    a = m_lstEntries.next();
  }
}

KRegEntry* KRegistry::createEntry( QDataStream& _str, const QString& _file )
{
  kdebug( KDEBUG_INFO, 7011, "KRegistry: createEntry from datastream for %s", _file.ascii());
  KRegFactory *f;
  for( f = m_lstFactories.first(); f != 0L; f = m_lstFactories.next() )
  {
    if ( f->matchFile( _file ) )
    {
      KRegEntry *e = f->create( this, _file, _str );
      if ( e )
	return e;
      break;
    }
  }
  
  return new KRegDummy( this, _file );
}

KRegEntry* KRegistry::createEntry( const QString& file )
{
  // Just a backup file ?
  if ( file.right(1) == "~" || file.right(4) == ".bak" || ( file[0] == '%' && file.right(1) == "%" ) )
      return new KRegDummy( this, file );

  KSimpleConfig cfg( file, true );
  cfg.setDesktopGroup();
  QString type = cfg.readEntry( "Type" );
  
  KRegFactory *f;
  for( f = m_lstFactories.first(); f != 0L; f = m_lstFactories.next() )
  {
    if ( f->matchFile( file ) )
    {
      KRegEntry *e = f->create( this, file, cfg );
      if ( e )
	return e;
      break;
    }
  }
  
  return new KRegDummy( this, file );
}

/***************************************************
 *
 * KRegEntry
 *
 ***************************************************/

KRegEntry::KRegEntry( KRegistry* _reg, const QString& _file )
{
  m_bMarked = true;
  m_strFile = _file;
  m_pRegistry = _reg;
  
  struct stat statbuff;
  if ( stat( m_strFile, &statbuff ) == -1 )
  {
    kdebug( KDEBUG_ERROR, 7011, "Oooops %s", m_strFile.ascii() );
  }

  m_ctime = statbuff.st_ctime;
}

bool KRegEntry::isInDirectory( const QString& _path, bool _allow_subdir ) const
{
  if ( _allow_subdir )
  {
    if ( strncmp( m_strFile.ascii(), _path, _path.length() ) == 0 )
      return true;
    return false;
  }
  
  unsigned int pos = m_strFile.findRev( '/' );
  if ( pos != _path.length() )
    return false;
  
  if ( strncmp( _path, m_strFile.ascii(), pos + 1 ) == 0 )
    return true;
  
  return false;
}

bool KRegEntry::update()
{
  kdebug( KDEBUG_INFO, 7011, "Checking %s", m_strFile.ascii() );

  // Does the file still exist ?
  struct stat statbuff;
  if (stat( m_strFile, &statbuff) == -1)
  {
    kdebug( KDEBUG_INFO, 7011, "Removing us" );
    // We are going to be deleted now
    unmark();
    // No need to care about updates. However, we say that 
    // we managed to update
    return true;
  }

  // Still readable ?
  if ( access( m_strFile, R_OK ) == -1 )
  {
    kdebug( KDEBUG_INFO, 7011, "We are no longer readable" );
    // We are going to be deleted now
    unmark();
    // No need to care about updates. However, we say that 
    // we managed to update
    return true;
  }
 
  if ( statbuff.st_ctime == m_ctime )
    return true; // nothing happened 

  kdebug( KDEBUG_INFO, 7011, "OUTDATED %s %d old was %d", m_strFile.data(), statbuff.st_ctime, m_ctime );
  
  return updateIntern();
}

void KRegEntry::load( QDataStream& _str )
{
  Q_INT32 t;
  _str >> t;
  m_ctime = (time_t)t;
}

void KRegEntry::save( QDataStream& _str ) const
{
  Q_INT32 t = (Q_INT32)m_ctime;
  _str << t;
}

/*******************************************************
 *
 * KRegFactory
 *
 *******************************************************/

bool KRegFactory::matchFile( const QString& _file ) const
{
  int i = _file.findRev( '/' );
  if ( i == -1 )
    return false;

  QString file = _file.left( i + 1 );

  QStringList p = pathList();
  QStringList::Iterator it = p.begin();
  for( ; it != p.end() ; ++it )
  {    
    QString p = *it;
    if ( p.right(1) != "/" )
      p += "/";
  
    if ( strncmp( p.ascii(), file.ascii(), p.length() ) == 0 && p.length() <= file.length() )
      return true;
  }
  
  return false;
}

#include "kregistry.moc"
