
#include "khtml_run.h"
#include "khtml_part.h"

KHTMLRun::KHTMLRun( KHTMLPart *part, khtml::ChildFrame *child, const KURL &url )
: KRun( url.url(), 0, false, true )
{
  m_part = part;
  m_child = child;
  m_url = url;
}

void KHTMLRun::foundMimeType( const QString &mimetype )
{
  m_part->processChildRequest( m_child, m_url, mimetype );
  m_bFinished = true;
  m_timer.start( 0, true );
}
