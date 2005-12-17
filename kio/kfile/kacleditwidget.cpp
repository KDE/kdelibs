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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include "kacleditwidget.h"
#include "kacleditwidget_p.h"

#ifdef USE_POSIX_ACL

#include <qpainter.h>
#include <qptrlist.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qwidgetstack.h>
#include <qheader.h>

#include <klocale.h>
#include <kfileitem.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kdialogbase.h>

#ifdef HAVE_ACL_LIBACL_H
# include <acl/libacl.h>
#endif
extern "C" {
#include <pwd.h>
#include <grp.h>
}
#include <assert.h>

#include "images.h"

static struct {
    const char* label;
    const char* pixmapName;
    QPixmap* pixmap;
} s_itemAttributes[] = {
    { I18N_NOOP( "Owner" ), "user-grey", 0 },
    { I18N_NOOP( "Owning Group" ), "group-grey", 0 },
    { I18N_NOOP( "Others" ), "others-grey", 0 },
    { I18N_NOOP( "Mask" ), "mask", 0 },
    { I18N_NOOP( "Named User" ), "user", 0 },
    { I18N_NOOP( "Named Group" ), "group", 0 },
};

KACLEditWidget::KACLEditWidget( QWidget *parent, const char *name )
   :QWidget( parent, name )
{
    QHBox *hbox = new QHBox( parent );
    hbox->setSpacing(  KDialog::spacingHint() );
    m_listView = new KACLListView( hbox, "acl_listview" );
    connect( m_listView, SIGNAL( selectionChanged() ),
            this, SLOT( slotUpdateButtons() ) );
    QVBox *vbox = new QVBox( hbox );
    vbox->setSpacing(  KDialog::spacingHint() );
    m_AddBtn = new QPushButton( i18n( "Add Entry..." ), vbox, "add_entry_button" );
    connect( m_AddBtn, SIGNAL( clicked() ), m_listView, SLOT( slotAddEntry() ) );
    m_EditBtn = new QPushButton( i18n( "Edit Entry..." ), vbox, "edit_entry_button" );
    connect( m_EditBtn, SIGNAL( clicked() ), m_listView, SLOT( slotEditEntry() ) );
    m_DelBtn = new QPushButton( i18n( "Delete Entry" ), vbox, "delete_entry_button" );
    connect( m_DelBtn, SIGNAL( clicked() ), m_listView, SLOT( slotRemoveEntry() ) );
    QWidget *spacer = new QWidget( vbox );
    spacer->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding );
    slotUpdateButtons();
}

void KACLEditWidget::slotUpdateButtons()
{
    int selectedItemsCount = 0;
    QListViewItemIterator it( m_listView, QListViewItemIterator::Selected );
    while ( it.current() ) {
        ++it;
        if ( ++selectedItemsCount > 1 )
            break;
    }
    m_EditBtn->setEnabled( selectedItemsCount == 1 );
    m_DelBtn->setEnabled( selectedItemsCount > 0 );
}

KACL KACLEditWidget::getACL() const
{
  return m_listView->getACL();
}

KACL KACLEditWidget::getDefaultACL() const
{
  return m_listView->getDefaultACL();
}

void KACLEditWidget::setACL( const KACL &acl )
{
  return m_listView->setACL( acl );
}

void KACLEditWidget::setDefaultACL( const KACL &acl )
{
  return m_listView->setDefaultACL( acl );
}

void KACLEditWidget::setAllowDefaults( bool value )
{
    m_listView->setAllowDefaults( value );
}

KACLListViewItem::KACLListViewItem( QListView* parent,
                                    KACLListView::EntryType _type,
                                    unsigned short _value, bool defaults,
                                    const QString& _qualifier )
 : KListViewItem( parent, parent->lastItem() ), // we want to append
   type( _type ), value( _value ), isDefault( defaults ),
   qualifier( _qualifier ), isPartial( false )
{
    m_pACLListView = dynamic_cast<KACLListView*>( parent );
    repaint();
}


KACLListViewItem::~ KACLListViewItem()
{

}

QString KACLListViewItem::key( int, bool ) const
{
    QString key;
    if ( !isDefault )
        key = "A";
    else
        key = "B";
    switch ( type )
    {
        case KACLListView::User:
            key += "A";
            break;
        case KACLListView::Group:
            key += "B";
            break;
        case KACLListView::Others:
            key += "C";
            break;
        case KACLListView::Mask:
            key += "D";
            break;
        case KACLListView::NamedUser:
            key += "E" + text( 1 );
            break;
        case KACLListView::NamedGroup:
            key += "F" + text( 1 );
            break;
        default:
            key += text( 0 );
            break;
    }
    return key;
}

