#ifndef KNEWSTUFF2_DXS_ENGINE_H
#define KNEWSTUFF2_DXS_ENGINE_H

#include <knewstuff2/core/coreengine.h>

namespace KNS {

class Dxs;

class KDE_EXPORT DxsEngine : public CoreEngine
{
    Q_OBJECT
  public:
    DxsEngine();
    ~DxsEngine();

    enum Policy
    {
      DxsNever,
      DxsIfPossible,
      DxsAlways
    };

    void setDxsPolicy(Policy policy);

    void loadEntries(Provider *provider);
    //void downloadPreview(Entry *entry);
    //void downloadPayload(Entry *entry);
    // FIXME: the upload/download stuff is only necessary when we use
    // integrated base64-encoded files; maybe delay to later version?

    //bool uploadEntry(Provider *provider, Entry *entry);

  //signals:
    // FIXME: we need a lot more signals here for DXS!

  //private slots:
    // FIXME: idem for slots
    void slotEntriesLoaded(KNS::Entry::List *list);
    void slotEntriesFailed();

  private:
    Dxs *m_dxs;
    Policy m_dxspolicy;
};

}

#endif
