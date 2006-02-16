/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
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
 **/

#include "treecombobox.h"

#include <qheaderview.h>
#include <qstandarditemmodel.h>
#include <qtreeview.h>

TreeComboBox::TreeComboBox(QWidget *parent, const char *name)
	: QComboBox(parent)
{
  setObjectName( name );

	QTreeView   *view = new QTreeView(this);
	view->header()->hide();
	view->setRootIsDecorated(false);
	setView(view);
}

void TreeComboBox::insertItem(const QIcon& icon, const QString& text, bool oneBlock)
{
	QStringList	path;
	if (oneBlock)
		path = QStringList(text);
	else
		path = text.split('/', QString::SkipEmptyParts);
	int	depth = path.count()-1;

	if (depth == 0)
	{
		model()->insertRow(model()->rowCount());
		QModelIndex	index = model()->index(model()->rowCount()-1, 0);
		model()->setData(index, icon, Qt::DecorationRole);
		model()->setData(index, text, Qt::DisplayRole);
	}
	else
	{
		// Find parent item
		QString	parentStr = text.left(text.length()-path[depth].length()-1);
		QModelIndexList	matches = model()->match(model()->index(0, 0), Qt::DisplayRole, parentStr, 1, Qt::MatchExactly);

		QModelIndex	parentIndex;
		if (matches.isEmpty())
		{
			// parent not found, add parent first into model
			model()->insertRow(model()->rowCount());
			parentIndex = model()->index(model()->rowCount()-1, 0);
			model()->setData(parentIndex, parentStr, Qt::DisplayRole);
			model()->insertColumn(0, parentIndex);
			static_cast<QTreeView*>(view())->setExpanded(parentIndex, true);
		}
		else
			parentIndex = matches.first();

                // TODO: QComboBox doesn't let us select these items.
		model()->insertRow(model()->rowCount(parentIndex), parentIndex);
		QModelIndex	index = model()->index(model()->rowCount(parentIndex)-1, 0, parentIndex);
		model()->setData(index, icon, Qt::DecorationRole);
		model()->setData(index, text, Qt::DisplayRole);
	}
}
