#include "kpart.h"

#include <qfile.h>
#include <qpoint.h>
#include <qpointarray.h>
#include <qpainter.h>

#include <klibloader.h>

KPart::KPart( QWidget* parent, const char* name )
    : QWidget( parent, name )
{
}

KPart::~KPart()
{
}

QString KPart::readConfigFile( const QString& filename ) const
{
    QFile file( filename );
    if ( !file.open( IO_ReadOnly ) )
	return QString::null;

    uint size = file.size();
    char* buffer = new char[ size + 1 ];
    file.readBlock( buffer, size );
    buffer[ size ] = 0;
    file.close();

    QString text = QString::fromUtf8( buffer, size );
    delete[] buffer;

    return text;
}

QString KPart::config()
{
    if (  m_config.isEmpty() )
	m_config = configFile();

    return m_config;
}

QAction* KPart::action( const char* name )
{
    return m_collection.action( name );
}

QActionCollection* KPart::actionCollection()
{
    return &m_collection;
}

KPlugin* KPart::plugin( const char* libname )
{
    QObject* ch = child( libname, "KPlugin" );
    if ( ch )
	return (KPlugin*)ch;

    KLibLoader* loader = KLibLoader::self();
    if ( !loader )
    {
	qDebug("KPart: No library loader installed");
	return 0;
    }

    KLibFactory* f = loader->factory( libname );
    if ( !f )
    {
	qDebug("KPart: Could not initialize library");
	return 0;
    }
    QObject* obj = f->create( this, libname, "KPlugin" );
    if ( !obj->inherits("KPlugin" ) )
    {
	qDebug("The library does not feature an object of class Plugin");
	delete obj;
	return 0;
    }

    return (KPlugin*)obj;
}

KReadOnlyPart::KReadOnlyPart( QWidget *parent, const char *name )
 : KPart( parent, name )
{
}

KReadOnlyPart::~KReadOnlyPart()
{
}

void KReadOnlyPart::init()
{
}

KReadWritePart::KReadWritePart( QWidget *parent, const char *name )
 : KReadOnlyPart( parent, name )
{
}

KReadWritePart::~KReadWritePart()
{
}



#include "kpart.moc"
