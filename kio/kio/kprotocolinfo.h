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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __kprotocolinfo_h__
#define __kprotocolinfo_h__

#include <qstring.h>
#include <qstringlist.h>
#include <kurl.h>
#include <ksycocaentry.h>
#include <ksycocatype.h>

/**
 * Information about I/O (Internet, etc.) protocols supported by KDE.
 
 * This class is useful if you want to know which protocols
 * KDE supports. In addition you can find out lots of information
 * about a certain protocol. A KProtocolInfo instance represents a 
 * single protocol. Most of the functionality is provided by the static 
 m that scan the *.protocol files of all installed kioslaves to get 
 * this information.
 *
 * *.protocol files are installed in the "services" resource.
 *
 * @author Torben Weis <weis@kde.org>
 */
class KProtocolInfo : public KSycocaEntry
{
  friend class KProtocolInfoFactory;
  K_SYCOCATYPE( KST_KProtocolInfo, KSycocaEntry )

public:
  typedef KSharedPtr<KProtocolInfo> Ptr;

public:
  /**
   * Read a protocol description file
   * @param path the path of the description file
   */
  KProtocolInfo( const QString & path);

  /**
   * Returns whether the protocol description file is valid.
   * @return true if valid, false otherwise
   */
  virtual bool isValid() const { return !m_name.isEmpty(); }

  /**
   * Returns the name of the protocol.
   *
   * This corresponds to the "protocol=" field in the protocol description file.
   *
   * @return the name of the protocol
   * @see KURL::protocol()
   */  
  virtual QString name() const { return m_name; }

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
  static bool isKnownProtocol( const KURL &url );

  /**
   * Returns the library / executable to open for the protocol @p protocol
   * Example : "kio_ftp", meaning either the executable "kio_ftp" or
   * the library "kio_ftp.la" (recommended), whichever is available.
   *
   * This corresponds to the "exec=" field in the protocol description file.
   * @param protocol the protocol to check
   * @return the executable of library to open, or QString::null for
   *         unsupported protocols
   * @see KURL::protocol()
   */
  static QString exec( const QString& protocol );

  /**
   * Describes the type of a protocol. 
   */
  enum Type { T_STREAM, ///< protocol returns a stream
	      T_FILESYSTEM, ///<protocol describes location in a file system
	      T_NONE,   ///< no information about the tyope available
	      T_ERROR   ///< used to signal an error
  };

  /** 
   * Returns whether the protocol should be treated as a filesystem
   * or as a stream when reading from it.
   *
   * This corresponds to the "input=" field in the protocol description file.
   * Valid values for this field are "filesystem", "stream" or "none" (default).
   *
   * @param url the url to check
   * @return the input type of the given @p url
   */   
  static Type inputType( const KURL &url );

  /** 
   * Returns whether the protocol should be treated as a filesystem
   * or as a stream when writing to it.
   *
   * This corresponds to the "output=" field in the protocol description file.
   * Valid values for this field are "filesystem", "stream" or "none" (default).
   *
   * @param url the url to check
   * @return the output type of the given @p url
   */   
  static Type outputType( const KURL &url );

  /**
   * Returns the list of fields this protocol returns when listing
   * The current possibilities are
   * Name, Type, Size, Date, AccessDate, Access, Owner, Group, Link, URL, MimeType
   *
   * This corresponds to the "listing=" field in the protocol description file.
   * The supported fields should be seperated with ',' in the protocol description file.
   *
   * @param url the url to check
   * @return a list of field names
   */
  static QStringList listing( const KURL &url );

  /**
   * Returns whether the protocol can act as a source protocol.
   *
   * A source protocol retrieves data from or stores data to the 
   * location specified by a URL.
   * A source protocol is the opposite of a filter protocol.
   *
   * The "source=" field in the protocol description file determines
   * whether a protocol is a source protocol or a filter protocol.
   * @param url the url to check
   * @return true if the protocol is a source of data (e.g. http), false if the 
   *         protocol is a filter (e.g. gzip)
   */
  static bool isSourceProtocol( const KURL &url );

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
  static bool isHelperProtocol( const KURL &url );

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
  static bool isFilterProtocol( const KURL &url );

  /**
   * Returns whether the protocol can list files/objects. 
   * If a protocol supports listing it can be browsed in e.g. file-dialogs 
   * and konqueror.
   *
   * Whether a protocol supports listing is determined by the "listing=" 
   * field in the protocol description file.
   * If the protocol support listing it should list the fields it provides in
   * this field. If the protocol does not support listing this field should
   * remain empty (default.)
   *
   * @param url the url to check
   * @return true if the protocol support listing
   * @see listing()
   */
  static bool supportsListing( const KURL &url );

  /**
   * Returns whether the protocol can retrieve data from URLs. 
   *
   * This corresponds to the "reading=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if it is possible to read from the URL
   */
  static bool supportsReading( const KURL &url );

