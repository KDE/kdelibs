/* This file is part of the KDE libraries
   Copyright (C) 2001-2005 Christoph Cullmann <cullmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "config.h"

#include "kateglobal.h"

#include <kparts/factory.h>

/**
 * wrapper factory to be sure nobody external deletes our kateglobal object
 * each instance will just increment the reference counter of our internal
 * super private global instance ;)
 */
class KateFactory : public KParts::Factory
{
  public:
    KateFactory ( QObject *parent = 0, const char *name = 0 )
      : KParts::Factory (parent, name)
    {
      KateGlobal::incRef ();
    }

    virtual ~KateFactory ()
    {
      KateGlobal::decRef ();
    }

    /**
     * reimplemented create object method
     * @param parentWidget parent widget
     * @param widgetName widget name
     * @param parent QObject parent
     * @param name object name
     * @param args additional arguments
     * @return constructed part object
     */
    KParts::Part *createPartObject ( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const char *classname, const QStringList &args )
    {
      return KateGlobal::self()->createPartObject (parentWidget, widgetName, parent, name, classname, args);
    }
};

K_EXPORT_COMPONENT_FACTORY( libkatepart, KateFactory )

// kate: space-indent on; indent-width 2; replace-tabs on;
