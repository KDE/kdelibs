/*
    This file is part of KNewStuff.
    Copyright (c) 2003 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "downloaddialog.h"
#include "downloaddialog.moc"

#include <klocale.h>
#include <ktabctl.h>
#include <kdebug.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kpagewidgetmodel.h>
#include <kurl.h>
#include <kconfig.h>
#include <ktoolinvocation.h>
#include <kiconloader.h>

#include <knewstuff/entry.h>
#include <knewstuff/knewstuffgeneric.h>
#include <knewstuff/engine.h>

#include <QDomDocument>
#include <QDomElement>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QTextBrowser>
#include <QTimer> // hack
#include <QTreeWidget>


using namespace KNS;

struct DownloadDialog::Private
{
    QString m_providerlist;
    QWidget *m_page;
    QTreeWidget *m_lvtmp_r, *m_lvtmp_d, *m_lvtmp_l;
    QList<Entry*> m_installlist;
    QMap<KJob*, Provider*> m_variantjobs;
    QMap<KJob*, QStringList> m_variants;
    QMap<Provider*, Provider*> m_newproviders;
};

class NumSortListViewItem : public QTreeWidgetItem
{
  public:
  NumSortListViewItem( QTreeWidget * parent, const QStringList &texts )
    : QTreeWidgetItem( parent, texts )
  {
  }

  bool operator<( const QTreeWidgetItem &other ) const
  {
    int thisValue = text(2).toInt();
    int otherValue = other.text(2).toInt();

    return (thisValue < otherValue);
  }
};

class DateSortListViewItem : public QTreeWidgetItem
{
  public:
  DateSortListViewItem( QTreeWidget * parent, const QStringList &texts )
    : QTreeWidgetItem( parent, texts )
  {
  }

  bool operator<( const QTreeWidgetItem &other ) const
  {
    QDate thisDate = KGlobal::locale()->readDate(text(2));
    QDate otherDate = KGlobal::locale()->readDate(other.text(2));

    return (thisDate < otherDate);
  }
};


DownloadDialog::DownloadDialog(Engine *engine, QWidget *parent, const QString& caption)
: KPageDialog(parent),d(new Private())
{
  setFaceType( KPageDialog::List );
  if ( caption.isEmpty() )
    setCaption( i18n("Get Hot New Stuff") );
  else
    setCaption( caption );

  setButtons( KDialog::Close );
  setDefaultButton( KDialog::Close );

  init(engine);
}

void DownloadDialog::init(Engine *engine)
{
  resize(700, 400);

  m_engine = engine;
  d->m_page = NULL;

  connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*, KPageWidgetItem*)),
          this, SLOT(slotPage(KPageWidgetItem*)));

  if(!engine)
  {
    m_loader = new ProviderLoader(this);
    connect(m_loader, SIGNAL(providersLoaded(Provider::List*)), SLOT(slotProviders(Provider::List*)));
  }
}

DownloadDialog::~DownloadDialog()
{
    delete d;
}

void DownloadDialog::load(const QString &providerList)
{
    m_loader->load(m_filter, providerList.isEmpty() ? d->m_providerlist: providerList);
}

void DownloadDialog::clear()
{
  QMap<QWidget*, QList<QTreeWidget*>* >::Iterator it;
  QMap<QWidget*, QList<QTreeWidget*>* >::Iterator end(m_map.end());
  for(it = m_map.begin(); it != end; ++it)
  {
    QList<QTreeWidget*> *v = it.value();
    kDebug() << "clear listviews in " << v << endl;
    if(v)
    {
      (*(v->at(0))).clear();
      (*(v->at(1))).clear();
      (*(v->at(2))).clear();

      //delete (*it);
    }

    delete it.key();
  }
  m_map.clear();
}

void DownloadDialog::slotProviders(Provider::List *list)
{
  Provider *p;
  /*QFrame *frame;*/

  for (int i = 0; i < list->size(); ++i) {
	p = list->at(i);
    kDebug() << "++ provider ++ " << p->name() << endl;

    if(!m_filter.isEmpty())
      loadProvider(p);
    else
      addProvider(p);
    /*if(p == list->getFirst())
      slotPage(m_item);*/ // only if !qtbug
  }
}

