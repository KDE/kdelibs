/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KURLBAR_H
#define KURLBAR_H

#include <qevent.h>
#include <qframe.h>
#include <qtooltip.h>

#include <kdialogbase.h>
#include <kicontheme.h>
#include <klistbox.h>
#include <kurl.h>

class KConfig;
class KURLBar;

/**
 * An item to be used in @ref KURLBar / @ref KURLBarListBox. All the properties
 * (url, icon, descroption, tooltip) can be changed dynamically.
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @see KURLBar
 * @see KURLBarListBox
 */
class KURLBarItem : public QListBoxPixmap
{
public:
    /**
     * Creates a KURLBarItem to be used in the @p parent KURLBar. You need
     * to insert the item into the listbox manually, if you don't use
     * KURLBar::insertItem().
     *
     * If description is empty, it will try to use the filename/directory
     * of @p url, which will be shown as text of the item.
     * @p url will be used as tooltip, unless you set a different tip with
     * @ref setToolTip()
     */
    KURLBarItem( KURLBar *parent, const KURL& url,
                 const QString& description = QString::null,
                 const QString& icon = QString::null,
                 KIcon::Group group = KIcon::Panel );
    /**
     * Destroys the item
     */
    ~KURLBarItem();

    /**
     * Sets @p url for this item. Also updates the visible text to the
     * filename/directory of the url, if no description is set.
     * @see #url
     */
    void setURL( const KURL& url );
    /**
     * @p sets the icon for this item. See @ref KIconLoader for a description
     * of the icon groups.
     * @see #icon
     */
    void setIcon( const QString& icon, KIcon::Group group = KIcon::Panel );
    /**
     * Sets the description of this item that will be shown as item-text.
     * @see #description
     */
    void setDescription( const QString& desc );
    /**
     * Sets a tooltip to be used for this item.
     * @see #tooolTip
     */
    void setToolTip( const QString& tip );

    /**
     * returns the preferred size of this item
     * @since 3.1
     */
    QSize sizeHint() const;

    /**
     * returns the width of this item.
     */
    virtual int width( const QListBox * ) const;
    /**
     * returns the height of this item.
     */
    virtual int height( const QListBox * ) const;

    /**
     * returns the url  of this item.
     * @see #setURL
     */
    const KURL& url() const                     { return m_url; }
    /**
     * returns the description of this item.
     * @see #setDescription
     */
    const QString& description() const          { return m_description; }
    /**
     * returns the icon of this item.
     * @see #setIcon
     */
    const QString& icon() const                 { return m_icon; }
    /**
     * returns the tooltip of this item.
     * @see #setToolTip
     */
    QString toolTip() const;
    /**
     * returns the icon-group of this item (determines icon-effects).
     * @see #setIcon
     */
    KIcon::Group iconGroup() const              { return m_group; }
    /**
     * returns the pixmap  of this item.
     */
    virtual const QPixmap * pixmap() const      { return &m_pixmap; }

    /**
     * Makes this item a local or global one.
     * @see #applicationLocal
     */
    void setApplicationLocal( bool local )      { m_appLocal = local; }
    /**
     * returns whether this is a global item or a local one. @ref KURLBar
     * can differentiate between global and local items (only for the current
     * application) for easy extensiblity.
     * @see #setApplicationLocal
     */
    bool applicationLocal() const               { return m_appLocal; }


protected:
    virtual void paint( QPainter *p );

private:
    int iconSize() const;

    KURL m_url;
    QString m_description;
    QString m_icon;
    QString m_toolTip;
    QPixmap m_pixmap;
    KIcon::Group m_group;
    KURLBar *m_parent;
    bool m_appLocal :1;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KURLBarItemPrivate;
    KURLBarItemPrivate *d;
};


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


class KURLBarListBox;

/**
 * KURLBar is a widget that displays icons together with a description. They
 * can be arranged either horizontally or vertically. Clicking on an item
 * will make the @ref activated() signal being emitted. The user can edit
 * existing items by choosing "Edit entry" in the contextmenu. He can also
 * remove or add new entries (via drag&drop or the context menu).
 *
 * KURLBar offers the methods @ref readConfig() and @ref writeConfig() to
 * read and write the configuration of all the entries. It can differentiate
 * between global and local entries -- global entries will be saved in the
 * global configuration (kdeglobals), while local entries will be saved in
 * your application's KConfig object.
 *
 * Due to the configurability, you usually only insert some default entries
 * once and then solely use the read and writeConfig methods to preserve the
 * user's configuration.
 *
 * The widget has a "current" item, that is visualized to differentiate it
 * from others.
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @short A URL-bar widget, as used in the KFileDialog
 */
