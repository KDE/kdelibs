#include <qobject.h>
#include <qevent.h>
#include <qapplication.h>
#include <qsemaphore.h>

#include "qsynchronizing.h"

namespace
{
  class QSynchronizer : public QObject
  {
  public:
    QSynchronizer() : QObject(NULL, "synchronizer object")
    { }

    virtual bool event(QEvent *e);

    static QSynchronizer synchronizer;
  };

  class QSynchroEvent : public QEvent
  {
  public:
    static const Type SynchroType = static_cast<Type>(102);

    void (*target)(QSynchronizing::ArgumentsBase*);
    QSynchronizing::ArgumentsBase *arguments;
    QSemaphore *sem;

    QSynchroEvent() : QEvent(SynchroType)
    { }
  };

  QSynchronizer QSynchronizer::synchronizer;

  inline void doEvent(QSynchroEvent *e)
  {
    QSemaphore sem(1);
    e->sem = &sem;

    sem++;
    QApplication::postEvent(&QSynchronizer::synchronizer, e);
    sem++;			// max is 1, so this will wait
  }

  void QSynchronizing::doSynchronize(void (* target)(ArgumentsBase*), 
				   ArgumentsBase &arguments)
  {
    QSynchroEvent *e = new QSynchroEvent;
    e->target = target;
    e->arguments = &arguments;
    doEvent(e);
  }

  bool QSynchronizer::event(QEvent *e)
  {
    if (e->type() != QSynchroEvent::SynchroType)
      return false;

    QSynchroEvent *se = static_cast<QSynchroEvent*>(e);
    (se->target)(se->arguments);
    (*se->sem)--;
    return true;
  }
}
    
