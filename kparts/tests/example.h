
#ifndef __example_h__
#define __example_h__

#include <kpart.h>
#include <kpartmanager.h>
#include <ktmainwindow.h>

class QVBox;
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
  void slotActivePartChanged( KPart *newPart, KPart *oldPart );

protected:
  virtual void resizeEvent( QResizeEvent * );
  
private:
  KReadOnlyPart *m_part1;
  KPart *m_part2;
  KPartManager *m_manager;
  QSplitter *m_splitter;
  KTMainWindowGUIBuilder *m_builder;
};

class Part1 : public KReadOnlyPart
{
  Q_OBJECT
public:
  Part1();
  virtual ~Part1();

  //  virtual QWidget *widget();

protected:
  virtual bool openFile();

protected:
  virtual QString configFile() const;
  QMultiLineEdit * m_edit;
  //  QVBox *m_box;
};

class Part2 : public KPart
{
  Q_OBJECT
public:
  Part2();
  virtual ~Part2();

  //  virtual QWidget *widget();

protected:
  virtual QString configFile() const;

  //  QWidget *m_widget;
};

#endif
