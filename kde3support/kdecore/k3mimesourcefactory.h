/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (C) 1997 Matthias Kalle Dalheimer <kalle@kde.org>
 
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

#ifndef KMIMESOURCEFACTORY_H
#define KMIMESOURCEFACTORY_H

#include <kde3support_export.h>
#include <kglobal.h>

#include <QtGui/QMacMime>
#include <Qt3Support/Q3MimeSourceFactory>

class K3MimeSourceFactoryPrivate;
class KIconLoader;
/**
 * An extension to QMimeSourceFactory that uses KIconLoader to
 * find images.
 *
 * Normally you don't have to instantiate this class at all, KApplication does that for
 * you automagically and sets QMimeSourceFactory::setDefaultFactory.
 *
 * @author Peter Putzer <putzer@kde.org>
 */
class KDE3SUPPORT_EXPORT K3MimeSourceFactory : public Q3MimeSourceFactory
{
public:

  /**
   * Set up a K3MimeSourceFactory instance as the default mimesource factory.
   */
  static void install();

  /**
   * Constructor.
   *
   * @param loader is the iconloader used to find images.
   */
  explicit K3MimeSourceFactory (KIconLoader* loader = 0);

  /**
   * Destructor.
   */
  virtual ~K3MimeSourceFactory();

  /**
   * This function is maps an absolute or relative name for a resource to 
   * the absolute one.
   *
   * To load an icon, prepend the @p category name before the @p icon name, in the style
   * of \<category>|\<icon>.
   *
   * Example:
   * \code "<img src=\"user|ksysv_start\"/>", "<img src="\desktop|trash\">", ...
   * \endcode
   *
   * @param abs_or_rel_name is the absolute or relative pathname.
   * @param context is the path of the context object for the queried resource. Almost always empty.
   */
  virtual QString makeAbsolute (const QString& abs_or_rel_name, const QString& context) const;

protected:
  /** Virtual hook, used to add new "virtual" functions while maintaining
      binary compatibility. Unused in this class.
  */
  virtual void virtual_hook( int id, void* data );
private:
  K3MimeSourceFactoryPrivate* const d;
};

#endif // KMIMESOURCEFACTORY_H
