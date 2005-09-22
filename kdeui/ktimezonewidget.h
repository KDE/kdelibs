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

#ifndef KTIMEZONEWIDGET_H
#define KTIMEZONEWIDGET_H

#include <kdelibs_export.h>
#include <klistview.h>
#include <qstring.h>

class KTimezone;
class KTimezones;
class KTimezoneWidgetPrivate;

/**
 * @brief A timezone selection widget.
 *
 * \b Detail:
 *
 * This class provides for selection of one or more timezones.
 *
 * \b Example:
 *
 * To use the class to implement a system timezone selection feature:
 * \code
 *
 *  // This adds a timezone widget to a dialog.
 *  m_timezones =  new KTimezoneWidget(this);
 *  ...
 * \endcode
 *
 * To use the class to implement a multiple-choice custom timezone selector:
 * \code
 *
 *  m_timezones =  new KTimezoneWidget(this, "Timezones", vcalendarTimezones);
 *  m_timezones->setSelectionModeExt(KListView::Multi);
 *  ...
 * \endcode
 *
 * @author S.R.Haque <srhaque@iee.org>
 * @since 3.5
 */
class KDEUI_EXPORT KTimezoneWidget :
    public KListView
{
    Q_OBJECT

public:
    /**
     * Constructs a timezone selection widget.
     *
     * @param parent The parent widget.
     * @param name The name of this widget.
     * @param db The timezone database to use. If 0, the system timezone
     *           database is used.
     */
    KTimezoneWidget(QWidget *parent = 0, KTimezones *db = 0);

    /**
     * Destroys the timezone selection widget.
     */
    virtual ~KTimezoneWidget();

    /**
     * Returns the currently selected timezones. See QListView::selectionChanged().
     *
     * @return a list of timezone names, in the format used by the database
     *         supplied to the {@link KTimezoneWidget() } constructor.
     */
    QStringList selection() const;

    /**
     * Select/deselect the named timezone.
     *
     * @param zone The timezone name to be selected. Ignored if not recognised!
     * @param selected The new selection state.
     */
    void setSelected(const QString &zone, bool selected);

    /**
     * Format a timezone name in a standardised manner. The returned value is
     * transformed via an i18n lookup, so the caller should previously have
     * set the timezone catalog:
     * \code
     * KGlobal::locale()->insertCatalog("timezones");
     * \endcode
     *
     * @return formatted timezone name.
     */
    static QString displayName(const KTimezone *zone);

private:
    KTimezoneWidgetPrivate *d;
};

#endif
