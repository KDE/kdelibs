#ifndef GEOTEST_H
#define GEOTEST_H

#include <qobject.h>

class GeoTest : public QObject
{
  Q_OBJECT

  private slots:
    void emptyConstructor();
    void constructor();
    void isValid();
    void setData();
    void equals();
    void differs();
    void serialization();
};

#endif
