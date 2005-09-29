/*
 *  KUser - represent a user/account
 *  Copyright (C) 2002 Tim Jansen <tim@tjansen.de>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <kuser.h>

#include "kstringhandler.h"
#include <qvaluelist.h>
#include <qstringlist.h>

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <grp.h>


class KUserPrivate : public KShared
{
public:
	bool valid;
	long uid, gid;
	QString loginName, fullName;
	QString roomNumber, workPhone, homePhone;
	QString homeDir, shell;

	KUserPrivate() : valid(false) {}

	KUserPrivate(long _uid,
		     long _gid,
		     const QString &_loginname,
		     const QString &_fullname,
		     const QString &_room,
		     const QString &_workPhone,
		     const QString &_homePhone,
		     const QString &_homedir,
		     const QString &_shell) :
		valid(true),
		uid(_uid),
		gid(_gid),
		loginName(_loginname),
		fullName(_fullname),
		roomNumber(_room),
		workPhone(_workPhone),
		homePhone(_homePhone),
		homeDir(_homedir),
		shell(_shell) {}
};


KUser::KUser(UIDMode mode) {
	long _uid = ::getuid(), _euid;
	if (mode == UseEffectiveUID && (_euid = ::geteuid()) != _uid )
		fillPasswd( ::getpwuid( _euid ) );
	else {
		fillName( ::getenv( "LOGNAME" ) );
		if (uid() != _uid) {
			fillName( ::getenv( "USER" ) );
			if (uid() != _uid)
				fillPasswd( ::getpwuid( _uid ) );
		}
	}
}

KUser::KUser(long uid) {
	fillPasswd( ::getpwuid( uid ) );
}

KUser::KUser(const QString& name) {
	fillName( name.local8Bit().data() );
}

KUser::KUser(const char *name) {
	fillName( name );
}

KUser::KUser(struct passwd *p) {
    fillPasswd(p);
}

KUser::KUser(const KUser & user) 
  : d(user.d) 
{
}

KUser& KUser::operator =(const KUser& user) 
{
  d = user.d;
  return *this;
}

bool KUser::operator ==(const KUser& user) const {
    if (isValid() != user.isValid())
	return false;
    if (isValid())
	return uid() == user.uid();
    else
	return true;
}

bool KUser::operator !=(const KUser& user) const {
	return !operator ==(user);
}

void KUser::fillName(const char *name) {
	fillPasswd(name ? ::getpwnam( name ) : 0);
}

void KUser::fillPasswd(struct passwd *p) {
	if (p) {
		QString gecos = KStringHandler::from8Bit(p->pw_gecos); 
		QStringList gecosList = QStringList::split(',', gecos, true);

		d = new KUserPrivate(p->pw_uid,
				     p->pw_gid,
				     QString::fromLocal8Bit(p->pw_name),
				     (gecosList.size() > 0) ? gecosList[0] : QString::null,
				     (gecosList.size() > 1) ? gecosList[1] : QString::null,
				     (gecosList.size() > 2) ? gecosList[2] : QString::null,
				     (gecosList.size() > 3) ? gecosList[3] : QString::null,
				     QString::fromLocal8Bit(p->pw_dir),
				     QString::fromLocal8Bit(p->pw_shell));
	}
	else
		d = new KUserPrivate();
}

bool KUser::isValid() const {
	return d->valid;
}

long KUser::uid() const {
	if (d->valid)
		return d->uid;
	else
		return -1;
}

long KUser::gid() const {
	if (d->valid)
		return d->gid;
	else
		return -1;
}

bool KUser::isSuperUser() const {
	return uid() == 0;
}

QString KUser::loginName() const {
	if (d->valid)
		return d->loginName;
	else
		return QString::null;
}

QString KUser::fullName() const {
	if (d->valid)
		return d->fullName;
	else
		return QString::null;
}

QString KUser::roomNumber() const {
	if (d->valid)
		return d->roomNumber;
	else
		return QString::null;
}

QString KUser::workPhone() const {
	if (d->valid)
		return d->workPhone;
	else
		return QString::null;
}

QString KUser::homePhone() const {
	if (d->valid)
		return d->homePhone;
	else
		return QString::null;
}

QString KUser::homeDir() const {
	if (d->valid)
		return d->homeDir;
	else
		return QString::null;
}

QString KUser::shell() const {
	if (d->valid)
		return d->shell;
	else
		return QString::null;
}

QValueList<KUserGroup> KUser::groups() const {
  QValueList<KUserGroup> result;
  QValueList<KUserGroup> allGroups = KUserGroup::allGroups();
  QValueList<KUserGroup>::const_iterator it;
  for ( it = allGroups.begin(); it != allGroups.end(); ++it ) {
    QValueList<KUser> users = (*it).users();
    if ( users.find( *this ) != users.end()) {
       result.append(*it);
    }
  }
  return result;
}

QStringList KUser::groupNames() const {
  QStringList result;
  QValueList<KUserGroup> allGroups = KUserGroup::allGroups();
  QValueList<KUserGroup>::const_iterator it;
  for ( it = allGroups.begin(); it != allGroups.end(); ++it ) {
    QValueList<KUser> users = (*it).users();
    if ( users.find( *this ) != users.end()) {
       result.append((*it).name());
    }
  }
  return result;
}


QValueList<KUser> KUser::allUsers() {
  QValueList<KUser> result;

  struct passwd* p;

  while ((p = getpwent()))  {
    result.append(KUser(p));
  }

  endpwent();

  return result;
}

QStringList KUser::allUserNames() {
  QStringList result;

  struct passwd* p;

  while ((p = getpwent()))  {
    result.append(QString::fromLocal8Bit(p->pw_name));
  }

  endpwent();
  return result;
}


KUser::~KUser() {
}

class KUserGroupPrivate : public KShared
{
public:
  bool valid;
  long gid;
  QString name;
  QValueList<KUser> users;
  
  KUserGroupPrivate() : valid(false) {}
  
  KUserGroupPrivate(long _gid, 
                const QString & _name, 
                const QValueList<KUser> & _users):
    valid(true), 
    gid(_gid), 
    name(_name),
    users(_users) {}
};

KUserGroup::KUserGroup(KUser::UIDMode mode) {
  KUser user(mode);
  fillGroup(getgrgid(user.gid()));
}

KUserGroup::KUserGroup(long gid) {
  fillGroup(getgrgid(gid));
}

KUserGroup::KUserGroup(const QString& name) {
  fillName(name.local8Bit().data());
}

KUserGroup::KUserGroup(const char *name) {
  fillName(name);
}

KUserGroup::KUserGroup(struct group *g) {
  fillGroup(g);
}


KUserGroup::KUserGroup(const KUserGroup & group) 
  : d(group.d)
{
}

KUserGroup& KUserGroup::operator =(const KUserGroup& group) {
  d = group.d;
  return *this;
}

bool KUserGroup::operator ==(const KUserGroup& group) const {
  if (isValid() != group.isValid())
    return false;
  if (isValid())
    return gid() == group.gid();
  else
    return true;
}

bool KUserGroup::operator !=(const KUserGroup& user) const {
  return !operator ==(user);
}

void KUserGroup::fillName(const char *name) {
  fillGroup(name ? ::getgrnam( name ) : 0);
}

void KUserGroup::fillGroup(struct group *p) {
  if (!p) {
    d = new KUserGroupPrivate();
    return;
  }
  
  QString name = KStringHandler::from8Bit(p->gr_name); 
  QValueList<KUser> users;
  
  char **user = p->gr_mem;  
  for ( ; *user; user++) {
    KUser kUser(QString::fromLocal8Bit(*user));
    users.append(kUser);
  }
  
  d = new KUserGroupPrivate(p->gr_gid,
            QString::fromLocal8Bit(p->gr_name),
            users);  

}

bool KUserGroup::isValid() const {
  return d->valid;
}

long KUserGroup::gid() const {
  if (d->valid)
    return d->gid;
  else
    return -1;
}

QString KUserGroup::name() const {
  if (d->valid)
    return d->name;
  else
    return QString::null;
}

const QValueList<KUser>& KUserGroup::users() const {
  return d->users;
}

QStringList KUserGroup::userNames() const {
  QStringList result;
  QValueList<KUser>::const_iterator it;
  for ( it = d->users.begin(); it != d->users.end(); ++it ) {
    result.append((*it).loginName());
  }
  return result;
}



QValueList<KUserGroup> KUserGroup::allGroups() {
  QValueList<KUserGroup> result;
  
  struct group* g;
  while ((g = getgrent()))  {
     result.append(KUserGroup(g));
  }

  endgrent();

  return result;
}

QStringList KUserGroup::allGroupNames() {
  QStringList result;
  
  struct group* g;
  while ((g = getgrent()))  {
     result.append(QString::fromLocal8Bit(g->gr_name));
  }

  endgrent();

  return result;
}


KUserGroup::~KUserGroup() {
}

