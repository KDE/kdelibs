#ifndef _KDOCKTOOLBAR_H
#define _KDOCKTOOLBAR_H
#include <qcombo.h>
#include <qframe.h>
#include <qstring.h> 
#include <kbutton.h> 
#include <qpixmap.h>
#include <qlist.h>
#include <qpopmenu.h>

#include "klined.h"
#include "kcombo.h"
#include "ktoolbar.h"

/**
* Floating/Docking toolbar.
* @short Floating/Docking toolbar.
*/
class KDockToolBar : public KToolBar
{
  Q_OBJECT

public:
  KDockToolBar( QWidget *parent=NULL,char *name=NULL );
  virtual ~KDockToolBar();

  virtual void mousePressEvent( QMouseEvent* pEvent );
  virtual void mouseReleaseEvent( QMouseEvent* pEvent );
  virtual void mouseMoveEvent( QMouseEvent* pEvent );
  virtual void mouseDoubleClickEvent( QMouseEvent* pEvent );

private:
  bool _bMightDock;
  QPoint _aOldPos;
  BarPosition _oldPosition;
  int _oldX, _oldY; // could use those from KToolbar, but they are private
  QWidget* _pParent; // same here
};

#endif
