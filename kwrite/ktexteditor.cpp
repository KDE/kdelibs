
#include "ktexteditor.h"

using namespace KTextEditor;

class View::ViewPrivate
{
public:
  ViewPrivate()
  {
  }
  ~ViewPrivate()
  {
  }

  Document *m_doc;
  bool m_bContextPopup;
};

View::View( Document *doc, QWidget *parent, const char *name )
: QWidget( parent, name )
{
  d = new ViewPrivate;
  d->m_doc = doc;
  d->m_bContextPopup = true;
}

View::~View()
{
  delete d;
}

Document *View::textDocument() const
{
  return d->m_doc;
}

void View::setInternalContextMenuEnabled( bool b )
{
  d->m_bContextPopup = b; 
}

bool View::internalContextMenuEnabled() const
{
  return d->m_bContextPopup; 
} 


class Document::DocumentPrivate
{
public:
  DocumentPrivate()
  {
  }
  ~DocumentPrivate()
  {
  }

  QList<View> m_views;
};

Document::Document( QObject *parent, const char *name )
 : KParts::ReadWritePart( parent, name )
{
  d = new DocumentPrivate;
}

Document::~Document()
{
  delete d;
}

QList<View> Document::views() const
{
  return d->m_views;
}

void Document::addView( View *view )
{
  if ( !view )
    return;

  if ( d->m_views.findRef( view ) != -1 )
    return;

  d->m_views.append( view );
  connect( view, SIGNAL( destroyed() ),
	   this, SLOT( slotViewDestroyed() ) );
}

void Document::removeView( View *view )
{
  if ( !view )
    return;

  disconnect( view, SIGNAL( destroyed() ),
	      this, SLOT( slotViewDestroyed() ) );

  d->m_views.removeRef( view );
}

void Document::slotViewDestroyed()
{
  const View *view = static_cast<const View *>( sender() );
  d->m_views.removeRef( view );
}

#include "ktexteditor.moc"
