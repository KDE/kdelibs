/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef __kio_global_h__
#define __kio_global_h__

#include <kdelibs_export.h>

#include <qstring.h>
#include <q3valuelist.h>
#include <qmap.h>

class KURL;

class QTime;

/**
 * @short A namespace for KIO globals
 *
 */
namespace KIO
{
  /// 64-bit file offset
  typedef qlonglong fileoffset_t;
  /// 64-bit file size
  typedef qulonglong filesize_t;

  /**
   * Converts @p size from bytes to the string representation.
   *
   * @param  size  size in bytes
   * @return converted size as a string - e.g. 123.4 kB , 12.0 MB
   */
  KIO_EXPORT QString convertSize( KIO::filesize_t size );

  /**
   * Converts a size to a string representation
   * Not unlike QString::number(...)
   *
   * @param size size in bytes
   * @return  converted size as a string - e.g. 123456789
   */
  KIO_EXPORT QString number( KIO::filesize_t size );

  /**
   * Converts size from kilo-bytes to the string representation.
   *
   * @param  kbSize  size in kilo-bytes
   * @return converted size as a string - e.g. 123.4 kB , 12.0 MB
   */
   KIO_EXPORT QString convertSizeFromKB( KIO::filesize_t kbSize );

  /**
   * Calculates remaining time in seconds from total size, processed size and speed.
   *
   * @param  totalSize      total size in bytes
   * @param  processedSize  processed size in bytes
   * @param  speed          speed in bytes per second
   * @return calculated remaining time in seconds
   *
   * @since 3.4
   */
  KIO_EXPORT unsigned int calculateRemainingSeconds( KIO::filesize_t totalSize,
                                                     KIO::filesize_t processedSize, KIO::filesize_t speed );

  /**
   * Convert @p seconds to a string representing number of days, hours, minutes and seconds
   *
   * @param  seconds number of seconds to convert
   * @return string representation in a locale depending format
   *
   * @since 3.4
   */
  KIO_EXPORT QString convertSeconds( unsigned int seconds );

  /**
   * Calculates remaining time from total size, processed size and speed.
   * Warning: As QTime is limited to 23:59:59, use calculateRemainingSeconds() instead
   *
   * @param  totalSize      total size in bytes
   * @param  processedSize  processed size in bytes
   * @param  speed          speed in bytes per second
   * @return calculated remaining time
   */
  KIO_EXPORT QTime calculateRemaining( KIO::filesize_t totalSize, KIO::filesize_t processedSize, KIO::filesize_t speed ) KDE_DEPRECATED;

  /**
   * Helper for showing information about a set of files and directories
   * @param items the number of items (= @p files + @p dirs + number of symlinks :)
   * @param files the number of files
   * @param dirs the number of dirs
   * @param size the sum of the size of the @p files
   * @param showSize whether to show the size in the result
   * @return the summary string
   */
  KIO_EXPORT QString itemsSummaryString(uint items, uint files, uint dirs, KIO::filesize_t size, bool showSize);

  /**
   * Encodes (from the text displayed to the real filename)
   * This translates % into %% and / into %2f
   * Used by KIO::link, for instance.
   * @param str the file name to encode
   * @return the encoded file name
   */
  KIO_EXPORT QString encodeFileName( const QString & str );
  /**
   * Decodes (from the filename to the text displayed)
   * This translates %2[fF] into / and %% into %
   * @param str the file name to decode
   * @return the decoded file name
   */
  KIO_EXPORT QString decodeFileName( const QString & str );

