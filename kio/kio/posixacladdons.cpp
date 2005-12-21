/***************************************************************************
 *   Copyright (C) 2005 by Markus Brueffer <markus@brueffer.de>            *
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

#include "posixacladdons.h"

#if defined(USE_POSIX_ACL) && !defined(HAVE_NON_POSIX_ACL_EXTENSIONS)

#include <errno.h>
#include <sys/stat.h>

#include <qptrlist.h>

class SortedEntryList : public QPtrList<acl_entry_t>
{
protected:
    int compareItems( QPtrCollection::Item i1,
                      QPtrCollection::Item i2 )
    {
        acl_entry_t *e1 = static_cast<acl_entry_t*>( i1 );
        acl_entry_t *e2 = static_cast<acl_entry_t*>( i2 );

        acl_tag_t tag1, tag2;
        uid_t uid1 = 0, uid2 = 0;

        acl_get_tag_type( *e1, &tag1 );
        acl_get_tag_type( *e2, &tag2 );

        if ( tag1 == ACL_USER || tag1 == ACL_GROUP )
                uid1 = *( (uid_t*) acl_get_qualifier( *e1 ) );

        if ( tag2 == ACL_USER || tag2 == ACL_GROUP )
                uid2 = *( (uid_t*) acl_get_qualifier( *e2 ) );

        if ( tag1 < tag2 )
            return -1;
        else if ( tag1 > tag2 )
            return 1;

        if ( uid1 < uid2 )
            return -1;
        else if ( uid1 > uid2 )
            return 1;

        return 0;
    }
};

int acl_cmp(acl_t acl1, acl_t acl2)
{
    if ( !acl1 || !acl2 )
        return -1;

    SortedEntryList entries1, entries2;
    entries1.setAutoDelete( true );
    entries2.setAutoDelete( true );

    /* Add ACL entries to vectors */
    acl_entry_t *entry = new acl_entry_t;
    int ret = acl_get_entry( acl1, ACL_FIRST_ENTRY, entry );
    while( ret == 1 ) {
        entries1.append( entry );
        entry = new acl_entry_t;
        ret = acl_get_entry( acl1, ACL_NEXT_ENTRY, entry );
    }
    delete entry;

    entry = new acl_entry_t;
    ret = acl_get_entry( acl2, ACL_FIRST_ENTRY, entry );
    while ( ret == 1 ) {
        entries2.append( entry );
        entry = new acl_entry_t;
        ret = acl_get_entry( acl2, ACL_NEXT_ENTRY, entry );
    }
    delete entry;

    /* If the entry count differs, we are done */
    if ( entries1.count() != entries2.count() )
        return 1;

    /* Sort vectors */
    entries1.sort();
    entries2.sort();

    /* Compare all entries */
    acl_permset_t permset1, permset2;
    acl_tag_t tag1, tag2;
    uid_t uid1, uid2;
    acl_entry_t *e1, *e2;

    for ( e1 = entries1.first(), e2 = entries2.first(); e1; e1 = entries1.next(), e2 = entries2.next() ) {
        /* Compare tag */
        if ( acl_get_tag_type( *e1, &tag1 ) != 0 ) return 1;
        if ( acl_get_tag_type( *e2, &tag2 ) != 0 ) return 1;
        if ( tag1 != tag2 ) return 1;

        /* Compare permissions */
        if ( acl_get_permset( *e1, &permset1 ) != 0 ) return 1;
        if ( acl_get_permset( *e2, &permset2 ) != 0 ) return 1;
        if ( *permset1 != *permset2) return 1;

        /* Compare uid */
        switch( tag1 ) {
            case ACL_USER:
            case ACL_GROUP:
                uid1 = *( (uid_t*) acl_get_qualifier( *e1 ) );
                uid2 = *( (uid_t*) acl_get_qualifier( *e2 ) );
                if ( uid1 != uid2 ) return 1;
        }
    }

    return 0;
}

