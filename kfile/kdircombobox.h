/* This file is part of the KDE libraries
    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef KDIRCOMBOBOX_H
#define KDIRCOMBOBOX_H

#include <qcombobox.h>
#include <qlist.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qstringlist.h>

#include <kurl.h>

/**
 * This combobox shows a number of recent URLs/directories, as well as some
 * default directories.
 * It will manage the default dirs root-directory, home-directory and
 * Desktop-directory, as well as a number of URLs set via @ref setURLs()
 * and one additional entry to be set via @ref setURL().
 *
 * @short A ComboBox showing a number of recent URLs/directories
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KDirComboBox : public QComboBox
{
    Q_OBJECT

public:
    KDirComboBox( QWidget *parent=0, const char *name=0 );
    ~KDirComboBox();

    /**
     * Sets the current url. This combo handles exactly one url additionally
     * to the default items and those set via @ref setURLs(). So you can call
     * setURL() as often as you want, it will always replace the previous one
     * set via setURL().
     * If @param @p url is already in the combo, the last item will stay there
     * and the existing item becomes the current item.
     * The current item will always have the open-directory-pixmap as icon.
     */
    void setURL( const KURL& url );


    void setURLs( const QStringList& urls );

    /**
     * @returns a list of all urls currently handled. The list contains at most
     * @ref maxItems() items.
     * Use this to save the list of urls in a config-file and reinsert them
     * via @ref setURLs() next time.
     * Note that the default directories (root, home, desktop) are not
     * returned, they will automatically be set via setURLs() or setURL().
     * You will always get fully qualified urls, i.e. with protocol like
     * file:/
     */
    QStringList urls() const;

    /**
     * Sets how many items should be handled and displayed by the combobox.
     * @see #maxItems
     */
    void setMaxItems( int );

    /**
     * @returns the maximum of items the combobox handles.
     * @see #setMaxItems
     */
    int maxItems() const { return myMaximum; }

    /**
     * Inserts the default directories into the combo. Will be implicitly
     * called upon the first call to @ref setURLs() or @ref setURL().
     */
    void insertDefaults();

signals:
    /**
     * emitted when an item was clicked at
     * @param url is the url of the now current item. If it is a local url,
     * it won't have a protocol (file:/), otherwise it will.
     */
    void urlActivated( const QString& url );

protected slots:
    void slotActivated( int );

protected:
    struct _KDirComboItem {
	QString text;
	QString url;
    };
    typedef _KDirComboItem KDirComboItem;
    QList<KDirComboItem> itemList;
    QMap<int,const KDirComboItem*> itemMapper;

    void insertItem( const KDirComboItem * );

    KDirComboItem *rootItem;
    KDirComboItem *homeItem;
    KDirComboItem *desktopItem;

    QPixmap opendirPix;
    QPixmap dirPix;
    QPixmap rootPix;
    QPixmap homePix;
    QPixmap desktopPix;

    static const int FIRSTITEM = 3;

private:
    bool urlAdded;
    int myMaximum;

};


#endif // KDIRCOMBOBOX_H