  /**
   * Commands that can be invoked by a job.
   */
  enum Command {
    CMD_HOST = '0', // 48
    CMD_CONNECT = '1', // 49
    CMD_DISCONNECT = '2', // 50
    CMD_SLAVE_STATUS = '3', // 51
    CMD_SLAVE_CONNECT = '4', // 52
    CMD_SLAVE_HOLD = '5', // 53
    CMD_NONE = 'A', // 65
    CMD_TESTDIR = 'B', // 66
    CMD_GET = 'C', // 67
    CMD_PUT = 'D', // 68
    CMD_STAT = 'E', // 69
    CMD_MIMETYPE = 'F', // 70
    CMD_LISTDIR = 'G', // 71
    CMD_MKDIR = 'H', // 72
    CMD_RENAME = 'I', // 73
    CMD_COPY = 'J', // 74
    CMD_DEL = 'K', // 75
    CMD_CHMOD = 'L', // 76
    CMD_SPECIAL = 'M', // 77
    CMD_USERPASS = 'N', // 78
    CMD_REPARSECONFIGURATION = 'O', // 79
    CMD_META_DATA = 'P', // 80
    CMD_SYMLINK = 'Q', // 81
    CMD_SUBURL = 'R', // 82  Inform the slave about the url it is streaming on.
    CMD_MESSAGEBOXANSWER = 'S', // 83
    CMD_RESUMEANSWER = 'T', // 84
    CMD_CONFIG = 'U', // 85
    CMD_MULTI_GET = 'V' // 86
    // Add new ones here once a release is done, to avoid breaking binary compatibility.
    // Note that protocol-specific commands shouldn't be added here, but should use special.
  };

  /**
   * Error codes that can be emitted by KIO.
   */
  enum Error {
    ERR_CANNOT_OPEN_FOR_READING = 1,
    ERR_CANNOT_OPEN_FOR_WRITING = 2,
    ERR_CANNOT_LAUNCH_PROCESS = 3,
    ERR_INTERNAL = 4,
    ERR_MALFORMED_URL = 5,
    ERR_UNSUPPORTED_PROTOCOL = 6,
    ERR_NO_SOURCE_PROTOCOL = 7,
    ERR_UNSUPPORTED_ACTION = 8,
    ERR_IS_DIRECTORY = 9, // ... where a file was expected
    ERR_IS_FILE = 10, // ... where a directory was expected (e.g. listing)
    ERR_DOES_NOT_EXIST = 11,
    ERR_FILE_ALREADY_EXIST = 12,
    ERR_DIR_ALREADY_EXIST = 13,
    ERR_UNKNOWN_HOST = 14,
    ERR_ACCESS_DENIED = 15,
    ERR_WRITE_ACCESS_DENIED = 16,
    ERR_CANNOT_ENTER_DIRECTORY = 17,
    ERR_PROTOCOL_IS_NOT_A_FILESYSTEM = 18,
    ERR_CYCLIC_LINK = 19,
    ERR_USER_CANCELED = 20,
    ERR_CYCLIC_COPY = 21,
    ERR_COULD_NOT_CREATE_SOCKET = 22, // KDE4: s/COULD_NOT/CANNOT/ or the other way round
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
    ERR_ABORTED = 47, // Action got aborted from application side
    ERR_INTERNAL_SERVER = 48,
    ERR_SERVER_TIMEOUT = 49,
    ERR_SERVICE_NOT_AVAILABLE = 50,
    ERR_UNKNOWN = 51,
    // (was a warning) ERR_CHECKSUM_MISMATCH = 52,
    ERR_UNKNOWN_INTERRUPT = 53,
    ERR_CANNOT_DELETE_ORIGINAL = 54,
    ERR_CANNOT_DELETE_PARTIAL = 55,
    ERR_CANNOT_RENAME_ORIGINAL = 56,
    ERR_CANNOT_RENAME_PARTIAL = 57,
    ERR_NEED_PASSWD = 58,
    ERR_CANNOT_SYMLINK = 59,
    ERR_NO_CONTENT = 60, // Action succeeded but no content will follow.
    ERR_DISK_FULL = 61,
    ERR_IDENTICAL_FILES = 62, // src==dest when moving/copying
    ERR_SLAVE_DEFINED = 63, // for slave specified errors that can be
                            // rich text.  Email links will be handled
                            // by the standard email app and all hrefs
                            // will be handled by the standard browser.
                            // <a href="exec:/khelpcenter ?" will be
                            // forked.
    ERR_UPGRADE_REQUIRED = 64, // A transport upgrade is required to access this
                               // object.  For instance, TLS is demanded by
                               // the server in order to continue.
    ERR_POST_DENIED = 65    // Issued when trying to POST data to a certain Ports
                               // see job.cpp
  };

