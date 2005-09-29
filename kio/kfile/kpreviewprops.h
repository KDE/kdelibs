/* This file is part of the KDE libraries
    Copyright (C) 2005 Stephan Binner <binner@kde.org>

    This library is free software; you can redistribute it and/or
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

#ifndef __KPREVIEWPROPS_H__
#define __KPREVIEWPROPS_H__

#include <kpropertiesdialog.h>

class KFileMetaPreview;

/*!
 * PreviewProps plugin
 * This plugin displays a preview of the given file
 * @since 3.5
 */
class KIO_EXPORT KPreviewPropsPlugin : public KPropsDlgPlugin
{
  Q_OBJECT

public:

  KPreviewPropsPlugin( KPropertiesDialog *_props );
  virtual ~KPreviewPropsPlugin();

  /**
   * Tests whether a preview for the first item should be shown
   */
  static bool supports( KFileItemList _items );

private slots:
  void aboutToShowPage( QWidget* );

private:
  KFileMetaPreview* preview;
  void createLayout();

  class KPreviewPropsPluginPrivate;
  KPreviewPropsPluginPrivate *d;
};

#endif