void KACLListViewItem::paintCell( QPainter* p, const QColorGroup &cg,
                                  int column, int width, int alignment )
{
    QColorGroup mycg = cg;
    if ( isDefault ) {
        mycg.setColor( QColorGroup::Text, QColor( 0, 0, 255 ) );
    }
    if ( isPartial ) {
        QFont font = p->font();
        font.setItalic( true );
        mycg.setColor( QColorGroup::Text, QColor( 100, 100, 100 ) );
        p->setFont( font );
    }
    KListViewItem::paintCell( p, mycg, column, width, alignment );

    KACLListViewItem *below =0;
    if ( itemBelow() )
        below = static_cast<KACLListViewItem*>( itemBelow() );
    const bool lastUser = type == KACLListView::NamedUser && below && below->type == KACLListView::NamedGroup;
    const bool lastNonDefault = !isDefault && below && below->isDefault;
    if ( type == KACLListView::Mask || lastUser || lastNonDefault )
    {
        p->setPen( QPen( Qt::gray, 0, QPen::DotLine ) );
        if ( type == KACLListView::Mask )
            p->drawLine( 0, 0, width - 1, 0 );
        p->drawLine( 0, height() - 1, width - 1, height() - 1 );
    }
}


void KACLListViewItem::updatePermPixmaps()
{
    unsigned int partialPerms = value;

    if ( value & ACL_READ )
        setPixmap( 2, m_pACLListView->getYesPixmap() );
    else if ( partialPerms & ACL_READ )
        setPixmap( 2, m_pACLListView->getYesPartialPixmap() );
    else
        setPixmap( 2, QPixmap() );

    if ( value & ACL_WRITE )
        setPixmap( 3, m_pACLListView->getYesPixmap() );
    else if ( partialPerms & ACL_WRITE )
        setPixmap( 3, m_pACLListView->getYesPartialPixmap() );
    else
        setPixmap( 3, QPixmap() );

    if ( value & ACL_EXECUTE )
        setPixmap( 4, m_pACLListView->getYesPixmap() );
    else if ( partialPerms & ACL_EXECUTE )
        setPixmap( 4, m_pACLListView->getYesPartialPixmap() );
    else
        setPixmap( 4, QPixmap() );
}

void KACLListViewItem::repaint()
{
    int idx = 0;
    switch ( type )
    {
      case KACLListView::User:
          idx = KACLListView::OWNER_IDX;
            break;
        case KACLListView::Group:
          idx = KACLListView::GROUP_IDX;
            break;
        case KACLListView::Others:
          idx = KACLListView::OTHERS_IDX;
            break;
        case KACLListView::Mask:
          idx = KACLListView::MASK_IDX;
            break;
        case KACLListView::NamedUser:
          idx = KACLListView::NAMED_USER_IDX;
            break;
        case KACLListView::NamedGroup:
          idx = KACLListView::NAMED_GROUP_IDX;
            break;
        default:
          idx = KACLListView::OWNER_IDX;
            break;
    }
    setText( 0, s_itemAttributes[idx].label );
    setPixmap( 0, *s_itemAttributes[idx].pixmap );
    if ( isDefault )
        setText( 0, text( 0 ) + i18n( " (Default)" ) );
    setText( 1, qualifier );
    // Set the pixmaps for which of the perms are set
    updatePermPixmaps();
}

