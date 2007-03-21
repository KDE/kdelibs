#ifndef KNEWSTUFF2_ENGINE_H
#define KNEWSTUFF2_ENGINE_H

#include <knewstuff2/dxs/dxsengine.h>

namespace KNS {

class KDE_EXPORT Engine : public DxsEngine
{
    Q_OBJECT
  public:
    Engine();
    ~Engine();

    KNS::Entry::List downloadDialogModal();
    KNS::Entry *uploadDialogModal();

    void downloadDialog();
    void uploadDialog();
};

}

#endif
