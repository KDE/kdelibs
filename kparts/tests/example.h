
#ifndef __example_h__
#define __example_h__

#include <kpart.h>
#include <kpartmanager.h>
#include <ktmainwindow.h>

class KAction;
class QSplitter;
class QMultiLineEdit;
class KTMainWindowGUIBuilder;

class Shell : public KTMainWindow
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

  KReadOnlyPart *m_part1;
  KPart *m_part2;
  KReadWritePart *m_editorpart;
  KPartManager *m_manager;
  QWidget *m_splitter;
  KTMainWindowGUIBuilder *m_builder;
};

class Part1 : public KReadOnlyPart
{
  Q_OBJECT
public:
  Part1( QWidget * parentWidget );
  virtual ~Part1();

protected:
  virtual bool openFile();

protected:
  virtual QString configFile() const;
  QMultiLineEdit * m_edit;
};

class Part2 : public KPart
{
  Q_OBJECT
public:
  Part2( QWidget * parentWidget );
  virtual ~Part2();

protected:
  virtual QString configFile() const;
};

#endif
