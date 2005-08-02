#ifndef netiftest_h
#define netiftest_h

#include <QObject>
#include "knetworkinterface.h"

using namespace KNetwork;

class Test : QObject 
{
  Q_OBJECT

public:
  KNetworkInterface *m_interface;

  Test(const QString& interface);
  void display();
  
};


#endif
