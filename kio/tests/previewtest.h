
#include <qwidget.h>
#include <kio/job.h>

class KLineEdit;
class QLabel;

class PreviewTest : public QWidget
{
    Q_OBJECT
public:
    PreviewTest();

private slots:
    void slotGenerate();
    void slotResult(KIO::Job *);
    void slotPreview( const KURL &, const QPixmap & );
    void slotFailed();

private:
    KLineEdit *m_url;
    QLabel *m_preview;
};

