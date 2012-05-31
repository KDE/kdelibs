
#include "previewtest.h"

#include <QLabel>
#include <QApplication>
#include <QLayout>
#include <QPushButton>

#include <kdebug.h>
#include <klineedit.h>
#include <kio/previewjob.h>

#include "previewtest.moc"

PreviewTest::PreviewTest()
    :QWidget()
{
    QGridLayout *layout = new QGridLayout(this);
    m_url = new KLineEdit(this);

    QString path;
    KIconLoader().loadMimeTypeIcon("video-x-generic", KIconLoader::Desktop, 256,
                                   KIconLoader::DefaultState, QStringList(),
                                   &path);

    m_url->setText(path);
    layout->addWidget(m_url, 0, 0);
    QPushButton *btn = new QPushButton("Generate", this);
    connect(btn, SIGNAL(clicked()), SLOT(slotGenerate()));
    layout->addWidget(btn, 0, 1);
    m_preview = new QLabel(this);
    m_preview->setMinimumSize(400, 300);
    layout->addWidget(m_preview, 1, 0, 1, 2);
}

void PreviewTest::slotGenerate()
{
    KFileItemList items;
    items.append(KFileItem(KFileItem::Unknown, KFileItem::Unknown, KUrl(m_url->text()), true));

    KIO::PreviewJob *job = KIO::filePreview(items, QSize(m_preview->width(), m_preview->height()));
    connect(job, SIGNAL(result(KJob*)), SLOT(slotResult(KJob*)));
    connect(job, SIGNAL(gotPreview(KFileItem,QPixmap)), SLOT(slotPreview(KFileItem,QPixmap)));
    connect(job, SIGNAL(failed(KFileItem)), SLOT(slotFailed()));
}

void PreviewTest::slotResult(KJob*)
{
    kDebug() << "PreviewTest::slotResult(...)";
}

void PreviewTest::slotPreview(const KFileItem&, const QPixmap &pix)
{
    kDebug() << "PreviewTest::slotPreview()";
    m_preview->setPixmap(pix);
}

void PreviewTest::slotFailed()
{
    kDebug() << "PreviewTest::slotFailed()";
    m_preview->setText("failed");
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    PreviewTest *w = new PreviewTest;
    w->show();
    return app.exec();
}

