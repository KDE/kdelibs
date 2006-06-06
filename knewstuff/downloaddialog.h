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

#include <kpagedialog.h>
#include <knewstuff/provider.h>

#include <qmap.h>

class KJob;
namespace KIO
{
  class Job;
}

class QFrame;
class QTreeWidget;
class QTreeWidgetItem;
class QTextBrowser;
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
class KNEWSTUFF_EXPORT DownloadDialog : public KPageDialog
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
    DownloadDialog(Engine *engine = 0L, QWidget *parent =0L , const QString& caption = QString());

    /**
      Destructor.
    */
    ~DownloadDialog();

    /**
      Restricts the display of available data to a certain data type.

      @param category a Hotstuff data type such as "korganizer/calendar"
    */
    void setCategory(const QString &category);
    KDE_DEPRECATED void setType(const QString &type);

    /**
      Explicitly uses this provider list instead of the one read from
      the application configuration.

      @param providerList the URL of the provider list
    */
    void setProviderList(const QString& providerList);

    /**
      Fetches descriptions of all available data, optionally considering
      a previously set type.

      @param providerList the URl to the list of providers; if empty
             we first try the ProvidersUrl from KGlobal::config, then we
             fall back to a hardcoded value.
    */
    void load(const QString &providerList = QString());

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
      Adds an additional entry to the current provider.
      This is normally done internally.
      This version takes into accounts the download variant.

      @param entry a Hotstuff data entry to be added
      @param variants all variants this entry is intended for
    */
    void addEntry(Entry *entry, const QStringList& variants);

    /**
      Clears the entry list of the current provider.
      This is normally done internally.
    */
    void clear();

    /**
      Opens the download dialog.
      This is a convenience method which automatically sets up the dialog.
      @see setCategory()
      @see load()

      @param category a data type such as "korganizer/calendar"
      @param caption the dialog caption
    */
    static void open(const QString& category , const QString& caption=QString());

    /**
      Returns the list of installed data entries.

      @return list of data entries which have been installed
    */
    QList<Entry*> installedEntries();
    // ### KDE 4.0: the open() method should return this

  public Q_SLOTS:
    /**
      Availability of the provider list.

      @param list list of Hotstuff providers
    */
    void slotProviders(Provider::List *list);

  protected Q_SLOTS:
    void slotApply();
    void slotOk();

  private Q_SLOTS:
    void slotResult(KJob *job);
    void slotData(KIO::Job *job, const QByteArray &a);
    void slotInstall();
    void slotDetails();
    void slotInstalled(KJob *job);
    void slotTab(int tab);
    void slotSelected();
    void slotPage(QWidget *w);
    void slotFinish();
    void slotEmail(const QString& link);

  private:
    void init(Engine *e);
    Entry *getEntry();
    void loadProvider(Provider *p);
    void install(Entry *e);
    int installStatus(Entry *e);
    QTreeWidgetItem *currentEntryItem();

    ProviderLoader *m_loader;
    QString m_entryname;
    QTreeWidget *lv_r, *lv_d, *lv_l;
    QTextBrowser *m_rt;
    QFrame *m_frame;
    QTreeWidgetItem *m_entryitem;
    QList<Entry*> m_entries;
    Entry *m_entry;
    KNewStuffGeneric *m_s;
    int m_curtab;
    QMap<QWidget*, QList<QTreeWidget*>* > m_map;
    QMap<QWidget*, Provider*> m_providers;
    QMap<QWidget*, QTextBrowser*> m_rts;
    QMap<QWidget*, QList<QPushButton*>* > m_buttons;
    QMap<KJob*, Provider*> m_jobs;
    QMap<KJob*, QString> m_data;
    QString m_filter;
    Engine *m_engine;
    Private* const d;
};

}

#endif

