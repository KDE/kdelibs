/* Insert license header here */
#ifndef MYCLASS_H
#define MYCLASS_H

#include <foobar_export.h>

#include <QtCore/QObject>

class FOOBAR_EXPORT MyClass : public QObject
{
    Q_OBJECT
public:
    MyClass();
};

#endif /* MYCLASS_H */