void DownloadDialog::addProvider(Provider *p)
{
  QFrame *frame;
  QTabWidget *ctl;
  QWidget *w_d, *w_r, *w_l;
  QWidget *w2;
  QTextBrowser *rt;
  QString tmp;
  int ret;
  QPixmap pix;

  if(m_map.count() == 0)
  {
    KPageWidgetItem *item = addPage( new QWidget(), i18n("Welcome") );
    item->setHeader( i18n("Welcome") );
  }

  kDebug() << "addProvider()/begin" << endl;

  ret = true;
  if(p->icon().path().isEmpty()) ret = false;
  else
  {
    if(!p->icon().protocol().isEmpty())
    {
      ret = KIO::NetAccess::download(p->icon(), tmp, this);
      if(ret) pix = QPixmap(tmp);
    }
    else
    {
      pix = KGlobal::iconLoader()->loadIcon(p->icon().path(), K3Icon::Panel);
      ret = true;
    }
  }
  if(!ret) pix = KGlobal::iconLoader()->loadIcon("knewstuff", K3Icon::Panel);

  frame = new QFrame( this );
  m_item = addPage(frame, p->name());
  m_item->setHeader( p->name() );
  //FIXME: set icon: item->setIcon(pix);

  w2 = new QWidget(frame);
  w_d = new QWidget(frame);
  w_r = new QWidget(frame);
  w_l = new QWidget(frame);

  ctl = new QTabWidget(frame);
  ctl->addTab(w_r, i18n("Highest Rated"));
  ctl->addTab(w_d, i18n("Most Downloads"));
  ctl->addTab(w_l, i18n("Latest"));

  m_curtab = 0;
  connect(ctl, SIGNAL(currentChanged(int)), SLOT(slotTab(int)));

  QHBoxLayout *box = new QHBoxLayout(frame);
  box->addWidget(ctl);

  QStringList headerLabels;
  d->m_lvtmp_r = new QTreeWidget(w_r);
  headerLabels << i18n("Name") << i18n("Version") << i18n("Rating");
  d->m_lvtmp_r->setHeaderLabels( headerLabels );

  d->m_lvtmp_d = new QTreeWidget(w_d);
  headerLabels.clear();
  headerLabels << i18n("Name") << i18n("Version") << i18n("Downloads");
  d->m_lvtmp_d->setHeaderLabels( headerLabels );

  d->m_lvtmp_l = new QTreeWidget(w_l);
  headerLabels.clear();
  headerLabels << i18n("Name") << i18n("Version") << i18n("Release Date");
  d->m_lvtmp_l->setHeaderLabels( headerLabels );

  connect(d->m_lvtmp_r, SIGNAL(itemSelectionChanged()), SLOT(slotSelected()));
  connect(d->m_lvtmp_d, SIGNAL(itemSelectionChanged()), SLOT(slotSelected()));
  connect(d->m_lvtmp_l, SIGNAL(itemSelectionChanged()), SLOT(slotSelected()));

  rt = new QTextBrowser(frame);
  rt->setMinimumWidth(150);

  QPushButton *in = new QPushButton(i18n("Install"), frame);
  QPushButton *de = new QPushButton(i18n("Details"), frame);
  in->setEnabled(false);
  de->setEnabled(false);

  box->addSpacing(spacingHint());
  QVBoxLayout *vbox = new QVBoxLayout();
  vbox->addWidget(rt);
  vbox->addSpacing(spacingHint());
  vbox->addWidget(de);
  vbox->addWidget(in);

  box->addLayout(vbox);

  connect(rt, SIGNAL(linkClicked(const QString&)), SLOT(slotEmail(const QString&)));

  connect(in, SIGNAL(clicked()), SLOT(slotInstall()));
  connect(de, SIGNAL(clicked()), SLOT(slotDetails()));

  QVBoxLayout *box2 = new QVBoxLayout(w_r);
  box2->addWidget(d->m_lvtmp_r);
  QVBoxLayout *box3 = new QVBoxLayout(w_d);
  box3->addWidget(d->m_lvtmp_d);
  QVBoxLayout *box4 = new QVBoxLayout(w_l);
  box4->addWidget(d->m_lvtmp_l);

  QList<QTreeWidget*> *v = new QList<QTreeWidget*>;
  v->append(d->m_lvtmp_r);
  v->append(d->m_lvtmp_d);
  v->append(d->m_lvtmp_l);
  m_map[frame] = v;
  m_rts[frame] = rt;
  QList<QPushButton*> *vb = new QList<QPushButton*>;
  vb->append(in);
  vb->append(de);
  m_buttons[frame] = vb;
  m_providers[frame] = p;

  kDebug() << "addProvider()/end; d->m_lvtmp_r = " << d->m_lvtmp_r << endl;

  if(m_engine) slotPage(m_item);

  QTimer::singleShot(100, this, SLOT(slotFinish()));
}

