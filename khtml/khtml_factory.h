/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
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
#ifndef __khtml_factory_h__
#define __khtml_factory_h__

#include <kparts/factory.h>

class KInstance;
class KAboutData;

class KHTMLFactory : public KParts::Factory
{
  Q_OBJECT
public:
  KHTMLFactory();
  virtual ~KHTMLFactory();

  virtual KParts::Part *createPart( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const char *className, const QStringList &args );

  static KInstance *instance();

private:
  static KInstance *s_instance;
  static KAboutData *s_about;
};

#endif
