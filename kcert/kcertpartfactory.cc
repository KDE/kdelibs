/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include "kcertpartfactory.h"


#include <klocale.h>
#include <kstddirs.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include "kcertpart.h"

extern "C" {
  void * init_libkcertpart() {
    return new KCertPartFactory;
  }
};

KInstance *KCertPartFactory::_instance = 0;
KAboutData *KCertPartFactory::_about = 0;


KCertPartFactory::KCertPartFactory(QObject *parent, const char *name)
                 : KLibFactory(parent, name) {
}


KCertPartFactory::~KCertPartFactory() {
  delete _instance;
  _instance = 0;
  delete _about;
}


QObject *KCertPartFactory::create(QObject *parent, const char *name,
                                  const char *classname, const QStringList &) {
  if (parent && !parent->inherits("QWidget")) {
    kdError() << "KCertPartFactory: parent does not inherit QWidget" << endl;
    return NULL;
  }

  KCertPart *part = new KCertPart((QWidget *)parent, name);

  if (QCString(classname) == "KParts::ReadOnlyPart") {
    part->setReadWrite(false);
  } else if (QCString(classname) != "KParts::ReadWritePart") {
    kdError() << "KCertPartFactory: part isn't ReadOnly or ReadWrite?!" << endl;
    return NULL;
  }

  emit objectCreated(part);
  return part;
}

KInstance *KCertPartFactory::instance() {
  if (!_instance) {
    _about = new KAboutData("KCertPart", I18N_NOOP("KDE Certificate Part"), "1.0");
    _instance = new KInstance(_about);
  }
  return _instance;
}

#include "kcertpartfactory.moc"

