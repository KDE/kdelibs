#include "knewstuff2_download.h"

#include <knewstuff2/core/coreengine.h>
#include <knewstuff2/core/author.h>

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kicon.h>

#include <qpushbutton.h>
#include <qlayout.h>
#include <qlistwidget.h>
#include <qtabwidget.h>
#include <qtablewidget.h>
//FIXME just include as needed
#include <QtGui>

#include <unistd.h> // for exit()
#include <stdio.h> // for stdout

class KNSButton : public QAbstractButton
{
public:
    KNSButton(QWidget *parent = 0);
    void setText(const QString &text) { t = text; }
    const QString text() { return t; }
protected:
    void paintEvent(QPaintEvent *event);
private:
    QString t;
};

KNSButton::KNSButton(QWidget *parent /*= 0*/)
    : QAbstractButton(parent)
{
    t = "";
    setCheckable(true);
    connect(this, SIGNAL(toggled()), this, SLOT(update()));
}

void KNSButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPalette palette;
    QPainter p(this);
    int h = height();
    int w = width();

    //Draw the background
    p.setPen(Qt::NoPen);
    if (isChecked()) {
    p.setBrush(palette.brush(QPalette::Highlight));
    } else {
    p.setBrush(palette.brush(QPalette::Base));
    }
    p.drawRect(QRect((rect().x() + 1), (rect().y() + 2), (w - 1), (h - 4)));

    //Draw the text and the icon
    p.setPen(Qt::black);
    QIcon ic = icon();
    if (!ic.isNull()) {
        p.drawPixmap(QRect(10, ((h/2) - 16), 32, 32),
                     ic.pixmap(QSize(32, 32)), QRect(0, 0, 32, 32));
    }

    p.drawText(QRect((w - (w - 40)), (h/2 - 20), (w - 40), h), Qt::AlignCenter, text());
}

class FeedWidget : public QTableWidget
{
public:
    FeedWidget()
    : QTableWidget()
    {
        m_entries = 0;
        m_previewentries = 0;

        setRowCount(20);
        setColumnCount(5);

        QStringList headers;
        headers << "Name";
        headers << "Author";
        headers << "Downloads";
        headers << "Rating";
        headers << "Preview";
        setHorizontalHeaderLabels(headers);
    }

    void addEntry(KNS::Entry *entry)
    {
        QTableWidgetItem *nameitem = new QTableWidgetItem();
        nameitem->setText(entry->name().representation());

        QTableWidgetItem *authoritem = new QTableWidgetItem();
        authoritem->setText(entry->author().name());

        QTableWidgetItem *downloadsitem = new QTableWidgetItem();
        downloadsitem->setText(QString::number(entry->downloads()));

        QTableWidgetItem *ratingitem = new QTableWidgetItem();
        ratingitem->setText(QString::number(entry->rating()));

        setItem(m_entries, 0, nameitem);
        setItem(m_entries, 1, authoritem);
        setItem(m_entries, 2, downloadsitem);
        setItem(m_entries, 3, ratingitem);

        m_entries++;
    }

    void addPreview(KUrl previewfile)
    {
        QTableWidgetItem *previewitem = new QTableWidgetItem();
        previewitem->setSizeHint(QSize(64, 64));
        previewitem->setIcon(QIcon(previewfile.path()));

        setItem(m_previewentries, 4, previewitem);
        m_previewentries++;
        // FIXME: mapping from entry to url and to vertical position(s)
    }

private:
    int m_entries;
    int m_previewentries;
};

