 /*
  This file is or will be part of KDE desktop environment

  Copyright 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

  It is licensed under GPL version 2.
  
  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */


#ifndef _KREGISTRY_H
#define _KREGISTRY_H

#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <qstrlist.h>
#include <qtimer.h>
#include <qlist.h>

#include "kdirwatch.h"

#include "ksimpleconfig.h"

#define kreg KRegistry::getKRegistry

class KDirWatch;
class KRegistry;

class KRegEntry
{
public:
  KRegEntry( KRegistry* _reg, const char* _file );
  virtual ~KRegEntry() { };
  
  void mark() { m_bMarked = true; }
  void unmark() { m_bMarked = false; }
  bool isMarked() { return m_bMarked; }
  
  const char* file() { return m_strFile; }
  
  /**
   * @param _path MUST have a trailing '/'
   */
  bool isInDirectory( const char *_path, bool _allow_subdir = false );

  KRegEntry* update();

protected:
  virtual bool updateIntern() = 0L;

  bool m_bMarked;
  QString m_strFile;
  time_t m_ctime;
  KRegistry* m_pRegistry;
};

class KRegFactory
{
public:
  virtual KRegEntry* create( KRegistry* _reg, const char *_file, KSimpleConfig &_cfg ) = 0L;
  virtual const char* type() = 0L;
  /**
   * @return the path for which this factory is responsible.
   *         A MimeType factory for example would return "/opt/kde/share/mimelnk".
   *         Please note that the return value may not have a trailing '/'.
   */
  virtual QStrList& pathList() = 0L;

  virtual bool matchFile( const char *_file );
};

class KRegDummy : public KRegEntry
{
public:
  KRegDummy( KRegistry* _reg, const char* _file ) : KRegEntry( _reg, _file ) { }

protected:
  virtual bool updateIntern() { return false; }
};

/**
 * <b>KRegistry</b> is a class for keeping a database of MimeTypes and Applications
 * (kdelnks of type 'Application').<br>
 * Database is internal - no special files are created. <b>KRegistry</b> reads and
 * watches for change local and global directories with mimetypes (`mimelnk'
 * directory) and applications ('applnk' directory).<br>
 * The database is self updating - you don't have to notify <b>KRegistry</b> if
 * you or anyone make changes to (local or global) 'applnk' or 'mimelnk
 * directories'. Updating is inteligent; only changed directories are scanned
 * and only changed items (kdelnks) are reread. This makes <b>KRegistry</b> very
 * fast.<br>
 * You can ask Kregistry for info about any file or data. Query can be made
 * by full path of file ("/usr/home/you/phd.lyx"), by mime type ("image/gif"),
 * or by magic. To resolve file by magic, you can send a fullpath of file
 * or first 64 bytes of data.<br>
 * You can pass paths that are not on local file systems - those that begin
 * with 'ftp:' or 'http' are not checked for existance - only extension is
 * used. If this fails, you can pull first 64 bytes of that file and try
 * get binding by contents.<br>
 * It is almost allways easier to use KRegEntry class.<br>
 * <b>KRegistry</b> answers by returning pointer to structure KRegStruct.<br>
 * Since local (user's) dirs are scanned first, and <b>KRegistry</b> uses
 * first item  found, user's changes take precedense before global dirs.
 * This works both for mimetypes and applications. <br>
 * <b>KRegistry</b> doesn't write anything to disc.<br>
 * Class KRegEntry uses <b>KRegistry</b>; you can always create KRegEntry class,
 * and it will use <b>KRegistry</b> to get data.<br>
 * <b>KRegistry</b> has private constructor to prevent more than one instance per
 * application. You must use static KRegistry::getKRegistry() function (or
 * macro kreg) to get pointer to it or to create new instance. This
 * will eventually move to KApplication later. Untill then, you should
 * delete registry on app exit to avoid memory leak.
 * @see KRegEntry
 *
 * @short <b>KRegistry</b> is a database of MimeTypes and Applications
 * @author Sven Radej <sven@lisa.exp.univie.ac.at>
 */
class KRegistry : public QObject
{
  friend KRegEntry;
  
 Q_OBJECT   
public: 
  /**
   * Constructor.
   */
  KRegistry();
   
  /**
   * Destructor. Untill integration with KApplication you should
   * delete registry on app exit to avoid memory leak.
   */
  ~KRegistry();

  void addFactory( KRegFactory *_factory );
  
public slots:
    
  /**
   * Rereads specified dir (if needed). If dir = 0, rereads whole
   * registry (= all mimelnk and applnk dirs) if they need updating.
   * Please don't pass here stupid dirs like /var/log/ or /tmp. You <i>don't</i>
   * need to call this, since it is called when needed by @ref KDirWatch
   */
 void update (const char *dir = 0);

protected:

  KRegEntry* createEntry( const char *_file );
  
 /**
  * Extremely internal - reads directory. List is list to fill. Call this
  * always with inner=false. Inner = true means that you have to clean up
  * the list for missing files. Returns false only if path is not a directory
  * or if it doesn't exist. Otherwise returns true.
  */
 
 bool readDirectory(const char *dir, QList<KRegEntry> &list, bool _init = false );

 /**
  * Internal - checks if file exists in internal list
  */
 int exists( const char *file, QList<KRegEntry>& list );

 /**
  * Unimplemented - Saves registry if needed (= never). Use this to annoy
  * people who can't stand human-unreadable config files.
  */
 bool save();
 /**
  * Unimplemented - Loads registry if needed (= never) and if not corrupted
  * (also never, since it does not exist). Use this to annoy people who can't
  * stand human-unreadable config files.
  */
 bool load();

protected slots:

 /**
  * Internal - @ref KDirWatch saw kill of dir. And what do I do? I delete
  * all list entries that doesn't live on disk.
  */
 void dirDeleted(const char *path);
 
protected:

  QStrList m_lstToplevelDirs; 
  QList<KRegEntry> m_lstEntries;
  QList<KRegFactory> m_lstFactories;
  
  KDirWatch* m_pDirWatch;

  struct stat m_statbuff;		// buffer for stat
  
  static KRegistry *m_pSelf;      // me, myself and eye
};

#endif

// sven
