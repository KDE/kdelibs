
#include <qstring.h>
class QEvent;

class KQEventUtil {
    public:
        static QString getQtEventName(QEvent *e);
};
