#include "khtmlembed.h"

KHTMLEmbededWidget::KHTMLEmbededWidget( QWidget *_parent, int _frameborder, bool _allowresize ) : QWidget( _parent )
{
  iFrameBorder = _frameborder;
  bAllowResize = _allowresize;
}

#include "khtmlembed.moc"
