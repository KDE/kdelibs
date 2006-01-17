
#include <qwidget.h>
#include <kio/job.h>

class KLineEdit;
class QLabel;
class KFileItem;

class PreviewTest : public QWidget
{
    Q_OBJECT
public:
    PreviewTest();

private Q_SLOTS:
    void slotGenerate();
    void slotResult(KIO::Job *);
    void slotPreview( const KFileItem *, const QPixmap & );
    void slotFailed();

private:
    KLineEdit *m_url;
    QLabel *m_preview;
};

