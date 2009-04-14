/*
    This file is part of KNewStuff2.
    Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
    Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007 Dirk Mueller <mueller@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

// own include
#include "itemsview.h"

// qt/kde includes
#include <QtCore/QFile>
#include <QtGui/QWidget>
#include <QtCore/QTimer>
#include <QtGui/QLayout>
#include <QtGui/QImage>
#include <QtGui/QFont>
#include <QtGui/QComboBox>
#include <QtGui/QPushButton>
#include <QtCore/QMutableVectorIterator>
#include <QtCore/QRect>
#include <QtGui/QPainter>
#include <QtGui/QScrollArea>
#include <QtGui/QApplication>
#include <QtGui/QTextDocument>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcomponentdata.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <klineedit.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <ktitlewidget.h>
#include <ktoolinvocation.h>

#include "knewstuff2/core/provider.h"
#include "knewstuff2/core/providerhandler.h"
#include "knewstuff2/core/entry.h"
#include "knewstuff2/core/entryhandler.h"
#include "knewstuff2/core/category.h"

#include "knewstuff2/dxs/dxs.h"

#include "knewstuff2/ui/qprogressindicator.h"

// local includes
#include "ui_DownloadDialog.h"
#include "kdxsbutton.h"
#include "qasyncimage_p.h"

using namespace KNS;

static bool NameSorter(const Entry* e1, const Entry* e2)
{
    return e1->name().representation() < e2->name().representation();
}

static bool RatingSorter(const Entry* e1, const Entry* e2)
{
    return e1->rating() < e2->rating();
}

static bool RecentSorter(const Entry* e1, const Entry* e2)
{
    // return > instead of < to sort in reverse order
    return e1->releaseDate() > e2->releaseDate();
}

static bool DownloadsSorter(const Entry* e1, const Entry* e2)
{
    // return > instead of < to sort most downloads at the top
    return e1->downloads() > e2->downloads();
}

ItemsView::ItemsView(QWidget* _parent)
        : QListView(_parent),
        m_currentProvider(0), m_currentFeed(0), m_root(0), m_sorting(0), m_engine(0)
{
    m_root = new QWidget(this);
    setFrameStyle(QFrame::Plain | QFrame::StyledPanel);
    setVerticalScrollMode(ScrollPerPixel);
    //setWidgetResizable(true);
}

ItemsView::~ItemsView()
{
}

void ItemsView::setEngine(DxsEngine *engine)
{
    m_engine = engine;
}

void ItemsView::setProvider(const Provider * provider, const Feed * feed)
{
    m_currentProvider = provider;
    m_currentFeed = feed;
    buildContents();
}

void ItemsView::setSorting(int sortType)
{
    m_sorting = sortType;
    buildContents();
}

void ItemsView::setFeed(const Feed * feed)
{
    m_currentFeed = feed;
    buildContents();
}

void ItemsView::setSearchText(const QString & text)
{
    m_searchText = text;
    buildContents();
}

void ItemsView::updateItem(Entry *entry)
{
    // FIXME: change this to call updateEntry once it is complete
//   if (m_views.contains(entry)) {
//        m_views[entry]->setEntry(entry);
    //  }
}

void ItemsView::buildContents()
{
    m_views.clear();

    m_root->setBackgroundRole(QPalette::Base);
    QVBoxLayout* _layout = new QVBoxLayout(m_root);

    if (m_currentFeed != NULL) {
        Entry::List entries = m_currentFeed->entries();
        //switch (m_sorting)
        //{
        //    case 0:
        //        qSort(entries.begin(), entries.end(), NameSorter);
        //        break;
        //    case 1:
        //        qSort(entries.begin(), entries.end(), RatingSorter);
        //        break;
        //    case 2:
        //        qSort(entries.begin(), entries.end(), RecentSorter);
        //        break;
        //    case 3:
        //        qSort(entries.begin(), entries.end(), DownloadsSorter);
        //        break;
        //}

        Entry::List::iterator it = entries.begin(), iEnd = entries.end();
        for (unsigned row = 0; it != iEnd; ++it) {
            Entry* entry = (*it);

            if (entry->name().representation().toLower().contains(m_searchText.toLower())) {
                QHBoxLayout * itemLayout = new QHBoxLayout;
                _layout->addLayout(itemLayout);

                EntryView *part = new EntryView(m_root);
                part->setBackgroundRole(row & 1 ? QPalette::AlternateBase : QPalette::Base);
                itemLayout->addWidget(part);

                QVBoxLayout * previewLayout = new QVBoxLayout;
                itemLayout->insertLayout(0, previewLayout);

                KDXSButton *dxsbutton = new KDXSButton(m_root);
                dxsbutton->setEntry(entry);
                dxsbutton->setProvider(m_currentProvider);
                dxsbutton->setEngine(m_engine);

                QString imageurl = entry->preview().representation();
                if (!imageurl.isEmpty()) {
                    QLabel *f = new QLabel(m_root);
                    f->setFrameStyle(QFrame::Panel | QFrame::Sunken);
                    QAsyncImage *pix = new QAsyncImage(imageurl, m_root);
                    f->setFixedSize(64, 64);
                    //connect(pix, SIGNAL(signalLoaded(const QImage&)),
                    //        f, SLOT(setImage(const QImage&)));
                    previewLayout->addWidget(f);
                }
                //previewLayout->addWidget(dxsbutton);

                part->setEntry(entry);
                m_views.insert(entry, part);
                ++row;
            }
        }
    }

    //setWidget(m_root);
}

EntryView::EntryView(QWidget * _parent)
        : QLabel(_parent)
{
    connect(this, SIGNAL(linkActivated(const QString&)), SLOT(urlSelected(const QString&)));
}

void EntryView::setEntry(Entry *entry)
{
    m_entry = entry;
    buildContents();
}

void EntryView::updateEntry(Entry *entry)
{
    // get item id string and iformations
    QString idString = QString::number((unsigned long)entry);
    //            AvailableItem::State state = item->state();
    //            bool showProgress = state != AvailableItem::Normal;
    //            int pixelProgress = showProgress ? (int)(item->progress() * 80.0) : 0;

    // perform internal scripting operations over the element
    //            executeScript( "document.getElementById('" + idString + "').style.color='red'" );
    //            executeScript( "document.getElementById('bar" + idString + "').style.width='" +
    //                           QString::number( pixelProgress ) + "px'" );
    //            executeScript( "document.getElementById('bc" + idString + "').style.backgroundColor='" +
    //                           (showProgress ? "gray" : "transparent") + "'" );
    //            executeScript( "document.getElementById('btn" + idString + "').value='" +
    //                           (item->installed() ? i18n( "Uninstall" ) : i18n( "Install" )) + "'" );
}

void EntryView::buildContents()
{
    // write the html header and contents manipulation scripts
    QString t;

    t += "<html><body>";

    //t += setTheAaronnesqueStyle();
    // precalc the status icon
    Entry::Status status = m_entry->status();
    QString statusIcon;
    KIconLoader *loader = KIconLoader::global();

    switch (status) {
    case Entry::Invalid:
        statusIcon = "<img src='" + loader->iconPath("dialog-error", -KIconLoader::SizeSmall) + "' />";
        break;
    case Entry::Downloadable:
        // find a good icon to represent downloadable data
        //statusIcon = "<img src='" + loader->iconPath("network-server", -KIconLoader::SizeSmall) + "' />";
        break;
    case Entry::Installed:
        statusIcon = "<img src='" + loader->iconPath("dialog-ok", -KIconLoader::SizeSmall) + "' />";
        break;
    case Entry::Updateable:
        statusIcon = "<img src='" + loader->iconPath("software-update-available", -KIconLoader::SizeSmall) + "' />";
        break;
    case Entry::Deleted:
        statusIcon = "<img src='" + loader->iconPath("user-trash", -KIconLoader::SizeSmall) + "' />";
        break;
    }

    // precalc the title string
    QString titleString = m_entry->name().representation();
    if (!m_entry->version().isEmpty()) titleString += " v." + Qt::escape(m_entry->version());

    // precalc the string for displaying stars (normal+grayed)
    QString starIconPath = KStandardDirs::locate("data", "knewstuff/pics/ghns_star.png");
    QString starBgIconPath = KStandardDirs::locate("data", "knewstuff/pics/ghns_star_gray.png");

    int starPixels = 11 + 11 * (m_entry->rating() / 10);
    QString starsString = "<div style='width: " + QString::number(starPixels) + "px; background-image: url(" + starIconPath + "); background-repeat: repeat-x;'>&nbsp;</div>";
    int grayPixels = 22 + 22 * (m_entry->rating() / 20);
    starsString = "<div style='width: " + QString::number(grayPixels) + "px;background-image: url(" + starBgIconPath + "); background-repeat: repeat-x;'>" + starsString + "&nbsp;</div>";

    // precalc the string for displaying author (parsing email)
    KNS::Author author = m_entry->author();
    QString authorString = author.name();

    QString emailString = author.email();
    if (!emailString.isEmpty()) {
        authorString = "<a href='mailto:" + Qt::escape(emailString) + "'>"
                       + Qt::escape(authorString) + "</a>";
    }

    // write the HTML code for the current item
    t += //QLatin1String("<table class='contentsHeader' cellspacing='2' cellpadding='0'>")
        statusIcon + Qt::escape(titleString) + "<br />"
        //+   "<span align='right'>" + starsString +  "</span><br />"
        +      Qt::escape(m_entry->summary().representation())
        +   "<br />";

    if (m_entry->rating() > 0) {
        t += i18n("Rating: ") + QString::number(m_entry->rating())
             +   "<br />";
    }

    if (m_entry->downloads() > 0) {
        t += i18n("Downloads: ") + QString::number(m_entry->downloads())
             +   "<br />";
    }

    if (!authorString.isEmpty()) {
        t += "<em>" + authorString + "</em>, ";
    }
    t += KGlobal::locale()->formatDate(m_entry->releaseDate(), KLocale::ShortDate)
         + "<br />" + "</body></html>";

    setText(t);
}

void EntryView::setTheAaronnesqueStyle()
{
    QString hoverColor = "#000000"; //QApplication::palette().active().highlightedText().name();
    QString hoverBackground = "#f8f8f8"; //QApplication::palette().active().highlight().name();
    QString starIconPath = KStandardDirs::locate("data", "knewstuff/pics/ghns_star.png");
    QString starBgIconPath = KStandardDirs::locate("data", "knewstuff/pics/ghns_star_gray.png");

    // default elements style
    QString s;
    s += "body { background-color: white; color: black; padding: 0; margin: 0; }";
    s += "table, td, th { padding: 0; margin: 0; text-align: left; }";
    s += "input { color: #000080; font-size:120%; }";

    // the main item container (custom element)
    s += ".itemBox { background-color: white; color: black; width: 100%;  border-bottom: 1px solid gray; margin: 0px 0px; }";
    s += ".itemBox:hover { background-color: " + hoverBackground + "; color: " + hoverColor + "; }";

    // s of the item elements (4 cells with multiple containers)
    s += ".leftColumn { width: 100px; height:100%; text-align: center; }";
    s += ".leftImage {}";
    s += ".leftButton {}";
    s += ".leftProgressContainer { width: 82px; height: 10px; background-color: transparent; }";
    s += ".leftProgressBar { left: 1px; width: 0px; top: 1px; height: 8px; background-color: red; }";
    s += ".contentsColumn { vertical-align: top; }";
    s += ".contentsHeader { width: 100%; font-size: 120%; font-weight: bold; border-bottom: 1px solid #c8c8c8; }";
    s += ".contentsBody {}";
    s += ".contentsFooter {}";
    s += ".star { width: 0px; height: 24px; background-image: url(" + starIconPath + "); background-repeat: repeat-x; }";
    s += ".starbg { width: 110px; height: 24px; background-image: url(" + starBgIconPath + "); background-repeat: repeat-x; }";
    setStyleSheet(s);
}

void EntryView::urlSelected(const QString &link)
{
    //kDebug() << "Clicked on URL " << link;

    KUrl url(link);
    QString urlProtocol = url.protocol();
    QString urlPath = url.path();

    if (urlProtocol == "mailto") {
        // clicked over a mail address
        // FIXME: if clicked with MRB, show context menu with IM etc.
        // FIXME: but RMB never reaches this method?!
        KToolInvocation::invokeMailer(url);
    } else if (urlProtocol == "item") {
        // clicked over an item
        bool ok;
        unsigned long itemPointer = urlPath.toULong(&ok);
        if (!ok) {
            kWarning() << "ItemsView: error converting item pointer.";
            return;
        }

        // I love to cast pointers
        Entry *entry = (Entry*)itemPointer;
        if (entry != m_entry) {
            kWarning() << "ItemsView: error retrieving item pointer.";
            return;
        }

        // XXX ???
        // install/uninstall the item
        //                if ( item->installed() )
        //                    m_newStuffDialog->removeItem( item );   // synchronous
        //                else
        //                    m_newStuffDialog->installItem( item );  // asynchronous
    }
}

#include "itemsview.moc"

