#include "kpartsmainwindow.h"
#include "kpart.h"
#include "kplugin.h"

#include <qdom.h>
#include <qmenubar.h>
#include <qaction.h>
#include <qapplication.h>
#include <qfile.h>
#include <qstatusbar.h>
#include <qobjectlist.h>

// Less debug output for now..
// #define DEBUG_BUILDER

KPartsMainWindow::KPartsMainWindow( const char *name, WFlags f )
  : KTMainWindow( name, f ), m_actionCollection( this )
{
  m_toolBars.setAutoDelete( true );
  m_statusBar = 0;
  m_activePart = 0;
}

KPartsMainWindow::~KPartsMainWindow()
{
 // The mainwindow (our parent) has just been deleted
 // (this is the only way to destroy this class, right ?)
 // so don't do anything (David)
 // clearGUI();
}

QAction *KPartsMainWindow::action( const QDomElement &element )
{
  return m_actionCollection.action( element.attribute( "name" ) );
}

void KPartsMainWindow::setXMLFile( const QString &file )
{
  m_xml = KXMLGUIFactory::readConfigFile( file );
}

void KPartsMainWindow::setXML( const QString &document )
{
  m_xml = document;
}

QWidget *KPartsMainWindow::createContainer( QObject *parent, const QDomElement &element )
{
#ifdef DEBUG_BUILDER
  qDebug( "KPartsMainWindow::createContainer()" );
  qDebug( "tagName() : %s", element.tagName().ascii() );
  if ( parent )
   qDebug( "parent.className() : %s", parent->className() );
#endif

  if ( element.tagName() == "MenuBar" )
    return menuBar();

  if ( element.tagName() == "Menu" && parent && parent->inherits( "QWidget" ) )
  {
    QPopupMenu *popup = new QPopupMenu( this );

    QString text = element.namedItem( "text" ).toElement().text();

    if ( parent->inherits( "KMenuBar" ) )
      ((KMenuBar *)parent)->insertItem( text, popup );
    else if ( parent->inherits( "QPopupMenu" ) )
      ((QPopupMenu *)parent)->insertItem( text, popup );

    return popup;
  }

  if ( element.tagName() == "Separator" && parent && parent->inherits( "QWidget" ) )
  {
    if ( parent->inherits( "KMenuBar" ) )
      ((KMenuBar *)parent)->insertSeparator();
    else if ( parent->inherits( "QPopupMenu" ) )
      ((QPopupMenu *)parent)->insertSeparator();

    //indicate that we handle this element on our own :)
    return 0L;
  }

  if ( element.tagName() == "ToolBar" )
  {
    bool honor = (element.attribute( "name" ) == "mainToolBar") ? true : false;
    KToolBar *bar = new KToolBar(this, element.attribute( "name" ), -1, honor);

    // (David) Hmm, this list exists in KTMainWindow as well - but it's private...
    // I've asked for an iterator on the toolbars, on kde-core-devel
    // We would remove m_toolBars, and we would use the iterator in clearGUI(),
    // to save the position of the (parts, or all?) toolbars
    m_toolBars.append( bar );

    addToolBar( bar );

    QString attrFullWidth = element.attribute( "fullWidth" ).lower();
    QString attrPosition = element.attribute( "position" ).lower();

    if ( !attrFullWidth.isEmpty() && attrFullWidth == "true" )
      bar->setFullWidth( true );
    else
      bar->setFullWidth( false );

    if ( !attrPosition.isEmpty() )
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

    bar->show();

    return bar;
  }

  if ( element.tagName() == "StatusBar" )
  {
    if ( m_statusBar )
      delete m_statusBar;

    m_statusBar = new KStatusBar( this );

    setStatusBar( m_statusBar );

    m_statusBar->show();

    return m_statusBar;
  }

  return 0L;
}

void KPartsMainWindow::createGUI( KPart * part )
{
  qDebug(QString("KPartsMainWindow::createGUI for %1").arg(part?part->name():"0L"));
  // start the factory with this as an input (shell servant),
  // the part servant (or none) as the other input, and this as an output (GUI builder)

  setUpdatesEnabled( false );
  clearGUI();

  if ( !part )
  {
    KNullGUIServant nullServant;
    KXMLGUIFactory::createGUI( this, &nullServant, this );
  } else {
    KPartGUIServant partServant( part );
    KXMLGUIFactory::createGUI( this, &partServant, this );
  }

  setUpdatesEnabled( true );

  m_activePart = part;
}

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

  if ( m_statusBar )
    {
      delete m_statusBar;
      m_statusBar = 0;
      setStatusBar( 0 );
    }

  m_toolBars.clear();
}

/*
Removed, shouldn't be necessary anymore
QWidget *KPartsMainWindow::topLevelContainer( const QString &name )
{
  if ( name == "MenuBar" )
    return menuBar();

  if ( name == "StatusBar" )
    return statusBar();

  return (KToolBar *)child( name, "KToolBar" );
}
*/

#include "kpartsmainwindow.moc"
