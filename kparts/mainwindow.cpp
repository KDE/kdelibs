#include <kparts/mainwindow.h>
#include <kparts/event.h>
#include <kparts/part.h>
#include <kparts/plugin.h>

#include <qdom.h>
#include <qmenubar.h>
#include <qaction.h>
#include <qapplication.h>
#include <qdatastream.h>
#include <qobjectlist.h>

#include <kaction.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kinstance.h>
#include <kmenubar.h>
#include <kstddirs.h>
#include <ktoolbar.h>

using namespace KParts;

namespace KParts
{
class MainWindowPrivate
{
public:
  MainWindowPrivate()
  {
    m_activePart = 0;
    m_bShellGUIActivated = false;
  }
  ~MainWindowPrivate()
  {
  }

  QGuardedPtr<Part> m_activePart;
  bool m_bShellGUIActivated;
};
};

MainWindow::MainWindow( const char *name, WFlags f )
  : KTMainWindow( name, f )
{
  d = new MainWindowPrivate();
  m_factory = new XMLGUIFactory( this );
  setInstance( KGlobal::instance() );
}

MainWindow::~MainWindow()
{
  delete d;
}

void MainWindow::createGUI( Part * part )
{
  kDebugStringArea( 1000, QString("MainWindow::createGUI for %1").arg(part?part->name():"0L"));

  setUpdatesEnabled( false );

  QValueList<KXMLGUIServant *> plugins;
  QValueList<KXMLGUIServant *>::ConstIterator pIt, pBegin, pEnd;

  if ( d->m_activePart )
  {
    kDebugStringArea( 1000, QString("deactivating GUI for %1").arg(d->m_activePart->name()) );

    GUIActivateEvent ev( false );
    QApplication::sendEvent( d->m_activePart, &ev );

    plugins = Plugin::pluginServants( d->m_activePart );
    pIt = plugins.fromLast();
    pBegin = plugins.begin();

    for (; pIt != pBegin ; --pIt )
      m_factory->removeServant( *pIt );

    if ( pIt != plugins.end() )
      m_factory->removeServant( *pIt );

    m_factory->removeServant( d->m_activePart );

    disconnect( d->m_activePart, SIGNAL( setWindowCaption( const QString & ) ),
             this, SLOT( setCaption( const QString & ) ) );
    disconnect( d->m_activePart, SIGNAL( setStatusBarText( const QString & ) ),
             this, SLOT( slotSetStatusBarText( const QString & ) ) );
  }

  if ( !d->m_bShellGUIActivated )
  {
    GUIActivateEvent ev( true );
    QApplication::sendEvent( this, &ev );

    m_factory->addServant( this );

    plugins = Plugin::pluginServants( this );
    pIt = plugins.begin();
    pEnd = plugins.end();
    for (; pIt != pEnd; ++pIt )
      m_factory->addServant( *pIt );

    d->m_bShellGUIActivated = true;
  }

  if ( part )
  {
    // do this before sending the activate event
    connect( part, SIGNAL( setWindowCaption( const QString & ) ),
             this, SLOT( setCaption( const QString & ) ) );
    connect( part, SIGNAL( setStatusBarText( const QString & ) ),
             this, SLOT( slotSetStatusBarText( const QString & ) ) );

    GUIActivateEvent ev( true );
    QApplication::sendEvent( part, &ev );

    m_factory->addServant( part );

    plugins = Plugin::pluginServants( part );
    pIt = plugins.begin();
    pEnd = plugins.end();

    for (; pIt != pEnd; ++pIt )
      m_factory->addServant( *pIt );
  }

  setUpdatesEnabled( true );
  updateRects();

  d->m_activePart = part;
}

void MainWindow::slotSetStatusBarText( const QString & text )
{
  statusBar()->message( text );
}

#include "mainwindow.moc"
