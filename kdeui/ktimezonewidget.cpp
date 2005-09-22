/*
    Copyright (C) 2005, S.R.Haque <srhaque@iee.org>.
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kdialog.h>
#include <kdebug.h>
#include <kfile.h>
#include <klistview.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktimezones.h>
#include <ktimezonewidget.h>
#include <qpixmap.h>
#include <time.h>

#define COLUMN_CITY 0
#define COLUMN_REGION 1
#define COLUMN_COMMENT 2
#define COLUMN_ZONE 3

KTimezoneWidget::KTimezoneWidget(QWidget *parent, const char *name, KTimezones *db) :
    KListView(parent),
    d(0)
{
    // If the user did not provide a timezone database, we'll use the system default.
    bool userDb = (db != 0);
    if (!userDb)
        db = new KTimezones();

    addColumn(i18n("Area"));
    addColumn(i18n("Region"));
    addColumn(i18n("Comment"));

    const KTimezones::ZoneMap zones = db->allZones();
    for (KTimezones::ZoneMap::ConstIterator it = zones.begin(); it != zones.end(); ++it)
    {
        const KTimezone *zone = it.data();
        QString tzName = zone->name();
        QString comment = zone->comment();
        if (!comment.isEmpty())
            comment = i18n(comment.utf8());

        // Convert:
        //
        //  "Europe/London", "GB" -> "London", "Europe/GB".
        //  "UTC",           ""   -> "UTC",    "".
        QStringList continentCity = QStringList::split("/", displayName(zone));
        Q3ListViewItem *listItem = new Q3ListViewItem(this, continentCity[continentCity.count() - 1]);
        continentCity[continentCity.count() - 1] = zone->countryCode();
        listItem->setText(COLUMN_REGION, continentCity.join("/"));
        listItem->setText(COLUMN_COMMENT, comment);
        listItem->setText(COLUMN_ZONE, tzName); /* store complete path in ListView */

        // Locate the flag from /l10n/%1/flag.png.
        QString flag = locate("locale", QString("l10n/%1/flag.png").arg(zone->countryCode().toLower()));
        if (QFile::exists(flag))
            listItem->setPixmap(COLUMN_REGION, QPixmap(flag));
    }

    if (!userDb)
        delete db;

}

KTimezoneWidget::~KTimezoneWidget()
{
    // FIXME when needed:
    // delete d;
}

QString KTimezoneWidget::displayName(const KTimezone *zone)
{
    return i18n(zone->name().utf8()).replace("_", " ");
}

QStringList KTimezoneWidget::selection() const
{
    QStringList selection;

    // Loop through all entries.
    Q3ListViewItem *listItem = firstChild();
    while (listItem)
    {
        if (listItem->isSelected())
        {
            selection.append(listItem->text(COLUMN_ZONE));
        }
        listItem = listItem->nextSibling();
    }
    return selection;
}

void KTimezoneWidget::setSelected(const QString &zone, bool selected)
{
    bool found = false;

    // Loop through all entries.
    Q3ListViewItem *listItem = firstChild();
    while (listItem)
    {
        if (listItem->text(COLUMN_ZONE) == zone)
        {
            KListView::setSelected(listItem, selected);

            // Ensure the selected item is visible as appropriate.
            listItem = selectedItem();
            if (listItem)
                ensureItemVisible(listItem);
            found = true;
            break;
        }
        listItem = listItem->nextSibling();
    }
    if (!found)
        kdDebug() << "No such zone: " << zone << endl;
}

#include "ktimezonewidget.moc"
