#ifndef streamsockettest_h
#define streamsockettest_h

#include "kstreamsocket.h"
#include "kresolver.h"
using namespace KNetwork;

class Test : public QObject
{
  Q_OBJECT

public:
  KStreamSocket socket;

  Test(QString host, QString service, QString lhost, QString lservice, bool blocking);

public Q_SLOTS:
  void stateChangedSlot(int newstate);
  void gotErrorSlot(int errorcode);
  void hostFoundSlot();
  void boundSlot(const KNetwork::KResolverEntry& target);
  void aboutToConnectSlot(const KNetwork::KResolverEntry& target);
  void connectedSlot(const KNetwork::KResolverEntry& target);
  void timedOutSlot();
  void closedSlot();
  void readyReadSlot();
  void readyWriteSlot();
};

#endif
