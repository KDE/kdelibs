
#ifndef __example_h__
#define __example_h__

#include <kpart.h>
#include <kpartmanager.h>
#include <kpartsmainwindow.h>

class KAction;
class QSplitter;
class QMultiLineEdit;

class Shell : public KParts::MainWindow
{
  Q_OBJECT
public:
  Shell();
  virtual ~Shell();

protected slots:
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

class Part1 : public KParts::ReadOnlyPart
{
  Q_OBJECT
public:
  Part1( QWidget * parentWidget );
  virtual ~Part1();

  virtual KInstance *instance();

protected:
  virtual bool openFile();

protected:
  QMultiLineEdit * m_edit;
  KInstance *m_instance;
};

class Part2 : public KParts::Part
{
  Q_OBJECT
public:
  Part2( QWidget * parentWidget );
  virtual ~Part2();

  virtual KInstance *instance();

protected:

  KInstance *m_instance;
};

#endif
