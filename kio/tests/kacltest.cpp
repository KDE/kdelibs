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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kacltest.h"

#include <config-kio.h>

#include <kacl.h>
#include <QDebug>

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QEventLoop>

// The code comes partly from kdebase/kioslave/trash/testtrash.cpp

static bool check(const QString& txt, QString a, QString b)
{
    if (a.isEmpty())
        a.clear();
    if (b.isEmpty())
        b.clear();
    if (a == b) {
        qDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok";
    }
    else {
        qDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !";
        exit(1);
    }
    return true;
}

template<typename T>
static bool check(const QString& txt, T a, T b)
{
    if (a == b) {
        qDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok";
    }
    else {
        qDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !";
        exit(1);
    }
    return true;
}

int main(int argc, char *argv[])
{
    QApplication::setApplicationName("kacltest");
    QApplication app(argc, argv);

#if HAVE_POSIX_ACL
    KACLTest test;
    test.setup();
    test.runAll();
    test.cleanup();
    qDebug() << "All tests OK.";
#else
    qDebug() << "All tests skipped - no ACL support";
#endif
    return 0; // success. The exit(1) in check() is what happens in case of failure.
}

static const QString s_testACL( "user::rw-\nuser:bin:rwx\ngroup::rw-\nmask::rwx\nother::r--\n" );
static const QString s_testACL2( "user::rwx\nuser:bin:rwx\ngroup::rw-\ngroup:users:r--\ngroup:audio:--x\nmask::r-x\nother::r--\n" );
static const QString s_testACLEffective("user::rwx\nuser:bin:rwx    #effective:r-x\ngroup::rw-      #effective:r--\ngroup:audio:--x\ngroup:users:r--\nmask::r-x\nother::r--\n");

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

  testNewMask();
}

void KACLTest::cleanup()
{
}

void KACLTest::testAsString()
{
  check( "asString: ", s_testACL, m_acl.asString() );
}

void KACLTest::testSetACL()
{
  m_acl.setACL( s_testACL2 );
  check( "setACL: ", s_testACLEffective.simplified(), m_acl.asString().simplified() );
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
  ACLUserPermissionsConstIterator it = list.constBegin();
  QString name;
  unsigned short permissions = 0;
  int count = 0;
  while ( it != list.constEnd() ) {
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
  ACLGroupPermissionsConstIterator it = list.constBegin();
  QString name;
  unsigned short permissions;
  int count = 0;
  while ( it != list.constEnd() ) {
    name = ( *it ).first;
    permissions = ( *it ).second;
    // setACL sorts them alphabetically ...
    if ( count == 0 ) {
      check( "All groups name: ", name, "audio" );
      check( "All groups permissions: ", QString::number( permissions ), "1" );
    } else if ( count == 1 ) {
      check( "All groups name: ", name, "users" );
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

  const QString expected2( "user::rw-\nuser:bin:rwx\ngroup::rw-\ngroup:audio:-wx\ngroup:users:r--\nmask::rwx\nother::r--\n" );
  CharlesII.setACL( s_testACL ); // reset
  ACLGroupPermissionsList groups;
  ACLGroupPermissions group = qMakePair( QString( "audio" ), ( unsigned short )3 );
  groups.append( group );
  group = qMakePair( QString( "users" ), ( unsigned short )4 );
  groups.append( group );
  CharlesII.setAllGroupPermissions( groups );
  check( "setAllGroupPermissions: ", CharlesII.asString(), expected2 );

  CharlesII.setACL( s_testACL ); // reset
  CharlesII.setNamedGroupPermissions( QString( "audio" ), 3 );
  CharlesII.setNamedGroupPermissions( QString( "users" ), 4 );
  check( "setNamedGroupPermissions: ", CharlesII.asString(), expected2 );
}

void KACLTest::testSettingErrorHandling()
{
  KACL foo( s_testACL );
  bool v = foo.setNamedGroupPermissions( "audio", 7 ); // existing group
  check( "Existing group: ", v, true );
  v = foo.setNamedGroupPermissions( "jongel", 7 ); // non-existing group
  check( "Non-existing group: ", v, false );

  v = foo.setNamedUserPermissions( "bin", 7 ); // existing user
  check( "Existing user: ", v, true );
  v = foo.setNamedUserPermissions( "jongel", 7 ); // non-existing user
  check( "Non-existing user: ", v, false );
}

void KACLTest::testNewMask()
{
  KACL CharlesII( "user::rw-\ngroup::rw-\nother::rw\n" );
  bool dummy = false;
  CharlesII.maskPermissions( dummy );
  check( "mask exists: ", dummy, false );

  CharlesII.setMaskPermissions( 6 );
  check( "new mask set: ", QString::number( CharlesII.maskPermissions( dummy ) ), "6" );
  check( "mask exists now: ", dummy, true );
}
