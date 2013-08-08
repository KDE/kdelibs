/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Rolf Magnus <ramagnus@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

 */

#ifndef __KMETAPROPS_H__
#define __KMETAPROPS_H__
#include <kpropertiesdialog.h>
#include <kde4support_export.h>

class KFileMetaInfoItem;

/*!
 * 'MetaProps plugin
 * In this plugin you can modify meta information like id3 tags of mp3 files
 */
class KDE4SUPPORT_EXPORT KFileMetaPropsPlugin : public KPropertiesDialogPlugin
{
  Q_OBJECT
public:
  KFileMetaPropsPlugin( KPropertiesDialog *_props );
  virtual ~KFileMetaPropsPlugin();

  virtual void applyChanges();

  /**
   * Tests whether the file specified by _items has a 'MetaInfo' plugin.
   */
  static bool supports( const KFileItemList& _items );

private:
  class KFileMetaPropsPluginPrivate;
  KFileMetaPropsPluginPrivate* const d;

  Q_PRIVATE_SLOT(d, void configureShownMetaData())
};

#endif