void KACLListViewItem::calcEffectiveRights()
{
    QString strEffective = QString( "---" );

    // Do we need to worry about the mask entry? It applies to named users,
    // owning group, and named groups
    if ( m_pACLListView->hasMaskEntry()
            && ( type == KACLListView::NamedUser
              || type == KACLListView::Group
              || type == KACLListView::NamedGroup ) 
            && !isDefault )
    {

        strEffective[0] = ( m_pACLListView->maskPermissions() & value & ACL_READ ) ? 'r' : '-';
        strEffective[1] = ( m_pACLListView->maskPermissions() & value & ACL_WRITE ) ? 'w' : '-';
        strEffective[2] = ( m_pACLListView->maskPermissions() & value & ACL_EXECUTE ) ? 'x' : '-';
/*
        // What about any partial perms?
        if ( maskPerms & partialPerms & ACL_READ || // Partial perms on entry
             maskPartialPerms & perms & ACL_READ || // Partial perms on mask
             maskPartialPerms & partialPerms & ACL_READ ) // Partial perms on mask and entry
            strEffective[0] = 'R';
        if ( maskPerms & partialPerms & ACL_WRITE || // Partial perms on entry
             maskPartialPerms & perms & ACL_WRITE || // Partial perms on mask
             maskPartialPerms & partialPerms & ACL_WRITE ) // Partial perms on mask and entry
            strEffective[1] = 'W';
        if ( maskPerms & partialPerms & ACL_EXECUTE || // Partial perms on entry
             maskPartialPerms & perms & ACL_EXECUTE || // Partial perms on mask
             maskPartialPerms & partialPerms & ACL_EXECUTE ) // Partial perms on mask and entry
            strEffective[2] = 'X';
*/
    }
    else
    {
        // No, the effective value are just the value in this entry
        strEffective[0] = ( value & ACL_READ ) ? 'r' : '-';
        strEffective[1] = ( value & ACL_WRITE ) ? 'w' : '-';
        strEffective[2] = ( value & ACL_EXECUTE ) ? 'x' : '-';

        /*
        // What about any partial perms?
        if ( partialPerms & ACL_READ )
            strEffective[0] = 'R';
        if ( partialPerms & ACL_WRITE )
            strEffective[1] = 'W';
        if ( partialPerms & ACL_EXECUTE )
            strEffective[2] = 'X';
            */
    }
    setText( 5, strEffective );
}


void KACLListViewItem::togglePerm( acl_perm_t perm )
{
    value ^= perm; // Toggle the perm
    if ( type == KACLListView::Mask && !isDefault ) {
        m_pACLListView->setMaskPermissions( value );
    }
    calcEffectiveRights();
    updatePermPixmaps();
/*
    // If the perm is in the partial perms then remove it. i.e. Once
    // a user changes a partial perm it then applies to all selected files.
    if ( m_pEntry->m_partialPerms & perm )
        m_pEntry->m_partialPerms ^= perm;

    m_pEntry->setPartialEntry( false );
    // Make sure that all entries have their effective rights calculated if
    // we are changing the ACL_MASK entry.
    if ( type == Mask )
    {
        m_pACLListView->setMaskPartialPermissions( m_pEntry->m_partialPerms );
        m_pACLListView->setMaskPermissions( value );
        m_pACLListView->calculateEffectiveRights();
    }
*/
}



