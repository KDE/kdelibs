
#ifndef KQEVENTUTIL_H
#define KQEVENTUTIL_H

#include <qstring.h>
class QEvent;

class KQEventUtil {
    public:
        static QString getQtEventName(QEvent *e);
};

#endif
