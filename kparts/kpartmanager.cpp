
#include "kpartmanager.h"
#include "kpart.h"

#include <qapplication.h>

#include <assert.h>

KPartManager::KPartManager( QWidget * parent, const char * name )
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
      m_activePart = part;
      qDebug(QString("Part %1 made active because %2 got focus").arg(part->name()).arg(w->className()));
      emit activePartChanged( m_activePart );
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

  part->setManager( this );

  m_activePart = part;
  //qDebug(QString("Part %1 added. Making active").arg(part->name()));
  emit activePartChanged( m_activePart );
}

void KPartManager::removePart( KPart *part )
{
  if ( m_parts.findRef( part ) == -1 )
  {
    qDebug(QString("Can't remove part %1, not in KPartManager's list.").arg(part->name()));
    return;
  }

  //qDebug(QString("Part %1 removed").arg(part->name()));
  m_parts.removeRef( part );

  if ( part == m_activePart )
  {
    m_activePart = 0;
    emit activePartChanged( 0 );
  }
}

void KPartManager::slotObjectDestroyed()
{
  removePart( (KPart *)sender() );
}

#include "kpartmanager.moc"
