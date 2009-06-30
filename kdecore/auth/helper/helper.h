#ifndef HELPER_CLASS_H
#define HELPER_CLASS_H

#include <QObject>

class Class : public QObject
{
    Q_OBJECT
    
    public slots:
        void action();
};

#endif