  /**
   * Returns whether the protocol can store data to URLs. 
   *
   * This corresponds to the "writing=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol supports writing
   */
  static bool supportsWriting( const KURL &url );

  /**
   * Returns whether the protocol can create directories/folders. 
   *
   * This corresponds to the "makedir=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol can create directories
   */
  static bool supportsMakeDir( const KURL &url );

  /**
   * Returns whether the protocol can delete files/objects. 
   *
   * This corresponds to the "deleting=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol supports deleting
   */
  static bool supportsDeleting( const KURL &url );

  /**
   * Returns whether the protocol can create links between files/objects. 
   *
   * This corresponds to the "linking=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol supports linking
   */
  static bool supportsLinking( const KURL &url );

  /**
   * Returns whether the protocol can move files/objects between different
   * locations.
   *
   * This corresponds to the "moving=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol supports moving
   */
  static bool supportsMoving( const KURL &url );

  /**
   * Returns whether the protocol can copy files/objects directly from the
   * filesystem itself. If not, the application will read files from the 
   * filesystem using the file-protocol and pass the data on to the destination
   * protocol.
   *
   * This corresponds to the "copyFromFile=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol can copy files from the local file system
   */
  static bool canCopyFromFile( const KURL &url );

  /**
   * Returns whether the protocol can copy files/objects directly to the
   * filesystem itself. If not, the application will receive the data from
   * the source protocol and store it in the filesystem using the 
   * file-protocol.
   *
   * This corresponds to the "copyToFile=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol can copy files to the local file system
   */
  static bool canCopyToFile( const KURL &url );

  /**
   * Returns default mimetype for this URL based on the protocol.
   *
   * This corresponds to the "defaultMimetype=" field in the protocol description file.
   *
   * @param url the url to check
   * @return the default mime type of the protocol, or null if unknown
   */
  static QString defaultMimetype( const KURL& url );

  /**
   * Returns the name of the icon, associated with the specified protocol.
   *
   * This corresponds to the "Icon=" field in the protocol description file. 
   *
   * @param protocol the protocol to check
   * @return the icon of the protocol, or null if unknown
   */
  static QString icon( const QString& protocol );

  /**
   * Returns the name of the config file associated with the 
   * specified protocol. This is usefull if two similar protocols
   * need to share a single config file, e.g. http and https.
   *
   * This corresponds to the "config=" field in the protocol description file.
   * The default is the protocol name, see @ref name()
   *
   * @param protocol the protocol to check
   * @return the config file, or null if unknown
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

public:
  // Internal functions:
  /**
   * @internal construct a KProtocolInfo from a stream
   */
  KProtocolInfo( QDataStream& _str, int offset);

  virtual ~KProtocolInfo();

  /**
   * @internal
   * Load the protocol info from a stream.
   */
  virtual void load(QDataStream& );

  /**
   * @internal
   * Save the protocol info to a stream.
   */
  virtual void save(QDataStream& );

  ////////////////////////// DEPRECATED /////////////////////////
  // The following methods are deprecated:

  /// @deprecated
  static Type inputType( const QString& protocol );
  /// @deprecated
  static Type outputType( const QString& protocol );
  /**
   * @deprecated
   * Returns the list of fields this protocol returns when listing
   * The current possibilities are
   * Name, Type, Size, Date, AccessDate, Access, Owner, Group, Link, URL, MimeType   
   */
  static QStringList listing( const QString& protocol );
  /// @deprecated
  static bool isSourceProtocol( const QString& protocol );
  /// @deprecated
  static bool isHelperProtocol( const QString& protocol );
  /// @deprecated
  static bool isFilterProtocol( const QString& protocol );
  /// @deprecated
  static bool isKnownProtocol( const QString& protocol );
  /// @deprecated
  static bool supportsListing( const QString& protocol );
  /// @deprecated
  static bool supportsReading( const QString& protocol );
  /// @deprecated
  static bool supportsWriting( const QString& protocol );
  /// @deprecated
  static bool supportsMakeDir( const QString& protocol );
  /// @deprecated
  static bool supportsDeleting( const QString& protocol );
  /// @deprecated
  static bool supportsLinking( const QString& protocol );
  /// @deprecated
  static bool supportsMoving( const QString& protocol );
  /// @deprecated
  static bool canCopyFromFile( const QString& protocol );
  /// @deprecated
  static bool canCopyToFile( const QString& protocol );
  /// @deprecated
  static QString defaultMimetype( const QString& protocol);
  //////////////////////// END DEPRECATED ///////////////////////

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
  QString m_defaultMimetype;
  bool m_determineMimetypeFromExtension;
  QString m_icon;
  bool m_canCopyFromFile;
  bool m_canCopyToFile;
  QString m_config;
  int m_maxSlaves;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KProtocolInfoPrivate* d;
};

#endif
