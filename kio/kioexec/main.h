#ifndef _main_h
#define _main_h

#include <qobject.h>
#include <qstring.h>
#include <qstrlist.h>
#include <qtimer.h>

namespace KIO { class Job; }

class KIOExec : public QObject
{
    Q_OBJECT
public:
    KIOExec();

public slots:
    void slotResult( KIO::Job * );
    void slotRunApp();

protected:
    bool tempfiles;
    QString suggestedFileName;
    int counter;
    int expectedCounter;
    QString command;
    struct fileInfo {
       QString path;
       KURL url;
       int time;
    };
    QValueList<fileInfo> fileList;
};

#endif
