
#include <kparts/partmanager.h>
#include <kparts/part.h>
#include <kdebug.h>

#include <qapplication.h>

using namespace KParts;

template class QList<Part>;

const char *PartActivateEvent::s_strPartActivateEvent = "KParts/PartActivateEvent";
const char *PartSelectEvent::s_strPartSelectEvent = "KParts/PartSelectEvent";

namespace KParts {

class PartManagerPrivate
{
public:
  PartManagerPrivate()
  {
    m_activeWidget = 0;
    m_activePart = 0;
    m_selectedPart = 0;
    m_selectedWidget = 0;
  }
  ~PartManagerPrivate()
  {
  }


  Part * m_activePart;
  QWidget *m_activeWidget;

  QList<Part> m_parts;

  PartManager::SelectionPolicy m_policy;

  Part *m_selectedPart;
  QWidget *m_selectedWidget;
};

};

PartManager::PartManager( QWidget * parent, const char * name )
 : QObject( parent, name )
{
  d = new PartManagerPrivate; 

  qApp->installEventFilter( this );
  
  d->m_policy = Direct;
}

PartManager::~PartManager()
{
  setActivePart( 0L );
  qApp->removeEventFilter( this );
  delete d;
}

void PartManager::setSelectionPolicy( SelectionPolicy policy )
{
  d->m_policy = policy; 
}

PartManager::SelectionPolicy PartManager::selectionPolicy() const
{
  return d->m_policy; 
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
       w->testWFlags( WType_Popup ) || w->testWFlags( WStyle_Tool ) )
    return false;

  Part * part;
  while ( w )
  {
    QPoint pos;
    
    if ( ev->type() == QEvent::MouseButtonPress || ev->type() == QEvent::MouseButtonDblClick )
      pos = ((QMouseEvent *)ev)->pos();
    
    part = findPartFromWidget( w, pos );
    if ( part ) // We found a part whose widget is w
    {
      if ( d->m_policy == PartManager::TriState )
      {
        if ( ev->type() == QEvent::MouseButtonDblClick )
	{
	  if ( part == d->m_activePart && w == d->m_activeWidget )
	    return false;
	  
	  setActivePart( part, w );
	  return true;
	}
	
#warning TODO: implement and hide koffice root part stuff behind part specific activation handling
	
	if ( ( d->m_selectedWidget != w || d->m_selectedPart != part ) &&
	     ( d->m_activeWidget != w || d->m_activePart != part ) )
	{
	  setSelectedPart( part, w );
	  return true;
	}
	else if ( d->m_selectedWidget == w && d->m_selectedPart == part )
	{
	  setActivePart( part, w );
	  return true;
	}
	else if ( d->m_activeWidget == w && d->m_activePart == part )
	{
  	  setSelectedPart( 0L );
	  return false;
	}
	     
	return false;
      }
      else if ( part != d->m_activePart )
      {
        kDebugInfo( 1000, QString("Part %1 made active because %2 got event").arg(part->name()).arg(w->className()) );

        setActivePart( part, w );
      }

      // I suppose we don't return here in case of child parts, right ?
      // But it means we'll emit the event for each intermediate parent ? (David)
      // Perhaps we should store the new part and emit at the end ?

      // I think we should return here (Simon)
      return false; // Ok, let's return. We'll test child parts later on. (David)
    }

    w = w->parentWidget();

    if ( w && ( ( w->testWFlags( WStyle_Dialog ) && w->isModal() ) ||
                w->testWFlags( WType_Popup ) || w->testWFlags( WStyle_Tool ) ) )
    {
      kDebugInfo( 1000, QString("No part made active although %1/%2 got event - loop aborted").arg(obj->name()).arg(obj->className()) );
      return false;
    }

  }

  kDebugInfo( 1000, QString("No part made active although %1/%2 got event").arg(obj->name()).arg(obj->className()) );
  return false;
}

Part * PartManager::findPartFromWidget( QWidget * widget, const QPoint &pos )
{
  QListIterator<Part> it ( d->m_parts );
  for ( ; it.current() ; ++it )
  {
    Part *part = it.current()->hitTest( widget, pos );
    if ( part )
      return part;
  }
  return 0L;
}

void PartManager::addPart( Part *part, bool setActive )
{
  connect( part, SIGNAL( destroyed() ), this, SLOT( slotObjectDestroyed() ) );

  d->m_parts.append( part );

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
  if ( d->m_parts.findRef( part ) == -1 )
  {
    kDebugFatal (1000, QString("Can't remove part %1, not in KPartManager's list.").arg(part->name()) );
    return;
  }
  disconnect( part, SIGNAL( destroyed() ), this, SLOT( slotObjectDestroyed() ) );

  kDebugInfo( 1000, QString("Part %1 removed").arg(part->name()) );
  d->m_parts.removeRef( part );

  if ( part == d->m_activePart )
    setActivePart( 0 );

  emit partRemoved( part );
}

void PartManager::setActivePart( Part *part, QWidget *widget )
{
  if ( d->m_activePart )
  {
    PartActivateEvent ev( false, d->m_activeWidget );
    QApplication::sendEvent( d->m_activePart, &ev );
  }

  setSelectedPart( 0L );
  
  d->m_activePart = part;
  d->m_activeWidget = widget;

  if ( d->m_activePart )
  {
    if ( !widget )
      d->m_activeWidget = part->widget();
  
    PartActivateEvent ev( true, d->m_activeWidget );
    QApplication::sendEvent( d->m_activePart, &ev );
  }

  emit activePartChanged( d->m_activePart );
}

Part *PartManager::activePart() const
{
  return d->m_activePart; 
}

QWidget *PartManager::activeWidget() const
{
  return  d->m_activeWidget;
} 

void PartManager::setSelectedPart( Part *part, QWidget *widget )
{
  if ( part == d->m_selectedPart && widget == d->m_selectedWidget )
    return;
  
  Part *oldPart = d->m_selectedPart;
  QWidget *oldWidget = d->m_selectedWidget;
  
  d->m_selectedPart = part;
  d->m_selectedWidget = widget;
  
  if ( part && !widget )
    d->m_selectedWidget = part->widget();
  
  if ( oldPart )
  {
    PartSelectEvent ev( false, oldWidget );
    QApplication::sendEvent( oldPart, &ev );
  }
  
  if ( d->m_selectedPart )
  {
    PartSelectEvent ev( true, d->m_selectedWidget );
    QApplication::sendEvent( d->m_selectedPart, &ev );
  }
}

Part *PartManager::selectedPart() const
{
  return d->m_selectedPart; 
}

QWidget *PartManager::selectedWidget() const
{
  return d->m_selectedWidget; 
} 

void PartManager::slotObjectDestroyed()
{
  kDebugInfo( 1000, "KPartManager::slotObjectDestroyed()" );
  removePart( (Part *)sender() );
}

const QList<Part> *PartManager::parts() const
{
  return &d->m_parts; 
} 

#include "partmanager.moc"
