/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                     David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef __kmimetype_h__
#define __kmimetype_h__

#include <sys/types.h>
#include <sys/stat.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qdict.h>
#include <qmap.h>

#include <kurl.h>
#include <ksimpleconfig.h>

#include "ksycocatype.h"
#include "kservicetype.h"

class KServiceTypeFactory;

/**
 * Represents a mime type.
 *
 * IMPORTANT : to use the public static methods of this class, you must do 
 * the following registry initialisation (in main() for instance)
 * <pre>
 * #include <kregistry.h>
 * #include <kregfactories.h> 
 *
 *   KRegistry registry;
 *   registry.addFactory( new KServiceTypeFactory );
 *   registry.load();
 * </pre>
 */
class KMimeType : public KServiceType
{
  K_SYCOCATYPE( KST_KMimeType, KServiceType );

public:
  KMimeType( const QString& _type, const QString& _icon, 
	     const QString& _comment, const QStringList& _patterns );
  KMimeType( KSimpleConfig& _cfg );
  KMimeType( QDataStream& _str );
  virtual ~KMimeType();
  
  /**
   * Return the filename of the icon associated with the mimetype.
   * The arguments are unused, but provided so that KMimeType derived classes
   * can use them (e.g. KFolderType uses the URL to return one out of 2 icons)
   * @return the path to the icon associated with this MIME type.
   */
  virtual QString icon( const QString& , bool ) const { return m_strIcon; }
  /**
   * This function differs from the above only in that a KURL may be
   * provided instead of a QString for convenience.
   */
  virtual QString icon( const KURL& , bool ) const { return m_strIcon; }
  /**
   * The arguments are unused, but provided so that KMimeType derived classes
   * can use them.
   * @return the descriptive comment associated with the MIME type, if any.
   */
  virtual QString comment( const QString&, bool ) const { return m_strComment; }
  /**
   * This function differs from the above only in that a KURL may be
   * provided instead of a QString for convenience.
   */
  virtual QString comment( const KURL&, bool ) const { return m_strComment; }
  /**
   * @deprecated
   * Use @ref KServiceType::name instead.
   */
  virtual QString mimeType() const { return m_strName; }
  
  /**
   * @return the list of patterns associated to the MIME Type
   */
  virtual const QStringList& patterns() const { return m_lstPatterns; }
  
  /** 
   * Looks whether the given filename matches this mimetypes
   * extension patterns.
   *
   * @param _filename is the real decoded filename or the decoded path
   *        without trailing '/'.
   *
   * @see #m_lstPatterns 
   */
  virtual bool matchFilename( const QString&_filename ) const;

  /**
   * Load the mimetype from a stream. 
   * @param _parentLoaded internal (set by the constructor)
   */
  virtual void load( QDataStream&, bool _parentLoaded = false );
  /**
   * Save the mimetype to a stream
   */
  virtual void save( QDataStream& );

  virtual PropertyPtr property( const QString& _name ) const;
  virtual QStringList propertyNames() const;
  
  /**
   * @return a pointer to the mime type '_name' or a pointer to the default
   *         mime type "application/octet-stream". 0L is NEVER returned.
   *
   * @see KServiceType::serviceType
   */
  static KMimeType* mimeType( const QString& _name );
  /** 
   * This function looks at mode_t first. If that does not help it
   * looks at the extension.  This is ok for FTP, FILE, TAR and
   * friends, but is not for HTTP ( cgi scripts! ). You should use
   * @ref KRun instead, but this function returns immediately while
   * @ref KRun is async. If no extension matches, then @ref
   * KMimeMagic is used if the URL a local file or
   * "application/octet-stream" is returned otherwise.
   *
   * @param _url is the right most URL with a filesystem protocol. It
   *        is up to you to find out about that if you have a nested
   *        URL.  For example
   *        "http://localhost/mist.gz#gzip:/decompress" would have to
   *        pass the "http://..." URL part, while
   *        "file:/tmp/x.tar#tar:/src/test.gz#gzip:/decompress" would
   *        have to pass the "tar:/..." part of the URL, since gzip is
   *        a filter protocol and not a filesystem protocol.
   *
   * @param _fast_mode If set to true no disk access is allowed to
   *        find out the mimetype. The result may be suboptimal, but
   *        it is * FAST.  * @return a pointer to the matching
   *        mimetype. 0L is NEVER returned.  */
  static KMimeType* findByURL( const KURL& _url, mode_t _mode = 0,
			       bool _is_local_file = false, bool _fast_mode = false );

