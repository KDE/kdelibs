#ifndef __khtml_run_h__
#define __khtml_run_h__

#include <krun.h>
#include <kurl.h>

class KHTMLPart;

namespace khtml
{
  struct ChildFrame;
};

class KHTMLRun : public KRun
{
  Q_OBJECT
public:
  KHTMLRun( KHTMLPart *part, khtml::ChildFrame *child, const KURL &url );

  virtual void foundMimeType( const QString &mimetype );

private:
  KHTMLPart *m_part;
  khtml::ChildFrame *m_child;
  KURL m_url;
};

#endif
