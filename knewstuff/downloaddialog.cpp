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
#include <klistview.h>
#include <kdebug.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kiconloader.h>

#include <knewstuff/entry.h>
#include <knewstuff/knewstuffgeneric.h>
#include <knewstuff/engine.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qdom.h>
#include <qlabel.h>
#include <q3valuelist.h>
#include <q3textbrowser.h>
#include <qtimer.h> // hack
#include <QTextBrowser>

using namespace KNS;

struct DownloadDialog::Private
{
    QString m_providerlist;
    QWidget *m_page;
    KListView *m_lvtmp_r, *m_lvtmp_d, *m_lvtmp_l;
};

class NumSortListViewItem : public KListViewItem
{
  public:
  NumSortListViewItem( Q3ListView * parent, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null )  :
  KListViewItem( parent, label1, label2, label3, label4, label5, label6, label7, label8 )
  {
  }

  QString key(int col, bool asc) const {
    if (col == 2)
    {
      QString s;
      s.sprintf("%08d", text(col).toInt());
      return s;
    }
    return KListViewItem::key( col, asc );
  }
};

class DateSortListViewItem : public KListViewItem
{
  public:
  DateSortListViewItem( Q3ListView * parent, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null )  :
  KListViewItem( parent, label1, label2, label3, label4, label5, label6, label7, label8 )
  {
  }

  QString key(int col, bool asc) const {
    if (col == 2)
    {
      QString s;
      QDate date = KGlobal::locale()->readDate(text(col));
      s.sprintf("%08d", date.year() * 366 + date.dayOfYear());
      return s;
    }
    return KListViewItem::key( col, asc );
  }
};


DownloadDialog::DownloadDialog(Engine *engine, QWidget *, const QString& caption)
: KDialogBase(KDialogBase::IconList, (caption.isNull() ? i18n("Get Hot New Stuff") : caption),
  KDialogBase::Close, KDialogBase::Close)
{
  init(engine);
}

