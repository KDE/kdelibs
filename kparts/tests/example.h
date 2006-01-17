
#ifndef __example_h__
#define __example_h__

#include <kparts/partmanager.h>
#include <kparts/mainwindow.h>

class KAction;
class QWidget;

class Shell : public KParts::MainWindow
{
  Q_OBJECT
public:
  Shell();
  virtual ~Shell();

protected Q_SLOTS:
  void slotFileOpen();
  void slotFileOpenRemote();
  void slotFileEdit();
  void slotFileCloseEditor();

protected:
  void embedEditor();

private:

  KAction * m_paEditFile;
  KAction * m_paCloseEditor;

  KParts::ReadOnlyPart *m_part1;
  KParts::Part *m_part2;
  KParts::ReadWritePart *m_editorpart;
  KParts::PartManager *m_manager;
  QWidget *m_splitter;
};

#endif
