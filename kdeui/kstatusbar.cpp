#include <kstatusbar.h>
#include "kstatusbar.moc"
#include <qpainter.h> 
// Revision 1.10  1998/04/21 20:37:48  radej
KStatusBar::KStatusBar(QWidget *parent, const char *name) :
  QFrame(parent, name, 0, FALSE) {
    resize(100,19);
};

void KStatusBar::drawContents(QPainter *p) {
  p->drawText(10,15,"TODO: Implementation");
};

//Eh!!!

