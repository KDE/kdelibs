
#include "kpartmanager.h"
#include "kpart.h"
#include <kdebug.h>

#include <qapplication.h>

using namespace KParts;

template class QList<Part>;

const char *PartActivateEvent::s_strPartActivateEvent = "KParts/PartActivateEvent";

PartManager::PartManager( QWidget * parent, const char * name )
 : QObject( parent, name )
{
  m_activePart = 0;

  qApp->installEventFilter( this );
}

PartManager::~PartManager()
{
  setActivePart( 0L );
  qApp->removeEventFilter( this );
}

bool PartManager::eventFilter( QObject *obj, QEvent *ev )
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

  Part * part;
  while ( w )
  {
    part = findPartFromWidget( w );
    if ( part ) // We found a part whose widget is w
    {
      if ( part != m_activePart )
      {
        kDebugInfo( 1000, QString("Part %1 made active because %2 got event").arg(part->name()).arg(w->className()) );

        setActivePart( part );
      }

      // I suppose we don't return here in case of child parts, right ?
      // But it means we'll emit the event for each intermediate parent ? (David)
      // Perhaps we should store the new part and emit at the end ?

      // I think we should return here (Simon)
      return false; // Ok, let's return. We'll test child parts later on. (David)
    }

    w = w->parentWidget();

    if ( w && ( ( w->testWFlags( WStyle_Dialog ) && w->isModal() ) ||
                w->testWFlags( WType_Popup ) ) )
    {
      kDebugInfo( 1000, QString("No part made active although %1/%2 got event - loop aborted").arg(obj->name()).arg(obj->className()) );
      return false;
    }

  }

  kDebugInfo( 1000, QString("No part made active although %1/%2 got event").arg(obj->name()).arg(obj->className()) );
  return false;
}

Part * PartManager::findPartFromWidget( QWidget * widget )
{
  QListIterator<Part> it ( m_parts );
  for ( ; it.current() ; ++it )
  {
    if ( it.current()->widget() == widget )
      return it.current();
  }
  return 0L;
}

void PartManager::addPart( Part *part, bool setActive )
{
  connect( part, SIGNAL( destroyed() ), this, SLOT( slotObjectDestroyed() ) );

  m_parts.append( part );

  part->setManager( this );

  if ( setActive )
    setActivePart( part );

  // Prevent focus problems
  if ( part->widget()->focusPolicy() == QWidget::NoFocus ||
       part->widget()->focusPolicy() == QWidget::TabFocus )
  {
    kDebugWarning( 1000, QString("Part %1 must have at least a ClickFocus policy. Prepare for trouble !").arg(part->name()) );
  }

  part->widget()->show();
  emit partAdded( part );
}

void PartManager::removePart( Part *part )
{
  if ( m_parts.findRef( part ) == -1 )
  {
    kDebugFatal (1000, QString("Can't remove part %1, not in KPartManager's list.").arg(part->name()) );
    return;
  }
  disconnect( part, SIGNAL( destroyed() ), this, SLOT( slotObjectDestroyed() ) );

  kDebugInfo( 1000, QString("Part %1 removed").arg(part->name()) );
  m_parts.removeRef( part );

  if ( part == m_activePart )
    setActivePart( 0 );

  emit partRemoved( part );
}

void PartManager::setActivePart( Part *part )
{
  if ( m_activePart )
  {
    PartActivateEvent ev( false );
    QApplication::sendEvent( m_activePart, &ev );
  }
  
  m_activePart = part;
  
  if ( m_activePart )
  {
    PartActivateEvent ev( true );
    QApplication::sendEvent( m_activePart, &ev );
  }
  
  emit activePartChanged( m_activePart );
}

void PartManager::slotObjectDestroyed()
{
  kDebugInfo( 1000, "KPartManager::slotObjectDestroyed()" );
  removePart( (Part *)sender() );
}

#include "kpartmanager.moc"
