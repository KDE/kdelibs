#ifndef test_h
#define test_h

#include "kserversocket.h"
#include "kresolver.h"
using namespace KNetwork;

class Test : public QObject
{
  Q_OBJECT

public:
  KServerSocket socket;

  Test(const QString &host, const QString &service, bool blocking);

public Q_SLOTS:
  void gotErrorSlot(int errorcode);
  void hostFoundSlot();
  void boundSlot(const KResolverEntry& target);
  void closedSlot();    
  void readyAcceptSlot();
};


#endif
