// KRollUp - Widget grabber
// This file is part of KDE project
// (c) Sven Radej 1997, sven.radej@iname.com

#include <qwidget.h>

#include "kfloater.moc"

KFloater::KFloater (const char *_title, const char * name)
  : QWidget (0, name, WStyle_Customize | WStyle_Title | WStyle_Minimize | WStyle_DialogBorder)

{
  title = _title;
  widgetIsMine = FALSE;
  noPaint = FALSE;
  noSize  = FALSE;
}

void KFloater::grabWidget (QWidget *_element)
{
  if (widgetIsMine == TRUE)
	return;

  element = _element;
  if (title != 0)
	setCaption (title);
  Parent = element->parentWidget();
  oldX = element->x();
  oldY = element->y();
  element->hide();
  resize (element->width(), element->height());
   
  element->recreate(this, 0, QPoint(0,0), TRUE);

  widgetIsMine = TRUE;
  // connect (this, SIGNAL(destroyed()), this, SLOT(killMe()));
  // warning ("KFloater: connected");
  show ();
}

void KFloater::releaseWidget (QWidget *_w)
{
  if ((widgetIsMine == TRUE) && (_w == element))
	{
	  element->recreate(Parent, 0, QPoint (oldX, oldY), TRUE);
	  widgetIsMine = FALSE;
	  hide ();
	}
}

KFloater::~KFloater ()
{
  if (widgetIsMine == TRUE)
	{
	  element->recreate(Parent, 0, QPoint (oldX, oldY), TRUE);
	  widgetIsMine = FALSE;
	  emit killed ();
	}
}

void KFloater::closeEvent (QCloseEvent *)
{
  delete this;
}

void KFloater::resizeEvent (QResizeEvent * )
{
  if (noSize == TRUE)           // Item might try to correct our size
    return;                    // Toolbars do that
  // That's why we need semaphores
  noSize = TRUE;
  if (widgetIsMine == TRUE)
	element->resize (width(), height());
  noSize = FALSE;
}

void KFloater::paintEvent (QPaintEvent * )
{
  if (noPaint == TRUE)
    return;
  
  noPaint = TRUE;
  if (widgetIsMine == TRUE)
    element->repaint();

  noPaint = FALSE;
}

