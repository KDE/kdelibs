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


#ifndef KEXTENDABLEITEMDELEGATE_H
#define KEXTENDABLEITEMDELEGATE_H

#include <QItemDelegate>
#include <QHash>

/*
This delegate makes it possible to display an arbitrary QWidget ("extender") that spans all columns below a line of items.
The extender will logically belong to a column in the row above it.


It is your responsibility to devise a way to trigger extension and contraction of items.
*/

//TODO:listen for removal of rows and columns to clean out dead persistent indexes/editors.
//TODO:follow binary compatibility rules


class QAbstractItemView;
class QPersistentModelIndex;

class KExtendableItemDelegate : public QItemDelegate {
	Q_OBJECT

public:
	enum auxDataRoles {ShowExtensionIndicatorRole = Qt::UserRole + 200};

	KExtendableItemDelegate(QAbstractItemView *parent);
	virtual ~KExtendableItemDelegate();

	//reimplemented from QItemDelegate
	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	
	/** ...
	 * If you need a parent for the extender at construction time, use the itemview's viewport().
	 * The extender will be reparented and resized to the viewport by this function.
	 */
	void extendItem(QWidget *extender, const QModelIndex &index);
	void contractItem(const QModelIndex &index);
	bool isExtended(const QModelIndex &index) const;
	virtual void updateExtenderGeometry(QWidget *extender, const QStyleOptionViewItem &option, const QModelIndex &index) const;

Q_SIGNALS:
	void extenderCreated(QWidget *extender, QModelIndex index);
	void extenderDestroyed(QWidget *extender, QModelIndex index);

protected:
	QRect extenderRect(QWidget *extender, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	//these two must have the same (screen) size!
	QPixmap m_extendIcon;
	QPixmap m_contractIcon;

protected Q_SLOTS:
	void extenderDestructionHandler(QObject *destroyed);

private:
	inline QSize maybeExtendedSize(const QStyleOptionViewItem &option/*maybe superfluous?*/, const QModelIndex &index) const;
	QModelIndex indexOfExtendedColumnInSameRow(const QModelIndex &index) const;
	inline void scheduleUpdateViewLayout() const;
	
	//this will trigger a lot of auto-casting QModelIndex <-> QPersistentModelIndex
	QHash<QPersistentModelIndex, QWidget *> m_extenders;
	QHash<QWidget *, QPersistentModelIndex> m_extenderIndices;
	//mostly for quick startup - don't look for extenders while the view
	//is being populated.
	mutable bool m_hasExtenders;
};
#endif // KEXTENDABLEITEMDELEGATE_H
