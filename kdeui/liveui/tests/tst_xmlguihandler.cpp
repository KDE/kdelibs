#include <QtGui/QAction>

#include "kaboutdata.h"
#include "klocale.h"
#include "kcmdlineargs.h"
#include "kapplication.h"
#include "kmainwindow.h"
#include "kactioncollection.h"
#include "../kliveui.h"
#include "../kliveui_p.h"

int main( int argc, char **argv )
{
  KAboutData aboutData("tst_xmlguihandler", I18N_NOOP("it is"),
                       "stupid having", "to specify this", KAboutData::License_GPL,
                       "all the way. it is not very convenient");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KApplication app;

  // rant: KMainWindow does not support creation on the stack
  KMainWindow *window = new KMainWindow;
  window->show();

  QAction *action = new QAction( "open", window->actionCollection() );
  action->setObjectName( "open" );

  action = new QAction( "undo", window->actionCollection() );
  action->setObjectName( "undo" );

  action = new QAction( "view", window->actionCollection() );
  action->setObjectName( "view" );

  KLiveUiBuilder builder( window );
  builder.populateFromXmlGui( "tst_xmlguihandlerui.rc" );

  return app.exec();
}
