
#ifndef __example_h__
#define __example_h__

#include <kpart.h>
#include <kshell.h>
#include <kembedmanager.h>

class QSplitter;
class QMultiLineEdit;

class Shell : public KShell
{
  Q_OBJECT
public:
  Shell( KReadOnlyPart *part1, KPart *part2 );
  virtual ~Shell();

protected slots:
  void slotFileOpen();
  void slotFileOpenRemote();
  void slotActivePartChanged( KPart *newPart, KPart *oldPart );

protected:
  virtual QString config() const;
  virtual void resizeEvent( QResizeEvent * );
  
private:
  KReadOnlyPart *m_part1;
  KPart *m_part2;
  KEmbedManager *m_manager;
  QSplitter *m_splitter;
};

class Part1 : public KReadOnlyPart
{
  Q_OBJECT
public:
  Part1();
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
  Part2();
  virtual ~Part2();

protected:
  virtual QString configFile() const;
};

#endif
