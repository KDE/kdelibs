#ifndef _KSTATUSBAR_H
#define _KSTATUSBAR_H

#include <qframe.h>

class KStatusBar : public QFrame {
  Q_OBJECT

public:
  KStatusBar(QWidget *parent=NULL, const char *name=NULL);

protected:
  void drawContents(QPainter *);
};

#endif
