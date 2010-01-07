/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000-2001 Waldo Bastian <bastian@kde.org>

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
#ifndef KPROTOCOLINFO_H
#define KPROTOCOLINFO_H

#include <kglobal.h>

#include <kurl.h>
#include <ksycocaentry.h>
#include <ksycocatype.h>
#include <QtCore/QVariant>
#include <QtCore/QStringList>

class QDataStream;
class KProtocolInfoPrivate;

/**
 * \class KProtocolInfo kprotocolinfo.h <KProtocolInfo>
 *
 * Information about I/O (Internet, etc.) protocols supported by KDE.

 * This class is useful if you want to know which protocols
 * KDE supports. In addition you can find out lots of information
 * about a certain protocol. A KProtocolInfo instance represents a
 * single protocol. Most of the functionality is provided by the static
 * methods that scan the *.protocol files of all installed kioslaves to get
 * this information.
 *
 * *.protocol files are installed in the "services" resource.
 *
 * @author Torben Weis <weis@kde.org>
 */
class KDECORE_EXPORT KProtocolInfo : public KSycocaEntry
{
  friend class KProtocolInfoFactory;
  friend class KBuildProtocolInfoFactory;
  friend class KProtocolManager;
public:
  typedef KSharedPtr<KProtocolInfo> Ptr;
    typedef QList<Ptr> List;

public:

  //
  // Static functions:
  //

  /**
   * Returns list of all known protocols.
   * @return a list of all known protocols
   */
  static QStringList protocols();

  /**
   * Returns whether a protocol is installed that is able to handle @p url.
   *
   * @param url the url to check
   * @return true if the protocol is known
   * @see name()
   */
  static bool isKnownProtocol( const KUrl &url );

  /**
   * Same as above except you can supply just the protocol instead of
   * the whole URL.
   */
  static bool isKnownProtocol( const QString& protocol );

  /**
   * Returns the library / executable to open for the protocol @p protocol
   * Example : "kio_ftp", meaning either the executable "kio_ftp" or
   * the library "kio_ftp.la" (recommended), whichever is available.
   *
   * This corresponds to the "exec=" field in the protocol description file.
   * @param protocol the protocol to check
   * @return the executable of library to open, or QString() for
   *         unsupported protocols
   * @see KUrl::protocol()
   */
  static QString exec( const QString& protocol );

  /**
   * Describes the type of a protocol.
   * For instance ftp:// appears as a filesystem with folders and files,
   * while bzip2:// appears as a single file (a stream of data),
   * and telnet:// doesn't output anything.
   * @see outputType
   */
  enum Type { T_STREAM, ///< stream of data (e.g. single file)
	      T_FILESYSTEM, ///< structured directory
	      T_NONE,   ///< no information about the type available
	      T_ERROR   ///< used to signal an error
  };

  /**
   * Definition of an extra field in the UDS entries, returned by a listDir operation.
   *
   * The name is the name of the column, translated.
   *
   * The type name comes from QVariant::typeName()
   * Currently supported types: "QString", "QDateTime" (ISO-8601 format)
   */
  struct ExtraField {

    enum Type { String = QVariant::String, DateTime = QVariant::DateTime, Invalid = QVariant::Invalid };

    ExtraField() : type(Invalid) {}
    ExtraField(const QString& _name, Type _type )
      : name(_name), type(_type) {
    }
    QString name;
    Type type;
  };
  typedef QList<ExtraField> ExtraFieldList;
  /**
   * Definition of extra fields in the UDS entries, returned by a listDir operation.
   *
   * This corresponds to the "ExtraNames=" and "ExtraTypes=" fields in the protocol description file.
   * Those two lists should be separated with ',' in the protocol description file.
   * See ExtraField for details about names and types
   */
  static ExtraFieldList extraFields( const KUrl& url );

