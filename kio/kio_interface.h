#ifndef __interface_h__
#define __interface_h__ "$Id$"

#include "kio_connection.h"

#include <vector>

#include <qstringlist.h>

#define CMD_NONE 0
#define CMD_GET 1
#define CMD_COPY 2
#define CMD_MCOPY 3
#define CMD_MOVE 4
#define CMD_MMOVE 5
#define CMD_DEL 6
#define CMD_MDEL 7
#define CMD_LIST 8
#define CMD_PUT 9
#define CMD_MKDIR 10
#define CMD_TESTDIR 11
#define CMD_MOUNT 12
#define CMD_UNMOUNT 13
// Used to transfer the sources of an upcoming CMD_COPY call
// This hack is needed, since a parameter can not be as big as it wants to.
#define CMD_SOURCE 14 
#define CMD_GET_SIZE 15

#define INF_TOTAL_SIZE 50
#define INF_TOTAL_COUNT_OF_FILES 51
#define INF_TOTAL_COUNT_OF_DIRS 52
#define INF_PROCESSED_SIZE 53
#define INF_PROCESSED_COUNT_OF_FILES 54
#define INF_PROCESSED_COUNT_OF_DIRS 55
#define INF_SCANNING_DIR 56
#define INF_SPEED 57
#define INF_COPYING_FILE 58
#define INF_MAKING_DIR 59
#define INF_REDIRECTION 60
#define INF_MIME_TYPE 61
#define INF_ERROR_PAGE 62
#define INF_GETTING_FILE 63
#define INF_DELETING_FILE 64

#define MSG_DATA 100
#define MSG_DATA_END 101
#define MSG_ERROR 102
#define MSG_READY 103
#define MSG_FINISHED 104
#define MSG_LIST_ENTRY 105
#define MSG_IS_DIRECTORY 106 // Used by CMD_ISDIR
#define MSG_IS_FILE 107      // Used by CMD_ISDIR
#define MSG_RENAMED 108
#define MSG_RESUME 109

#define ERR_CANNOT_OPEN_FOR_READING 1
#define ERR_CANNOT_OPEN_FOR_WRITING 2
#define ERR_CANNOT_LAUNCH_PROCESS 3
#define ERR_INTERNAL 4
#define ERR_MALFORMED_URL 5
#define ERR_UNSUPPORTED_PROTOCOL 6
#define ERR_NO_SOURCE_PROTOCOL 7
#define ERR_UNSUPPORTED_ACTION 8
#define ERR_IS_DIRECTORY 9 // ... where a file was expected
#define ERR_IS_FILE 10 // If one tries to get a listing of a file. This does only work for directories :-)
#define ERR_DOES_NOT_EXIST 11
#define ERR_DOES_ALREADY_EXIST 12
#define ERR_DOES_ALREADY_EXIST_FULL 13
#define ERR_UNKNOWN_HOST 14
#define ERR_ACCESS_DENIED 15
#define ERR_WRITE_ACCESS_DENIED 16
#define ERR_CANNOT_ENTER_DIRECTORY 17
#define ERR_PROTOCOL_IS_NOT_A_FILESYSTEM 18
#define ERR_CYCLIC_LINK 19
#define ERR_USER_CANCELED 20
#define ERR_CYCLIC_COPY 21
#define ERR_COULD_NOT_CREATE_SOCKET 22
#define ERR_COULD_NOT_CONNECT 23
#define ERR_CONNECTION_BROKEN 24
#define ERR_NOT_FILTER_PROTOCOL 25
#define ERR_COULD_NOT_MOUNT 26
#define ERR_COULD_NOT_UNMOUNT 27
#define ERR_COULD_NOT_READ 28
#define ERR_COULD_NOT_WRITE 29
#define ERR_COULD_NOT_BIND 30
#define ERR_COULD_NOT_LISTEN 31
#define ERR_COULD_NOT_ACCEPT 32
#define ERR_COULD_NOT_LOGIN 33
#define ERR_COULD_NOT_STAT 34
#define ERR_COULD_NOT_CLOSEDIR 35
#define ERR_COULD_NOT_READSIZE 36
#define ERR_COULD_NOT_MKDIR 37
#define ERR_COULD_NOT_RMDIR 38
#define ERR_CANNOT_RESUME 39
#define ERR_CANNOT_RENAME 40
#define ERR_CANNOT_CHMOD 41
#define ERR_CANNOT_DELETE 42
// The text argument is the protocol that the dead slave supported.
// This means for example: file, ftp, http, ...
#define ERR_SLAVE_DIED 43
#define ERR_OUT_OF_MEMORY 44
#define ERR_UNKNOWN_PROXY_HOST 45
#define ERR_COULD_NOT_AUTHENTICATE 46
#define ERR_IS_REALLY_A_WARNING 47
#define ERR_WARNING ERR_IS_REALLY_A_WARNING

/************
 *
 * Universal Directory Service
 *
 ************/

#define UDS_STRING 1
#define UDS_LONG 2
#define UDS_TIME ( 4 | UDS_LONG )
#define UDS_SIZE ( 8 | UDS_LONG )
#define UDS_USER ( 16 | UDS_STRING )
#define UDS_GROUP ( 32 | UDS_STRING )
#define UDS_NAME ( 64 | UDS_STRING )
#define UDS_ACCESS ( 128 | UDS_LONG )
#define UDS_MODIFICATION_TIME ( 256 | UDS_TIME )
#define UDS_ACCESS_TIME ( 512 | UDS_TIME )
#define UDS_CREATION_TIME ( 1024 | UDS_TIME )
#define UDS_FILE_TYPE ( 2048 | UDS_LONG )
#define UDS_LINK_DEST ( 4096 | UDS_STRING )

struct UDSAtom
{
  /**
   * Wether 'm_str' or 'm_long' is used depends on the value of 'm_uds'.
   */
  QString m_str;
  long m_long;
  /**
   * Holds one of the UDS_XXX constants
   */
  unsigned long m_uds;
};

typedef vector<UDSAtom> UDSEntry;

/******************
 *
 * Wrapper
 *
 *****************/

class ConnectionSignals
{
public:
  ConnectionSignals( Connection *_conn );
  virtual ~ConnectionSignals() { };

  void setConnection( Connection* _conn );
  
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
  virtual bool data( void *, int _len );
  virtual bool dataEnd();
  virtual bool error( int _errid, const char *_text );
  virtual bool ready();
  virtual bool finished();
  virtual bool listEntry( UDSEntry& _entry );
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

  Connection *m_pConnection;
};



class ConnectionSlots
{
public:
  ConnectionSlots( Connection *_conn );
  virtual ~ConnectionSlots() { };
  
  void setConnection( Connection *_conn );

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
  virtual void slotListEntry( const UDSEntry& ) { };
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

  virtual void dispatch( int _cmd, void *_p, int _len );

  Connection *m_pConnection;  

  QStringList m_lstSource;
};


class IOProtocol : public ConnectionSignals, public ConnectionSlots
{
public:
  IOProtocol( Connection *_conn );
  virtual ~IOProtocol() { }
  
  void setConnection( Connection* _conn );

  static void sigsegv_handler(int);
  static void sigchld_handler(int);

};


#endif
