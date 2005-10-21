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

#undef QT3_SUPPORT
#include <kuser.h>

#include <qstringlist.h>

#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <grp.h>

class KUser::Private : public KShared
{
public:
	uid_t uid;
	gid_t gid;
	QString loginName, fullName;
	QString roomNumber, workPhone, homePhone;
	QString homeDir, shell;

	Private() : uid(uid_t(-1)), gid(gid_t(-1)) {}

	Private(uid_t _uid,
		gid_t _gid,
		const QString &_loginname,
		const QString &_fullname,
		const QString &_room,
		const QString &_workPhone,
		const QString &_homePhone,
		const QString &_homedir,
		const QString &_shell) :
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
	uid_t _uid = ::getuid(), _euid;
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

KUser::KUser(uid_t _uid) {
	fillPasswd( ::getpwuid( _uid ) );
}

KUser::KUser(const QString& name) {
	fillName( name.toLocal8Bit().data() );
}

KUser::KUser(const char *name) {
	fillName( name );
}

KUser::KUser(const passwd *p) {
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
	return uid() == user.uid();
}

bool KUser::operator !=(const KUser& user) const {
	return uid() != user.uid();
}

void KUser::fillName(const char *name) {
	fillPasswd(name ? ::getpwnam( name ) : 0);
}

void KUser::fillPasswd(const passwd *p) {
	if (p) {
		QString gecos = QString::fromLocal8Bit(p->pw_gecos);
		QStringList gecosList = gecos.split(QLatin1Char(','));

		// fill up the list, should be at least 4 entries
		switch (gecosList.size()) {
		case 0: gecosList << QString::null;
		case 1: gecosList << QString::null;
		case 2: gecosList << QString::null;
		case 3: gecosList << QString::null;
		}

		d = new Private(p->pw_uid,
				p->pw_gid,
				QString::fromLocal8Bit(p->pw_name),
				gecosList[0], gecosList[1],
				gecosList[2], gecosList[3],
				QString::fromLocal8Bit(p->pw_dir),
				QString::fromLocal8Bit(p->pw_shell));
	}
	else
		d = new Private;
}

bool KUser::isValid() const {
	return uid() != uid_t(-1);
}

uid_t KUser::uid() const {
	return d->uid;
}

gid_t KUser::gid() const {
	return d->gid;
}

bool KUser::isSuperUser() const {
	return uid() == 0;
}

QString KUser::loginName() const {
	return d->loginName;
}

QString KUser::fullName() const {
	return d->fullName;
}

QString KUser::roomNumber() const {
	return d->roomNumber;
}

QString KUser::workPhone() const {
	return d->workPhone;
}

QString KUser::homePhone() const {
	return d->homePhone;
}

QString KUser::homeDir() const {
	return d->homeDir;
}

QString KUser::shell() const {
	return d->shell;
}

QList<KUserGroup> KUser::groups() const {
  QList<KUserGroup> result;
  QList<KUserGroup> allGroups = KUserGroup::allGroups();
  QList<KUserGroup>::const_iterator it;
  for ( it = allGroups.begin(); it != allGroups.end(); ++it ) {
    QList<KUser> users = (*it).users();
    if ( users.contains(*this) ) {
       result.append(*it);
    }
  }
  return result;
}

QStringList KUser::groupNames() const {
  QStringList result;
  QList<KUserGroup> allGroups = KUserGroup::allGroups();
  QList<KUserGroup>::const_iterator it;
  for ( it = allGroups.begin(); it != allGroups.end(); ++it ) {
    QList<KUser> users = (*it).users();
    if ( users.contains(*this) ) {
       result.append((*it).name());
    }
  }
  return result;
}


QList<KUser> KUser::allUsers() {
  QList<KUser> result;

  passwd* p;

  while ((p = getpwent()))  {
    result.append(KUser(p));
  }

  endpwent();

  return result;
}

QStringList KUser::allUserNames() {
  QStringList result;

  passwd* p;

  while ((p = getpwent()))  {
    result.append(QString::fromLocal8Bit(p->pw_name));
  }

  endpwent();
  return result;
}

KUser::~KUser() {
}

class KUserGroup::Private : public KShared
{
public:
  gid_t gid;
  QString name;
  QList<KUser> users;

  Private() : gid(gid_t(-1)) {}

  Private(gid_t _gid, const QString & _name, const QList<KUser> & _users):
    gid(_gid),
    name(_name),
    users(_users) {}
};

KUserGroup::KUserGroup(KUser::UIDMode mode) {
  KUser user(mode);
  fillGroup(getgrgid(user.gid()));
}

KUserGroup::KUserGroup(gid_t _gid) {
  fillGroup(getgrgid(_gid));
}

KUserGroup::KUserGroup(const QString& _name) {
  fillName(_name.toLocal8Bit().data());
}

KUserGroup::KUserGroup(const char *_name) {
  fillName(_name);
}

KUserGroup::KUserGroup(const group *g) {
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
	return gid() == group.gid();
}

bool KUserGroup::operator !=(const KUserGroup& user) const {
	return gid() != user.gid();
}

void KUserGroup::fillName(const char *_name) {
  fillGroup(_name ? ::getgrnam( _name ) : 0);
}

void KUserGroup::fillGroup(const group *p) {
  if (!p) {
    d = new Private;
    return;
  }

  QString aName = QString::fromLocal8Bit(p->gr_name);
  QList<KUser> userList;

  char **user = p->gr_mem;
  for ( ; *user; user++) {
    KUser kUser(*user);
    userList.append(kUser);
  }

  d = new Private(p->gr_gid, aName, userList);
}

bool KUserGroup::isValid() const {
	return gid() != gid_t(-1);
}

gid_t KUserGroup::gid() const {
	return d->gid;
}

QString KUserGroup::name() const {
	return d->name;
}

const QList<KUser>& KUserGroup::users() const {
	return d->users;
}

QStringList KUserGroup::userNames() const {
  QStringList result;
  QList<KUser>::const_iterator it;
  for ( it = d->users.begin(); it != d->users.end(); ++it ) {
    result.append((*it).loginName());
  }
  return result;
}

QList<KUserGroup> KUserGroup::allGroups() {
  QList<KUserGroup> result;

  group* g;
  while ((g = getgrent()))  {
     result.append(KUserGroup(g));
  }

  endgrent();

  return result;
}

QStringList KUserGroup::allGroupNames() {
  QStringList result;

  group* g;
  while ((g = getgrent()))  {
     result.append(QString::fromLocal8Bit(g->gr_name));
  }

  endgrent();

  return result;
}

KUserGroup::~KUserGroup() {
}
