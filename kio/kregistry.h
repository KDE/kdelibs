 /*
  This file is or will be part of KDE desktop environment

  Copyright 1998 Sven Radej <sven@lisa.exp.univie.ac.at>
  Copyright 1999 Torben Weis <weis@kde.org>

  It is licensed under GPL version 2.
  
  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */


#ifndef _KREGISTRY_H
#define _KREGISTRY_H

#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <qstringlist.h>
#include <qtimer.h>
#include <qlist.h>

#include "kdirwatch.h"

#include "ksimpleconfig.h"

#define kreg KRegistry::getKRegistry

class KDirWatch;
class KRegistry;

/**
 * Base class for all entries. This base class has support
 * for the "mark & sweep" algorithm of regentry.
 */
class KRegEntry
{
public:
  KRegEntry( KRegistry* _reg, const QString& _file );
  virtual ~KRegEntry() { };
  
  /**
   * You have to overload this function. But nevertheless
   * you should call this implementation to load all properties
   * of this base class.
   */
  virtual void load( QDataStream& _str );
  /**
   * You have to overload this function. But nevertheless
   * you should call this implementation to save all properties
   * of this base class.
   */
  virtual void save( QDataStream& _str ) const;

  /**
   * Intern function.
   *
   * Mark & Sweep.
   */
  void mark() { m_bMarked = true; }
  /**
   * Intern function.
   *
   * Mark & Sweep.
   */
  void unmark() { m_bMarked = false; }
  /**
   * Intern function.
   *
   * Mark & Sweep.
   */
  bool isMarked() const { return m_bMarked; }

  /**
   * @return the file for which this entry is responsible.
   */
  QString file() const { return m_strFile; }
  
  /**
   * Intern function.
   *
   * @param _path MUST have a trailing '/'
   *
   * @return true if @ref #m_strFile is located in the directory _path.
   */
  bool isInDirectory( const QString& _path, bool _allow_subdir = false ) const;

  /**
   * Intern function.
   *
   * Checks whether @ref #m_strFile is modified. If it is modified,
   * then @ref #updateIntern is called.
   *
   * @return TRUE if the update was successful. If it returns FALSE
   *         then the instance has to be deleted and recreated.
   */
  bool update();

  virtual bool isDummy() const { return false; }
  
protected:
  /**
   * This function is called from @ref #update if @ref #m_strFile
   * changed and the entry has to be updated.
   *
   * @return false if this instance is no longer valid and has
   *         to be deleted and recreated.
   *         Returns true if it was possible to update this instance
   *         successfully. By default this function returns FALSE.
   *         That works always but smart updates are perhaps faster.
   */
  virtual bool updateIntern() { return false; }

  /**
   * Mark & Sweep
   */
  bool m_bMarked;

  /**
   * The file for which this class is responsible.
   */
  QString m_strFile;
  /**
   * Last modification time.
   */
  time_t m_ctime;
  /**
   * The registry to which we belong.
   */
  KRegistry* m_pRegistry;
};

/**
 * Abstract factory
 */
class KRegFactory
{
public:
  virtual KRegEntry* create( KRegistry* _reg, const QString& _file, KSimpleConfig &_cfg ) = 0L;
  virtual KRegEntry* create( KRegistry* _reg, const QString& _file, QDataStream& _str ) = 0L;
  /** 
   * @return the path for which this factory is responsible.  A
   *         MimeType factory for example would return
   *         "/opt/kde/share/mimelnk".  Please note that the return
   *         value may not have a trailing '/'.  
   */
  virtual const QStringList & pathList() const = 0L;

  /**
   * @return true if this factory is responsible for files in this
   *         directory.
   */
  virtual bool matchFile( const QString&_file ) const;
};

/**
 * This class is used for backup files like "~", ".bak" etc.
 * We just want to have them represented in our internal database.
 *
 * This class is internal, so you wont come in touch with it.
 */
class KRegDummy : public KRegEntry
{
public:
  KRegDummy( KRegistry* _reg, const QString& _file ) : KRegEntry( _reg, _file ) { }

  virtual bool isDummy() const { return false; }
  
protected:
  /**
   * If we change, always return this regentry with a new one => return false.
   */
  virtual bool updateIntern() { return false; }
};

