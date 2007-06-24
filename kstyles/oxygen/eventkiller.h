#ifndef EVENTKILLER_H
#define EVENTKILLER_H

#include <QObject>

class EventKiller : public QObject
{
   Q_OBJECT
public:
   bool eventFilter( QObject *, QEvent *) {
      return true;
   }
};

static EventKiller *eventKiller = new EventKiller();

#define _BLOCKEVENTS_(obj) obj->installEventFilter(eventKiller)
#define _UNBLOCKEVENTS_(obj) obj->removeEventFilter(eventKiller)

#endif // EVENTKILLER_H
