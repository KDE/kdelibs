#ifndef __interface_h__
#define __interface_h__ "$Id$"

#include "kio_connection.h"

#include <qvaluelist.h>
#include <qstringlist.h>

/**
 * @short A namespace for KIO globals
 *
 * This class is used to hold global constants and methods for KIO.
 */
class KIO
{
public:
  /**
   * Error codes that can be emitted by KIO.
   */
  enum KIOError {
    ERR_CANNOT_OPEN_FOR_READING = 1,
    ERR_CANNOT_OPEN_FOR_WRITING = 2,
    ERR_CANNOT_LAUNCH_PROCESS = 3,
    ERR_INTERNAL = 4,
    ERR_MALFORMED_URL = 5,
    ERR_UNSUPPORTED_PROTOCOL = 6,
    ERR_NO_SOURCE_PROTOCOL = 7,
    ERR_UNSUPPORTED_ACTION = 8,
    ERR_IS_DIRECTORY = 9, // ... where a file was expected
    // If one tries to get a listing of a file. This only works for directories :-)
    ERR_IS_FILE = 10,
    ERR_DOES_NOT_EXIST = 11,
    ERR_DOES_ALREADY_EXIST = 12,
    ERR_DOES_ALREADY_EXIST_FULL = 13,
    ERR_UNKNOWN_HOST = 14,
    ERR_ACCESS_DENIED = 15,
    ERR_WRITE_ACCESS_DENIED = 16,
    ERR_CANNOT_ENTER_DIRECTORY = 17,
    ERR_PROTOCOL_IS_NOT_A_FILESYSTEM = 18,
    ERR_CYCLIC_LINK = 19,
    ERR_USER_CANCELED = 20,
    ERR_CYCLIC_COPY = 21,
    ERR_COULD_NOT_CREATE_SOCKET = 22,
    ERR_COULD_NOT_CONNECT = 23,
    ERR_CONNECTION_BROKEN = 24,
    ERR_NOT_FILTER_PROTOCOL = 25,
    ERR_COULD_NOT_MOUNT = 26,
    ERR_COULD_NOT_UNMOUNT = 27,
    ERR_COULD_NOT_READ = 28,
    ERR_COULD_NOT_WRITE = 29,
    ERR_COULD_NOT_BIND = 30,
    ERR_COULD_NOT_LISTEN = 31,
    ERR_COULD_NOT_ACCEPT = 32,
    ERR_COULD_NOT_LOGIN = 33,
    ERR_COULD_NOT_STAT = 34,
    ERR_COULD_NOT_CLOSEDIR = 35,
    ERR_COULD_NOT_READSIZE = 36,
    ERR_COULD_NOT_MKDIR = 37,
    ERR_COULD_NOT_RMDIR = 38,
    ERR_CANNOT_RESUME = 39,
    ERR_CANNOT_RENAME = 40,
    ERR_CANNOT_CHMOD = 41,
    ERR_CANNOT_DELETE = 42,
    // The text argument is the protocol that the dead slave supported.
    // This means for example: file, ftp, http, ...
    ERR_SLAVE_DIED = 43,
    ERR_OUT_OF_MEMORY = 44,
    ERR_UNKNOWN_PROXY_HOST = 45,
    ERR_COULD_NOT_AUTHENTICATE = 46,
    ERR_IS_REALLY_A_WARNING = 47,
    ERR_WARNING = 47,
    ERR_INTERNAL_SERVER = 48,
    ERR_SERVER_TIMEOUT = 49,
    ERR_SERVICE_NOT_AVAILABLE = 50,
    ERR_UNKNOWN = 51,
    ERR_CHECKSUM_MISMATCH = 52
  };

  /**
   * Identifiers for KIO informational messages.
   */
  enum KIOInfo {
    INF_TOTAL_SIZE = 50,
    INF_TOTAL_COUNT_OF_FILES = 51,
    INF_TOTAL_COUNT_OF_DIRS = 52,
    INF_PROCESSED_SIZE = 53,
    INF_PROCESSED_COUNT_OF_FILES = 54,
    INF_PROCESSED_COUNT_OF_DIRS = 55,
    INF_SCANNING_DIR = 56,
    INF_SPEED = 57,
    INF_COPYING_FILE = 58,
    INF_MAKING_DIR = 59,
    INF_REDIRECTION = 60,
    INF_MIME_TYPE = 61,
    INF_ERROR_PAGE = 62,
    INF_GETTING_FILE = 63,
    INF_DELETING_FILE = 64
  };

  /**
   * Identifiers for KIO data messages.
   */
  enum KIOMessage {
    MSG_DATA = 100,
    MSG_DATA_END = 101,
    MSG_ERROR = 102,
    MSG_READY = 103,
    MSG_FINISHED = 104,
    MSG_LIST_ENTRY = 105,
    MSG_IS_DIRECTORY = 106, // Used by CMD_ISDIR
    MSG_IS_FILE = 107,      // Used by CMD_ISDIR
    MSG_RENAMED = 108,
    MSG_RESUME = 109
  };

