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
 *
 * This class is useful if you want to know which protocols
 * KDE supports. In addition you can find out lots of information
 * about a certain protocol. KProtocolInfo scans the *.protocol
 * files of all installed kioslaves to get this information.
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
   */
  KProtocolInfo( const QString & path);

  /**
   * @returns whether the protocol description file is valid.
   */
  virtual bool isValid() const { return !m_name.isEmpty(); }

  /**
   * @returns the name of the protocol.
   *
   * This corresponds to the "protocol=" field in the protocol description file.
   *
   * @see KURL::protocol()
   */  
  virtual QString name() const { return m_name; }

  //
  // Static functions:
  //

  /**
   * @returns list of all known protocols
   */
  static QStringList protocols();

  /**
   * @returns whether a protocol is installed that is able to handle @p url.
   *
   * @see name()
   */
  static bool isKnownProtocol( const KURL &url );

  /**
   * @returns the library / executable to open for the protocol @p protocol
   * Example : "kio_ftp", meaning either the executable "kio_ftp" or
   * the library "kio_ftp.la" (recommended), whichever is available.
   *
   * This corresponds to the "exec=" field in the protocol description file.
   *
   */
  static QString exec( const QString& protocol );

  enum Type { T_STREAM, T_FILESYSTEM, T_NONE, T_ERROR };
  /** 
   * @returns whether the protocol should be treated as a filesystem
   * or as a stream when reading from it.
   *
   * This corresponds to the "input=" field in the protocol description file.
   * Valid values for this field are "filesystem", "stream" or "none" (default).
   *
   */   
  static Type inputType( const KURL &url );

  /** 
   * @returns whether the protocol should be treated as a filesystem
   * or as a stream when writing to it.
   *
   * This corresponds to the "output=" field in the protocol description file.
   * Valid values for this field are "filesystem", "stream" or "none" (default).
   *
   */   
  static Type outputType( const KURL &url );

  /**
   * @returns the list of fields this protocol returns when listing
   * The current possibilities are
   * Name, Type, Size, Date, AccessDate, Access, Owner, Group, Link, URL, MimeType
   *
   * This corresponds to the "listing=" field in the protocol description file.
   * The supported fields should be seperated with ',' in the protocol description file.
   */
  static QStringList listing( const KURL &url );

  /**
   * @returns whether the protocol can act as a source protocol.
   * TODO: What does that mean? Obsolete?
   * A source protocol retrieves data from or stores data to the 
   * location specified by a URL.
   * A source protocol is the opposite of a filter protocol.
   *
   * The "source=" field in the protocol description file determines
   * whether a protocol is a source protocol or a filter protocol.
   * Valid values for this field are "true" (default) for source protocol or 
   * "false" for filter protocol.
   */
  static bool isSourceProtocol( const KURL &url );

  /**
   * @returns whether the protocol can act as a helper protocol.
   * TODO: What does that mean?
   *
   * This corresponds to the "helper=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   */
  static bool isHelperProtocol( const KURL &url );

  /**
   * @returns whether the protocol can act as a filter protocol.
   * TODO: What does that mean?
   * A filter protocol can operate on data that is passed to it 
   * but does not retrieve/store data itself.
   * A filter protocol is the opposite of a source protocol.
   *
   * The "source=" field in the protocol description file determines
   * whether a protocol is a source protocol or a filter protocol.
   * Valid values for this field are "true" (default) for source protocol or 
   * "false" for filter protocol.
   */
  static bool isFilterProtocol( const KURL &url );

  /**
   * @returns whether the protocol can list files/objects. 
   * If a protocol supports listing it can be browsed in e.g. file-dialogs 
   * and konqueror.
   *
   * Whether a protocol supports listing is determined by the "listing=" 
   * field in the protocol description file.
   * If the protocol support listing it should list the fields it provides in
   * this field. If the protocol does not support listing this field should
   * remain empty (default.)
   * @see listing()
   */
  static bool supportsListing( const KURL &url );

  /**
   * @returns whether the protocol can retrieve data from URLs. 
   *
   * This corresponds to the "reading=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   */
  static bool supportsReading( const KURL &url );

  /**
   * @returns whether the protocol can store data to URLs. 
   *
   * This corresponds to the "writing=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   */
  static bool supportsWriting( const KURL &url );

  /**
   * @returns whether the protocol can create directories/folders. 
   *
   * This corresponds to the "makedir=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   */
  static bool supportsMakeDir( const KURL &url );

  /**
   * @returns whether the protocol can delete files/objects. 
   *
   * This corresponds to the "deleting=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   */
  static bool supportsDeleting( const KURL &url );

  /**
   * @returns whether the protocol can create links between files/objects. 
   *
   * This corresponds to the "linking=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   */
  static bool supportsLinking( const KURL &url );

  /**
   * @returns whether the protocol can move files/objects between different
   * locations.
   *
   * This corresponds to the "moving=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   */
  static bool supportsMoving( const KURL &url );

  /**
   * @returns whether the protocol can copy files/objects directly from the
   * filesystem itself. If not, the application will read files from the 
   * filesystem using the file-protocol and pass the data on to the destination
   * protocol.
   *
   * This corresponds to the "copyFromFile=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   */
  static bool canCopyFromFile( const KURL &url );

  /**
   * @returns whether the protocol can copy files/objects directly to the
   * filesystem itself. If not, the application will receive the data from
   * the source protocol and store it in the filesystem using the 
   * file-protocol.
   *
   * This corresponds to the "copyToFile=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   */
  static bool canCopyToFile( const KURL &url );

  /**
   * @returns default mimetype for this URL based on the protocol
   *
   * This corresponds to the "defaultMimetype=" field in the protocol description file.
   */
  static QString defaultMimetype( const KURL& url );

  /**
   * @returns the name of the icon, associated with the specified protocol
   *
   * This corresponds to the "Icon=" field in the protocol description file.
   */
  static QString icon( const QString& protocol );

  /**
   * @returns the name of the config file associated with the 
   * specified protocol. This is usefull if two similar protocols
   * need to share a single config file, e.g. http and https.
   *
   * This corresponds to the "config=" field in the protocol description file.
   * The default is the protocol name, see @ref name()
   */
  static QString config( const QString& protocol );

  /**
   * @returns the soft limit on the number of slaves for this protocol.
   * This limits the number of slaves used for a single operation, note
   * that multiple operations may result in a number of instances that 
   * exceeds this soft limit.
   *
   * This corresponds to the "maxInstances=" field in the protocol description file.
   * The default is 1.
   */
  static int maxSlaves( const QString& protocol );

  /**
   * @returns whether mimetypes can be determined based on extension for this
   * protocol. For some protocols, e.g. http, the filename extension in the URL
   * can not be trusted to truly reflect the file type.
   *
   * This corresponds to the "determineMimetypeFromExtension=" field in the protocol description file.
   * Valid values for this field are "true" (default) or "false".
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
  
  static Type inputType( const QString& protocol );
  static Type outputType( const QString& protocol );
  /**
   * @returns the list of fields this protocol returns when listing
   * The current possibilities are
   * Name, Type, Size, Date, AccessDate, Access, Owner, Group, Link, URL, MimeType
   */
  static QStringList listing( const QString& protocol );
  static bool isSourceProtocol( const QString& protocol );
  static bool isHelperProtocol( const QString& protocol );
  static bool isFilterProtocol( const QString& protocol );
  static bool isKnownProtocol( const QString& protocol );
  static bool supportsListing( const QString& protocol );
  static bool supportsReading( const QString& protocol );
  static bool supportsWriting( const QString& protocol );
  static bool supportsMakeDir( const QString& protocol );
  static bool supportsDeleting( const QString& protocol );
  static bool supportsLinking( const QString& protocol );
  static bool supportsMoving( const QString& protocol );
  static bool canCopyFromFile( const QString& protocol );
  static bool canCopyToFile( const QString& protocol );
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