void DownloadDialog::slotResult(KJob *job)
{
  QDomDocument dom;
  QDomElement knewstuff;

  kDebug() << "got data: " << m_data[job] << endl;

  // KConfigGroup cg(KGlobal::config(), "KNewStuffStatus");

  dom.setContent(m_data[job]);
  knewstuff = dom.documentElement();

  for(QDomNode pn = knewstuff.firstChild(); !pn.isNull(); pn = pn.nextSibling())
  {
    QDomElement stuff = pn.toElement();

    kDebug() << "element: " << stuff.tagName() << endl;

    if(stuff.tagName() == "stuff")
    {
      Entry *entry = new Entry(stuff);
      kDebug() << "TYPE::" << entry->category() << " FILTER::" << m_filter << endl;
      if(!entry->category().isEmpty())
      {
        if((!m_filter.isEmpty()) && (entry->category() != m_filter)) continue;
      }

      if((!m_filter.isEmpty()) && (d->m_variantjobs[job]))
      {
        Provider *p = d->m_variantjobs[job];
        if(d->m_newproviders[p])
        {
          addProvider(p);
          slotPage(m_item);
          d->m_newproviders[p] = 0;
        }
      }

      if(d->m_variantjobs[job]) addEntry(entry, d->m_variants[job]);
    }
  }
}

int DownloadDialog::installStatus(Entry *entry)
{
  QDate date;
  int installed;

  KConfigGroup cg(KGlobal::config(), "KNewStuffStatus");
  date = cg.readEntry(entry->name(), QDate());
  if(!date.isValid()) installed = 0;
  else if(date < entry->releaseDate()) installed = -1;
  else installed = 1;

  return installed;
}

void DownloadDialog::addEntry(Entry *entry, const QStringList& variants)
{
  QPixmap pix;
  int installed;

  installed = installStatus(entry);

  if(installed > 0) pix = KGlobal::iconLoader()->loadIcon("ok", K3Icon::Small);
  else if(installed < 0) pix = KGlobal::iconLoader()->loadIcon("history", K3Icon::Small);
  else pix = QPixmap();

  QString lang = KGlobal::locale()->language();
  QStringList texts;

  if(variants.contains("score"))
  {
    QString ratingstring = QString("%1").arg(entry->rating());
    texts << entry->name(lang) << entry->version() << ratingstring;
    QTreeWidgetItem *tmp_r = new NumSortListViewItem(lv_r, texts );
    texts.clear();
    tmp_r->setIcon(0, pix);
  }

  if(variants.contains("downloads"))
  {
    QString downloadsstring = QString("%1").arg(entry->downloads());
    texts << entry->name(lang) << entry->version() << downloadsstring;
    QTreeWidgetItem *tmp_d = new NumSortListViewItem(lv_d, texts);
    texts.clear();
    tmp_d->setIcon(0, pix);
  }

  if(variants.contains("latest"))
  {
    QString releasedatestring = KGlobal::locale()->formatDate(entry->releaseDate());
    texts << entry->name(lang) << entry->version() << releasedatestring;
    QTreeWidgetItem *tmp_l = new DateSortListViewItem(lv_l, texts);
    texts.clear();
    tmp_l->setIcon(0, pix);
  }

  m_entries.append(entry);

  kDebug() << "added entry " << entry->name() << " for variants " << variants << endl;
}

