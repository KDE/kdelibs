/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000 Waldo Bastian <bastian@kde.org>


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
   * @internal construct a KProtocolInfo from a stream
   */
  KProtocolInfo( QDataStream& _str, int offset);

  virtual ~KProtocolInfo();

  virtual QString name() const { return m_name; }

  virtual bool isValid() const { return !m_name.isEmpty(); }

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

  enum Type { T_STREAM, T_FILESYSTEM, T_NONE, T_ERROR };

  /**
   * @returns the library / executable to open for the protocol @p protocol
   * Example : "kio_ftp", meaning either the executable "kio_ftp" or
   * the library "kio_ftp.la" (recommended), whichever is available.
   */
  static QString exec( const QString& protocol );

  static Type inputType( const KURL &url );
  static Type outputType( const KURL &url );
  /**
   * @returns the list of fields this protocol returns when listing
   * The current possibilities are
   * Name, Type, Size, Date, AccessDate, Access, Owner, Group, Link, URL, MimeType
   */
  static QStringList listing( const KURL &url );
  static bool isSourceProtocol( const KURL &url );
  static bool isHelperProtocol( const KURL &url );
  static bool isFilterProtocol( const KURL &url );
  static bool isKnownProtocol( const KURL &url );
  static bool supportsListing( const KURL &url );
  static bool supportsReading( const KURL &url );
  static bool supportsWriting( const KURL &url );
  static bool supportsMakeDir( const KURL &url );
  static bool supportsDeleting( const KURL &url );
  static bool supportsLinking( const KURL &url );
  static bool supportsMoving( const KURL &url );
  static bool canCopyFromFile( const KURL &url );
  static bool canCopyToFile( const KURL &url );
  /**
   * @returns default mimetype for this URL based on the protocol
   */
  static QString defaultMimetype( const KURL& url );

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

  /**
   * @returns the name of the icon, associated with the specified protocol
   */
  static QString icon( const QString& protocol );

  /**
   * @returns the name of the config file associated with the 
   * specified protocol
   */
  static QString config( const QString& protocol );


  static bool determineMimetypeFromExtension( const QString &protocol );

  /**
   * @returns list of all known protocols
   */
  static QStringList protocols();


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
};

#endif
