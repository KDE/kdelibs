#ifndef KDOCKTEST_H
#define KDOCKTEST_H

#include <kdockwidget.h>

class QWidget;
class DockTest : public KDockArea
{
  Q_OBJECT
public:
  DockTest( QWidget* parent=0 );

private:
  KDockWidget* m_blueDock;
  KDockWidget* m_redDock;
  KDockWidget* m_yellowDock;
};

#endif