  /**
   * Returns a translated error message for @p errorCode using the
   * additional error information provided by @p errorText.
   * @param errorCode the error code
   * @param errorText the additional error text
   * @return the created error string
   */
  KIO_EXPORT QString buildErrorString(int errorCode, const QString &errorText);

  /**
   * Returns a translated html error message for @p errorCode using the
   * additional error information provided by @p errorText , @p reqUrl
   * (the request URL), and the ioslave @p method .
   * @param errorCode the error code
   * @param errorText the additional error text
   * @param reqUrl the request URL
   * @param method the ioslave method
   * @return the created error string
   */
  KIO_EXPORT QString buildHTMLErrorString(int errorCode, const QString &errorText,
                                const KURL *reqUrl = 0L, int method = -1 );

  /**
   * Returns translated error details for @p errorCode using the
   * additional error information provided by @p errorText , @p reqUrl
   * (the request URL), and the ioslave @p method .
   *
   * @param errorCode the error code
   * @param errorText the additional error text
   * @param reqUrl the request URL
   * @param method the ioslave method
   * @return the following data:
   * @li QString errorName - the name of the error
   * @li QString techName - if not null, the more technical name of the error
   * @li QString description - a description of the error
   * @li QStringList causes - a list of possible causes of the error
   * @li QStringList solutions - a liso of solutions for the error
   */
  KIO_EXPORT QByteArray rawErrorDetail(int errorCode, const QString &errorText,
                                const KURL *reqUrl = 0L, int method = -1 );

  /**
   * Returns an appropriate error message if the given command @p cmd
   * is an unsupported action (ERR_UNSUPPORTED_ACTION).
   * @param protocol name of the protocol
   * @param cmd given command
   * @see enum Command
   * @since 3.2
   */
  KIO_EXPORT QString unsupportedActionErrorString(const QString &protocol, int cmd);

  /**
   * Constants used to specify the type of a KUDSAtom.
   */
  enum UDSAtomTypes {
    /// First let's define the item types
    UDS_STRING = 1,
    UDS_LONG = 2,
    UDS_TIME = 4 | UDS_LONG,

    // To add new UDS entries below, you can use a step of 8
    // (i.e. 8, 16, 24, 32, etc.) Only the last 3 bits are a bitfield,
    // the rest isn't.

    /// Size of the file
    UDS_SIZE = 8 | UDS_LONG,
    UDS_SIZE_LARGE = 32768 | UDS_LONG, // For internal use only
    /// User ID of the file owner
    UDS_USER = 16 | UDS_STRING,
    /// Name of the icon, that should be used for displaying.
    /// It overrides all other detection mechanisms
    /// @since 3.2
    UDS_ICON_NAME = 24 | UDS_STRING,
    /// Group ID of the file owner
    UDS_GROUP =	32 | UDS_STRING,
    /// Extra data (used only if you specified Columns/ColumnsTypes)
    /// This is the only UDS entry that can be repeated.
    /// @since 3.2
    UDS_EXTRA = 48 | UDS_STRING,
    /// Filename - as displayed in directory listings etc.
    /// "." has the usual special meaning of "current directory"
    UDS_NAME = 64 | UDS_STRING,
    /// A local file path if the ioslave display files sitting
    /// on the local filesystem (but in another hierarchy, e.g. media:/)
    UDS_LOCAL_PATH = 72 | UDS_STRING,
    /// Treat the file as a hidden file or as a normal file,
    /// regardless of (the absence of) a leading dot in the filename.
    UDS_HIDDEN = 80 | UDS_LONG,
    /// Indicates that the entry has extended ACL entries
    /// @since 3.5
    UDS_EXTENDED_ACL = 88 | UDS_LONG,
    /// The access control list serialized into a single string.
    /// @since 3.5
    UDS_ACL_STRING = 96 | UDS_STRING,
    /// The default access control list serialized into a single string.
    /// Only available for directories.
    /// @since 3.5
    UDS_DEFAULT_ACL_STRING = 104 | UDS_STRING,

