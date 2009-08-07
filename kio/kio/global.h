// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE libraries
   Copyright (C) 2000-2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KIO_GLOBAL_H
#define KIO_GLOBAL_H

#include <kio/kio_export.h>

#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QVariant>

#include <kiconloader.h>
#include <QtGui/QPixmap> // for pixmapForUrl

#include <sys/stat.h>  // S_ISDIR
#include <sys/types.h> // mode_t

#include <kjob.h>

#ifdef Q_OS_WIN
template class KDE_EXPORT QMap<QString, QString>;
#endif

class KUrl;
class KJobTrackerInterface;

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
   * @return converted size as a string - e.g. 123.4 KiB , 12.0 MiB
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
   * Converts size from kibi-bytes (2^10) to the string representation.
   *
   * @param  kibSize  size in kibi-bytes (2^10)
   * @return converted size as a string - e.g. 123.4 KiB , 12.0 MiB
   */
   KIO_EXPORT QString convertSizeFromKiB( KIO::filesize_t kibSize );

  /**
   * Calculates remaining time in seconds from total size, processed size and speed.
   *
   * @param  totalSize      total size in bytes
   * @param  processedSize  processed size in bytes
   * @param  speed          speed in bytes per second
   * @return calculated remaining time in seconds
   */
  KIO_EXPORT unsigned int calculateRemainingSeconds( KIO::filesize_t totalSize,
                                                     KIO::filesize_t processedSize, KIO::filesize_t speed );

  /**
   * Convert @p seconds to a string representing number of days, hours, minutes and seconds
   *
   * @param  seconds number of seconds to convert
   * @return string representation in a locale depending format
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
  KIO_EXPORT_DEPRECATED QTime calculateRemaining( KIO::filesize_t totalSize, KIO::filesize_t processedSize, KIO::filesize_t speed );

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
   * @internal
   * Commands that can be invoked by a job.
   *
   * (Move this to a non-public header)
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
    CMD_SETMODIFICATIONTIME = 'N', // 78
    CMD_REPARSECONFIGURATION = 'O', // 79
    CMD_META_DATA = 'P', // 80
    CMD_SYMLINK = 'Q', // 81
    CMD_SUBURL = 'R', // 82  Inform the slave about the url it is streaming on.
    CMD_MESSAGEBOXANSWER = 'S', // 83
    CMD_RESUMEANSWER = 'T', // 84
    CMD_CONFIG = 'U', // 85
    CMD_MULTI_GET = 'V', // 86
    CMD_SETLINKDEST = 'W', // 87
    CMD_OPEN = 'X', // 88
    CMD_CHOWN = 'Y', // 89
    CMD_READ = 'Z', // 90
    CMD_WRITE = 91,
    CMD_SEEK = 92,
    CMD_CLOSE = 93,
    CMD_HOST_INFO = 94
    // Add new ones here once a release is done, to avoid breaking binary compatibility.
    // Note that protocol-specific commands shouldn't be added here, but should use special.
  };

  /**
   * Error codes that can be emitted by KIO.
   */
  enum Error {
    ERR_CANNOT_OPEN_FOR_READING = KJob::UserDefinedError + 1,
    ERR_CANNOT_OPEN_FOR_WRITING = KJob::UserDefinedError + 2,
    ERR_CANNOT_LAUNCH_PROCESS = KJob::UserDefinedError + 3,
    ERR_INTERNAL = KJob::UserDefinedError + 4,
    ERR_MALFORMED_URL = KJob::UserDefinedError + 5,
    ERR_UNSUPPORTED_PROTOCOL = KJob::UserDefinedError + 6,
    ERR_NO_SOURCE_PROTOCOL = KJob::UserDefinedError + 7,
    ERR_UNSUPPORTED_ACTION = KJob::UserDefinedError + 8,
    ERR_IS_DIRECTORY = KJob::UserDefinedError + 9, // ... where a file was expected
    ERR_IS_FILE = KJob::UserDefinedError + 10, // ... where a directory was expected (e.g. listing)
    ERR_DOES_NOT_EXIST = KJob::UserDefinedError + 11,
    ERR_FILE_ALREADY_EXIST = KJob::UserDefinedError + 12,
    ERR_DIR_ALREADY_EXIST = KJob::UserDefinedError + 13,
    ERR_UNKNOWN_HOST = KJob::UserDefinedError + 14,
    ERR_ACCESS_DENIED = KJob::UserDefinedError + 15,
    ERR_WRITE_ACCESS_DENIED = KJob::UserDefinedError + 16,
    ERR_CANNOT_ENTER_DIRECTORY = KJob::UserDefinedError + 17,
    ERR_PROTOCOL_IS_NOT_A_FILESYSTEM = KJob::UserDefinedError + 18,
    ERR_CYCLIC_LINK = KJob::UserDefinedError + 19,
    ERR_USER_CANCELED = KJob::KilledJobError,
    ERR_CYCLIC_COPY = KJob::UserDefinedError + 21,
    ERR_COULD_NOT_CREATE_SOCKET = KJob::UserDefinedError + 22, // KDE4: s/COULD_NOT/CANNOT/ or the other way round
    ERR_COULD_NOT_CONNECT = KJob::UserDefinedError + 23,
    ERR_CONNECTION_BROKEN = KJob::UserDefinedError + 24,
    ERR_NOT_FILTER_PROTOCOL = KJob::UserDefinedError + 25,
    ERR_COULD_NOT_MOUNT = KJob::UserDefinedError + 26,
    ERR_COULD_NOT_UNMOUNT = KJob::UserDefinedError + 27,
    ERR_COULD_NOT_READ = KJob::UserDefinedError + 28,
    ERR_COULD_NOT_WRITE = KJob::UserDefinedError + 29,
    ERR_COULD_NOT_BIND = KJob::UserDefinedError + 30,
    ERR_COULD_NOT_LISTEN = KJob::UserDefinedError + 31,
    ERR_COULD_NOT_ACCEPT = KJob::UserDefinedError + 32,
    ERR_COULD_NOT_LOGIN = KJob::UserDefinedError + 33,
    ERR_COULD_NOT_STAT = KJob::UserDefinedError + 34,
    ERR_COULD_NOT_CLOSEDIR = KJob::UserDefinedError + 35,
    ERR_COULD_NOT_MKDIR = KJob::UserDefinedError + 37,
    ERR_COULD_NOT_RMDIR = KJob::UserDefinedError + 38,
    ERR_CANNOT_RESUME = KJob::UserDefinedError + 39,
    ERR_CANNOT_RENAME = KJob::UserDefinedError + 40,
    ERR_CANNOT_CHMOD = KJob::UserDefinedError + 41,
    ERR_CANNOT_DELETE = KJob::UserDefinedError + 42,
    // The text argument is the protocol that the dead slave supported.
    // This means for example: file, ftp, http, ...
    ERR_SLAVE_DIED = KJob::UserDefinedError + 43,
    ERR_OUT_OF_MEMORY = KJob::UserDefinedError + 44,
    ERR_UNKNOWN_PROXY_HOST = KJob::UserDefinedError + 45,
    ERR_COULD_NOT_AUTHENTICATE = KJob::UserDefinedError + 46,
    ERR_ABORTED = KJob::UserDefinedError + 47, // Action got aborted from application side
    ERR_INTERNAL_SERVER = KJob::UserDefinedError + 48,
    ERR_SERVER_TIMEOUT = KJob::UserDefinedError + 49,
    ERR_SERVICE_NOT_AVAILABLE = KJob::UserDefinedError + 50,
    ERR_UNKNOWN = KJob::UserDefinedError + 51,
    // (was a warning) ERR_CHECKSUM_MISMATCH = 52,
    ERR_UNKNOWN_INTERRUPT = KJob::UserDefinedError + 53,
    ERR_CANNOT_DELETE_ORIGINAL = KJob::UserDefinedError + 54,
    ERR_CANNOT_DELETE_PARTIAL = KJob::UserDefinedError + 55,
    ERR_CANNOT_RENAME_ORIGINAL = KJob::UserDefinedError + 56,
    ERR_CANNOT_RENAME_PARTIAL = KJob::UserDefinedError + 57,
    ERR_NEED_PASSWD = KJob::UserDefinedError + 58,
    ERR_CANNOT_SYMLINK = KJob::UserDefinedError + 59,
    ERR_NO_CONTENT = KJob::UserDefinedError + 60, // Action succeeded but no content will follow.
    ERR_DISK_FULL = KJob::UserDefinedError + 61,
    ERR_IDENTICAL_FILES = KJob::UserDefinedError + 62, // src==dest when moving/copying
    ERR_SLAVE_DEFINED = KJob::UserDefinedError + 63, // for slave specified errors that can be
                                                     // rich text.  Email links will be handled
                                                     // by the standard email app and all hrefs
                                                     // will be handled by the standard browser.
                                                     // <a href="exec:/khelpcenter ?" will be
                                                     // forked.
    ERR_UPGRADE_REQUIRED = KJob::UserDefinedError + 64, // A transport upgrade is required to access this
                                                        // object.  For instance, TLS is demanded by
                                                        // the server in order to continue.
    ERR_POST_DENIED = KJob::UserDefinedError + 65, // Issued when trying to POST data to a certain Ports
                                                  // see job.cpp
    ERR_COULD_NOT_SEEK = KJob::UserDefinedError + 66,
    ERR_CANNOT_SETTIME = KJob::UserDefinedError + 67, // Emitted by setModificationTime
    ERR_CANNOT_CHOWN = KJob::UserDefinedError + 68
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
                                const KUrl *reqUrl = 0L, int method = -1 );

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
                                const KUrl *reqUrl = 0L, int method = -1 );

  /**
   * Returns an appropriate error message if the given command @p cmd
   * is an unsupported action (ERR_UNSUPPORTED_ACTION).
   * @param protocol name of the protocol
   * @param cmd given command
   * @see enum Command
   */
  KIO_EXPORT QString unsupportedActionErrorString(const QString &protocol, int cmd);

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
   * Convenience method to find the pixmap for a URL.
   *
   * Call this one when you don't know the mimetype.
   *
   * @param _url URL for the file.
   * @param _mode the mode of the file. The mode may modify the icon
   *              with overlays that show special properties of the
   *              icon. Use 0 for default
   * @param _group The icon group where the icon is going to be used.
   * @param _force_size Override globally configured icon size.
   *        Use 0 for the default size
   * @param _state The icon state, one of: KIconLoader::DefaultState,
   * KIconLoader::ActiveState or KIconLoader::DisabledState.
   * @param _path Output parameter to get the full path. Seldom needed.
   *              Ignored if 0
   * @return the pixmap of the URL, can be a default icon if not found
   */
  KIO_EXPORT QPixmap pixmapForUrl( const KUrl & _url, mode_t _mode = 0, KIconLoader::Group _group = KIconLoader::Desktop,
                                   int _force_size = 0, int _state = 0, QString * _path = 0 );

  KIO_EXPORT KJobTrackerInterface *getJobTracker();