void DownloadDialog::addEntry(Entry *entry)
{
  QStringList variants;

  variants << "score";
  variants << "downloads";
  variants << "latest";

  addEntry(entry, variants);

  // not used anymore due to variants (but still used by engine)
  kDebug() << "added entry " << entry->name() << endl;
}

void DownloadDialog::slotData(KIO::Job *job, const QByteArray &a)
{
  m_data[job].append(QString::fromUtf8(a)); // ####### The fromUtf8 conversion should be done at the end, not chunk by chunk
}

void DownloadDialog::slotDetails()
{
  Entry *e = getEntry();
  if(!e) return;

  QString lang = KGlobal::locale()->language();

  QString info = ki18n
  (
    "Name: %1\n"
    "Author: %2\n"
    "License: %3\n"
    "Version: %4\n"
    "Release: %5\n"
    "Rating: %6\n"
    "Downloads: %7\n"
    "Release date: %8\n"
    "Summary: %9\n"
    ).subs(e->name(lang)
    ).subs(e->author()
    ).subs(e->license()
    ).subs(e->version()
    ).subs(e->release()
    ).subs(e->rating()
    ).subs(e->downloads()
    ).subs(KGlobal::locale()->formatDate(e->releaseDate())
    ).subs(e->summary(lang)
  ).toString();

  info.append(i18n
  (
    "Preview: %1\n"
    "Payload: %2\n"
    , e->preview(lang).url()
    , e->payload(lang).url()
  ));

  KMessageBox::information(this, info, i18n("Details"));
}

QTreeWidgetItem *DownloadDialog::currentEntryItem()
{
  if((m_curtab == 0) && (lv_r->currentItem())) return lv_r->currentItem();
  if((m_curtab == 1) && (lv_d->currentItem())) return lv_d->currentItem();
  if((m_curtab == 2) && (lv_l->currentItem())) return lv_l->currentItem();

  return 0;
}

void DownloadDialog::slotInstall()
{
  Entry *e = getEntry();
  if(!e) return;
  d->m_lvtmp_r->setEnabled( false );
  d->m_lvtmp_l->setEnabled( false );
  d->m_lvtmp_d->setEnabled( false );

  m_entryitem = currentEntryItem();
  m_entryname = m_entryitem->text(0);

  kDebug() << "download entry now" << endl;

  if(m_engine)
  {
    m_engine->download(e);
    install(e);
  }
  else
  {
    m_s = new KNewStuffGeneric(e->category(), this);

    m_entry = e;

    KUrl source = e->payload();
    KUrl dest = KUrl(m_s->downloadDestination(e));

    KIO::FileCopyJob *job = KIO::file_copy(source, dest, -1, true);
    connect(job, SIGNAL(result(KJob*)), SLOT(slotInstalled(KJob*)));
  }
}

void DownloadDialog::install(Entry *e)
{
  KConfigGroup cg(KGlobal::config(), "KNewStuffStatus");
  cg.writeEntry(m_entryname, e->releaseDate());

  QPixmap pix = KGlobal::iconLoader()->loadIcon("ok", K3Icon::Small);

  QString lang = KGlobal::locale()->language();

  if(m_entryitem)
  {
    m_entryitem->setIcon(0, pix);

    QList<QTreeWidgetItem*> list;
    QTreeWidgetItem *item;
    list = lv_r->findItems(e->name(lang), Qt::MatchExactly, 0);
    if(list.count() > 0)
    {
      item = list.at(0);
      //item->setPixmap(0, pix);
    }
    list = lv_d->findItems(e->name(lang), Qt::MatchExactly, 0);
    if(list.count() > 0)
    {
      item = list.at(0);
      //item->setPixmap(0, pix);
    }
    list = lv_l->findItems(e->name(lang), Qt::MatchExactly, 0);
    if(list.count() > 0)
    {
      item = list.at(0);
      //item->setPixmap(0, pix);
    }
  }

  if(currentEntryItem() == m_entryitem)
  {
    QPushButton *in;
    in = (m_buttons[d->m_page]->at(0));
    if(in) in->setEnabled(false);
  }

  d->m_installlist.append(e);
}