EditACLEntryDialog::EditACLEntryDialog( KACLListView *listView, KACLListViewItem *item,
                                        const QStringList &users,
                                        const QStringList &groups,
                                        const QStringList &defaultUsers,
                                        const QStringList &defaultGroups,
                                        int allowedTypes, int allowedDefaultTypes,
                                        bool allowDefaults )
      : KDialogBase( listView, "edit_entry_dialog", true,
              i18n( "Edit ACL Entry" ), KDialogBase::Ok|KDialogBase::Cancel,
              KDialogBase::Ok, false ), 
        m_listView( listView ), m_item( item ), m_users( users ), m_groups( groups ),
        m_defaultUsers( defaultUsers ), m_defaultGroups( defaultGroups ),
        m_allowedTypes( allowedTypes ), m_allowedDefaultTypes( allowedDefaultTypes ),
        m_defaultCB( 0 )
{
    QWidget *page = new QWidget(  this );
    setMainWidget( page );
    QVBoxLayout *mainLayout = new QVBoxLayout( page, 0, spacingHint(), "mainLayout" );
    m_buttonGroup = new QVButtonGroup( i18n("Entry Type"), page, "bg" );

    if ( allowDefaults ) {
        m_defaultCB = new QCheckBox( i18n("Default for new files in this folder"), page, "defaultCB" );
        mainLayout->addWidget( m_defaultCB );
        connect( m_defaultCB, SIGNAL( toggled( bool ) ),
                 this, SLOT( slotUpdateAllowedUsersAndGroups() ) );
        connect( m_defaultCB, SIGNAL( toggled( bool ) ),
                 this, SLOT( slotUpdateAllowedTypes() ) );

    }

    mainLayout->addWidget( m_buttonGroup );

    QRadioButton *ownerType = new QRadioButton( i18n("Owner"), m_buttonGroup, "ownerType" );
    m_buttonGroup->insert( ownerType, KACLListView::User );
    QRadioButton *owningGroupType = new QRadioButton( i18n("Owning Group"), m_buttonGroup, "owningGroupType" );
    m_buttonGroup->insert( owningGroupType, KACLListView::Group );
    QRadioButton *othersType = new QRadioButton( i18n("Others"), m_buttonGroup, "othersType" );
    m_buttonGroup->insert( othersType, KACLListView::Others );
    QRadioButton *maskType = new QRadioButton( i18n("Mask"), m_buttonGroup, "maskType" );
    m_buttonGroup->insert( maskType, KACLListView::Mask );
    QRadioButton *namedUserType = new QRadioButton( i18n("Named User"), m_buttonGroup, "namesUserType" );
    m_buttonGroup->insert( namedUserType, KACLListView::NamedUser );
    QRadioButton *namedGroupType = new QRadioButton( i18n("Named Group"), m_buttonGroup, "namedGroupType" );
    m_buttonGroup->insert( namedGroupType, KACLListView::NamedGroup );

    connect( m_buttonGroup, SIGNAL( clicked( int ) ),
             this, SLOT( slotSelectionChanged( int ) ) );

    m_widgetStack = new QWidgetStack( page );
    mainLayout->addWidget( m_widgetStack );

    QHBox *usersBox = new QHBox( m_widgetStack );
    m_widgetStack->addWidget( usersBox, KACLListView::NamedUser );

    QHBox *groupsBox = new QHBox( m_widgetStack );
    m_widgetStack->addWidget( groupsBox, KACLListView::NamedGroup );

    QLabel *usersLabel = new QLabel( i18n( "User: " ), usersBox );
    m_usersCombo = new QComboBox( false, usersBox, "users" );
    usersLabel->setBuddy( m_usersCombo );

    QLabel *groupsLabel = new QLabel( i18n( "Group: " ), groupsBox );
    m_groupsCombo = new QComboBox( false, groupsBox, "groups" );
    groupsLabel->setBuddy( m_groupsCombo );

    if ( m_item ) {
        m_buttonGroup->setButton( m_item->type );
        if ( m_defaultCB )
            m_defaultCB->setChecked( m_item->isDefault );
        slotUpdateAllowedTypes();
        slotSelectionChanged( m_item->type );
        slotUpdateAllowedUsersAndGroups();
        if ( m_item->type == KACLListView::NamedUser ) {
            m_usersCombo->setCurrentText( m_item->qualifier );
        } else if ( m_item->type == KACLListView::NamedGroup ) {
            m_groupsCombo->setCurrentText( m_item->qualifier );
        }
    } else {
        // new entry, preselect "named user", arguably the most common one
        m_buttonGroup->setButton( KACLListView::NamedUser );
        slotUpdateAllowedTypes();
        slotSelectionChanged( KACLListView::NamedUser );
        slotUpdateAllowedUsersAndGroups();
    }
    incInitialSize(  QSize( 100, 0 ) );
}

void EditACLEntryDialog::slotUpdateAllowedTypes()
{
    int allowedTypes = m_allowedTypes;
    if ( m_defaultCB && m_defaultCB->isChecked() ) {
        allowedTypes = m_allowedDefaultTypes;
    }
    for ( int i=1; i < KACLListView::AllTypes; i=i*2 ) {
        if ( allowedTypes & i )
            m_buttonGroup->find( i )->show();
        else
            m_buttonGroup->find( i )->hide();
    }
}

void EditACLEntryDialog::slotUpdateAllowedUsersAndGroups()
{
    const QString oldUser = m_usersCombo->currentText();
    const QString oldGroup = m_groupsCombo->currentText();
    m_usersCombo->clear();
    m_groupsCombo->clear();
    if ( m_defaultCB && m_defaultCB->isChecked() ) {
        m_usersCombo->insertStringList( m_defaultUsers );
        if ( m_defaultUsers.find( oldUser ) != m_defaultUsers.end() )
            m_usersCombo->setCurrentText( oldUser );
        m_groupsCombo->insertStringList( m_defaultGroups );
        if ( m_defaultGroups.find( oldGroup ) != m_defaultGroups.end() )
            m_groupsCombo->setCurrentText( oldGroup );
    } else {
        m_usersCombo->insertStringList( m_users );
        if ( m_users.find( oldUser ) != m_users.end() )
            m_usersCombo->setCurrentText( oldUser );
        m_groupsCombo->insertStringList( m_groups );
        if ( m_groups.find( oldGroup ) != m_groups.end() )
            m_groupsCombo->setCurrentText( oldGroup );
    }
}
void EditACLEntryDialog::slotOk()
{
    KACLListView::EntryType type = static_cast<KACLListView::EntryType>( m_buttonGroup->selectedId() );

    QString qualifier;
    if ( type == KACLListView::NamedUser )
      qualifier = m_usersCombo->currentText();
    if ( type == KACLListView::NamedGroup )
      qualifier = m_groupsCombo->currentText();

    if ( !m_item ) {
        m_item = new KACLListViewItem( m_listView, type, ACL_READ | ACL_WRITE | ACL_EXECUTE, false, qualifier );
    } else {
        m_item->type = type;
        m_item->qualifier = qualifier;
    }
    if ( m_defaultCB )
        m_item->isDefault = m_defaultCB->isChecked();
    m_item->repaint();

    KDialogBase::slotOk();
}

