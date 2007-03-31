/* This file is part of the KDE libraries
    Copyright (C) 2006,2007 Andreas Hartmetz (ahartmetz@gmail.com)

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

#ifndef KSHORTCUTSDIALOG_P_H
#define KSHORTCUTSDIALOG_P_H


#include <kextendableitemdelegate.h>
class QAbstractItemView;
class KShortcutsEditorDelegate : public KExtendableItemDelegate
{
	Q_OBJECT
public:
	KShortcutsEditorDelegate(QAbstractItemView *parent);
	//virtual QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
Q_SIGNALS:
	void editInstead(QModelIndex index);
private:
	//mutable QModelIndex *m_index;
private Q_SLOTS:
	void itemActivated(QModelIndex index);
};


#endif /* KSHORTCUTSDIALOG_P_H */

