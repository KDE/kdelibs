#include "notepad_factory.h"

#include <klocale.h>
#include <kstandarddirs.h>
#include <kinstance.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <assert.h>

#include "notepad.h"

extern "C"
{
    void* init_libnotepadpart()
    {
	return new NotepadFactory;
    }
};

KInstance* NotepadFactory::s_instance = 0L;
KAboutData* NotepadFactory::s_about = 0L;

NotepadFactory::NotepadFactory()
  : KLibFactory()
{
}

NotepadFactory::~NotepadFactory()
{
  delete s_instance;
  delete s_about;

  s_instance = 0L;
}

QObject* NotepadFactory::create( QObject* parent, const char* name, const char* classname, const QStringList & )
{
  if ( parent && !parent->inherits("QWidget") )
  {
    kdDebug() << "NotepadFactory: parent does not inherit QWidget" << endl;
    return 0L;
  }

  NotepadPart* obj = new NotepadPart( (QWidget*) parent, name );
  // readwrite ?
  if (QString(classname) == "KParts::ReadWritePart")
      obj->setReadWrite(true);

  // otherwise, it has to be readonly
  assert (QString(classname) == "KParts::ReadOnlyPart");

  emit objectCreated( obj );
  return obj;
}

KInstance* NotepadFactory::instance()
{
  if( !s_instance )
  {
    s_about = new KAboutData( "notepadpart", I18N_NOOP( "Notepad" ), "2.0pre" );
    s_instance = new KInstance( s_about );
  }
  return s_instance;
}

#include "notepad_factory.moc"