/** 
 * <b>KRegistry</b> is a class for keeping a database of MimeTypes
 * and Applications (desktop entries of type 'Application').<br>
 * Database is internal - no special files are created.  The database
 * is self updating - you don't have to notify <b>KRegistry</b> if you
 * or anyone make changes to (local or global) 'applnk' or 'mimelnk
 * directories'. Updating is intelligent; only changed directories are
 * scanned and only changed items (desktop entries) are reread. This makes
 * <b>KRegistry</b> very fast.<br>
 *
 * The Usual way of using this class is to say:
 * <pre>
 * KRegistry* reg = new KRegistry;
 * reg->addFactory( .... );
 * reg->load( ... );
 * </pre>
 *
 * @short <b>KRegistry</b> is a database of all kind of desktop entry files.
 * @author Sven Radej <sven@lisa.exp.univie.ac.at>
 * @author Torben Weis <weis@kde.org> 
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
   * Destructor. Until integration with KApplication you should
   * delete the registry on app exit to avoid memory leaks.
   */
  ~KRegistry();

  /**
   * Adds a new factory. Each factory is responsible for a certain
   * directory and all of its child-directories and desktop entry files in
   * them. The registry does now control the lifetime of the factory.
   */
  void addFactory( KRegFactory *_factory );

  /**
   * Writes the registry to the specified file. If no file is
   * specified then the db file will be written to $HOME/.kde/share/config/kregistry
   *
   * @see #load
   */
  bool save( const QString& _dbfile = QString::null ) const;
  /**
   * @param _dbfile may be empty. In this case all entries are created
   *                by scanning the filesystem.
   *
   * @see #save
   */
  void load( const QString& _dbfile = QString::null );

  /** 
   * After a call to @ref #load you can use this function to find out
   * whether the loaded db file was still up to date or whether the
   * desktop entry files had changed. In the last case the registry
   * would have been modified after the db file had been loaded. That
   * in turn means that this function would return TRUE in this case.
   * Usually you will write code like this: 
   * <pre>
   * KRegistry registry;
   * registry.addFactory( new KServiceTypeFactory );
   * registry.addFactory( new KServiceFactory );
   * registry.load();
   * if ( registry.isModified() )
   *   registry.save();
   * </pre> */
  bool isModified() const { return m_bModified; }

  static KRegistry* self();
  
public slots:
    
  /**
   * Rereads specified dir (if needed). If dir is empty, rereads whole
   * registry (= all mimelnk and applnk dirs) if they need updating.
   * Please don't pass here stupid dirs like /var/log/ or /tmp. You <i>don't</i>
   * need to call this, since it is called when needed by @ref KDirWatch
   */
  void update (const QString& dir = QString::null );

protected:

  /**
   * Checks out which factory is responsible for the given path. It then causes
   * this factory to create a new entry. If no factory is responsible, then an
   * instance of @ref RegDummyEntry is created.
   */
  KRegEntry* createEntry( const QString& _file );

  KRegEntry* createEntry( QDataStream& _str, const QString& _file );
  
  /**
   * Scans _dir for new files and new subdirectories.
   *
   * @param _init All calls are just needed for updates, so
   *              _init is false. There is one exception: If a new factory
   *              is added, then the toplevel dirs for which this factory
   *              is responsible are initialized with this function and _init
   *              set to true.
   */
  bool readDirectory(const QString& _dir, bool _init = false );

  /**
   * Internal - checks if @ref #m_lstEntries contains an entry which
   *            is responsible for _file.
   *
   * @return the position of the entry in @ref #m_lstEntries or -1 if no such
   *         entry exists.
   */
  int exists( const QString& _file ) const;

protected slots:

  /**
   * Internal - @ref KDirWatch saw kill of dir. And what do I do? I delete
   * all list entries that doesn't live on disk.
   */
  void dirDeleted(const QString& path);
 
protected:

  /**
   * List of all toplevel directories we have to keep track of.
   */
  QStringList m_lstToplevelDirs;
  /**
   * List of all files we have in the registry.
   */
  QList<KRegEntry> m_lstEntries;
  /**
   * List of all factories.
   */
  QList<KRegFactory> m_lstFactories;
  
  /**
   * Pointer to the dirwatch class which tells us, when some directories
   * changed.
   */
  KDirWatch* m_pDirWatch;

  /**
   * Is always false until someone called @ref #load.
   */
  bool m_bLoaded;

  bool m_bModified;
  
  /**
   * Internal.
   */
  struct stat m_statbuff;
  
  /**
   * Pointer to ourself.
   */
  static KRegistry *m_pSelf;
};

#endif

