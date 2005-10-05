/* This file is part of the KDE project
   Copyright (C) 2005 Till Adam <adam@kde.org>

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

#include "kacltest.h"

#include <config.h>

#include <kacl.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kcmdlineargs.h>

#include <qfileinfo.h>
#include <qeventloop.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <qdir.h>

// The code comes partly from kdebase/kioslave/trash/testtrash.cpp

static bool check(const QString& txt, QString a, QString b)
{
    if (a.isEmpty())
        a = QString::null;
    if (b.isEmpty())
        b = QString::null;
    if (a == b) {
        kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
    }
    else {
        kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
        exit(1);
    }
    return true;
}

template<typename T> 
static bool check(const QString& txt, T a, T b)
{
    if (a == b) {
        kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
    }
    else {
        kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
        exit(1);
    }
    return true;
}

int main(int argc, char *argv[])
{
    KApplication::disableAutoDcopRegistration();
    KCmdLineArgs::init(argc,argv,"kacltest", 0, 0, 0, 0);
    KApplication app;

    KACLTest test;
    test.setup();
    test.runAll();
    test.cleanup();
    kdDebug() << "All tests OK." << endl;
    return 0; // success. The exit(1) in check() is what happens in case of failure.
}

#ifdef Q_OS_FREEBSD
static const QString s_group1 = QString::fromLatin1("staff");
static const QString s_group2 = QString::fromLatin1("guest");
#else
static const QString s_group1 = QString::fromLatin1("audio");
static const QString s_group2 = QString::fromLatin1("users");
#endif

static const QString s_testACL = QString::fromLatin1( "user::rw-\nuser:bin:rwx\ngroup::rw-\nmask::rwx\nother::r--\n" );
static const QString s_testACL2 = QString::fromLatin1( "user::rwx\nuser:bin:rwx\ngroup::rw-\n") +
                                  QString::fromLatin1( "group:" ) + s_group1 + QString::fromLatin1( ":--x\n" ) +
                                  QString::fromLatin1( "group:" ) + s_group2 + QString::fromLatin1( ":r--\n" ) +
                                  QString::fromLatin1( "mask::r-x\nother::r--\n" );
static const QString s_testACLEffective = QString::fromLatin1( "user::rwx\nuser:bin:rwx #effective:r-x\ngroup::rw- #effective:r--\n" ) + 
                                          QString::fromLatin1( "group:" ) + s_group1 + QString::fromLatin1( ":--x\n" ) +
                                          QString::fromLatin1( "group:" ) + s_group2 + QString::fromLatin1( ":r--\n" ) +
                                          QString::fromLatin1( "mask::r-x\nother::r--\n" );

KACLTest::KACLTest()
:m_acl( s_testACL )
{
}

void KACLTest::setup()
{
}

void KACLTest::runAll()
{
  testAsString();
  testGetOwnerPermissions();
  testGetOwningGroupPermissions();
  testGetOthersPermissions();
  
  testGetMaskPermissions();
  testGetAllUserPermissions();

  testIsExtended();

  // from here on we operate with the second test string
  testSetACL();
  testGetAllGroupsPermissions();

  testOperators();
  testSettingBasic();
  testSettingExtended();
  testSettingErrorHandling();
}

void KACLTest::cleanup()
{
}

void KACLTest::testAsString()
{
  check( "asString: ", m_acl.asString(), s_testACL );
}

void KACLTest::testSetACL()
{
  m_acl.setACL( s_testACL2 );
  check( "setACL: ", m_acl.asString().simplifyWhiteSpace().remove(" "), s_testACLEffective.simplifyWhiteSpace().remove(" ") );
}

void KACLTest::testGetOwnerPermissions()
{
  check( "Owner permissions: ", QString::number( m_acl.ownerPermissions() ), "6" );
}

void KACLTest::testGetOwningGroupPermissions()
{
  check( "Owning group permissions: ", QString::number( m_acl.owningGroupPermissions() ), "6" );
}

void KACLTest::testGetOthersPermissions()
{
  check( "Others permissions: ", QString::number( m_acl.othersPermissions() ), "4" );
}

void KACLTest::testGetMaskPermissions()
{
  bool exists = false;
  unsigned short mask = m_acl.maskPermissions( exists );
  check( "Mask permissions: ", QString::number( mask ), "7" );
  check( "Mask permissions: ", exists, true );
}

void KACLTest::testGetAllUserPermissions()
{
  ACLUserPermissionsList list = m_acl.allUserPermissions();
  ACLUserPermissionsConstIterator it = list.begin();
  QString name;
  unsigned short permissions;
  int count = 0;
  while ( it != list.end() ) {
    name = ( *it ).first;
    permissions = ( *it ).second;
    ++it;
    ++count;
  }
  check( "All users count: ", QString::number( count ), "1" );
  check( "All users name: ", name, "bin" );
  check( "All users permissions: ", QString::number( permissions ), "7" );
}

void KACLTest::testGetAllGroupsPermissions()
{
  ACLGroupPermissionsList list = m_acl.allGroupPermissions();
  ACLGroupPermissionsConstIterator it = list.begin();
  QString name;
  unsigned short permissions;
  int count = 0;
  while ( it != list.end() ) {
    name = ( *it ).first;
    permissions = ( *it ).second;
    // setACL sorts them alphabetically ...
    if ( count == 0 ) {
      check( "All groups name: ", name, s_group1 );
      check( "All groups permissions: ", QString::number( permissions ), "1" );
    } else if ( count == 1 ) {
      check( "All groups name: ", name, s_group2 );
      check( "All groups permissions: ", QString::number( permissions ), "4" );
    }
    ++it;
    ++count;
  }
  check( "All users count: ", QString::number( count ), "2" );
}

void KACLTest::testIsExtended()
{
  KACL dukeOfMonmoth( s_testACL );
  check( "isExtended on an extended one: ", dukeOfMonmoth.isExtended(), true );
  KACL earlOfUpnor( "user::r--\ngroup::r--\nother::r--\n" );
  check( "isExtended on a not extended one: ", earlOfUpnor.isExtended(), false );
}

void KACLTest::testOperators()
{
  KACL dukeOfMonmoth( s_testACL );
  KACL JamesScott( s_testACL );
  KACL earlOfUpnor( s_testACL2 );
  check( "operator== on different ones: ", dukeOfMonmoth == earlOfUpnor, false );
  check( "operator== on identical ones: ", dukeOfMonmoth == JamesScott, true );
  check( "operator!= on diffenrent ones: ", dukeOfMonmoth != earlOfUpnor, true );
  check( "operator!=: on identical ones: ", dukeOfMonmoth != JamesScott, false );
}

void KACLTest::testSettingBasic()
{
  KACL CharlesII( s_testACL );
  CharlesII.setOwnerPermissions( 7 ); // clearly
  CharlesII.setOwningGroupPermissions( 0 );
  CharlesII.setOthersPermissions( 0 );
  check( "setOwnerPermissions: ", QString::number( CharlesII.ownerPermissions() ),"7" );
  check( "setOwningGroupPermissions: ", QString::number( CharlesII.owningGroupPermissions() ),"0" );
  check( "setOthersPermissions: ", QString::number( CharlesII.othersPermissions() ),"0" );
}

void KACLTest::testSettingExtended()
{
  KACL CharlesII( s_testACL );
  CharlesII.setMaskPermissions( 7 ); // clearly
  bool dummy = false;
  check( "setMaskPermissions: ", QString::number( CharlesII.maskPermissions( dummy ) ),"7" );

  const QString expected( "user::rw-\nuser:root:rwx\nuser:bin:r--\ngroup::rw-\nmask::rwx\nother::r--\n" );
  
  ACLUserPermissionsList users;
  ACLUserPermissions user = qMakePair( QString( "root" ), ( unsigned short )7 );
  users.append( user );
  user = qMakePair( QString( "bin" ), ( unsigned short )4 );
  users.append( user );
  CharlesII.setAllUserPermissions( users );
  check( "setAllUserPermissions: ", CharlesII.asString(), expected );

  CharlesII.setACL( s_testACL ); // reset
  // it already has an entry for bin, let's change it
  CharlesII.setNamedUserPermissions( QString("bin"), 4 );
  CharlesII.setNamedUserPermissions( QString( "root" ), 7 );
  check( "setNamedUserPermissions: ", CharlesII.asString(), expected );

  // groups, all and named
  
  const QString expected2 = QString::fromLatin1( "user::rw-\nuser:bin:rwx\ngroup::rw-\ngroup:" ) + s_group1 +
                            QString::fromLatin1( ":-wx\ngroup:" ) + s_group2 + QString::fromLatin1(":r--\nmask::rwx\nother::r--\n" );
  CharlesII.setACL( s_testACL ); // reset
  ACLGroupPermissionsList groups;
  ACLGroupPermissions group = qMakePair( s_group1, ( unsigned short )3 );
  groups.append( group );
  group = qMakePair( s_group2, ( unsigned short )4 );
  groups.append( group );
  CharlesII.setAllGroupPermissions( groups );
  check( "setAllGroupPermissions: ", CharlesII.asString(), expected2 );

  CharlesII.setACL( s_testACL ); // reset
  CharlesII.setNamedGroupPermissions( s_group1, 3 );
  CharlesII.setNamedGroupPermissions( s_group2, 4 );
  check( "setNamedGroupPermissions: ", CharlesII.asString(), expected2 );
}

void KACLTest::testSettingErrorHandling()
{
  KACL foo( s_testACL );
  bool v = foo.setNamedGroupPermissions( s_group1, 7 ); // existing group
  check( "Existing group: ", v, true );
  v = foo.setNamedGroupPermissions( "jongel", 7 ); // non-existing group
  check( "Non-existing group: ", v, false );

  v = foo.setNamedUserPermissions( "bin", 7 ); // existing user
  check( "Existing user: ", v, true );
  v = foo.setNamedUserPermissions( "jongel", 7 ); // non-existing user
  check( "Non-existing user: ", v, false );
}
