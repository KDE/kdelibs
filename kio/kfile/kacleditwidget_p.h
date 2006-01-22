/***************************************************************************
 *   Copyright (C) 2005 by Sean Harmer <sh@rama.homelinux.org>             *
 *                         Till Adam <adam@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by  the Free Software Foundation; either version 2 of the   *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef KACLEDITWIDGET_P_H
#define KACLEDITWIDGET_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_POSIX_ACL
#include <klistview.h>
#include <sys/acl.h>
#include <kacl.h>
#include <kfileitem.h>
#include <kdialogbase.h>
#include <qpixmap.h>
#include <qcombobox.h>

class KACLListViewItem;
class QPushButton;
class QVButtonGroup;
class KACLListView;
class QWidgetStack;
class QCheckBox;

/**
@author Sean Harmer
*/
class KACLListView : public KListView
{
Q_OBJECT
    friend class KACLListViewItem;
public:
    enum Types
    {
        OWNER_IDX = 0,
        GROUP_IDX,
        OTHERS_IDX,
        MASK_IDX,
        NAMED_USER_IDX,
        NAMED_GROUP_IDX,
        LAST_IDX
    };
    enum EntryType { User = 1,
                     Group = 2,
                     Others = 4,
                     Mask = 8,
                     NamedUser = 16,
                     NamedGroup = 32,
                     AllTypes = 63 };

    KACLListView( QWidget* parent = 0, const char* name = 0 );
    ~KACLListView();

    bool hasMaskEntry() const { return m_hasMask; }
    bool hasDefaultEntries() const;
    bool allowDefaults() const { return m_allowDefaults; }
    void setAllowDefaults( bool v ) { m_allowDefaults = v; }
    unsigned short maskPermissions() const;
    void setMaskPermissions( unsigned short maskPerms );
    acl_perm_t maskPartialPermissions() const;
    void setMaskPartialPermissions( acl_perm_t maskPerms );

    bool maskCanBeDeleted() const;
    bool defaultMaskCanBeDeleted() const;

    const KACLListViewItem* findDefaultItemByType( EntryType type ) const;
    const KACLListViewItem* findItemByType( EntryType type,
                                            bool defaults = false ) const;
    unsigned short calculateMaskValue( bool defaults ) const;
    void calculateEffectiveRights();

    QStringList allowedUsers( bool defaults, KACLListViewItem *allowedItem = 0 );
    QStringList allowedGroups( bool defaults, KACLListViewItem *allowedItem = 0 );

    const KACL getACL() const { return getACL(); }
    KACL getACL();

    const KACL getDefaultACL() const { return getDefaultACL(); }
    KACL getDefaultACL();

    QPixmap getYesPixmap() const { return *m_yesPixmap; }
    QPixmap getYesPartialPixmap() const { return *m_yesPartialPixmap; }

public slots:
    void slotAddEntry();
    void slotEditEntry();
    void slotRemoveEntry();
    void setACL( const KACL &anACL );
    void setDefaultACL( const KACL &anACL );

protected slots:
    void entryClicked( QListViewItem* pItem, const QPoint& pt, int col );
protected:
    void contentsMousePressEvent( QMouseEvent * e );

private:
    void fillItemsFromACL( const KACL &pACL, bool defaults = false );
    KACL itemsToACL( bool defaults ) const;

    KACL m_ACL;
    KACL m_defaultACL;
    unsigned short m_mask;
    bool m_hasMask;
    bool m_allowDefaults;
    QStringList m_allUsers;
    QStringList m_allGroups;
    QPixmap* m_yesPixmap;
    QPixmap* m_yesPartialPixmap;
};

class EditACLEntryDialog : public KDialogBase
{
    Q_OBJECT
public:
    EditACLEntryDialog( KACLListView *listView, KACLListViewItem *item,
                        const QStringList &users,
                        const QStringList &groups,
                        const QStringList &defaultUsers,
                        const QStringList &defaultGroups,
                        int allowedTypes = KACLListView::AllTypes,
                        int allowedDefaultTypes = KACLListView::AllTypes,
                        bool allowDefault = false );
    KACLListViewItem* item() const { return m_item; }
public slots:
     void slotOk();
     void slotSelectionChanged( int id );
private slots:
     void slotUpdateAllowedUsersAndGroups();
     void slotUpdateAllowedTypes();
private:
     KACLListView *m_listView;
     KACLListViewItem *m_item;
     QStringList m_users;
     QStringList m_groups;
     QStringList m_defaultUsers;
     QStringList m_defaultGroups;
     int m_allowedTypes;
     int m_allowedDefaultTypes;
     QVButtonGroup *m_buttonGroup;
     QComboBox *m_usersCombo;
     QComboBox *m_groupsCombo;
     QWidgetStack *m_widgetStack;
     QCheckBox *m_defaultCB;
};


class KACLListViewItem : public KListViewItem
{
public:
    KACLListViewItem( QListView* parent, KACLListView::EntryType type,
                      unsigned short value,
                      bool defaultEntry,
                      const QString& qualifier = QString::null );
    virtual ~KACLListViewItem();
    virtual QString key( int column, bool ascending ) const;

    void calcEffectiveRights();

    bool isDeletable() const;
    bool isAllowedToChangeType() const;

    void togglePerm( acl_perm_t perm );

    virtual void paintCell( QPainter *p, const QColorGroup &cg,
                            int column, int width, int alignment );

    void updatePermPixmaps();
    void repaint();

    KACLListView::EntryType type;
    unsigned short value;
    bool isDefault;
    QString qualifier;
    bool isPartial;

private:
    KACLListView* m_pACLListView;
};


#endif
#endif
