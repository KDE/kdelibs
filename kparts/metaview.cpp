
#include "metaview.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include <kdebug.h>

using namespace KParts;

namespace KParts
{
class MetaViewPrivate
{
public:
};
};

MetaView::MetaView( QObject *parent, const char *name )
 : Part( parent, name )
{
  d = new MetaViewPrivate;
}

MetaView::~MetaView()
{
  delete d;
}

namespace KParts
{
class MetaDataProviderPrivate
{
public:
};
};

MetaDataProvider::MetaDataProvider( QObject *parent, const char *name )
: QObject( parent, name )
{
  d = new MetaDataProviderPrivate;
}

MetaDataProvider::~MetaDataProvider()
{
  delete d;
}

namespace KParts
{
class MetaDataWriterPrivate
{
public:
};
};

MetaDataWriter::MetaDataWriter( QObject *parent, const char *name )
: MetaDataProvider( parent, name )
{
  d = new MetaDataWriterPrivate;
}

MetaDataWriter::~MetaDataWriter()
{
  delete d;
}

namespace KParts
{
class LocalMetaDataWriterPrivate
{
public:
  LocalMetaDataWriterPrivate()
  {
    dirServiceType = QString::fromLatin1( "inode/directory" );
    metaDataName = QString::fromLatin1( "kmetadata" );
    metaDataFileName = metaDataName;
    metaDataFileName.prepend( '.' );
  }

  QString dirServiceType;
  QString metaDataName;
  QString metaDataFileName;
};
};

LocalMetaDataWriter::LocalMetaDataWriter( QObject *parent, const char *name )
 : MetaDataWriter( parent, name )
{
  d = new LocalMetaDataWriterPrivate;
}

LocalMetaDataWriter::~LocalMetaDataWriter()
{
  delete d;
}

QDomDocument LocalMetaDataWriter::data( const KURL &url, const QString &serviceType )
{
  if ( !url.isLocalFile() )
   return QDomDocument();

  if ( serviceType == d->dirServiceType )
  {
    KURL u( url );
    QFileInfo dir( u.path() );
    u.addPath( d->metaDataFileName );
    QFile file( u.path() );
    if ( !file.exists( u.path() ) || !file.open( IO_ReadOnly ) )
    {
      QDomDocument doc = createData( url, serviceType );
      if ( dir.isWritable() )
        saveData( url, serviceType, doc );
      else
        doc.documentElement().setAttribute( "readonly", "true" );

      return doc;
    }
    else
    {
      QDomDocument doc( &file );
      file.close();
      if ( !dir.isWritable() )
        doc.documentElement().setAttribute( "readonly", "true" );
      return doc;
    }
  }
  else
    kdFatal() << k_lineinfo << "better implement reading of metadata for files ;-)" << endl;

  return QDomDocument();
}

bool LocalMetaDataWriter::saveData( const KURL &url, const QString &serviceType, const QDomDocument &data )
{
  if ( !url.isLocalFile() )
    return false;

  if ( serviceType != d->dirServiceType )
  {
    kdFatal() << k_lineinfo << "better implement saving of metadata for files ;-)" << endl;
    return false;
  }

  KURL u( url );
  u.addPath( d->metaDataFileName );
  QFile file( u.path() );

  if ( !file.open( IO_WriteOnly ) )
    return false;

  QTextStream stream( &file );
  stream << data;

  file.close();
  return true;
}

QDomDocument LocalMetaDataWriter::createData( const KURL &url, const QString &serviceType )
{
  QDomDocument doc( d->metaDataName );
  doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
  QDomElement data = doc.createElement( d->metaDataName );
  if ( createData( data, url, serviceType ) )
  {
    static QString attrURL = QString::fromUtf8( "url" );
    static QString attrServiceType = QString::fromUtf8( "servicetype" );
    data.setAttribute( attrURL, url.url() );
    data.setAttribute( attrServiceType, serviceType );
    
    doc.appendChild( data );
  }
  return doc;
}

bool LocalMetaDataWriter::createData( QDomElement &, const KURL &, const QString & )
{
  return true; 
} 

#include "metaview.moc"