void DownloadDialog::slotInstalled(KJob *job)
{
  bool ret = job && (job->error() == 0);

  if(ret)
  {
    KIO::FileCopyJob *cjob = ::qobject_cast<KIO::FileCopyJob*>(job);
    if(cjob)
    {
      ret = m_s->install(cjob->destUrl().path());
    }
    else ret = false;
  }

  if(ret)
  {
    install(m_entry);

    KMessageBox::information(this, i18n("Installation successful."), i18n("Installation"));
  }
  else KMessageBox::error(this, i18n("Installation failed."), i18n("Installation"));
  d->m_lvtmp_r->setEnabled( true );
  d->m_lvtmp_l->setEnabled( true );
  d->m_lvtmp_d->setEnabled( true );
  delete m_s;
}

void DownloadDialog::slotTab(int tab)
{
  kDebug() << "switch tab to: " << tab << endl;

  Entry *eold = getEntry();
  m_curtab = tab;
  Entry *e = getEntry();

  if(e == eold) return;

  if(e)
  {
    slotSelected();
  }
  else
  {
    QPushButton *de, *in;
    in = (m_buttons[d->m_page]->at(0));
    de = (m_buttons[d->m_page]->at(1));

    if(in) in->setEnabled(false);
    if(de) de->setEnabled(false);

    m_rt->clear();
  }
}

void DownloadDialog::slotSelected()
{
  QString tmp;
  bool enabled;
  Entry *e = getEntry();
  QString lang = KGlobal::locale()->language();
  bool ret;

  if(e)
  {
    QList<QTreeWidgetItem*> list;
    QTreeWidgetItem *item;
    if(m_curtab != 0)
    {
      lv_r->clearSelection();
      list = lv_r->findItems(e->name(lang), Qt::MatchExactly, 0);
      if(list.count() > 0)
      {
        item = list.at(0);
        lv_r->setItemSelected(item, true);
      }
    }
    if(m_curtab != 1)
    {
      lv_d->clearSelection();
      list = lv_d->findItems(e->name(lang), Qt::MatchExactly, 0);
      if(list.count() > 0)
      {
        item = list.at(0);
        lv_d->setItemSelected(item, true);
      }
    }
    if(m_curtab != 2)
    {
      lv_l->clearSelection();
      list = lv_l->findItems(e->name(lang), Qt::MatchExactly, 0);
      if(list.count() > 0)
      {
        item = list.at(0);
        lv_l->setItemSelected(item, true);
      }
    }

    if(!e->preview(lang).isValid())
    {
      ret = 0;
    }
    else
    {
      ret = KIO::NetAccess::download(e->preview(lang), tmp, this);
    }

    QString desc = QString("<b>%1</b><br>").arg(e->name());
    if(!e->authorEmail().isNull())
    {
      desc += QString("<a href='mailto:" + e->authorEmail() + "'>" + e->author() + "</a>");
    }
    else
    {
      desc += QString("%1").arg(e->author());
    }
    desc += QString("<br>%1").arg(KGlobal::locale()->formatDate(e->releaseDate()));
    desc += QString("<br><br>");
    if(ret)
    {
      desc += QString("<img src='%1'>").arg(tmp);
    }
    else
    {
      desc += i18n("Preview not available.");
    }
    desc += QString("<br><i>%1</i>").arg(e->summary(lang));
    desc += QString("<br>(%1)").arg(e->license());

    m_rt->setHtml(desc);

    if(installStatus(e) == 1) enabled = false;
    else enabled = true;

    QPushButton *de, *in;
    in = (m_buttons[d->m_page]->at(0));
    de = (m_buttons[d->m_page]->at(1));
    if(in) in->setEnabled(enabled);
    if(de) de->setEnabled(true);
  }
}

void DownloadDialog::slotEmail(const QString& link)
{
  kDebug() << "EMAIL: " << link << endl;
  KToolInvocation::invokeMailer(link);
  slotSelected(); // QTextBrowser oddity workaround as it cannot handle mailto: URLs
}