acl_t acl_from_mode(mode_t mode)
{
    acl_t newACL = acl_init( 3 );
    acl_entry_t entry;
    acl_permset_t permset;
    int error = 0;

    /* Add owner entry */
    if ( ( error = acl_create_entry( &newACL, &entry ) ) == 0 ) {
        /* Set owner permissions */
        acl_set_tag_type( entry, ACL_USER_OBJ );
        acl_get_permset( entry, &permset );
        acl_clear_perms( permset );
        if ( mode & S_IRUSR ) acl_add_perm( permset, ACL_READ );
        if ( mode & S_IWUSR ) acl_add_perm( permset, ACL_WRITE );
        if ( mode & S_IXUSR ) acl_add_perm( permset, ACL_EXECUTE );
        acl_set_permset( entry, permset );

        /* Add group entry */
        if ( ( error = acl_create_entry( &newACL, &entry ) ) == 0 ) {
            /* Set group permissions */
            acl_set_tag_type( entry, ACL_GROUP_OBJ );
            acl_get_permset( entry, &permset );
            acl_clear_perms( permset );
            if ( mode & S_IRGRP ) acl_add_perm( permset, ACL_READ );
            if ( mode & S_IWGRP ) acl_add_perm( permset, ACL_WRITE );
            if ( mode & S_IXGRP ) acl_add_perm( permset, ACL_EXECUTE );
            acl_set_permset( entry, permset );

            /* Add other entry */
            if ( ( error = acl_create_entry( &newACL, &entry ) ) == 0) {
                /* Set other permissions */
                acl_set_tag_type( entry, ACL_OTHER );
                acl_get_permset( entry, &permset );
                acl_clear_perms( permset );
                if ( mode & S_IROTH ) acl_add_perm( permset, ACL_READ );
                if ( mode & S_IWOTH ) acl_add_perm( permset, ACL_WRITE );
                if ( mode & S_IXOTH ) acl_add_perm( permset, ACL_EXECUTE );
                acl_set_permset( entry, permset );
            }
        }
    }

    if ( error ) {
        acl_free ( &newACL );
        return NULL;
    }

    return newACL;
}

int acl_equiv_mode(acl_t acl, mode_t *mode_p)
{
    acl_entry_t entry;
    acl_tag_t tag;
    acl_permset_t permset;
    mode_t mode = 0;
    int notEquiv = 0;

    if ( !acl )
        return -1;

    int ret = acl_get_entry( acl, ACL_FIRST_ENTRY, &entry );
    while ( ret == 1 ) {
        acl_get_tag_type( entry, &tag );
        acl_get_permset( entry, &permset );

        switch( tag ) {
            case ACL_USER_OBJ:
                if ( acl_get_perm( permset, ACL_READ ) ) mode |= S_IRUSR;
                if ( acl_get_perm( permset, ACL_WRITE ) ) mode |= S_IWUSR;
                if ( acl_get_perm( permset, ACL_EXECUTE ) ) mode |= S_IXUSR;
                break;

            case ACL_GROUP_OBJ:
                if ( acl_get_perm( permset, ACL_READ ) ) mode |= S_IRGRP;
                if ( acl_get_perm( permset, ACL_WRITE ) ) mode |= S_IWGRP;
                if ( acl_get_perm( permset, ACL_EXECUTE ) ) mode |= S_IXGRP;
                break;

            case ACL_OTHER:
                if ( acl_get_perm( permset, ACL_READ ) ) mode |= S_IROTH;
                if ( acl_get_perm( permset, ACL_WRITE ) ) mode |= S_IWOTH;
                if ( acl_get_perm( permset, ACL_EXECUTE ) ) mode |= S_IXOTH;
                break;

            case ACL_USER:
            case ACL_GROUP:
            case ACL_MASK:
                notEquiv = 1;
                break;

            default:
                errno = EINVAL;
                return -1;
        }

        ret = acl_get_entry( acl, ACL_NEXT_ENTRY, &entry );
    }

    if (mode_p)
        *mode_p = mode;

    return notEquiv;
}

#endif // USE_POSIX_ACL
