
#include "kpartmanager.h"
#include "kpart.h"
#include "kguibuilder.h"

#include <qapplication.h>

#include <assert.h>

KPartManager::KPartManager( KXMLGUIBuilder * builder )
 : QObject( builder, "KPartManager" )
{
  m_activePart = 0;

  // Oh oh, found a nice trick :-) Now I know why it's (new part, old part)
  // and not the other way round :)  (David)
  connect( this, SIGNAL( activePartChanged( KPart *, KPart * ) ),
           builder, SLOT( createGUI( KPart * ) ) );

  qApp->installEventFilter( this );
}

KPartManager::~KPartManager()
{
  qApp->removeEventFilter( this );
}

bool KPartManager::eventFilter( QObject *obj, QEvent *ev )
{
  /*
  if ( ev->type() == QEvent::FocusIn )
  {
    qDebug("Focus Event");
    QObject* o = obj;
    do
    {
      if ( o->inherits("KPart") )
      {
        KPart *part = (KPart *)o;
	if ( m_parts.findRef( part ) == -1 ) // TODO : port to findWidget
	  return FALSE;
		
        KPart *oldActivePart = m_activePart;
        m_activePart = part;
        emit activePartChanged( m_activePart, oldActivePart );
	return FALSE;
      }
      o = o->parent();
    } while( o );

    return FALSE;
  }
  */

  if ( ev->type() != QEvent::MouseButtonPress &&
       ev->type() != QEvent::MouseButtonDblClick )
    return false;

  if ( !obj->isWidgetType() )
    return false;

  QWidget *w = (QWidget *)obj;

  if ( ( w->testWFlags( WStyle_Dialog ) && w->isModal() ) ||
       w->testWFlags( WType_Popup ) )
    return false;

  KPart * part;
  while ( w )
  {
    part = findPartFromWidget( w );
    if ( part && part != m_activePart )
    {
      KPart *oldActivePart = m_activePart;
      m_activePart = part;
      emit activePartChanged( m_activePart, oldActivePart );
    }

    w = w->parentWidget();

    if ( w && ( ( w->testWFlags( WStyle_Dialog ) && w->isModal() ) ||
                w->testWFlags( WType_Popup ) ) )
      return false;

  }

  return false;
}

KPart * KPartManager::findPartFromWidget( QWidget * widget )
{
  QListIterator<KPart> it ( m_parts );
  for ( ; it.current() ; ++it )
  {
    if ( it.current()->widget() == widget )
      return it.current();
  }
  return 0L;
}

void KPartManager::addPart( KPart *part )
{
  connect( part, SIGNAL( destroyed() ), this, SLOT( slotObjectDestroyed() ) );

  m_parts.append( part );

  KPart *oldActivePart = m_activePart;
  m_activePart = part;
  emit activePartChanged( m_activePart, oldActivePart );
}

void KPartManager::removePart( KPart *part )
{
  if ( m_parts.findRef( part ) == -1 )
    return;

  m_parts.removeRef( part );

  if ( part == m_activePart )
  {
    m_activePart = 0;
    emit activePartChanged( 0, part );
  }
}

void KPartManager::slotObjectDestroyed()
{
  removePart( (KPart *)sender() );
}

#include "kpartmanager.moc"