  /**
   * Commands that can be invoked on a KIOProtocol.
   */
  enum KIOCommand {
    CMD_NONE = 0,
    CMD_GET = 1,
    CMD_COPY = 2,
    CMD_MCOPY = 3,
    CMD_MOVE = 4,
    CMD_MMOVE = 5,
    CMD_DEL = 6,
    CMD_MDEL = 7,
    CMD_LIST = 8,
    CMD_PUT = 9,
    CMD_MKDIR = 10,
    CMD_TESTDIR = 11,
    CMD_MOUNT = 12,
    CMD_UNMOUNT = 13,
    // Used to transfer the sources of an upcoming CMD_COPY call
    // This hack is needed, since a parameter can not be as big as it wants to.
    CMD_SOURCE = 14,
    CMD_GET_SIZE = 15
  };

  /**
   * Constants used to specify the type of a KUDSAtom.
   */
  enum KUDSAtomTypes {
    // First let's define the item types
    UDS_STRING = 1,
    UDS_LONG = 2,

    // Time of the file
    UDS_TIME = 4 | UDS_LONG,
    // Size of the file
    UDS_SIZE = 8 | UDS_LONG,
    // User ID of the file owner
    UDS_USER = 16 | UDS_STRING,
    // Group ID of the file owner
    UDS_GROUP =	32 | UDS_STRING,
    // Filename
    UDS_NAME = 64 | UDS_STRING,
    // Access permissions (mode returned by stat)
    UDS_ACCESS = 128 | UDS_LONG,
    // The last time the file was modified
    UDS_MODIFICATION_TIME = 256 | UDS_TIME,
    // The last time the file was opened
    UDS_ACCESS_TIME = 512 | UDS_TIME,
    // The time the file was created
    UDS_CREATION_TIME = 1024 | UDS_TIME,
    // File type, i.e. mode, as returned by stat or lstat
    // (for a link, this returns the file type of the pointed item)
    // check UDS_LINK_DEST to know if this is a link
    UDS_FILE_TYPE = 2048 | UDS_LONG,
    // Name of the file where the link points to
    // Allows to check for a symlink (don't use S_ISLNK !)
    UDS_LINK_DEST = 4096 | UDS_STRING,
    // An alternative URL (If different from the caption)
    UDS_URL = 8192 | UDS_STRING,
    // An mime type; prevents guessing
    UDS_MIME_TYPE = 16384 | UDS_STRING
  };

  /**
   * Converts an error code and a non-i18n error message into an
   * error message in the current language. The low level (non-i18n)
   * error message should be contained in the translated error message.
   */
  static QString kioErrorString( int _errid, const char *_errortext );
};

/************
 *
 * Universal Directory Service
 *
 * Any file or URL can be represented by the UDSEntry type below
 * A UDSEntry is a list of atoms
 * Each atom contains a specific bit of information for the file
 *
 * The following UDS constants represent the different possible values
 * for m_uds in the UDS atom structure below
 *
 * Each atom contains a specific bit of information for the file
 */
class KUDSAtom : public KIO
{
public:
  /**
   * Whether 'm_str' or 'm_long' is used depends on the value of 'm_uds'.
   */
  QString m_str;
  long m_long;

  /**
   * Holds one of the UDS_XXX constants
   */
  unsigned int m_uds;
};

/**
 * An entry is the list of atoms containing all the informations for a file or URL
 */
typedef QValueList<KUDSAtom> KUDSEntry;


/******************
 *
 * Wrapper
 *
 *****************/

class KIOConnectionSignals : public KIO
{
public:
  KIOConnectionSignals( KIOConnection *_conn );
  virtual ~KIOConnectionSignals() { };

  void setConnection( KIOConnection* _conn );

  ///////////
  // Command Signals
  ///////////

  virtual bool get( const char *_url );
  virtual bool getSize( const char *_url );
  /**
   * @param _mode may be -1. In this case no special permission mode is set.
   */

  virtual bool put( const char *_url, int _mode,
		    bool _overwrite, bool _resume, int _size );
  virtual bool mkdir( const char *_url, int _mode );

  virtual bool copy( const char* _source, const char *_dest );
  virtual bool copy( QStringList&_source, const char *_dest );
  virtual bool move( const char *_source, const char *_dest );
  virtual bool move( QStringList& _source, const char *_dest );
  virtual bool del( const char *_url );
  virtual bool del( QStringList& _source );

  virtual bool testDir( const char *_url );
  virtual bool listDir( const char *_url );

  virtual bool unmount( const char *_point );
  virtual bool mount( bool _ro, const char *_fstype, const char* _dev, const char *_point );

  ///////////
  // Message Signals
  ///////////
  virtual bool data( const void *, int _len );
  virtual bool dataEnd();
  virtual bool error( int _errid, const char *_text );
  virtual bool ready();
  virtual bool finished();
  virtual bool listEntry( KUDSEntry& _entry );
  virtual bool isDirectory();
  virtual bool isFile();
  virtual bool renamed( const char *_new );
  virtual bool canResume( bool _resume );

