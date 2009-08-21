
#ifndef __parts_h__
#define __parts_h__

#include <kparts/part.h>
#include <kcomponentdata.h>

class QTextEdit;
namespace KParts {
class GUIActivateEvent;
}

class Part1 : public KParts::ReadOnlyPart
{
  Q_OBJECT
public:
  Part1( QObject *parent, QWidget * parentWidget );
  virtual ~Part1();

public slots:
 void slotBlah();
 void slotFooBar();

protected:
  virtual bool openFile();

protected:
  QTextEdit * m_edit;
  KComponentData m_componentData;
};

class Part2 : public KParts::Part
{
  Q_OBJECT
public:
  Part2( QObject *parent, QWidget * parentWidget );
  virtual ~Part2();

protected:
  // This is not mandatory - only if you care about setting the
  // part caption when the part is used in a multi-part environment
  // (i.e. in a part manager)
  // There is a default impl for ReadOnlyPart...
  virtual void guiActivateEvent( KParts::GUIActivateEvent * );
  KComponentData m_componentData;
};

#endif
