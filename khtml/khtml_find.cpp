
#include "khtml_find.h"
#include "khtml_part.h"

KHTMLFind::KHTMLFind( KHTMLPart *part, const char *name )
: KEdFind( part->widget(), name )
{
  connect( this, SIGNAL( done() ),
	   this, SLOT( slotDone() ) );
  connect( this, SIGNAL( search() ),
	   this, SLOT( slotSearch() ) );
  m_first = true;
  m_part = part;
}

KHTMLFind::~KHTMLFind()
{ 
}

void KHTMLFind::slotDone()
{
  accept(); 
} 

void KHTMLFind::slotSearch()
{
  if ( m_first )
  {
    m_part->findTextBegin();
    m_first = false;
  }
  m_part->findTextNext( getText(), !get_direction(), case_sensitive() );
}

#include "khtml_find.moc"
