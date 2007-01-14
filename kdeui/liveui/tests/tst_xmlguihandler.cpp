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

  KActionCollection *coll = window->actionCollection();

  QAction *action = coll->addAction("open");
  action->setText("open");

  action = coll->addAction("undo");
  action->setText("undo");

  action = coll->addAction("view");
  action->setText("view");

  KLiveUiBuilder builder( window );
  builder.populateFromXmlGui( "tst_xmlguihandlerui.rc", window->actionCollection() );

  return app.exec();
}