void DownloadDialog::init(Engine *engine)
{
  resize(700, 400);
  d = new Private();

  m_engine = engine;
  d->m_page = NULL;

  connect(this, SIGNAL(aboutToShowPage(QWidget*)), SLOT(slotPage(QWidget*)));

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
  QMap<QWidget*, Q3ValueList<KListView*>* >::Iterator it;
  QMap<QWidget*, Q3ValueList<KListView*>* >::Iterator end(m_map.end());
  for(it = m_map.begin(); it != end; ++it)
  {
    Q3ValueList<KListView*> *v = it.data();
    kdDebug() << "clear listviews in " << v << endl;
    if(v)
    {
      (*(v->at(0)))->clear();
      (*(v->at(1)))->clear();
      (*(v->at(2)))->clear();

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
    kdDebug() << "++ provider ++ " << p->name() << endl;

    if(!m_filter.isEmpty())
      loadProvider(p);
    else
      addProvider(p);
    /*if(p == list->getFirst())
      slotPage(m_frame);*/ // only if !qtbug
  }
}

void DownloadDialog::addProvider(Provider *p)
{
  QFrame *frame;
  KTabCtl *ctl;
  QWidget *w_d, *w_r, *w_l;
  QWidget *w2;
  QTextBrowser *rt;
  QString tmp;
  int ret;
  QPixmap pix;

  if(m_map.count() == 0)
  {
    frame = addPage(i18n("Welcome"), i18n("Welcome"), QPixmap(""));
    delete frame;
  }

  kdDebug() << "addProvider()/begin" << endl;

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
      pix = KGlobal::iconLoader()->loadIcon(p->icon().path(), KIcon::Panel);
      ret = true;
    }
  }
  if(!ret) pix = KGlobal::iconLoader()->loadIcon("knewstuff", KIcon::Panel);
  frame = addPage(p->name(), p->name(), pix);
  m_frame = frame;

  w2 = new QWidget(frame);
  w_d = new QWidget(frame);
  w_r = new QWidget(frame);
  w_l = new QWidget(frame);

  ctl = new KTabCtl(frame);
  ctl->addTab(w_r, i18n("Highest Rated"));
  ctl->addTab(w_d, i18n("Most Downloads"));
  ctl->addTab(w_l, i18n("Latest"));

  m_curtab = 0;
  connect(ctl, SIGNAL(tabSelected(int)), SLOT(slotTab(int)));

  QHBoxLayout *box = new QHBoxLayout(frame);
  box->add(ctl);

  d->m_lvtmp_r = new KListView(w_r);
  d->m_lvtmp_r->addColumn(i18n("Name"));
  d->m_lvtmp_r->addColumn(i18n("Version"));
  d->m_lvtmp_r->addColumn(i18n("Rating"));
  d->m_lvtmp_r->setSorting(2, false);

  d->m_lvtmp_d = new KListView(w_d);
  d->m_lvtmp_d->addColumn(i18n("Name"));
  d->m_lvtmp_d->addColumn(i18n("Version"));
  d->m_lvtmp_d->addColumn(i18n("Downloads"));
  d->m_lvtmp_d->setSorting(2, false);

  d->m_lvtmp_l = new KListView(w_l);
  d->m_lvtmp_l->addColumn(i18n("Name"));
  d->m_lvtmp_l->addColumn(i18n("Version"));
  d->m_lvtmp_l->addColumn(i18n("Release Date"));
  d->m_lvtmp_l->setSorting(2, false);

  connect(d->m_lvtmp_r, SIGNAL(selectionChanged()), SLOT(slotSelected()));
  connect(d->m_lvtmp_d, SIGNAL(selectionChanged()), SLOT(slotSelected()));
  connect(d->m_lvtmp_l, SIGNAL(selectionChanged()), SLOT(slotSelected()));

  rt = new QTextBrowser(frame);
  rt->setMinimumWidth(150);

  QPushButton *in = new QPushButton(i18n("Install"), frame);
  QPushButton *de = new QPushButton(i18n("Details"), frame);
  in->setEnabled(false);
  de->setEnabled(false);

  box->addSpacing(spacingHint());
  QVBoxLayout *vbox = new QVBoxLayout(box);
  vbox->add(rt);
  vbox->addSpacing(spacingHint());
  vbox->add(de);
  vbox->add(in);

  connect(in, SIGNAL(clicked()), SLOT(slotInstall()));
  connect(de, SIGNAL(clicked()), SLOT(slotDetails()));

  QVBoxLayout *box2 = new QVBoxLayout(w_r);
  box2->add(d->m_lvtmp_r);
  QVBoxLayout *box3 = new QVBoxLayout(w_d);
  box3->add(d->m_lvtmp_d);
  QVBoxLayout *box4 = new QVBoxLayout(w_l);
  box4->add(d->m_lvtmp_l);

  Q3ValueList<KListView*> *v = new Q3ValueList<KListView*>;
  *v << d->m_lvtmp_r << d->m_lvtmp_d << d->m_lvtmp_l;
  m_map[frame] = v;
  m_rts[frame] = rt;
  Q3ValueList<QPushButton*> *vb = new Q3ValueList<QPushButton*>;
  *vb << in << de;
  m_buttons[frame] = vb;
  m_providers[frame] = p;

  kdDebug() << "addProvider()/end; d->m_lvtmp_r = " << d->m_lvtmp_r << endl;

  if(m_engine) slotPage(frame);

  QTimer::singleShot(100, this, SLOT(slotFinish()));
}

void DownloadDialog::slotResult(KIO::Job *job)
{
  QDomDocument dom;
  QDomElement knewstuff;

  kdDebug() << "got data: " << m_data[job] << endl;

  kapp->config()->setGroup("KNewStuffStatus");

  dom.setContent(m_data[job]);
  knewstuff = dom.documentElement();

  for(QDomNode pn = knewstuff.firstChild(); !pn.isNull(); pn = pn.nextSibling())
  {
    QDomElement stuff = pn.toElement();

    kdDebug() << "element: " << stuff.tagName() << endl;

    if(stuff.tagName() == "stuff")
    {
      Entry *entry = new Entry(stuff);
      kdDebug() << "TYPE::" << entry->type() << " FILTER::" << m_filter << endl;
      if(!entry->type().isEmpty())
      {
        if((!m_filter.isEmpty()) && (entry->type() != m_filter)) continue;
      }

      if((!m_filter.isEmpty()) && (m_jobs[job]))
      {
        Provider *p = m_jobs[job];
        addProvider(p);
        slotPage(m_frame);
        m_jobs[job] = 0;
      }
      addEntry(entry);
    }
  }

  m_data[job] = "";
}