    // available: 112, 120 

    /// Access permissions (part of the mode returned by stat)
    UDS_ACCESS = 128 | UDS_LONG,
    /// The last time the file was modified
    UDS_MODIFICATION_TIME = 256 | UDS_TIME,
    /// The last time the file was opened
    UDS_ACCESS_TIME = 512 | UDS_TIME,
    /// The time the file was created
    UDS_CREATION_TIME = 1024 | UDS_TIME,
    /// File type, part of the mode returned by stat
    /// (for a link, this returns the file type of the pointed item)
    /// check UDS_LINK_DEST to know if this is a link
    UDS_FILE_TYPE = 2048 | UDS_LONG,
    /// Name of the file where the link points to
    /// Allows to check for a symlink (don't use S_ISLNK !)
    UDS_LINK_DEST = 4096 | UDS_STRING,
    /// An alternative URL (If different from the caption)
    UDS_URL = 8192 | UDS_STRING,
    /// A mime type; prevents guessing
    UDS_MIME_TYPE = 16384 | UDS_STRING,
    /// A mime type to be used for displaying only.
    /// But when 'running' the file, the mimetype is re-determined
    UDS_GUESSED_MIME_TYPE = 16392 | UDS_STRING,
    /// XML properties, e.g. for WebDAV
    /// @since 3.1
    UDS_XML_PROPERTIES = 32768 | UDS_STRING
  };

  /**
   * Specifies how to use the cache.
   * @see parseCacheControl()
   * @see getCacheControlString()
   */
  enum CacheControl
  {
      CC_CacheOnly, ///< Fail request if not in cache
      CC_Cache,     ///< Use cached entry if available
      CC_Verify,    ///< Validate cached entry with remote site if expired
      CC_Refresh,   ///< Always validate cached entry with remote site
                    ///< @since 3.1
      CC_Reload     ///< Always fetch from remote site.
  };

  /**
   * Parses the string representation of the cache control option.
   *
   * @param cacheControl the string representation
   * @return the cache control value
   * @see getCacheControlString()
   */
  KIO_EXPORT KIO::CacheControl parseCacheControl(const QString &cacheControl);

  /**
   * Returns a string representation of the given cache control method.
   *
   * @param cacheControl the cache control method
   * @return the string representation
   * @see parseCacheControl()
   */
  KIO_EXPORT QString getCacheControlString(KIO::CacheControl cacheControl);

  /**
   * Returns the mount point where @p device is mounted
   * right now. This means, it has to be mounted, not just
   * defined in fstab.
   */
  KIO_EXPORT QString findDeviceMountPoint( const QString& device );

  /**
   * Returns the mount point on which resides @p filename.
   * For instance if /home is a separate partition, findPathMountPoint("/home/user/blah")
   * will return /home
   * @param filename the file name to check
   * @return the mount point of the given @p filename
   */
  KIO_EXPORT QString findPathMountPoint( const QString & filename );

  /**
   * Checks if the path belongs to a filesystem that is probably
   * slow. It checks for NFS or for paths belonging to automounted
   * paths not yet mounted
   * @param filename the file name to check
   * @return true if the filesystem is probably slow
   */
  KIO_EXPORT bool probably_slow_mounted(const QString& filename);

  /**
   * Checks if the path belongs to a filesystem that is manually
   * mounted.
   * @param filename the file name to check
   * @return true if the filesystem is manually mounted
   */
  KIO_EXPORT bool manually_mounted(const QString& filename);

