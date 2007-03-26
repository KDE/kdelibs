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


#include "kextendableitemdelegate.h"
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QTreeView>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
//remove once we have the icons we need reachable via KIconLoader
#include <QImageReader>
//remove once we stop using it
#include <kdebug.h>

KExtendableItemDelegate::KExtendableItemDelegate(QAbstractItemView* parent)
 : QItemDelegate(parent),
   m_hasExtenders(false)
{
	QImageReader imgReader;
	//in the final version, just take these as arguments.
	imgReader.setScaledSize(QSize(12, 12));
	imgReader.setFileName("/home/kdehase/arrow_right.svg");
	m_extendIcon = QPixmap::fromImage(imgReader.read());
	imgReader.setFileName("/home/kdehase/arrow_down.svg");
	m_contractIcon = QPixmap::fromImage(imgReader.read());
	parent->installEventFilter(this);
}


KExtendableItemDelegate::~KExtendableItemDelegate()
{
//hmmm, nothing?
}


void KExtendableItemDelegate::extendItem(QWidget *ext, const QModelIndex &index)
{
	if (!ext || !index.isValid())
		return;
	//maintain the invariant "zero or one extender per row"
	contractItem(indexOfExtendedColumnInSameRow(index));
	//reparent, as promised in the docs
	QAbstractItemView *aiv = qobject_cast<QAbstractItemView *>(parent());
	if (!aiv)
		return;
	ext->setParent(aiv->viewport());
	m_extenders.insert(index, ext);
	m_extenderIndices.insert(ext, index);
	m_hasExtenders = true;
	connect(ext, SIGNAL(destroyed(QObject *)), this, SLOT(extenderDestructionHandler(QObject *)));
	emit extenderCreated(ext, index);
	scheduleUpdateViewLayout();
}


void KExtendableItemDelegate::contractItem(const QModelIndex& index)
{
	QWidget *extender = m_extenders.take(index);
	if (!extender)
		return;

	extender->deleteLater();

	scheduleUpdateViewLayout();
}


//slot
void KExtendableItemDelegate::extenderDestructionHandler(QObject *destroyed)
{
	QWidget *extender = static_cast<QWidget *>(destroyed);

	//An invalid model index here is a "can't happen" situation. We don't catch it because
	//everything would be broken already.

	if (receivers(SIGNAL(extenderDestroyed(QObject *, QModelIndex)))) {
		QPersistentModelIndex persistentIndex = m_extenderIndices.take(extender);
		QModelIndex index = persistentIndex;
		emit extenderDestroyed(extender, index);
		m_extenders.remove(persistentIndex);
	} else
		m_extenders.remove(m_extenderIndices.take(extender));

	if (m_extenders.isEmpty())
		m_hasExtenders = false;

	scheduleUpdateViewLayout();
}


bool KExtendableItemDelegate::isExtended(const QModelIndex &index) const {
	return m_extenders.value(index);
}


QSize KExtendableItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (m_hasExtenders)
		return maybeExtendedSize(option, index);
	else
		return QItemDelegate::sizeHint(option, index);
}


void KExtendableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	int indicatorX, indicatorY;
	
	QStyleOptionViewItem modOption(option);
	bool showExtensionIndicator = index.model()->data(index, ShowExtensionIndicatorRole).toBool();
	
	if (showExtensionIndicator) {
		indicatorX = option.rect.left();
		indicatorY = modOption.rect.top() + ((modOption.rect.height() - m_extendIcon.height()) >> 1);
		modOption.rect.setLeft(modOption.rect.left() + m_extendIcon.width());
	}
	
	//fast path
	if (!m_hasExtenders) {
		QItemDelegate::paint(painter, modOption, index);
		if (showExtensionIndicator)
			painter->drawPixmap(indicatorX, indicatorY,  m_extendIcon);
		return;
	}

	//indexOfExtendedColumnInSameRow() is very expensive, try to avoid calling it.
	static int cachedRow = -20; //Qt uses -1 for invalid indexes
	static QModelIndex cachedParentIndex;
	static QWidget *extender;
	static int extenderHeight;
	int row = index.row();
	QModelIndex parentIndex = index.parent();

	//for some reason, caching doesn't work on row 0 (in QTreeView).
	if (row != cachedRow || cachedParentIndex != parentIndex || row == 0) {
		extender = m_extenders.value(indexOfExtendedColumnInSameRow(index));
		cachedRow = row;
		cachedParentIndex = parentIndex;
		if (extender)
			extenderHeight = extender->sizeHint().height();
	}

	if (!extender) {
		QItemDelegate::paint(painter, modOption, index);
		if (showExtensionIndicator)
			painter->drawPixmap(indicatorX, indicatorY, m_extendIcon);
		return;
	}

	if (isExtended(index)) {
		QStyleOptionViewItem extOption(option);
		extOption.rect = extenderRect(extender, option, index);
		updateExtenderGeometry(extender, extOption, index);
		//if we show it before, it will briefly flash in the wrong location.
		//the downside is, of course, that an api user effectively can't hide it.
		extender->show();
	}
	
	modOption.rect.setHeight(modOption.rect.height() - extenderHeight);
	//TODO:make sure that modOption.rect really has the same height as the unchanged
	//option.rect if no extender is present
	QItemDelegate::paint(painter, modOption, index);
	
	if (showExtensionIndicator) {
		//modOption's height changed, change this too
		indicatorY = modOption.rect.top() + ((modOption.rect.height() - m_extendIcon.height()) >> 1);
		if (m_extenders.contains(index))
			painter->drawPixmap(indicatorX, indicatorY, m_contractIcon);
		else
			painter->drawPixmap(indicatorX, indicatorY, m_extendIcon);
	}
}


QRect KExtendableItemDelegate::extenderRect(QWidget *extender, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_ASSERT(extender);
	QRect rect(option.rect);
	rect.setTop(rect.bottom() + 1 - extender->sizeHint().height());
	
	rect.setLeft(0);
	QTreeView *tv = qobject_cast<QTreeView *>(parent());
	if (tv)
		for (QModelIndex idx(index.parent()); idx.isValid(); idx = idx.parent())
			rect.translate(tv->indentation(), 0);
	
	QAbstractScrollArea *container = qobject_cast<QAbstractScrollArea *>(parent());
	Q_ASSERT(container);
	rect.setRight(container->viewport()->width() - 1);
	return rect;
}


QSize KExtendableItemDelegate::maybeExtendedSize(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QWidget *extender = m_extenders.value(index);
	QSize size(QItemDelegate::sizeHint(option, index));
	if (!extender)
		return size;

	//add extender height to maximum height of any column in our row
	int itemHeight = size.height();
	
	const QAbstractItemModel *model = index.model();
	int row = index.row();
	int thisColumn = index.column();
	QModelIndex parentIndex(index.parent());

	//this is quite slow, but Qt is smart about when to call sizeHint().
	for (int column=0;; column++) {
		if (column == thisColumn)
			continue;
		
		QModelIndex neighborIndex(model->index(row, column, parentIndex));
		if (!neighborIndex.isValid())
			break;
		itemHeight = qMax(itemHeight, QItemDelegate::sizeHint(option, neighborIndex).height());
	}
	
	//we only want to reserve vertical space
	size.rheight() = itemHeight + extender->sizeHint().height();
	return size;
}


QModelIndex KExtendableItemDelegate::indexOfExtendedColumnInSameRow(const QModelIndex &index) const
{
	const QAbstractItemModel *model = index.model();
	QModelIndex parentIndex(index.parent());
	int row = index.row();

	//slow, slow, slow
	for (int column=0;; column++) {
		QModelIndex indexOfExt(model->index(row, column, parentIndex));
		if (!indexOfExt.isValid())
			return QModelIndex();
		
		if (m_extenders.value(indexOfExt))
			return indexOfExt;
	}
}


void KExtendableItemDelegate::updateExtenderGeometry(QWidget *extender, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(index);
	extender->setGeometry(option.rect);
}


//make the view re-ask for sizeHint() and redisplay items with their new size
void KExtendableItemDelegate::scheduleUpdateViewLayout() const
{
	QAbstractItemView *aiv = qobject_cast<QAbstractItemView *>(parent());
	//prevent crashes during destruction of the view
	if (aiv)
		//dirty hack to call aiv's protected scheduleDelayedItemsLayout()
		aiv->setRootIndex(aiv->rootIndex());
}

#include "kextendableitemdelegate.moc"
