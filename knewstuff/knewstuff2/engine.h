#ifndef KNEWSTUFF2_ENGINE_H
#define KNEWSTUFF2_ENGINE_H

#include <knewstuff2/dxs/dxsengine.h>

namespace KNS {

class KDE_EXPORT XEngine : public DxsEngine
{
    Q_OBJECT
  public:
    XEngine();
    ~XEngine();

    KNS::Entry::List downloadDialogModal();
    KNS::Entry *uploadDialogModal();

    void downloadDialog();
    void uploadDialog();
};

}

#endif