int DownloadDialog::installStatus(Entry *entry)
{
  QDate date;
  QString datestring;
  int installed;

  kapp->config()->setGroup("KNewStuffStatus");
  datestring = kapp->config()->readEntry(entry->name());
  if(datestring.isNull()) installed = 0;
  else
  {
    date = QDate::fromString(datestring, Qt::ISODate);
    if(!date.isValid()) installed = 0;
    else if(date < entry->releaseDate()) installed = -1;
    else installed = 1;
  }

  return installed;
}

void DownloadDialog::addEntry(Entry *entry)
{
  QPixmap pix;
  int installed;

  /*if(m_engine)
  {
    if(m_map.count() == 0)
    {
      m_frame = addPage(i18n("Welcome"), i18n("Welcome"), QPixmap(""));
      Provider *p = new Provider();
      p->setName(i18n("Generic"));
      addProvider(p);
      slotPage(m_frame);
    }
  }*/
  installed = installStatus(entry);

  if(installed > 0) pix = KGlobal::iconLoader()->loadIcon("ok", KIcon::Small);
  else if(installed < 0) pix = KGlobal::iconLoader()->loadIcon("history", KIcon::Small);
  else pix = QPixmap();

  KListViewItem *tmp_r = new NumSortListViewItem(lv_r,
    entry->name(), entry->version(), QString("%1").arg(entry->rating()));
  KListViewItem *tmp_d = new NumSortListViewItem(lv_d,
    entry->name(), entry->version(), QString("%1").arg(entry->downloads()));
  KListViewItem *tmp_l = new DateSortListViewItem(lv_l,
    entry->name(), entry->version(), KGlobal::locale()->formatDate(entry->releaseDate()));

  tmp_r->setPixmap(0, pix);
  tmp_d->setPixmap(0, pix);
  tmp_l->setPixmap(0, pix);

  m_entries.append(entry);

  kdDebug() << "added entry " << entry->name() << endl;
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

  QString info = i18n
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
    ).arg(e->name()
    ).arg(e->author()
    ).arg(e->license()
    ).arg(e->version()
    ).arg(e->release()
    ).arg(e->rating()
    ).arg(e->downloads()
    ).arg(KGlobal::locale()->formatDate(e->releaseDate())
    ).arg(e->summary(lang)
  );

  info.append(i18n
  (
    "Preview: %1\n"
    "Payload: %2\n"
    ).arg(e->preview().url()
    ).arg(e->payload().url()
  ));

  KMessageBox::information(this, info, i18n("Details"));
}

Q3ListViewItem *DownloadDialog::currentEntryItem()
{
  switch(m_curtab)
  {
    case 0: return lv_r->currentItem(); break;
    case 1: return lv_d->currentItem(); break;
    case 2: return lv_l->currentItem(); break;
    default:
      return 0;
  }
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

  kdDebug() << "download entry now" << endl;

  if(m_engine)
  {
    m_engine->download(e);
    install(e);
  }
  else
  {
    m_s = new KNewStuffGeneric(e->type(), this);

    m_entry = e;

    KURL source = e->payload();
    KURL dest = KURL(m_s->downloadDestination(e));

    KIO::FileCopyJob *job = KIO::file_copy(source, dest, -1, true);
    connect(job, SIGNAL(result(KIO::Job*)), SLOT(slotInstalled(KIO::Job*)));
  }
}