void EditACLEntryDialog::slotSelectionChanged( int id )
{
    switch ( id ) {
        case KACLListView::User:
        case KACLListView::Group:
        case KACLListView::Others:
        case KACLListView::Mask:
            m_widgetStack->setEnabled( false );
            break;
        case KACLListView::NamedUser:
            m_widgetStack->setEnabled( true );
            m_widgetStack->raiseWidget( KACLListView::NamedUser );
            break;
        case KACLListView::NamedGroup:
            m_widgetStack->setEnabled( true );
            m_widgetStack->raiseWidget( KACLListView::NamedGroup );
            break;
        default:
            break;
    }
}


KACLListView::KACLListView( QWidget* parent, const char* name )
 : KListView( parent, name ),
   m_hasMask( false ), m_allowDefaults( false )
{
    // Add the columns
    addColumn( i18n( "Type" ) );
    addColumn( i18n( "Name" ) );
    addColumn( i18n( "read permission", "r" ) );
    addColumn( i18n( "write permission", "w" ) );
    addColumn( i18n( "execute permission", "x" ) );
    addColumn( i18n( "Effective" ) );

    header()->setClickEnabled( false );

    // Load the avatars
    for ( int i=0; i < LAST_IDX; ++i ) {
        s_itemAttributes[i].pixmap = new QPixmap( qembed_findImage( s_itemAttributes[i].pixmapName ) );
    }
    m_yesPixmap = new QPixmap( qembed_findImage( "yes" ) );
    m_yesPartialPixmap = new QPixmap( qembed_findImage( "yespartial" ) );

    setSelectionMode( QListView::Extended );

    // fill the lists of all legal users and groups
    struct passwd *user = 0;
    setpwent();
    while ( ( user = getpwent() ) != 0 ) {
       m_allUsers << QString::fromLatin1( user->pw_name );
    }
    endpwent();

    struct group *gr = 0;
    setgrent();
    while ( ( gr = getgrent() ) != 0 ) {
       m_allGroups << QString::fromLatin1( gr->gr_name );
    }
    endgrent();
    m_allUsers.sort();
    m_allGroups.sort();
}


KACLListView::~KACLListView()
{
    for ( int i=0; i < LAST_IDX; ++i ) {
       delete s_itemAttributes[i].pixmap;
    }
    delete m_yesPixmap;
    delete m_yesPartialPixmap;
}

QStringList KACLListView::allowedUsers( bool defaults, KACLListViewItem *allowedItem )
{
    QStringList allowedUsers = m_allUsers;
    QListViewItemIterator it( this );
    while ( it.current() ) {
        const KACLListViewItem *item = static_cast<const KACLListViewItem*>( *it );
        ++it;
        if ( !item->type == NamedUser || item->isDefault != defaults ) continue;
        if ( allowedItem && item == allowedItem && allowedItem->isDefault == defaults ) continue;
        allowedUsers.remove( item->qualifier );
    }
    return allowedUsers;
}

QStringList KACLListView::allowedGroups( bool defaults, KACLListViewItem *allowedItem )
{
    QStringList allowedGroups = m_allGroups;
    QListViewItemIterator it( this );
    while ( it.current() ) {
        const KACLListViewItem *item = static_cast<const KACLListViewItem*>( *it );
        ++it;
        if ( !item->type == NamedGroup || item->isDefault != defaults ) continue;
        if ( allowedItem && item == allowedItem && allowedItem->isDefault == defaults ) continue;
        allowedGroups.remove( item->qualifier );
    }
    return allowedGroups;
}

