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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KFILEFILTERCOMBO_H
#define KFILEFILTERCOMBO_H

#include <qstringlist.h>
#include <qptrdict.h>

#include <kcombobox.h>
#include <kmimetype.h>

class KFileFilterComboPrivate;

class KFileFilterCombo : public KComboBox
{
    Q_OBJECT

 public:
    KFileFilterCombo(QWidget *parent= 0, const char *name= 0);
    ~KFileFilterCombo();

    void setFilter(const QString& filter);

    /**
     * @returns the current filter, either something like "*.cpp *.h"
     * or the current mimetype, like "text/html", or a list of those, like
     " "text/html text/plain image/png", all separated with one space.
     */
    QString currentFilter() const;

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
     * @ref setFilter().
     *
     * By default, this is set to i18n("*|All Files")
     * @see #defaultFilter
     */
    void setDefaultFilter( const QString& filter );

    /**
     * @return the default filter, used when an empty filter is set.
     * @see #setDefaultFilter
     */
    QString defaultFilter() const;
    
 protected:
    virtual bool eventFilter( QObject *o, QEvent *e );

    QStringList filters;
    bool m_allTypes;

 signals:
    void filterChanged();

private slots:
    void slotFilterChanged();

private:
    class KFileFilterComboPrivate;
    KFileFilterComboPrivate *d;
};

#endif
