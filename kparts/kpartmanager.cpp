
#include "kpartmanager.h"
#include "kpart.h"

#include <qapplication.h>

#include <assert.h>

KPartManager::KPartManager( QObject *parent, const char *name )
 : QObject( parent, name )
{
  m_activePart = 0;
  
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
	if ( m_parts.findRef( part ) == -1 )
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

  while ( w )
  {
    if ( w->inherits( "KPart" ) && m_parts.findRef( (KPart *)w ) != -1 &&
         (KPart *)w != m_activePart )
    {
      KPart *oldActivePart = m_activePart;
      m_activePart = (KPart *)w;
      emit activePartChanged( m_activePart, oldActivePart );
    }

    w = w->parentWidget();

    if ( w && ( ( w->testWFlags( WStyle_Dialog ) && w->isModal() ) ||
                w->testWFlags( WType_Popup ) ) )
      return false;

  }

  return false;
}

void KPartManager::addPart( KPart *part )
{
  connect( part, SIGNAL( destroyed() ), this, SLOT( slotObjectDestroyed() ) );
  
  m_parts.append( part );
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
