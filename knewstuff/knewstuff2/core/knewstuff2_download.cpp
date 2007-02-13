#include "knewstuff2_download.h"

#include <knewstuff2/engine.h>
#include <knewstuff2/author.h>

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#include <qpushbutton.h>
#include <qlayout.h>
#include <qlistwidget.h>
#include <qtabwidget.h>
#include <qtablewidget.h>

#include <unistd.h> // for exit()
#include <stdio.h> // for stdout

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

	resize(800, 600);
	setWindowTitle("KNewStuff2 Download Dialog Test");

	QPushButton *closebutton = new QPushButton("Close");
	connect(closebutton, SIGNAL(clicked()), SLOT(close()));

	m_providerlist = new QListWidget();
	m_providerlist->setFixedWidth(200);

	m_feeds = new QTabWidget();

	QHBoxLayout *hbox = new QHBoxLayout();
	hbox->addWidget(m_providerlist);
	hbox->addWidget(m_feeds);

	QVBoxLayout *vbox = new QVBoxLayout();
	setLayout(vbox);
	vbox->addLayout(hbox);
	vbox->addWidget(closebutton);

	show();
}

void KNewStuff2Download::engineTest()
{
	kDebug() << "-- test kns2 engine" << endl;

	m_engine = new KNS::Engine();
	bool ret = m_engine->init("knewstuff2_test.knsrc");

	kDebug() << "-- engine test result: " << ret << endl;

	if(ret)
	{
		connect(m_engine,
			SIGNAL(signalProviderLoaded(KNS::Provider*)),
			SLOT(slotProviderLoaded(KNS::Provider*)));
		connect(m_engine,
			SIGNAL(signalProvidersFailed()),
			SLOT(slotProvidersFailed()));
		connect(m_engine,
			SIGNAL(signalEntryLoaded(KNS::Entry*)),
			SLOT(slotEntryLoaded(KNS::Entry*)));
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

		m_engine->start();
	}
	else
	{
		kWarning() << "ACHTUNG: you probably need to 'make install' the knsrc file first." << endl;
		kWarning() << "Although this is not required anymore, so something went really wrong." << endl;
	}
}

void KNewStuff2Download::slotProviderLoaded(KNS::Provider *provider)
{
	kDebug() << "SLOT: slotProviderLoaded" << endl;
	kDebug() << "-- provider: " << provider->name().representation() << endl;

	QListWidgetItem *item = new QListWidgetItem(QIcon(), provider->name().representation());
	m_providerlist->addItem(item);

	QStringList feeds = provider->feeds();
	for(QStringList::Iterator it = feeds.begin(); it != feeds.end(); it++)
	{
		KNS::Feed *feed = provider->downloadUrlFeed((*it));

		FeedWidget *feedtab = new FeedWidget();
		m_feeds->addTab(feedtab, feed->name().representation());
		m_activefeed = feedtab;
	}
	if(feeds.size() == 0)
	{
		FeedWidget *nofeedtab = new FeedWidget();
		m_feeds->addTab(nofeedtab, "Entries");
		m_activefeed = nofeedtab;
	}

	m_engine->loadEntries(provider);
}

void KNewStuff2Download::slotEntryLoaded(KNS::Entry *entry)
{
	kDebug() << "SLOT: slotEntryLoaded" << endl;
	kDebug() << "-- entry: " << entry->name().representation() << endl;

	FeedWidget *fw = dynamic_cast<FeedWidget*>(m_activefeed);
	fw->addEntry(entry);

	m_engine->downloadPreview(entry);
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

	KNewStuff2Download *test = new KNewStuff2Download();
	test->engineTest();

	return app.exec();
}

#include "knewstuff2_download.moc"
