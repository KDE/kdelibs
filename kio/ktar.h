#include <sys/stat.h>
#include <sys/types.h>       
#include <zlib.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdict.h>

class KTarDirectory;
class KTarFile;

/**
 * @short A class for reading/writing gzipped tar balls.
 * @author Torben Weis <weis@kde.org>
 */
class KTar
{
public:
  /**
   * Creates an instance that operates on the given filename.
   *
   * @param filename has the format "/home/weis/myfile.tgz" or something
   *        like that.
   *
   * @see #open
   */
  KTar( const QString& filename );
  /**
   * If the tar ball is still opened, then it will be
   * closed automatically by the destructor.
   */
  ~KTar();

  /**
   * Opens the tar file for reading or writing.
   *
   * @param mode may be IO_ReadOnly or IO_WriteOnly
   *
   * @see #close
   */
  bool open( int mode );
  /**
   * Closes the tar file.
   *
   * @see #open
   */
  void close();

  /**
   * @return true if the file is opened
   */
  bool isOpened() const { return m_open; }

  /**
   * If a tar file is opened for writing then you can add new directories
   * using this function. KTar won't write one directory twice.
   */
  void writeDir( const QString& name, const QString& user, const QString& group );
  /**
   * If a tar file is opened for writing then you can add a new file
   * using this function. If the file name is for example "mydir/test1" then
   * the directory "mydir" is automatically appended first if that did not
   * happen yet.
   */
  void writeFile( const QString& name, const QString& user, const QString& group, uint size, const char* data );

  /**
   * If a tar file is opened for reading, then the contents
   * of the file can be accessed via this function.
   */
  const KTarDirectory* directory() const;

private:
  /** @internal
   * Ensure path exists, create otherwise.
   * This handles tar files missing directory entries, like mico-2.3.0.tar.gz :)
   */
  KTarDirectory * findOrCreate( const QString & path );
  
  /** @internal
   * Fills the buffer as required by the tar format
   * Has to be called LAST, since it does the checksum
   * (normally, only the name has to be filled in before)
   * @param mode is expected to be 6 chars long, [uname and gname 31].
   */
  void fillBuffer( char * buffer, const char * mode, int size, char typeflag, const char * uname, const char * gname );
  
  gzFile m_f;
  bool m_open;
  char m_mode;
  KTarDirectory* m_dir;
  QString m_filename;
  QStringList m_dirList;
};

/**
 * @short Base class for the tar-file's directory structure.
 *
 * @see KTar
 * @see KTarFile
 * @see KTarDirectory
 */
class KTarEntry
{
public:
  KTarEntry( KTar*, const QString& name, int access, int date,
	     const QString& user, const QString& group );

  virtual ~KTarEntry() { }

  /**
   * Creation date of the file.
   */
  QDateTime datetime() const;
  int date() const { return m_date; }

  /**
   * Name of the file without path.
   */
  QString name() const { return m_name; }
  /**
   * The permissions and mode flags as returned by the stat() function
   * in st_mode.
   */
  mode_t permissions() const { return m_access; }
  /**
   * User who created the file.
   */
  QString user() const { return m_user; }
  /**
   * Group of the user who created the file.
   */
  QString group() const { return m_group; }

  /**
   * @return true if this entry is a file
   */
  virtual bool isFile() const { return false; }
  /**
   * @return true if this entry is a directory
   */
  virtual bool isDirectory() const { return false; }

protected:
  KTar* tar() { return m_tar; }

private:
  QString m_name;
  int m_date;
  mode_t m_access;
  QString m_user;
  QString m_group;
  KTar* m_tar;
};

/**
 * @short A file in a tar-file's directory structure.
 *
 * @see KTar
 * @see KTarDirectory
 */
class KTarFile : public KTarEntry
{
public:
  KTarFile( KTar*, const QString& name, int access, int date,
	    const QString& user, const QString& group,
	    int pos, int size, const QByteArray& data );

  virtual ~KTarFile() { }

  /**
   * Position of the data in the uncompressed tar file.
   */
  int position() const;
  /**
   * Size of the data.
   */
  int size() const;

  /**
   * @return the content of this file.
   */
  QByteArray data() const;

  /**
   * @return true, since this entry is a file
   */
  virtual bool isFile() const { return true; }

private:
  int m_pos;
  int m_size;
  QByteArray m_data;
};

/**
 * @short A directory in a tar-file's directory structure.
 *
 * @see KTar
 * @see KTarFile
 */
class KTarDirectory : public KTarEntry
{
public:
  KTarDirectory( KTar*, const QString& name, int access, int date,
		 const QString& user, const QString& group );

  virtual ~KTarDirectory() { }

  /**
   * @return the names of all entries in this directory (filenames, no path).
   */
  QStringList entries() const;
  /**
   * @return a pointer to the entry in the directory.
   *
   * @param name may be "test1", "mydir/test3", "mydir/mysubdir/test3", etc.
   */
  KTarEntry* entry( QString name );
  const KTarEntry* entry( QString name ) const;

  /**
   * @internal
   * Adds a new entry to the directory.
   */
  void addEntry( KTarEntry* );

  /**
   * @return true, since this entry is a directory
   */
  virtual bool isDirectory() const { return true; }

private:
  QDict<KTarEntry> m_entries;
};
