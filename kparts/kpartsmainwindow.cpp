#include "kpartsmainwindow.h"
#include "kpart.h"
#include "kplugin.h"

#include <qdom.h>
#include <qmenubar.h>
#include <qaction.h>
#include <qapplication.h>
#include <qfile.h>
#include <qstatusbar.h>
#include <qdatastream.h>
#include <qobjectlist.h>

#include <kaction.h>

// Less debug output for now..
// #define DEBUG_BUILDER

class KPartsMainWindowPrivate
{
public:
  KPartsMainWindowPrivate()
  {
    m_toolBars.setAutoDelete( true );
    m_statusBar = 0;
    m_activePart = 0;
    m_bShellGUIActivated = false;
  }
  ~KPartsMainWindowPrivate()
  {
  }

  QDomDocument m_doc;
  KStatusBar *m_statusBar;
  QList<KToolBar> m_toolBars;
  QGuardedPtr<KPart> m_activePart;
  bool m_bShellGUIActivated;
};

KPartsMainWindow::KPartsMainWindow( const char *name, WFlags f )
  : KTMainWindow( name, f ), m_actionCollection( this )
{
  d = new KPartsMainWindowPrivate();
  m_factory = new KXMLGUIFactory( this );
}

KPartsMainWindow::~KPartsMainWindow()
{
 // The mainwindow (our parent) has just been deleted
 // (this is the only way to destroy this class, right ?)
 // so don't do anything (David)
 // clearGUI();
  delete d;
  delete m_factory;
}

QAction *KPartsMainWindow::action( const QDomElement &element )
{
  return m_actionCollection.action( element.attribute( "name" ) );
}

KXMLGUIFactory *KPartsMainWindow::guiFactory() const
{
  return m_factory;
}

QDomDocument KPartsMainWindow::document()
{
  return d->m_doc;
}

void KPartsMainWindow::setXMLFile( const QString &file )
{
  QString xml = KXMLGUIFactory::readConfigFile( file );
  setXML( xml );
}

void KPartsMainWindow::setXML( const QString &document )
{
  //XXX: We might eventually call removeServant( this ) and addServant( this ) to update
  // the GUI dynamically! :-) (that'd be cool IMHO ;)
  // It just needs testing, as although KXMLGUIFactory should handle that correctly, there might
  // be bugs left :)
  // (update) It should really work ;-) (see commented code below) 
  d->m_doc.setContent( document );
  /*
  if ( d->m_bShellGUIActivated )
  {
    KPart *part = d->m_activePart;
    if ( part ) 
      createGUI( 0L );
    m_factory->removeServant( this );
    d->m_bShellGUIActivated = false;
    createGUI( part );
  }
  */
}

QObject *KPartsMainWindow::createContainer( QWidget *parent, int index, const QDomElement &element, const QByteArray &containerStateBuffer )
{
#ifdef DEBUG_BUILDER
  qDebug( "KPartsMainWindow::createContainer()" );
  qDebug( "tagName() : %s", element.tagName().ascii() );
  if ( parent )
   qDebug( "parent.className() : %s", parent->className() );
#endif

  if ( element.tagName() == "MenuBar" )
    return menuBar();

  if ( element.tagName() == "Menu" && parent )
  {
    QPopupMenu *popup = new QPopupMenu( this );

    QString text = element.namedItem( "text" ).toElement().text();

    if ( parent->inherits( "KMenuBar" ) )
      ((KMenuBar *)parent)->insertItem( text, popup, -1, index );
    else if ( parent->inherits( "QPopupMenu" ) )
      ((QPopupMenu *)parent)->insertItem( text, popup, -1, index );

    return popup;
  }

  if ( element.tagName() == "Separator" && parent )
  {
    KActionSeparator *separator = new KActionSeparator;
    separator->plug( parent, index );
    return separator;
  }

  if ( element.tagName() == "ToolBar" )
  {
    bool honor = (element.attribute( "name" ) == "mainToolBar") ? true : false;
    KToolBar *bar = new KToolBar(this, element.attribute( "name" ), -1, honor);

    // (David) Hmm, this list exists in KTMainWindow as well - but it's private...
    // I've asked for an iterator on the toolbars, on kde-core-devel
    // We would remove m_toolBars, and we would use the iterator in clearGUI(),
    // to save the position of the (parts, or all?) toolbars
    d->m_toolBars.append( bar );

    addToolBar( bar );

    QString attrFullWidth = element.attribute( "fullWidth" ).lower();
    QString attrPosition = element.attribute( "position" ).lower();

    if ( !attrFullWidth.isEmpty() && attrFullWidth == "true" )
      bar->setFullWidth( true );
    else
      bar->setFullWidth( false );

    if ( !attrPosition.isEmpty() && containerStateBuffer.size() == 0 )
    {
      if ( attrPosition == "top" )
        bar->setBarPos( KToolBar::Top );
      else if ( attrPosition == "left" )
        bar->setBarPos( KToolBar::Left );
      else if ( attrPosition == "right" )
	bar->setBarPos( KToolBar::Right );
      else if ( attrPosition == "bottom" )
	bar->setBarPos( KToolBar::Bottom );
      else if ( attrPosition == "floating" )
	bar->setBarPos( KToolBar::Floating );
      else if ( attrPosition == "flat" )
	bar->setBarPos( KToolBar::Flat );
    }
    else if ( containerStateBuffer.size() > 0 )
    {
      QDataStream stream( containerStateBuffer, IO_ReadOnly );
      Q_INT32 pos;
      stream >> pos;
      bar->setBarPos( (KToolBar::BarPosition)pos );
    }

    bar->show();

    return bar;
  }

  if ( element.tagName() == "StatusBar" )
  {
    if ( d->m_statusBar )
      delete d->m_statusBar;

    d->m_statusBar = new KStatusBar( this );

    setStatusBar( d->m_statusBar );

    d->m_statusBar->show();

    return d->m_statusBar;
  }

  return 0L;
}

