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
#include <ksycocaentry.h>

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
   * @internal construct a ImageIOFormat from a stream
   */ 
  KProtocolInfo( QDataStream& _str, int offset);

  virtual ~KProtocolInfo();

  virtual QString name() const { return m_name; }

  virtual bool isValid() const { return true; } 

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

  static Type inputType( const QString& protocol );
  static Type outputType( const QString& protocol );
  /**
   * @returns the list of fields this protocol returns when listing
   * The current possibilities are
   * Name, Type, Size, Date, AccessDate, CreationDate, Access, Owner, Group, Link, URL, MimeType
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

  /**
   * @returns the name of the icon, associated with the specified protocol
   */
  static QString icon( const QString& protocol );    
    
  /**
   * @returns default mimetype for this protocol
   */
  static QString defaultMimetype( const QString& protocol );

  /**
   * Determining the mimetype of a URL can be done by simply looking at
   * the extension (that's the fast mode). However, when
   * the extension is unknown, a KIO::mimetype job will be necessary
   * (i.e. downloading the beginning of the file or asking the server for
   * the mimetype).
   *
   * @returns true if we can trust the mimetype @p mimetype for
   * the protocol @p protocol. A HTTP URL ending with
   * .pl or .asp may not return that actual type, but anything else.
   * This information is used by KRun to know whether it should trust
   * or not the result of KMimeType::findByURL.
   * Defaults to true.
   */
  static bool mimetypeFastMode( const QString& protocol, const QString& mimetype );

  /**
   * Same as @ref mimetypeFastMode but using patterns to match the filename.
   * Used to avoid conflicts with existing mimetypes.
   * Example: *.stm is both audio/x-mod and shtml, over HTTP.
   * Using mimetypeFastMode is preferred in other cases, though.
   */
  static bool patternFastMode( const QString& protocol, const QString& filename );

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
  QStringList m_mimetypesExcludedFromFastMode;
  QStringList m_patternsExcludedFromFastMode;
  QString m_icon;
};

#endif