void KACLListView::fillItemsFromACL( const KACL &pACL, bool defaults )
{
    // clear out old entries of that ilk
    QListViewItemIterator it( this );
    while ( KACLListViewItem *item = static_cast<KACLListViewItem*>( it.current() ) ) {
        ++it;
        if ( item->isDefault == defaults )
            delete item;
    }
    KACLListViewItem *item =
        new KACLListViewItem( this, User, pACL.ownerPermissions(), defaults );

    item = new KACLListViewItem( this, Group, pACL.owningGroupPermissions(), defaults );

    item = new KACLListViewItem( this, Others, pACL.othersPermissions(), defaults );

    bool hasMask = false;
    unsigned short mask = pACL.maskPermissions( hasMask );
    if ( hasMask ) {
        item = new KACLListViewItem( this, Mask, mask, defaults );
    }

    // read all named user entries
    const ACLUserPermissionsList &userList =  pACL.allUserPermissions();
    ACLUserPermissionsConstIterator itu = userList.begin();
    while ( itu != userList.end() ) {
        new KACLListViewItem( this, NamedUser, (*itu).second, defaults, (*itu).first );
        ++itu;
    }

    // and now all named groups
    const ACLUserPermissionsList &groupList =  pACL.allGroupPermissions();
    ACLUserPermissionsConstIterator itg = groupList.begin();
    while ( itg != groupList.end() ) {
        new KACLListViewItem( this, NamedGroup, (*itg).second, defaults, (*itg).first );
        ++itg;
    }
}

void KACLListView::setACL( const KACL &acl )
{
    if ( !acl.isValid() ) return;
    // Remove any entries left over from displaying a previous ACL
    m_ACL = acl;
    fillItemsFromACL( m_ACL );

    m_mask = acl.maskPermissions( m_hasMask );
    calculateEffectiveRights();
}

void KACLListView::setDefaultACL( const KACL &acl )
{
    if ( !acl.isValid() ) return;
    m_defaultACL = acl;
    fillItemsFromACL( m_defaultACL, true );
    calculateEffectiveRights();
}

KACL KACLListView::itemsToACL( bool defaults ) const
{
    KACL newACL( 0 );
    bool atLeastOneEntry = false;
    ACLUserPermissionsList users;
    ACLGroupPermissionsList groups;
    QListViewItemIterator it( const_cast<KACLListView*>( this ) );
    while ( QListViewItem* qlvi = it.current() ) {
        ++it;
        const KACLListViewItem* item = static_cast<KACLListViewItem*>( qlvi );
        if ( item->isDefault != defaults ) continue;
        atLeastOneEntry = true;
        switch ( item->type ) {
            case User:
                newACL.setOwnerPermissions( item->value );
                break;
            case Group:
                newACL.setOwningGroupPermissions( item->value );
                break;
            case Others:
                newACL.setOthersPermissions( item->value );
                break;
            case Mask:
                newACL.setMaskPermissions( item->value );
                break;
            case NamedUser:
                users.append( qMakePair( item->text( 1 ), item->value ) );
                break;
            case NamedGroup:
                groups.append( qMakePair( item->text( 1 ), item->value ) );
                break;
            default:
                break;
        }
    }
    if ( atLeastOneEntry ) {
        newACL.setAllUserPermissions( users );
        newACL.setAllGroupPermissions( groups );
        if ( newACL.isValid() )
            return newACL;
    }
    return KACL();
}

KACL KACLListView::getACL()
{
    return itemsToACL( false );
}


KACL KACLListView::getDefaultACL()
{
    return itemsToACL( true );
}

void KACLListView::contentsMousePressEvent( QMouseEvent * e )
{
    QListViewItem *clickedItem = itemAt( contentsToViewport(  e->pos() ) );
    if ( !clickedItem ) return;
    // if the click is on an as yet unselected item, select it first
    if ( !clickedItem->isSelected() )
        KListView::contentsMousePressEvent( e );

    if ( !currentItem() ) return;
    int column = header()->sectionAt( e->x() );
    acl_perm_t perm;
    switch ( column )
    {
        case 2:
            perm = ACL_READ;
            break;
        case 3:
            perm = ACL_WRITE;
            break;
        case 4:
            perm = ACL_EXECUTE;
            break;
        default:
            return KListView::contentsMousePressEvent( e );
    }
    KACLListViewItem* referenceItem = static_cast<KACLListViewItem*>( clickedItem );
    unsigned short referenceHadItSet = referenceItem->value & perm;
    QListViewItemIterator it( this );
    while ( KACLListViewItem* item = static_cast<KACLListViewItem*>( it.current() ) ) {
        ++it;
        if ( !item->isSelected() ) continue;
        // toggle those with the same value as the clicked item, leave the others
        if ( referenceHadItSet == ( item->value & perm ) )
            item->togglePerm( perm );
    }
}