QByteArray KPartsMainWindow::removeContainer( QObject *container, QWidget *parent )
{
  QByteArray stateBuff;

  if ( !container->isWidgetType() )
  {
    if ( !container->inherits( "QAction" ) )
      return stateBuff;

    ((QAction *)container)->unplug( parent );
    delete container;
    return stateBuff;
  }

  if ( container->inherits( "QPopupMenu" ) )
  {
    delete container;
    return stateBuff;
  }

  if ( container->inherits( "KToolBar" ) )
  {
    QDataStream stream( stateBuff, IO_WriteOnly );
    stream << (int)((KToolBar *)container)->barPos();
    d->m_toolBars.removeRef( (KToolBar *)container ); //will also delete the tb as autodelete is on
    return stateBuff;
  }

  if ( container->inherits( "KStatusBar" ) && d->m_statusBar )
  {
    delete d->m_statusBar;
    d->m_statusBar = 0;
    setStatusBar( 0 );
    return stateBuff;
  }

  return stateBuff;
}

void KPartsMainWindow::createGUI( KPart * part )
{
  qDebug(QString("KPartsMainWindow::createGUI for %1").arg(part?part->name():"0L"));
  // start the factory with this as an input (shell servant),
  // the part servant (or none) as the other input, and this as an output (GUI builder)

  setUpdatesEnabled( false );
  //  m_factory->clearGUI();

  if ( d->m_activePart )
  {
    qDebug( "deactivating GUI for %s", d->m_activePart->name() );
    QListIterator<KXMLGUIServant> pIt( *d->m_activePart->pluginServants() );
    pIt.toLast();
    for (; pIt.current(); --pIt )
      m_factory->removeServant( pIt.current() );

    m_factory->removeServant( d->m_activePart->servant() );
  }

  if ( !d->m_bShellGUIActivated )
  {
    m_factory->addServant( this );
    d->m_bShellGUIActivated = true;
  }

  if ( part )
  {
    m_factory->addServant( part->servant() );

    QListIterator<KXMLGUIServant> pIt( *part->pluginServants() );
    for (; pIt.current(); ++pIt )
      m_factory->addServant( pIt.current() );
  }

  setUpdatesEnabled( true );
  updateRects();

  d->m_activePart = part;
}

/* should be obsolete (Simon)
void KPartsMainWindow::clearGUI()
{
  // this is for actionmenus -- they must not be destroyed, but instead unplugged properly.
  const QObjectList *children = menuBar()->children();
  if ( children )
  {
    QObjectListIt it( *children );
    for (; it.current(); ++it )
      if ( it.current()->inherits( "QAction" ) )
	((QAction *)it.current())->unplug( menuBar() );
  }

  menuBar()->clear();

  if ( d->m_statusBar )
    {
      delete d->m_statusBar;
      d->m_statusBar = 0;
      setStatusBar( 0 );
    }

  d->m_toolBars.clear();
}
*/

#include "kpartsmainwindow.moc"