  /**
   * Returns whether the protocol can act as a helper protocol.
   * A helper protocol invokes an external application and does not return
   * a file or stream.
   *
   * This corresponds to the "helper=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol is a helper protocol (e.g. vnc), false
   *              if not (e.g. http)
   */
  static bool isHelperProtocol( const KUrl &url );

  /**
   * Same as above except you can supply just the protocol instead of
   * the whole URL.
   */
  static bool isHelperProtocol( const QString& protocol );

  /**
   * Returns whether the protocol can act as a filter protocol.
   *
   * A filter protocol can operate on data that is passed to it
   * but does not retrieve/store data itself, like gzip.
   * A filter protocol is the opposite of a source protocol.
   *
   * The "source=" field in the protocol description file determines
   * whether a protocol is a source protocol or a filter protocol.
   * Valid values for this field are "true" (default) for source protocol or
   * "false" for filter protocol.
   *
   * @param url the url to check
   * @return true if the protocol is a filter (e.g. gzip), false if the
   *         protocol is a helper or source
   */
  static bool isFilterProtocol( const KUrl &url );

  /**
   * Same as above except you can supply just the protocol instead of
   * the whole URL.
   */
  static bool isFilterProtocol( const QString& protocol );

  /**
   * Returns the name of the icon, associated with the specified protocol.
   *
   * This corresponds to the "Icon=" field in the protocol description file.
   *
   * @param protocol the protocol to check
   * @return the icon of the protocol, or an empty string if unknown
   */
  static QString icon( const QString& protocol );

  /**
   * Returns the name of the config file associated with the
   * specified protocol. This is useful if two similar protocols
   * need to share a single config file, e.g. http and https.
   *
   * This corresponds to the "config=" field in the protocol description file.
   * The default is the protocol name, see name()
   *
   * @param protocol the protocol to check
   * @return the config file, or an empty string if unknown
   */
  static QString config( const QString& protocol );

  /**
   * Returns the soft limit on the number of slaves for this protocol.
   * This limits the number of slaves used for a single operation, note
   * that multiple operations may result in a number of instances that
   * exceeds this soft limit.
   *
   * This corresponds to the "maxInstances=" field in the protocol description file.
   * The default is 1.
   *
   * @param protocol the protocol to check
   * @return the maximum number of slaves, or 1 if unknown
   */
  static int maxSlaves( const QString& protocol );


  /**
   * Returns the limit on the number of slaves for this protocol per host.
   *
   * This corresponds to the "maxInstancesPerHost=" field in the protocol description file.
   * The default is 0 which means there is no per host limit.
   *
   * @param protocol the protocol to check
   * @return the maximum number of slaves, or 1 if unknown
   *
   * @since 4.4
   */
  static int maxSlavesPerHost( const QString& protocol );

  /**
   * Returns whether mimetypes can be determined based on extension for this
   * protocol. For some protocols, e.g. http, the filename extension in the URL
   * can not be trusted to truly reflect the file type.
   *
   * This corresponds to the "determineMimetypeFromExtension=" field in the protocol description file.
   * Valid values for this field are "true" (default) or "false".
   *
   * @param protocol the protocol to check
   * @return true if the mime types can be determined by extension
   */
  static bool determineMimetypeFromExtension( const QString &protocol );

  /**
   * Returns the documentation path for the specified protocol.
   *
   * This corresponds to the "X-DocPath=" or "DocPath=" field in the protocol description file.
   *
   * @param protocol the protocol to check
   * @return the docpath of the protocol, or an empty string if unknown
   */
  static QString docPath( const QString& protocol );

  /**
   * Returns the protocol class for the specified protocol.
   *
   * This corresponds to the "Class=" field in the protocol description file.
   *
   * The following classes are defined:
   * @li ":internet" for common internet protocols
   * @li ":local" for protocols that access local resources
   *
   * Protocol classes always start with a ':' so that they can not be confused with
   * the protocols themselves.
   *
   * @param protocol the protocol to check
   * @return the class of the protocol, or an empty string if unknown
   */
  static QString protocolClass( const QString& protocol );