KNewStuff2Download::KNewStuff2Download()
: QWidget()
{
    m_engine = NULL;
    m_activefeed = NULL;
    m_activeentry = NULL;

    resize(800, 600);
    setWindowTitle("KNewStuff2 Download Dialog Test");

    QPushButton *installbutton = new QPushButton("Install");
    connect(installbutton, SIGNAL(clicked()), SLOT(slotInstall()));

    QPushButton *closebutton = new QPushButton("Close");
    connect(closebutton, SIGNAL(clicked()), SLOT(close()));

    m_providerlist = new QListWidget();
    m_providerlist->setFixedWidth(200);

    m_feeds = new QTabWidget();

    frame = new QFrame(this);
    frame->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    QPalette palette = this->palette();
    palette.setColor(backgroundRole(), palette.color(QPalette::Base));
    frame->setPalette(palette);
    frame->setLineWidth(1);
    frame->setMidLineWidth(0);

    recentButton = new KNSButton(frame);
    recentButton->setIcon(KIcon("alarmclock"));
    recentButton->setText("Most recent");

    estimatedButton = new KNSButton(frame);
    estimatedButton->setIcon(KIcon("favorites"));
    estimatedButton->setText("Most estimated");

    wantedButton = new KNSButton(frame);
    wantedButton->setIcon(KIcon("kget"));
    wantedButton->setText("Most wanted");

    connect(recentButton, SIGNAL(toggled(bool)), this, SLOT(buttonToggled(bool)));
    connect(estimatedButton, SIGNAL(toggled(bool)), this, SLOT(buttonToggled(bool)));
    connect(wantedButton, SIGNAL(toggled(bool)), this, SLOT(buttonToggled(bool)));

    frame->setMinimumHeight(40);
    recentButton->setChecked(true);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addWidget(m_providerlist);
    hbox->addWidget(m_feeds);

    QVBoxLayout *vbox = new QVBoxLayout();
    setLayout(vbox);
    vbox->addWidget(frame);
    vbox->addLayout(hbox);
    vbox->addWidget(installbutton);
    vbox->addWidget(closebutton);

    show();
}

void KNewStuff2Download::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    int frameWidth = frame->size().width();
    int frameHeight = frame->size().height();

    recentButton->setGeometry(0, 0, frameWidth/3, frameHeight);
    estimatedButton->setGeometry(frameWidth/3, 0, frameWidth/3, frameHeight);

    //NOTE int have just integer prcision, so, they can be approximated when scaling the object.
    //That's why those cases are possible.
    if (((frameWidth/3)*3) > (frameWidth)) {
        wantedButton->setGeometry((frameWidth/3)*2, 0, (frameWidth/3 - 2), frameHeight);
    } else if (((frameWidth/3)*3) == (frameWidth)) {
        wantedButton->setGeometry((frameWidth/3)*2, 0, frameWidth/3 - 1, frameHeight);
    } else {
        wantedButton->setGeometry((frameWidth/3)*2, 0, (frameWidth/3), frameHeight);
    }
}
void KNewStuff2Download::buttonToggled(bool checked)
{
    if (checked) {
        if (sender() == recentButton) {
            m_checkedButton = KNewStuff2Download::Recent;
            estimatedButton->setChecked(false);
            wantedButton->setChecked(false);
            switchCategory(KNewStuff2Download::Recent);
        } else if (sender() == estimatedButton) {
            m_checkedButton = KNewStuff2Download::Estimated;
            recentButton->setChecked(false);
            wantedButton->setChecked(false);
            switchCategory(KNewStuff2Download::Estimated);
        } else if (sender() == wantedButton) {
            m_checkedButton = KNewStuff2Download::Wanted;
            recentButton->setChecked(false);
            estimatedButton->setChecked(false);
            switchCategory(KNewStuff2Download::Wanted);
        }
    } else {
        if (m_checkedButton == KNewStuff2Download::Recent && sender() == recentButton) {
                recentButton->setChecked(true);
        } else if (m_checkedButton == KNewStuff2Download::Estimated && sender() == estimatedButton) {
            estimatedButton->setChecked(true);
        } else if (m_checkedButton == KNewStuff2Download::Wanted && sender() == wantedButton) {
            wantedButton->setChecked(true);
        }
    }
}

void KNewStuff2Download::switchCategory(KNewStuff2Download::Category category)
{
    Q_UNUSED(category);
//TODO
}

void KNewStuff2Download::run()
{
    kDebug() << "-- test kns2 engine" << endl;

    m_engine = new KNS::CoreEngine();
    bool success = m_engine->init("knewstuff2_test.knsrc");

    kDebug() << "-- engine test result: " << success << endl;

    if(success) {
        connect(m_engine,
            SIGNAL(signalProviderLoaded(KNS::Provider*)),
            SLOT(slotProviderLoaded(KNS::Provider*)));
        connect(m_engine,
            SIGNAL(signalProvidersFailed()),
            SLOT(slotProvidersFailed()));
        connect(m_engine,
            SIGNAL(signalEntryLoaded(KNS::Entry*, const KNS::Feed*, const KNS::Provider*)),
            SLOT(slotEntryLoaded(KNS::Entry*, const KNS::Feed*, const KNS::Provider*)));
        connect(m_engine,
            SIGNAL(signalEntriesFailed()),
            SLOT(slotEntriesFailed()));
        connect(m_engine,
            SIGNAL(signalPayloadLoaded(KUrl)),
            SLOT(slotPayloadLoaded(KUrl)));
        connect(m_engine,
            SIGNAL(signalPayloadFailed()),
            SLOT(slotPayloadFailed()));
        connect(m_engine,
            SIGNAL(signalPreviewLoaded(KUrl)),
            SLOT(slotPreviewLoaded(KUrl)));
        connect(m_engine,
            SIGNAL(signalPreviewFailed()),
            SLOT(slotPreviewFailed()));

        m_engine->start(false);
    } else {
        kWarning() << "ACHTUNG: you probably need to 'make install' the knsrc file first." << endl;
        kWarning() << "Although this is not required anymore, so something went really wrong." << endl;
    }
}

