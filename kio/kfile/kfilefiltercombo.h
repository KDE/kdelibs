/* This file is part of the KDE libraries
    Copyright (C) Stephan Kulow <coolo@kde.org>

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

#ifndef KFILEFILTERCOMBO_H
#define KFILEFILTERCOMBO_H

#include <qstringlist.h>

#include <kcombobox.h>
#include <kmimetype.h>

class KFileFilterComboPrivate;

class KIO_EXPORT KFileFilterCombo : public KComboBox
{
    Q_OBJECT

public:
    KFileFilterCombo(QWidget *parent=0);
    ~KFileFilterCombo();

    void setFilter(const QString& filter);

    /**
     * @returns the current filter, either something like "*.cpp *.h"
     * or the current mimetype, like "text/html", or a list of those, like
     " "text/html text/plain image/png", all separated with one space.
     */
    QString currentFilter() const;

    /**
     * Sets the current filter. Filter must match one of the filter items
     * passed before to this widget.
     * @since 3.4
     */
    void setCurrentFilter( const QString& filter );

    /**
     * Sets a list of mimetypes.
     * If @p defaultType is set, it will be set as the current item.
     * Otherwise, a first item showing all the mimetypes will be created.
     */
    void setMimeFilter( const QStringList& types, const QString& defaultType );

    /**
     * @return true if the filter's first item is the list of all mimetypes
     */
    bool showsAllTypes() const { return m_allTypes; }

    /**
     * This method allows you to set a default-filter, that is used when an
     * empty filter is set. Make sure you call this before calling
     * setFilter().
     *
     * By default, this is set to i18n("*|All Files")
     * @see defaultFilter
     */
    void setDefaultFilter( const QString& filter );

    /**
     * @return the default filter, used when an empty filter is set.
     * @see setDefaultFilter
     */
    QString defaultFilter() const;

    /**
     * @return all filters (this can be a list of patterns or a list of mimetypes)
     */
    QStringList filters() const { return m_filters; }

protected:
    virtual bool eventFilter( QObject *o, QEvent *e );
    virtual void virtual_hook( int id, void* data );

signals:
    void filterChanged();

private slots:
    void slotFilterChanged();

private:
    QStringList m_filters;
    bool m_allTypes;
    class KFileFilterComboPrivate;
    KFileFilterComboPrivate* const d;
};

#endif