void KACLListView::entryClicked( QListViewItem* pItem, const QPoint& /*pt*/, int col )
{
    if ( !pItem ) return;

    QListViewItemIterator it( this );
    while ( KACLListViewItem* item = static_cast<KACLListViewItem*>( it.current() ) ) {
        ++it;
        if ( !item->isSelected() ) continue;
        switch ( col )
        {
            case 2:
                item->togglePerm( ACL_READ );
                break;
            case 3:
                item->togglePerm( ACL_WRITE );
                break;
            case 4:
                item->togglePerm( ACL_EXECUTE );
                break;

            default:
                ; // Do nothing
        }
    }
    /*
    // Has the user changed one of the required entries in a default ACL?
    if ( m_pACL->aclType() == ACL_TYPE_DEFAULT &&
    ( col == 2 || col == 3 || col == 4 ) &&
    ( pACLItem->entryType() == ACL_USER_OBJ ||
    pACLItem->entryType() == ACL_GROUP_OBJ ||
    pACLItem->entryType() == ACL_OTHER ) )
    {
    // Mark the required entries as no longer being partial entries.
    // That is, they will get applied to all selected directories.
    KACLListViewItem* pUserObj = findACLEntryByType( this, ACL_USER_OBJ );
    pUserObj->entry()->setPartialEntry( false );

    KACLListViewItem* pGroupObj = findACLEntryByType( this, ACL_GROUP_OBJ );
    pGroupObj->entry()->setPartialEntry( false );

    KACLListViewItem* pOther = findACLEntryByType( this, ACL_OTHER );
    pOther->entry()->setPartialEntry( false );

    update();
    }
     */
}


void KACLListView::calculateEffectiveRights()
{
    QListViewItemIterator it( this );
    KACLListViewItem* pItem;
    while ( ( pItem = dynamic_cast<KACLListViewItem*>( it.current() ) ) != 0 )
    {
        ++it;
        pItem->calcEffectiveRights();
    }
}


unsigned short KACLListView::maskPermissions() const
{
  return m_mask;
}


void KACLListView::setMaskPermissions( unsigned short maskPerms )
{
    m_mask = maskPerms;
    calculateEffectiveRights();
}


acl_perm_t KACLListView::maskPartialPermissions() const
{
  //  return m_pMaskEntry->m_partialPerms;
  return 0;
}


void KACLListView::setMaskPartialPermissions( acl_perm_t /*maskPartialPerms*/ )
{
    //m_pMaskEntry->m_partialPerms = maskPartialPerms;
    calculateEffectiveRights();
}

bool KACLListView::hasDefaultEntries() const
{
    QListViewItemIterator it( const_cast<KACLListView*>( this ) );
    while ( it.current() ) {
        const KACLListViewItem *item = static_cast<const KACLListViewItem*>( it.current() );
        ++it;
        if ( item->isDefault ) return true;
    }
    return false;
}

const KACLListViewItem* KACLListView::findDefaultItemByType( EntryType type ) const
{
    return findItemByType( type, true );
}

const KACLListViewItem* KACLListView::findItemByType( EntryType type, bool defaults ) const
{
    QListViewItemIterator it( const_cast<KACLListView*>( this ) );
    while ( it.current() ) {
        const KACLListViewItem *item = static_cast<const KACLListViewItem*>( it.current() );
        ++it;
        if ( item->isDefault == defaults && item->type == type ) {
            return item;
        }
    }
    return 0;
}


unsigned short KACLListView::calculateMaskValue( bool defaults ) const
{
    // KACL auto-adds the relevant maks entries, so we can simply query
    bool dummy;
    return itemsToACL( defaults ).maskPermissions( dummy );
}

