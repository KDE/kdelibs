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
#ifndef KNEWSTUFF_DOWNLOADDIALOG_H
#define KNEWSTUFF_DOWNLOADDIALOG_H

#include <kdialogbase.h>
#include <knewstuff/provider.h>

#include <qmap.h>

namespace KIO
{
  class Job;
}

class KListView;
class QTextBrowser;
class QFrame;
class Q3ListViewItem;
template<typename T> class QList;

class KNewStuffGeneric;

namespace KNS
{

class ProviderLoader;
class Entry;
class Provider;
class Engine;

/**
 * @short Common download dialog for data browsing and installation.
 *
 * It provides an easy-to-use convenience method named open() which does all
 * the work, unless a more complex operation is needed.
 * \code
 * KNewStuff::DownloadDialog::open("kdesktop/wallpapers");
 * \endcode
 *
 * @author Josef Spillner (spillner@kde.org)
 * \par Maintainer:
 * Josef Spillner (spillner@kde.org)
 */
class KDE_EXPORT DownloadDialog : public KDialogBase
{
    Q_OBJECT
    struct Private;
  public:
    /**
      Constructor.

      @param engine a pre-built engine object, or NULL if the download
                    dialog should create an engine on its own
      @param parent the parent window
      @param caption the dialog caption
    */
    DownloadDialog(Engine *engine = 0L, QWidget *parent =0L , const QString& caption = QString::null);

    /**
      Destructor.
    */
    ~DownloadDialog();

    /**
      Restricts the display of available data to a certain data type.

      @param type a Hotstuff data type such as "korganizer/calendar"
    */
    void setType(const QString &type);

    /**
      Explicitly uses this provider list instead of the one read from
      the application configuration.

      @param providerList the URL of the provider list

      @since 3.4
    */
    void setProviderList(const QString& providerList);

    /**
      Fetches descriptions of all available data, optionally considering
      a previously set type.

      @param providerList the URl to the list of providers; if empty
             we first try the ProvidersUrl from KGlobal::config, then we
             fall back to a hardcoded value.
    */
    void load(const QString &providerList = QString::null);

    /**
      Adds another provider to the download dialog.
      This is normally done internally.

      @param p the Hotstuff provider to be added
    */
    void addProvider(Provider *p);

    /**
      Adds an additional entry to the current provider.
      This is normally done internally.

      @param entry a Hotstuff data entry to be added
    */
    void addEntry(Entry *entry);

    /**
      Clears the entry list of the current provider.
      This is normally done internally.
    */
    void clear();

    /**
      Opens the download dialog.
      This is a convenience method which automatically sets up the dialog.
      @see setType()
      @see load()

      @param type a data type such as "korganizer/calendar"
      @param caption the dialog caption
    */
    static void open(const QString& type, const QString& caption=QString::null);

  public slots:
    /**
      Availability of the provider list.

      @param list list of Hotstuff providers
    */
    void slotProviders(Provider::List *list);

  protected slots:
    void slotApply();
    void slotOk();

  private slots:
    void slotResult(KIO::Job *job);
    void slotData(KIO::Job *job, const QByteArray &a);
    void slotInstall();
    void slotDetails();
    void slotInstalled(KIO::Job *job);
    void slotTab(int tab);
    void slotSelected();
    void slotPage(QWidget *w);
    void slotFinish();

  private:
    void init(Engine *e);
    Entry *getEntry();
    void loadProvider(Provider *p);
    void install(Entry *e);
    int installStatus(Entry *e);
    Q3ListViewItem *currentEntryItem();

    ProviderLoader *m_loader;
    QString m_entryname;
    KListView *lv_r, *lv_d, *lv_l;
    QTextBrowser *m_rt;
    QFrame *m_frame;
    Q3ListViewItem *m_entryitem;
    Q3PtrList<Entry> m_entries;
    Entry *m_entry;
    KNewStuffGeneric *m_s;
    int m_curtab;
    QMap<QWidget*, QList<KListView*>* > m_map;
    QMap<QWidget*, Provider*> m_providers;
    QMap<QWidget*, QTextBrowser*> m_rts;
    QMap<QWidget*, QList<QPushButton*>* > m_buttons;
    QMap<KIO::Job*, Provider*> m_jobs;
    QMap<KIO::Job*, QString> m_data;
    QString m_filter;
    Engine *m_engine;
    Private *d;
};

}

#endif