class KURLBar : public QFrame
{
    Q_OBJECT

public:
    /**
     * Constructs a KURLBar. Set @p useGlobalItems to true if you want to
     * allow global/local item separation.
     */
    KURLBar( bool useGlobalItems,
             QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    /**
     * Destroys the KURLBar.
     */
    ~KURLBar();

    /**
     * Inserts a new item into the KURLBar and returns the created
     * @ref KURLBarItem.
     *
     * @p url the url of the item
     * @p description the description of the item (shown in the view)
     * @p applicationLocal whether this should be a global or a local item
     * @p icon an icon -- if empty, the default icon for the url will be used
     * @p group the icon-group for using icon-effects
     */
    virtual KURLBarItem * insertItem( const KURL& url,
                                      const QString& description,
                                      bool applicationLocal = true,
                                      const QString& icon = QString::null,
                                      KIcon::Group group = KIcon::Panel );
    /**
     * The items can be arranged either vertically in one column or
     * horizontally in one row.
     * @see orientation
     */
    virtual void setOrientation( Qt::Orientation orient );
    /**
     * @returns the current orientation mode.
     * @see #setOrientation
     */
    Orientation orientation() const;

    /**
     * Allows to set a custom KURLBarListBox.
     * Note: The previous listbox will be deleted. Items of the previous
     * listbox will not be moved to the new box.
     * @see #listBox
     */
    virtual void setListBox( KURLBarListBox * );
    /**
     * @returns the @ref KURLBarListBox that is used.
     * @see #setListBox
     */
    KURLBarListBox *listBox() const { return m_listBox; }

    /**
     * Sets the default iconsize to be used for items inserted with
     * @ref insertItem. By default KIcon::SizeMedium.
     * @see #iconsize
     */
    virtual void setIconSize( int size );
    /**
     * @returns the default iconsize used for items inserted with
     * @ref insertItem. By default KIcon::SizeMedium
     * @see setIconSize
     */
    int iconSize() const { return m_iconSize; }

    /**
     * Clears the view, removes all items.
     */
    virtual void clear();

    /**
     * @returns a proper sizehint, depending on the orientation and number
     * of items available.
     */
    virtual QSize sizeHint() const;
    /**
     * @reimpl
     */
    virtual QSize minimumSizeHint() const;

    /**
     * Call this method to read a saved configuration from @p config,
     * inside the group @p itemGroup. All items in there will be restored.
     * The reading of every item is delegated to the @ref readItem() method.
     */
    virtual void readConfig(  KConfig *config, const QString& itemGroup );
    /**
     * Call this method to save the current configuration into @p config,
     * inside the group @p iconGroup. The @ref writeItem() method is used
     * to save each item.
     */
    virtual void writeConfig( KConfig *config, const QString& itemGroup );

    /**
     * Called from @ref readConfig() to read the i'th from @p config.
     * After reading a KURLBarItem is created and initialized with the read
     * values (as well as the given @p applicationLocal).
     */
    virtual void readItem( int i, KConfig *config, bool applicationLocal );
    /**
     * Called from @ref writeConfig() to save the KURLBarItem @p item as the
     * i'th entry in the config-object.
     * @p global tell whether it should be saved in the global configuration
     * or not (using KConfig::writeEntry( key, value, true, global ) ).
     */
    virtual void writeItem( KURLBarItem *item, int i, KConfig *, bool global );

    /**
     * @returns the current KURLBarItem, or 0L if none.
     * @see #setCurrentItem
     * @see #currentURL
     */
    KURLBarItem * currentItem() const;
    /**
     * @returns the url of the current item or an invalid url, if there is
     * no current item.
     * @see #currentItem
     * @see #setCurrentItem
     */
    KURL currentURL() const;

    /**
     * @returns true when the urlbar was modified by the user (e.g. by
     * editing/adding/removing one or more entries). Will be reset to false
     * after calling @ref writeConfig().
     */
    bool isModified() const             { return m_isModified; }

    /**
     * @returns true if the bar is in vertical mode.
     */
    bool isVertical() const { return orientation() == Vertical; }

public slots:
    /**
     * Makes the item with the url @p url the current item. Does nothing
     * if no item with that url is available.
     * @see #currentItem
     * @see #currentURL
     */
    virtual void setCurrentItem( const KURL& url );

signals:
    /**
     * This signal is emitted when the user activated an item, e.g by
     * clicking on it.
     */
    void activated( const KURL& url );

protected:
    /**
     * Pops up a @ref KURLBarItemDialog to let the user add a new item.
     * Uses @ref editItem() to do the job.
     * @returns false if the user aborted the dialog and no item is added.
     */
    virtual bool addNewItem();
    /**
     * Pops up a @ref KURLBarItemDialog to let the user edit the properties
     * of @p item. Invoked e.g. by @ref addNewItem(), when the user drops
     * a url  onto the bar or from the contextmenu.
     * @returns false if the user aborted the dialog and @p item is not
     * changed.
     */
    virtual bool editItem( KURLBarItem *item );
    /**
     * @reimpl
     */
    virtual void resizeEvent( QResizeEvent * );

    /**
     * The currently active item.
     */
    KURLBarItem * m_activeItem;
    /**
     * Whether we support global entries or just local ones.
     */
    bool m_useGlobal  :1;

    /**
     * Whether the urlbar was modified by the user (e.g. by
     * editing/adding/removing an item).
     */
    bool m_isModified :1;

protected slots:
    /**
     * Reimplemented to show a contextmenu, allowing the user to add, edit
     * or remove items, or change the iconsize.
     */
    virtual void slotContextMenuRequested( QListBoxItem *, const QPoint& pos );
    /**
     * Called when an item has been selected. Emits the @ref activated()
     * signal.
     */
    virtual void slotSelected( QListBoxItem * );

    /**
     * Called when a url was dropped onto the bar to show a
     * @ref KURLBarItemDialog.
     */
    virtual void slotDropped( QDropEvent * );

private slots:
    void slotSelected( int button, QListBoxItem * );

private:
    KURLBarListBox *m_listBox;
    int m_iconSize;


protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KURLBarPrivate;
    KURLBarPrivate *d;
};


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


class QDragObject;
class KURLBarToolTip;

/**
 * This is the listbox used in KURLBar. It is a subclass of KListBox to support
 * drag & drop and to set up the row / column mode.
 *
 * The widget has just one row or one column, depending on @ref orientation().
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KURLBarListBox : public KListBox
{
    Q_OBJECT

public:
    /**
     * Constructs a KURLBarListBox.
     */
    KURLBarListBox( QWidget *parent = 0, const char *name = 0 );
    /**
     * Destroys the box.
     */
    ~KURLBarListBox();

