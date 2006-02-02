
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kcmdlineargs.h>
#include <kio/previewjob.h>

#include "previewtest.moc"

PreviewTest::PreviewTest()
    :QWidget()
{
    QGridLayout *layout = new QGridLayout(this, 2, 2);
    m_url = new KLineEdit(this);
    m_url->setText("/home/malte/gore_bush.jpg");
    layout->addWidget(m_url, 0, 0);
    QPushButton *btn = new QPushButton("Generate", this);
    connect(btn, SIGNAL(clicked()), SLOT(slotGenerate()));
    layout->addWidget(btn, 0, 1);
    m_preview = new QLabel(this);
    m_preview->setMinimumSize(400, 300);
    layout->addMultiCellWidget(m_preview, 1, 1, 0, 1);
}

void PreviewTest::slotGenerate()
{
    KUrl::List urls;
    urls.append(m_url->text());
    KIO::PreviewJob *job = KIO::filePreview(urls, m_preview->width(), m_preview->height(), true, 48);
    connect(job, SIGNAL(result(KIO::Job*)), SLOT(slotResult(KIO::Job*)));
    connect(job, SIGNAL(gotPreview(const KFileItem *, const QPixmap &)), SLOT(slotPreview(const KFileItem *, const QPixmap &)));
    connect(job, SIGNAL(failed(const KFileItem *)), SLOT(slotFailed()));
}

void PreviewTest::slotResult(KIO::Job*)
{
    kDebug() << "PreviewTest::slotResult(...)" << endl;
}

void PreviewTest::slotPreview(const KFileItem *, const QPixmap &pix)
{
    kDebug() << "PreviewTest::slotPreview()" << endl;
    m_preview->setPixmap(pix);
}

void PreviewTest::slotFailed()
{
    kDebug() << "PreviewTest::slotFailed()" << endl;
    m_preview->setText("failed");
}

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc,argv,"previewtest", 0, 0, 0, 0);
    KApplication app;
    PreviewTest *w = new PreviewTest;
    w->show();
    app.setMainWidget(w);
    return app.exec();
}

