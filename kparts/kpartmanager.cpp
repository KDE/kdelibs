
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
  // :-) ..and I was waiting for the day someone discovering that one :-) (Simon)
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
    qDebug("FocusIn Event");
    QObject* o = obj;
    do
    {
      if ( o->inherits("QWidget") )
      {
        KPart * part = findPartFromWidget( (QWidget*)o );
	if ( !part )
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
       ev->type() != QEvent::MouseButtonDblClick &&
       ev->type() != QEvent::FocusIn )
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
      // I suppose we don't return here in case of child parts, right ?
      // But it means we'll emit the event for each intermediate parent ? (David)
      // Perhaps we should store the new part and emit at the end ?
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
