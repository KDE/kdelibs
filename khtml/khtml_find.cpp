
#include "khtml_find.h"
#include "khtml_part.h"

#include <klocale.h>
#include <kmessagebox.h>

KHTMLFind::KHTMLFind( KHTMLPart *part, const char *name )
: KEdFind( part->widget(), name )
{
  connect( this, SIGNAL( done() ),
	   this, SLOT( slotDone() ) );
  connect( this, SIGNAL( search() ),
	   this, SLOT( slotSearch() ) );
  m_first = true;
  m_part = part;
  m_found = false;
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
  
  bool forward = !get_direction();
  
  if ( m_part->findTextNext( getText(), forward, case_sensitive() ) )
    m_found = true;
  else if ( m_found )
  {
    if ( forward )
    {
      if ( KMessageBox::questionYesNo( this,
           i18n("End of document reached.\n"\
                "Continue from the beginning?"),
	   i18n("Find") ) == KMessageBox::Yes )
      {
        m_first = true;
	slotSearch();
      }
    }
    else
    {
      if ( KMessageBox::questionYesNo( this,
           i18n("Beginning of document reached.\n"\
                "Continue from the end?"),
	   i18n("Find") ) == KMessageBox::Yes )
      {
        m_first = true;
	slotSearch();
      }
    }
  }
  else
    KMessageBox::information( this, i18n( "Search string not found." ), i18n( "Find" ) );
}

#include "khtml_find.moc"