  /** 
   * Get all the mimetypes dict. Useful for showing the list of
   * available mimetypes.  The returned dict contains a subset of the
   * entries returned by @ref KServiceType::serviceTypes 
   */
  // TODO static const QDict<KMimeType>& mimeTypes() { return *s_mapMimeTypes; } 

protected:
  /**
   * Signal a missing mime type
   */
  static void errorMissingMimeType( const QString& _type );

  /**
   * This function makes sure that vital mime types are installed.
   * It must be called by all public static methods
   */
  static void check();
  /**
   * True if check for vital mime types has been done
   */
  static bool s_bChecked;

  QStringList m_lstPatterns;
  
  static KMimeType* s_pDefaultType;
};

class KFolderType : public KMimeType
{
  K_SYCOCATYPE( KST_KFolderType, KMimeType );

public:
  KFolderType( const QString& _type, const QString& _icon, const QString& _comment,
	       const QStringList& _patterns );
  KFolderType( KSimpleConfig& _cfg ) : KMimeType( _cfg ) { }
  KFolderType( QDataStream& _str ) : KMimeType( _str ) { }

  virtual QString icon( const QString& _url, bool _is_local ) const;
  virtual QString icon( const KURL& _url, bool _is_local ) const;
  virtual QString comment( const QString& _url, bool _is_local ) const;
  virtual QString comment( const KURL& _url, bool _is_local ) const;
};

class KDEDesktopMimeType : public KMimeType
{
  K_SYCOCATYPE( KST_KDEDesktopMimeType, KMimeType );

public:
  enum ServiceType { ST_MOUNT, ST_UNMOUNT, /* ST_PROPERTIES, */ ST_USER_DEFINED };
		     
  struct Service
  {
    QString m_strName;
    QString m_strIcon;
    QString m_strExec;
    ServiceType m_type;
  };
  
  KDEDesktopMimeType( const QString& _type, const QString& _icon, const QString& _comment,
		  const QStringList& _patterns );
  KDEDesktopMimeType( KSimpleConfig& _cfg ) : KMimeType( _cfg ) { }
  KDEDesktopMimeType( QDataStream& _str ) : KMimeType( _str ) { }

  virtual QString icon( const QString& _url, bool _is_local ) const;
  virtual QString icon( const KURL& _url, bool _is_local ) const;
  virtual QString comment( const QString& _url, bool _is_local ) const;
  virtual QString comment( const KURL& _url, bool _is_local ) const;

  /**
   * @return a list of services for the given .desktop file that are handled
   * by kio itself. Namely mount/unmount for FSDevice files.
   */
  static QValueList<Service> builtinServices( const KURL& _url );
  /**
   * @return a list of services defined by the user as possible actions
   * on the given .desktop file.
   */
  static QValueList<Service> userDefinedServices( const KURL& _url );

  /** 
   * @param _url is the URL of the desktop entry. The URL must be
   *        local, otherwise nothing will happen.  
   */
  static void executeService( const QString& _url, KDEDesktopMimeType::Service& _service );

  /** 
   * Invokes the default action for the desktop entry. If the desktop
   * entry is not local, then only false is returned. Otherwise we
   * would create a security problem. Only types Link and Mimetype
   * could be followed.
   *
   * @return true on success and false on failure.
   *
   * @see KRun::runURL */

  static bool run( const QString& _url, bool _is_local );

protected:
  static bool runFSDevice( const QString& _url, KSimpleConfig &cfg );
  static bool runApplication( const QString& _url, KSimpleConfig &cfg );
  static bool runLink( const QString& _url, KSimpleConfig &cfg );
  static bool runMimeType( const QString& _url, KSimpleConfig &cfg );
};

class KExecMimeType : public KMimeType
{
  K_SYCOCATYPE( KST_KExecMimeType, KMimeType );

public:
  KExecMimeType( const QString& _type, const QString& _icon, const QString& _comment,
		 const QStringList& _patterns );
  KExecMimeType( KSimpleConfig& _cfg ) : KMimeType( _cfg ) { }
  KExecMimeType( QDataStream& _str ) : KMimeType( _str ) { }
};

#endif

