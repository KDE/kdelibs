
#ifndef __example_h__
#define __example_h__

#include <kpart.h>
#include <kembedmanager.h>

class QSplitter;

class Shell : public QWidget
{
  Q_OBJECT
public:
  Shell( KPart *part1, KPart *part2 );
  virtual ~Shell();

protected slots:
  void slotActivePartChanged( KPart *newPart, KPart *oldPart );

protected:
  virtual void resizeEvent( QResizeEvent * );
  
private:
  KPart *m_part1, *m_part2;
  KEmbedManager *m_manager;
  QSplitter *m_splitter;
};

class Part1 : public KPart
{
  Q_OBJECT
public:
  Part1();
  virtual ~Part1();

protected:
  virtual QString configFile() const;
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