/**
 * MetaData is a simple map of key/value strings.
 */
class KIO_EXPORT MetaData : public QMap<QString, QString>
{
public:
  /**
   * Creates an empty meta data map.
   */
   MetaData() : QMap<QString, QString>() { }
  /**
   * Copy constructor.
   */
   MetaData(const QMap<QString, QString>&metaData) :
     QMap<QString, QString>(metaData) { }

   /**
    * Creates a meta data map from a QVaraint map.
    * @since 4.3.1
    */
    MetaData(const QMap<QString,QVariant> &);

   /**
    * Adds the given meta data map to this map.
    * @param metaData the map to add
    * @return this map
    */
   MetaData & operator += ( const QMap<QString,QString> &metaData )
   {
      QMap<QString,QString>::ConstIterator it;
      for(it = metaData.constBegin(); it !=  metaData.constEnd(); ++it)
      {
         insert(it.key(), it.value());
      }
      return *this;
   }

   /**
    * Same as above except the value in the map is a QVariant.
    *
    * This convenience function allows you to easily assign the values
    * of a QVariant to this meta data class.
    *
    * @param metaData the map to add
    * @return this map
    * @since 4.3.1
    */
   MetaData & operator += ( const QMap<QString,QVariant> &metaData );

   /**
    * Sets the given meta data map to this map.
    * @param metaData the map to add
    * @return this map
    * @since 4.3.1
    */
   MetaData & operator = ( const QMap<QString,QVariant> &metaData );

   /**
    * Returns the contents of the map as a QVariant.
    *
    * @return a QVariant representation of the meta data map.
    * @since 4.3.1
    */
   QVariant toVariant() const;
};

}
#endif