void DownloadDialog::install(Entry *e)
{
  kapp->config()->setGroup("KNewStuffStatus");
  kapp->config()->writeEntry(m_entryname, e->releaseDate().toString(Qt::ISODate));
  kapp->config()->sync();

  QPixmap pix = KGlobal::iconLoader()->loadIcon("ok", KIcon::Small);

  if(m_entryitem)
    m_entryitem->setPixmap(0, pix);

  if(currentEntryItem() == m_entryitem)
  {
    QPushButton *in;
    in = *(m_buttons[d->m_page]->at(0));
    if(in) in->setEnabled(false);
  }
}

void DownloadDialog::slotInstalled(KIO::Job *job)
{
  bool ret = (job->error() == 0);
  KIO::FileCopyJob *cjob;

  if(ret)
  {
    cjob = static_cast<KIO::FileCopyJob*>(job);
    if(cjob)
    {
      ret = m_s->install(cjob->destURL().path());
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
  kdDebug() << "switch tab to: " << tab << endl;
  m_curtab = tab;
}

void DownloadDialog::slotSelected()
{
  QString tmp;
  bool enabled;
  Entry *e = getEntry();
  QString lang = KGlobal::locale()->language();

  if(e)
  {
    if(!e->preview(lang).isValid())
    {
      m_rt->setText(QString("<b>%1</b><br>%2<br>%3<br><br><i>%4</i><br>(%5)").arg(
        e->name()).arg(e->author()).arg(KGlobal::locale()->formatDate(e->releaseDate())).arg(e->summary(lang)).arg(e->license()));
    }
    else
    {
      KIO::NetAccess::download(e->preview(lang), tmp, this);
      m_rt->setText(QString("<b>%1</b><br>%2<br>%3<br><br><img src='%4'><br><i>%5</i><br>(%6)").arg(
        e->name()).arg(e->author()).arg(KGlobal::locale()->formatDate(e->releaseDate())).arg(tmp).arg(e->summary(lang)).arg(e->license()));
    }

    if(installStatus(e) == 1) enabled = false;
    else enabled = true;

    QPushButton *de, *in;
    in = *(m_buttons[d->m_page]->at(0));
    de = *(m_buttons[d->m_page]->at(1));
    if(in) in->setEnabled(enabled);
    if(de) de->setEnabled(true);
  }
}

Entry *DownloadDialog::getEntry()
{
  m_entryitem = currentEntryItem();

  if(!m_entryitem)
    return 0;

  QString entryName = m_entryitem->text(0);

  m_entryname = m_entryitem->text(0);

  for(Entry *e = m_entries.first(); e; e = m_entries.next())
    if(e->name() == entryName)
      return e;

  return 0;
}

void DownloadDialog::slotPage(QWidget *w)
{
  Provider *p;

  kdDebug() << "changed widget!!!" << endl;

  if(m_map.find(w) == m_map.end()) return;

  d->m_page = w;

  lv_r = *(m_map[w]->at(0));
  lv_d = *(m_map[w]->at(1));
  lv_l = *(m_map[w]->at(2));
  p = m_providers[w];
  m_rt = m_rts[w];

  kdDebug() << "valid change!!!; lv_r = " << lv_r << endl;

  if(m_engine) return;

  if(!m_filter.isEmpty()) return;

  lv_r->clear();
  lv_d->clear();
  lv_l->clear();

  kdDebug() << "-- fetch -- " << p->downloadUrl() << endl;

  loadProvider(p);
}

void DownloadDialog::loadProvider(Provider *p)
{
  KIO::TransferJob *job = KIO::get(p->downloadUrl());

  m_jobs[job] = p;

  connect(job, SIGNAL(result(KIO::Job*)), SLOT(slotResult(KIO::Job*)));
  connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)),
    SLOT(slotData(KIO::Job*, const QByteArray&)));
}

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

void DownloadDialog::open(const QString& type, const QString& caption)
{
  DownloadDialog d(0L, 0, caption);
  d.setType(type);
  d.load();
  d.exec();
}

void DownloadDialog::slotFinish()
{
  showPage(1);
  //updateBackground();
}