void KNewStuff2Download::slotInstall()
{
    kDebug() << "Attempt to install entry..." << endl;

    if(!m_activeentry) {
        kError() << "No entries loaded!" << endl;
        return;
    } else {
        kDebug() << "Entry to install is called " << m_activeentry->name().representation() << endl;
    }

    m_engine->downloadPayload(m_activeentry);
}

void KNewStuff2Download::slotProviderLoaded(KNS::Provider *provider)
{
    kDebug() << "SLOT: slotProviderLoaded" << endl;
    kDebug() << "-- provider: " << provider->name().representation() << endl;

    QListWidgetItem *item = new QListWidgetItem(QIcon(), provider->name().representation());
    m_providerlist->addItem(item);

    QStringList feeds = provider->feeds();
    for(QStringList::Iterator it = feeds.begin(); it != feeds.end(); it++) {
        KNS::Feed *feed = provider->downloadUrlFeed((*it));

        FeedWidget *feedtab = new FeedWidget();
        m_feeds->addTab(feedtab, feed->name().representation());
        m_activefeed = feedtab;
    }

    if(feeds.size() == 0) {
        FeedWidget *nofeedtab = new FeedWidget();
        m_feeds->addTab(nofeedtab, "Entries");
        m_activefeed = nofeedtab;
    }

    m_engine->loadEntries(provider);
}

void KNewStuff2Download::slotEntryLoaded(KNS::Entry *entry, const KNS::Feed *feed, const KNS::Provider *provider)
{
    Q_UNUSED(feed);
    Q_UNUSED(provider);

    kDebug() << "SLOT: slotEntryLoaded" << endl;
    kDebug() << "-- entry: " << entry->name().representation() << endl;

    FeedWidget *fw = dynamic_cast<FeedWidget*>(m_activefeed);
    fw->addEntry(entry);

    m_engine->downloadPreview(entry);

    m_activeentry = entry;
}

void KNewStuff2Download::slotPreviewLoaded(KUrl preview)
{
    kDebug() << "-- preview downloaded successfully" << endl;
    kDebug() << "-- downloaded to " << preview.prettyUrl() << endl;

    FeedWidget *fw = dynamic_cast<FeedWidget*>(m_activefeed);
    fw->addPreview(preview);
}

void KNewStuff2Download::slotPreviewFailed()
{
    kDebug() << "SLOT: slotPreviewFailed" << endl;
}

void KNewStuff2Download::slotPayloadLoaded(KUrl payload)
{
    kDebug() << "-- entry downloaded successfully" << endl;
    kDebug() << "-- downloaded to " << payload.prettyUrl() << endl;

    bool success = m_engine->install(payload.path());
    if(success) {
        kDebug() << "-- installation succeeded" << endl;
    } else {
        kError() << "-- installation failed" << endl;
    }
}

void KNewStuff2Download::slotPayloadFailed()
{
    kDebug() << "SLOT: slotPayloadFailed" << endl;
}

void KNewStuff2Download::slotProvidersFailed()
{
    kDebug() << "SLOT: slotProvidersFailed" << endl;
}

void KNewStuff2Download::slotEntriesFailed()
{
    kDebug() << "SLOT: slotEntriesFailed" << endl;
}

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "knewstuff2_download", NULL, NULL, NULL);
    KApplication app;

    // Take source directory into account
    kDebug() << "-- adding source directory " << KNSSRCDIR << endl;
    KGlobal::dirs()->addResourceDir("config", KNSSRCDIR);

    KNewStuff2Download *download = new KNewStuff2Download();
    download->run();

    return app.exec();
}

#include "knewstuff2_download.moc"
