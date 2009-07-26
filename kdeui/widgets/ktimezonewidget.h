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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KTIMEZONEWIDGET_H
#define KTIMEZONEWIDGET_H

#include <kdeui_export.h>

#include <QtGui/QTreeWidget>

class KTimeZone;
class KTimeZones;

/**
 * @brief A time zone selection widget.
 *
 * \b Detail:
 *
 * This class provides for selection of one or more time zones.
 *
 * \b Example:
 *
 * To use the class to implement a system timezone selection feature:
 * \code
 *
 *  // This adds a time zone widget to a dialog.
 *  m_timezones = new KTimeZoneWidget(this);
 *  ...
 * \endcode
 *
 * To use the class to implement a multiple-choice custom time zone selector:
 * \code
 *
 *  m_timezones = new KTimeZoneWidget( this, "Time zones", vcalendarTimezones );
 *  m_timezones->setSelectionMode( QTreeView::MultiSelection );
 *  ...
 * \endcode
 *
 * \image html ktimezonewidget.png "KDE Time Zone Widget"
 *
 * @author S.R.Haque <srhaque@iee.org>
 */
class KDEUI_EXPORT KTimeZoneWidget : public QTreeWidget
{
    Q_OBJECT

  public:
    /**
     * Constructs a time zone selection widget.
     *
     * @param parent The parent widget.
     * @param timeZones The time zone database to use. If 0, the system time zone
     *                  database is used.
     */
    explicit KTimeZoneWidget( QWidget *parent = 0, KTimeZones *timeZones = 0 );

    /**
     * Destroys the time zone selection widget.
     */
    virtual ~KTimeZoneWidget();

    /**
     * Returns the currently selected time zones. See QTreeView::selectionChanged().
     *
     * @return a list of time zone names, in the format used by the database
     *         supplied to the {@link KTimeZoneWidget() } constructor.
     */
    QStringList selection() const;

    /**
     * Select/deselect the named time zone.
     *
     * @param zone The time zone name to be selected. Ignored if not recognized!
     * @param selected The new selection state.
     */
    void setSelected( const QString &zone, bool selected );

    /**
     * Format a time zone name in a standardised manner. The returned value is
     * transformed via an i18n lookup, so the caller should previously have
     * set the time zone catalog:
     * \code
     *   KGlobal::locale()->insertCatalog( "timezones4" );
     * \endcode
     *
     * @return formatted time zone name.
     */
    static QString displayName( const KTimeZone &zone );

  private:
    class Private;
    Private* const d;
};

#endif
