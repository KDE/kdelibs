
#include <kapp.h>
#include <kparts/metaview.h>
#include <kdebug.h>
#include <qtextstream.h>
#include <qdir.h>

class MyMetaDataWriter : public KParts::LocalMetaDataWriter
{
public:
  MyMetaDataWriter() : KParts::LocalMetaDataWriter( 0 ) {};

  virtual bool createData( QDomElement &data, const KURL &url, const QString &serviceType )
  {
    kdDebug() << "createData( " << url.url() << " , " << serviceType << " ) " << endl;

    QDomElement bg = data.ownerDocument().createElement( "background" );
    bg.setAttribute( "pixmap", "some.png" );
    data.appendChild( bg );
    
    return true;
  }
};

int main( int argc, char **argv )
{
  KApplication app( argc, argv, "metadatatest" );

  MyMetaDataWriter test;

  KURL u( QDir::currentDirPath() );
  
  QDomDocument temp = test.data( u, "inode/directory" );

  QTextStream str( stdout, IO_WriteOnly );
  str << temp;
  
  if ( temp.documentElement().attribute( "readonly" ) != "true" )
    temp.documentElement().namedItem( "background" ).toElement().setAttribute( "pixmap", "some_other.png" );
  
  test.saveData( u, "inode/directory", temp );

  return 0;
}