  /**
   * Returns whether file previews should be shown for the specified protocol.
   *
   * This corresponds to the "ShowPreviews=" field in the protocol description file.
   *
   * By default previews are shown if protocolClass is :local.
   *
   * @param protocol the protocol to check
   * @return true if previews should be shown by default, false otherwise
   */
  static bool showFilePreview( const QString& protocol );

  /**
   * Returns the suggested URI parsing mode for the KUrl parser.
   *
   * This corresponds to the "URIMode=" field in the protocol description file.
   *
   * The following parsing modes are defined:
   * @li "url" for a standards compliant URL
   * @li "rawuri" for a non-conformant URI for which URL parsing would be meaningless
   * @li "mailto" for a mailto style URI (the path part contains only an email address)
   *
   * @param protocol the protocol to check
   * @return the suggested parsing mode, or KUrl::Auto if unspecified
   */
  //static KUrl::URIMode uriParseMode( const QString& protocol ); - gone in Qt-4.x

  /**
   * Returns the list of capabilities provided by the kioslave implementing
   * this protocol.
   *
   * This corresponds to the "Capabilities=" field in the protocol description file.
   *
   * The capability names are not defined globally, they are up to each
   * slave implementation. For example when adding support for a new
   * special command for mounting, one would add the string "Mount" to the
   * capabilities list, and applications could check for that string
   * before sending a special() command that would otherwise do nothing
   * on older kioslave implementations.
   *
   * @param protocol the protocol to check
   * @return the list of capabilities.
   */
  static QStringList capabilities( const QString& protocol );

  /**
   * Returns the name of the protocol through which the request
   * will be routed if proxy support is enabled.
   *
   * A good example of this is the ftp protocol for which proxy
   * support is commonly handled by the http protocol.
   *
   * This corresponds to the "ProxiedBy=" in the protocol description file.
   */
  static QString proxiedBy( const QString& protocol );

public:
  // Internal functions:
  /**
   * @internal construct a KProtocolInfo from a stream
   */
  KProtocolInfo( QDataStream& _str, int offset);

  virtual ~KProtocolInfo();

  typedef enum { Name, FromUrl } FileNameUsedForCopying;

  /// @internal. Use KProtocolManager instead.
  bool supportsListing() const;
  /// @internal. Use KProtocolManager instead.
  QString defaultMimeType() const;
  /// @internal. Use KProtocolManager instead.
  QStringList archiveMimeTypes() const;

protected:
  QString m_name;
  QString m_exec;
  Type m_inputType;
  Type m_outputType;
  QStringList m_listing;
  bool m_isSourceProtocol;
  bool m_isHelperProtocol;
  bool m_supportsListing;
  bool m_supportsReading;
  bool m_supportsWriting;
  bool m_supportsMakeDir;
  bool m_supportsDeleting;
  bool m_supportsLinking;
  bool m_supportsMoving;
  bool m_supportsOpening;
  QString m_defaultMimetype;
  bool m_determineMimetypeFromExtension;
  QString m_icon;
  bool m_canCopyFromFile;
  bool m_canCopyToFile;
  QString m_config;
  int m_maxSlaves;

  bool canRenameFromFile() const;
  bool canRenameToFile() const;
  bool canDeleteRecursive() const;
  FileNameUsedForCopying fileNameUsedForCopying() const;

private:
  /**
   * Read a protocol description file
   * @param path the path of the description file
   */
  KProtocolInfo( const QString & path);

    Q_DECLARE_PRIVATE(KProtocolInfo)

    void load(QDataStream &s);
};

KDECORE_EXPORT QDataStream& operator>>( QDataStream& s, KProtocolInfo::ExtraField& field );
KDECORE_EXPORT QDataStream& operator<<( QDataStream& s, const KProtocolInfo::ExtraField& field );

#endif
