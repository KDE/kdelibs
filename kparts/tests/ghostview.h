#ifndef __example_h__
#define __example_h__

#include <kparts/mainwindow.h>
class QWidget;

class Shell : public KParts::MainWindow
{
  Q_OBJECT
public:
  Shell();
  virtual ~Shell();

protected slots:
  void slotFileOpen();

private:
  KParts::ReadOnlyPart *m_gvpart;
  QWidget * m_mainWidget;
};

#endif
