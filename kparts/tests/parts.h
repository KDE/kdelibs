
#ifndef __parts_h__
#define __parts_h__

#include <kpart.h>

class QMultiLineEdit;

class Part1 : public KParts::ReadOnlyPart
{
  Q_OBJECT
public:
  Part1( QObject *parent, QWidget * parentWidget );
  virtual ~Part1();

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
  Part2( QObject *parent, QWidget * parentWidget );
  virtual ~Part2();

protected:

  KInstance *m_instance;
};

#endif
