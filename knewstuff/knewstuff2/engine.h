#ifndef KNEWSTUFF2_ENGINE_H
#define KNEWSTUFF2_ENGINE_H

#include <knewstuff2/dxs/dxsengine.h>

namespace KNS {

class UploadDialog;
class DownloadDialog;

class KDE_EXPORT Engine : public DxsEngine
{
    Q_OBJECT
  public:
    Engine();
    ~Engine();

    KNS::Entry::List downloadDialogModal();
    KNS::Entry *uploadDialogModal(QString file);

    void downloadDialog();
    void uploadDialog();

  public slots:
    void slotProviderLoaded(KNS::Provider *provider);
    void slotProvidersFailed();
    void slotEntryLoaded(KNS::Entry *entry);
    void slotEntriesFailed();
    void slotEntryUploaded();
    void slotEntryFailed();

    void slotProvidersFinished();
    void slotEntriesFinished();

  private:
    enum Command
    {
        none,
        upload,
        download
    };

    Command m_command;
    UploadDialog *m_uploaddialog;
    DownloadDialog *m_downloaddialog;
    QString m_uploadfile;
    KNS::Entry *m_entry;
    KNS::Provider::List m_providers;
};

}

#endif