    /**
     * Sets the orientation of the widget. Horizontal means, all items are
     * arranged in one row. Vertical means, all items are arranged in one
     * column.
     * @see #orientation
     */
    virtual void setOrientation( Qt::Orientation orient );
    /**
     * @returns the current orientation.
     * @see #setOrientation
     */
    Qt::Orientation orientation() const { return m_orientation; }

    bool isVertical() const { return m_orientation == Qt::Vertical; }

signals:
    /**
     * Emitted when a drop-event happened.
     */
    void dropped( QDropEvent *e );

protected:
    /**
     * @returns a suitable @ref QDragObject when an item is dragged.
     */
    virtual QDragObject * dragObject();
    /**
     * @reimpl
     */
    virtual void contentsDragEnterEvent( QDragEnterEvent * );
    /**
     * @reimpl
     */
    virtual void contentsDropEvent( QDropEvent * );

private:
    Qt::Orientation m_orientation;
    KURLBarToolTip *m_toolTip;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KURLBarListBoxPrivate;
    KURLBarListBoxPrivate *d;
};


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


class QCheckBox;
class KIconButton;
class KLineEdit;
class KURLRequester;

/**
 * A dialog that allows editing entries of a @ref KURLBar (@ref KURLBarItem).
 * The dialog offers to configure a given url, description and icon.
 * See the class-method @ref getInformation() for easy usage.
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KURLBarItemDialog : public KDialogBase
{
    Q_OBJECT

public:
    /**
     * A convenience method to show up the dialog and retrieve all the
     * properties via the given parameters. The parameters are used to
     * initialize the dialog and then return the user-configured values.
     *
     * See the @ref KURLBarItem constructor for the parameter description.
     */
    static bool getInformation( bool allowGlobal, KURL& url,
                                QString& description, QString& icon,
                                bool& appLocal, int iconSize,
                                QWidget *parent = 0 );

    /**
     * Constructs a KURLBarItemDialog.
     *
     * @p allowGlobal if you set this to true, the dialog will have a checkbox
     *                for the user to decide if he wants the entry to be
     *                available globally or just for the current application.
     * @p url the url of the item
     * @p description a short, translated description of the item
     * @p icon an icon for the item
     * @p appLocal tells whether the item should be local for this application
     *             or available globalle
     * @p iconSize determines the size of the icon that is shown/selectable
     * @p parent the parent-widget for the dialog
     *
     * If you leave the icon empty, the default icon for the given url will be
     * used (KMimeType::pixmapForURL()).
     */
    KURLBarItemDialog( bool allowGlobal, const KURL& url,
                       const QString& description, QString icon,
                       bool appLocal = true,
                       int iconSize = KIcon::SizeMedium,
                       QWidget *parent = 0, const char *name = 0 );
    /**
     * Destroys the dialog.
     */
    ~KURLBarItemDialog();

    /**
     * @returns the configured url
     */
    KURL url() const;

    /**
     * @returns the configured description
     */
    QString description() const;

    /**
     * @returns the configured icon
     */
    QString icon() const;

    /**
     * @returns whether the item should be local to the application or global.
     * if allowGlobal was set to false in the constructor, this will always
     * return true.
     */
    bool applicationLocal() const;

protected:
    /**
     * The @ref KURLRequester used for editing the url
     */
    KURLRequester * m_urlEdit;
    /**
     * The @ref KLineEdit used for editing the description
     */
    KLineEdit     * m_edit;
    /**
     * The @ref KIconButton to configure the icon
     */
    KIconButton   * m_iconButton;
    /**
     * The @ref QCheckBox to modify the local/global setting
     */
    QCheckBox     * m_appLocal;

public slots:
    void urlChanged(const QString & );

private:
    class KURLBarItemDialogPrivate;
    KURLBarItemDialogPrivate *d;
};


#endif // KURLBAR_H