void KACLListView::slotAddEntry()
{
    int allowedTypes = NamedUser | NamedGroup;
    if ( !m_hasMask )
        allowedTypes |= Mask;
    int allowedDefaultTypes = NamedUser | NamedGroup;
    if ( !findDefaultItemByType( Mask ) )
        allowedDefaultTypes |=  Mask;
    if ( !hasDefaultEntries() )
        allowedDefaultTypes |= User | Group;
    EditACLEntryDialog dlg( this, 0,
                            allowedUsers( false ), allowedGroups( false ),
                            allowedUsers( true ), allowedGroups( true ),
                            allowedTypes, allowedDefaultTypes, m_allowDefaults );
    dlg.exec();
    KACLListViewItem *item = dlg.item();
    if ( !item ) return; // canceled
    if ( item->type == Mask && !item->isDefault ) {
        m_hasMask = true;
        m_mask = item->value;
    }
    if ( item->isDefault && !hasDefaultEntries() ) {
        // first default entry, fill in what is needed
        if ( item->type != User ) {
            unsigned short v = findDefaultItemByType( User )->value;
            new KACLListViewItem( this, User, v, true );
        }
        if ( item->type != Group ) {
            unsigned short v = findDefaultItemByType( Group )->value;
            new KACLListViewItem( this, Group, v, true );
        }
        if ( item->type != Others ) {
            unsigned short v = findDefaultItemByType( Others )->value;
            new KACLListViewItem( this, Others, v, true );
        }
    }
    const KACLListViewItem *defaultMaskItem = findDefaultItemByType( Mask );
    if ( item->isDefault && !defaultMaskItem ) {
        unsigned short v = calculateMaskValue( true );
        new KACLListViewItem( this, Mask, v, true );
    }
    if ( !item->isDefault && !m_hasMask &&
            ( item->type == Group
              || item->type == NamedUser
              || item->type == NamedGroup ) ) {
        // auto-add a mask entry
        unsigned short v = calculateMaskValue( false );
        new KACLListViewItem( this, Mask, v, false );
        m_hasMask = true;
        m_mask = v;
    }
    calculateEffectiveRights();
    sort();
    setCurrentItem( item );
    // QListView doesn't seem to emit, in this case, and we need to update 
    // the buttons...
    if ( childCount() == 1 ) 
        emit currentChanged( item );
}

void KACLListView::slotEditEntry()
{
    QListViewItem * current = currentItem();
    if ( !current ) return;
    KACLListViewItem *item = static_cast<KACLListViewItem*>( current );
    int allowedTypes = item->type | NamedUser | NamedGroup;
    bool itemWasMask = item->type == Mask;
    if ( !m_hasMask || itemWasMask )
        allowedTypes |= Mask;
    int allowedDefaultTypes = item->type | NamedUser | NamedGroup;
    if ( !findDefaultItemByType( Mask ) )
        allowedDefaultTypes |=  Mask;
    if ( !hasDefaultEntries() )
        allowedDefaultTypes |= User | Group;

    EditACLEntryDialog dlg( this, item,
                            allowedUsers( false, item ), allowedGroups( false, item ),
                            allowedUsers( true, item ), allowedGroups( true, item ),
                            allowedTypes, allowedDefaultTypes, m_allowDefaults );
    dlg.exec();
    if ( itemWasMask && item->type != Mask ) {
        m_hasMask = false;
        m_mask = 0;
    }
    if ( !itemWasMask && item->type == Mask ) {
        m_mask = item->value;
        m_hasMask = true;
    }
    calculateEffectiveRights();
    sort();
}

void KACLListView::slotRemoveEntry()
{
    bool needsMask = findItemByType( NamedUser ) || findItemByType( NamedGroup );
    bool needsDefaultMask = findDefaultItemByType( NamedUser ) || findDefaultItemByType( NamedGroup );
    QListViewItemIterator it( this, QListViewItemIterator::Selected );
    while ( it.current() ) {
        KACLListViewItem *item = static_cast<KACLListViewItem*>( it.current() );
        ++it;
        /* First check if it's a mask entry and if so, make sure that there is
         * either no name user or group entry, which means the mask can be 
         * removed, or don't remove it, but reset it. That is allowed. */
        if ( item->type == Mask ) {
            bool itemWasDefault = item->isDefault;
            if ( !itemWasDefault && !needsMask ) {
                m_hasMask= false;
                m_mask = 0;
                delete item;
            } else if ( itemWasDefault && !needsDefaultMask ) {
                delete item;
            } else {
                item->value = 0;
                item->repaint();
            }
            if ( !itemWasDefault )
                calculateEffectiveRights();
        } else {
            // for the base permissions, disable them, which is what libacl does
            if ( !item->isDefault &&
                    ( item->type == User
                      || item->type == Group
                      || item->type == Others ) ) {
                item->value = 0;
                item->repaint();
            } else {
                delete item;
            }
        }
    }
}

#include "kacleditwidget.moc"
#include "kacleditwidget_p.moc"
#endif
// vim:set ts=8 sw=4:
