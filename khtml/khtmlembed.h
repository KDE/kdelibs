#ifndef __html_embed_h__
#define __html_embed_h__

#include <qwidget.h>

class KHTMLEmbededWidget : public QWidget
{
  Q_OBJECT
public:
  KHTMLEmbededWidget( QWidget *_parent, int _frameborder, bool _allowresize );

  int frameBorder() { return iFrameBorder; }
  bool allowResize() { return bAllowResize; }
  
protected:
  int iFrameBorder;
  bool bAllowResize;
};

#endif