  ///////////
  // Info Signals
  ///////////

  virtual bool totalSize( unsigned long _bytes );
  virtual bool totalFiles( unsigned long _files );
  virtual bool totalDirs( unsigned long _dirs );
  virtual bool processedSize( unsigned long _bytes );
  virtual bool processedFiles( unsigned long _files );
  virtual bool processedDirs( unsigned long _dirs );
  virtual bool scanningDir( const char *_dir );
  virtual bool speed( unsigned long _bytes_per_second );
  virtual bool copyingFile( const char *_from, const char *_to );
  virtual bool makingDir( const char *_dir );
  virtual bool redirection( const char *_url );
  virtual bool errorPage();
  virtual bool mimeType( const char *_type );
  virtual bool gettingFile( const char *_url );
  virtual bool deletingFile( const char *_url );

protected:
  virtual bool source( const char *_url );

  KIOConnection *m_pConnection;
};

class KIOConnectionSlots : public KIO
{
public:
  KIOConnectionSlots( KIOConnection *_conn );
  virtual ~KIOConnectionSlots() { };

  void setConnection( KIOConnection *_conn );

  ///////////
  // Commands
  ///////////

  virtual void slotGet( const char * ) { };
  virtual void slotGetSize( const char * ) { };
  virtual void slotPut( const char *, int ,
			bool , bool , int ) { };
  virtual void slotCopy( const char* , const char * ) { };
  virtual void slotCopy( QStringList&, const char * ) { };
  virtual void slotMove( const char *, const char * ) { };
  virtual void slotMove( QStringList& , const char *) { };
  virtual void slotDel( const char * );
  virtual void slotDel( QStringList& ) { };
  virtual void slotListDir( const char * ) { };
  virtual void slotMkdir( const char *, int ) { };
  virtual void slotTestDir( const char * ) { };

  virtual void slotUnmount( const char * ) { };
  virtual void slotMount( bool, const char *, const char* , const char *) { };

  ///////////
  // Messages
  ///////////

  virtual void slotData( void *, int ) { };
  virtual void slotDataEnd() { };
  virtual void slotError( int , const char * ) { };
  virtual void slotReady() { };
  virtual void slotFinished() { };
  virtual void slotListEntry( const KUDSEntry& ) { };
  virtual void slotIsDirectory() { };
  virtual void slotIsFile() { };
  virtual void slotRenamed( const char* ) { };
  virtual void slotCanResume( bool ) { };

  ///////////
  // Infos
  ///////////

  virtual void slotTotalSize( unsigned long ) { }
  virtual void slotTotalFiles( unsigned long ) { }
  virtual void slotTotalDirs( unsigned long ) { }
  virtual void slotProcessedSize( unsigned long ) { }
  virtual void slotProcessedFiles( unsigned long ) { }
  virtual void slotProcessedDirs( unsigned long ) { }
  virtual void slotScanningDir( const char * ) { }
  virtual void slotSpeed( unsigned long ) { }
  virtual void slotCopyingFile( const char *, const char *) { }
  virtual void slotMakingDir( const char * ) { }
  virtual void slotRedirection( const char * ) { };
  virtual void slotErrorPage() { };
  virtual void slotMimeType( const char * ) { };
  virtual void slotGettingFile( const char * ) { };
  virtual void slotDeletingFile( const char * ) { };

  ///////////
  // Dispatching
  //////////

  virtual bool dispatch();
  virtual void dispatchLoop();

protected:
  virtual void source( const char ) { };


  virtual void dispatch( KIOCommand _cmd, void *_p, int _len );

  KIOConnection *m_pConnection;

  QStringList m_lstSource;
};

/**
 * @short A base class from which all KIO protocols should be derived.
 *
 * The base class of all IOProtocols.
 */
class KIOProtocol : public KIOConnectionSignals, public KIOConnectionSlots
{
public:
  KIOProtocol( KIOConnection *_conn );
  virtual ~KIOProtocol() { }

  void setConnection( KIOConnection* _conn );

  static void sigsegv_handler(int);
  static void sigchld_handler(int);
};

#ifndef NO_KIO_COMPATABILITY

/**
 * For backwards compatability, do NOT rely on the presence
 * of this typedef, convert your code to use KUDSAtom instead.
 * To check if your code is ok, try compiling with NO_KIO_COMPATABILITY
 * defined.
 */
typedef KUDSAtom UDSAtom;
typedef KIOProtocol IOProtocol;
typedef KUDSEntry UDSEntry;
typedef KIOConnection Connection;
typedef KIOConnectionSignals ConnectionSignals;
typedef KIOConnectionSlots ConnectionSlots;
typedef class KIOJobBase IOJob;
typedef class KIOSlave Slave;
typedef class KIOFilter Filter;

#warning "KIO Compability is enabled, define NO_KIO_COMPATABILTY to test your code"

#endif // NO_KIO_COMPATABILITY


#endif
