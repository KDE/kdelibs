#ifndef __example_h__
#define __example_h__

#include <kparts/mainwindow.h>

class Shell : public KParts::MainWindow
{
  Q_OBJECT
public:
  Shell();
  virtual ~Shell();

  void openURL( const KURL & url );

protected Q_SLOTS:
  void slotFileOpen();

private:
  KParts::ReadOnlyPart *m_gvpart;
};

#endif
