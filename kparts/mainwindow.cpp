#include <kparts/mainwindow.h>
#include <kparts/event.h>
#include <kparts/part.h>
#include <kparts/plugin.h>

#include <qapplication.h>

#include <kdebug.h>

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
  PartBase::setObject( this );
}

MainWindow::~MainWindow()
{
  delete d;
}

void MainWindow::createGUI( Part * part )
{
  kDebugInfo( 1000, QString("MainWindow::createGUI for %1").arg(part?part->name():"0L"));

  KXMLGUIFactory *factory = guiFactory();

  setUpdatesEnabled( false );

  QValueList<KXMLGUIClient *> plugins;
  QValueList<KXMLGUIClient *>::ConstIterator pIt, pBegin, pEnd;

  if ( d->m_activePart )
  {
    kDebugInfo( 1000, QString("deactivating GUI for %1").arg(d->m_activePart->name()) );

    GUIActivateEvent ev( false );
    QApplication::sendEvent( d->m_activePart, &ev );

    plugins = Plugin::pluginClients( d->m_activePart );
    pIt = plugins.fromLast();
    pBegin = plugins.begin();

    for (; pIt != pBegin ; --pIt )
      factory->removeClient( *pIt );

    if ( pIt != plugins.end() )
      factory->removeClient( *pIt );

    factory->removeClient( d->m_activePart );

    disconnect( d->m_activePart, SIGNAL( setWindowCaption( const QString & ) ),
             this, SLOT( setCaption( const QString & ) ) );
    disconnect( d->m_activePart, SIGNAL( setStatusBarText( const QString & ) ),
             this, SLOT( slotSetStatusBarText( const QString & ) ) );
  }

  if ( !d->m_bShellGUIActivated )
  {
    GUIActivateEvent ev( true );
    QApplication::sendEvent( this, &ev );

    factory->addClient( this );

    plugins = Plugin::pluginClients( this );
    pIt = plugins.begin();
    pEnd = plugins.end();
    for (; pIt != pEnd; ++pIt )
      factory->addClient( *pIt );

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

    factory->addClient( part );

    plugins = Plugin::pluginClients( part );
    pIt = plugins.begin();
    pEnd = plugins.end();

    for (; pIt != pEnd; ++pIt )
      factory->addClient( *pIt );
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
