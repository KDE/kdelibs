#ifndef KDOCKTEST_H
#define KDOCKTEST_H

#include <k3dockwidget.h>

class QWidget;
class DockTest : public K3DockArea
{
  Q_OBJECT
public:
  DockTest( QWidget* parent=0 );

private:
  K3DockWidget* m_blueDock;
  K3DockWidget* m_redDock;
  K3DockWidget* m_yellowDock;
};

#endif