  enum FileSystemFlag { SupportsChmod, SupportsChown, SupportsUTime,
                        SupportsSymlinks, CaseInsensitive };
  /**
   * Checks the capabilities of the filesystem to which a given file belongs.
   * given feature (e.g. chmod).
   * @param filename the file name to check
   * @param flag the flag to check
   * @return true if the filesystem has that flag, false if not (or some error occurred)
   *
   * The availables flags are:
   * @li SupportsChmod: returns true if the filesystem supports chmod
   * (e.g. msdos filesystems return false)
   * @li SupportsChown: returns true if the filesystem supports chown
   * (e.g. msdos filesystems return false)
   * @li SupportsUtime: returns true if the filesystems supports utime
   * (e.g. msdos filesystems return false)
   * @li SupportsSymlinks: returns true if the filesystems supports symlinks
   * (e.g. msdos filesystems return false)
   * @li CaseInsensitive: returns true if the filesystem treats
   * "foo" and "FOO" as being the same file (true for msdos systems)
   *
   */
  KIO_EXPORT bool testFileSystemFlag(const QString& filename, FileSystemFlag flag);


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
class KIO_EXPORT UDSAtom
{
public:
  /**
   * Whether 'm_str' or 'm_long' is used depends on the value of 'm_uds'.
   */
  QString m_str;
  /**
   * Whether 'm_str' or 'm_long' is used depends on the value of 'm_uds'.
   */
  long long m_long;

  /**
   * Holds one of the UDS_XXX constants
   */
  unsigned int m_uds;
};

/**
 * An entry is the list of atoms containing all the informations for a file or URL
 */
typedef Q3ValueList<UDSAtom> UDSEntry;
typedef Q3ValueList<UDSEntry> UDSEntryList;
typedef Q3ValueListIterator<UDSEntry> UDSEntryListIterator;
typedef Q3ValueListConstIterator<UDSEntry> UDSEntryListConstIterator;

////// KDE4 TODO: m_uds is the key, QVariant would do the job as the value (to allow QDateTime etc.).
//////  -> remove UDSAtom, turn UDSEntry into QHash<uint, QVariant>, and UDSEntryList can be a QList.
////// KDE4 TODO: rename UDSAtomTypes to UDSFieldType or something (no more atoms).
////// The word UDS isn't really well-known either... ListEntry isn't really good for stat() result though...

/**
 * UDS entry is the data structure representing all the information about a given URL
 * (file or directory).
 *
 * The KIO::listDir() and KIO:stat() operations use this data structure.
 *
 * KIO defines a number of fields, see the UDS_XXX enums.
 *
 * For instance, to retrieve the name of the entry, use:
 * \code
 * QString displayName = entry.value( KIO::UDS_NAME ).toString();
 * \endcode
 */
typedef QHash<uint, QVariant> UDSEntry4;

/**
 * A list of UDS entries, as returned by KIO::listDir()
 */
typedef QList<UDSEntry4> UDSEntryList4;

/**
 * MetaData is a simple map of key/value strings.
 */
class KIO_EXPORT MetaData : public QMap<QString, QString>
{
public:
  /**
   * Creates an empty meta data map.
   */
   MetaData() : QMap<QString, QString>() { };
  /**
   * Copy constructor.
   */
   MetaData(const QMap<QString, QString>&metaData) :
     QMap<QString, QString>(metaData) { };

   /**
    * Adds the given meta data map to this map.
    * @param metaData the map to add
    * @return this map
    */
   MetaData & operator+= ( const QMap<QString,QString> &metaData )
   {
      QMap<QString,QString>::ConstIterator it;
      for( it = metaData.begin();
           it !=  metaData.end();
           ++it)
      {
         QMap<QString,QString>::Iterator match = find(it.key());
         if (match != end()) {
           insert(it.key(), it.value());
         }
      }
      return *this;
   }
};

}
#endif