Entry *DownloadDialog::getEntry()
{
  m_entryitem = currentEntryItem();

  if(!m_entryitem)
    return 0;

  QString entryName = m_entryitem->text(0);

  m_entryname = m_entryitem->text(0);

  QString lang = KGlobal::locale()->language();

  Q_FOREACH( Entry *e , m_entries )
    if(e->name(lang) == entryName)
      return e;

  return 0;
}

void DownloadDialog::slotPage(KPageWidgetItem *current)
{
  QWidget* w = current->widget();
  Provider *p;

  kDebug() << "changed widget!!!" << endl;

  if(m_map.find(w) == m_map.end()) return;

  d->m_page = w;

  lv_r = (m_map[w]->at(0));
  lv_d = (m_map[w]->at(1));
  lv_l = (m_map[w]->at(2));
  p = m_providers[w];
  m_rt = m_rts[w];

  kDebug() << "valid change!!!; lv_r = " << lv_r << endl;

  if(m_engine) return;

  if(!m_filter.isEmpty()) return;

  lv_r->clear();
  lv_d->clear();
  lv_l->clear();

  kDebug() << "-- fetch -- " << p->downloadUrl() << endl;

  loadProvider(p);
}

void DownloadDialog::loadProvider(Provider *p)
{
  QMap<KJob*, Provider*>::Iterator it;

  for(it = m_jobs.begin(); it != m_jobs.end(); it++)
  {
    if(it.value() == p)
    {
      kDebug() << "-- found provider data in cache" << endl;
      slotResult(it.key());
      return;
    }
  }

  QStringList variants;
  variants << "score";
  variants << "downloads";
  variants << "latest";

  // Optimise URLs so each unique URL only gets fetched once

  QMap<QString, QStringList> urls;

  for(QStringList::Iterator it = variants.begin(); it != variants.end(); /*it++ already done by erase*/)
  {
    QString url = p->downloadUrlVariant((*it)).url();
    if(!urls.contains(url))
    {
      urls[url] = QStringList();
    }
    urls[url] << (*it);

    it = variants.erase(it);
  }

  // Now fetch the URLs while keeping the variant list for each attached

  for(QMap<QString, QStringList>::Iterator it = urls.begin(); it != urls.end(); it++)
  {
    QString url = it.key();
    QStringList urlvariants = it.value();

    KIO::TransferJob *variantjob = KIO::get(url);
    d->m_newproviders[p] = p;
    d->m_variantjobs[variantjob] = p;
    d->m_variants[variantjob] = urlvariants;
    m_data[variantjob] = "";

    connect(variantjob, SIGNAL(result(KJob*)), SLOT(slotResult(KJob*)));
    connect(variantjob, SIGNAL(data(KIO::Job*, const QByteArray&)),
      SLOT(slotData(KIO::Job*, const QByteArray&)));
  }

  if(variants.count() == 0) return;

  // If not all variants are given, use default URL for those

  kDebug() << "-- reached old downloadurl section; variants left: " << variants.count() << endl;

  KIO::TransferJob *job = KIO::get(p->downloadUrl());

  d->m_newproviders[p] = p;
  d->m_variantjobs[job] = p;
  d->m_variants[job] = variants;
  //m_jobs[job] = p; // not used anymore due to variants
  m_data[job] = "";

  connect(job, SIGNAL(result(KJob*)), SLOT(slotResult(KJob*)));
  connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)),
    SLOT(slotData(KIO::Job*, const QByteArray&)));
}

void DownloadDialog::setCategory(const QString &category)
{
  m_filter = category;
}

// DEPRECATED
void DownloadDialog::setType(const QString &type)
{
  m_filter = type;
}

void DownloadDialog::setProviderList(const QString& providerList)
{
  d->m_providerlist = providerList;
}

void DownloadDialog::slotOk()
{
}

void DownloadDialog::slotApply()
{
}

void DownloadDialog::open(const QString& category, const QString& caption)
{
  DownloadDialog d(0L, 0, caption);
  d.setCategory(category);
  d.load();
  d.exec();
}

void DownloadDialog::slotFinish()
{
// tokoe:  showPage(1);
}

QList<Entry*> DownloadDialog::installedEntries()
{
  return d->m_installlist;
}

