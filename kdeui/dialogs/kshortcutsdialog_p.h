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

#include "kgesture.h"

#include <kextendableitemdelegate.h>
#include <QKeySequence>
#include <QModelIndex>
#include <QMetaType>

class QTreeWidget;
class QTreeWidgetItem;
class QRadioButton;
class KKeySequenceWidget;


class KShortcutsEditorDelegate : public KExtendableItemDelegate
{
	Q_OBJECT
public:
	KShortcutsEditorDelegate(QTreeWidget *parent, bool allowLetterShortcuts);
	//reimplemented to have some extra height
	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
Q_SIGNALS:
	void shortcutChanged(QVariant, const QModelIndex &);
public Q_SLOTS:
	void hiddenBySearchLine(QTreeWidgetItem *, bool);
protected:
	virtual bool eventFilter(QObject *, QEvent *);
private:
	mutable QModelIndex m_editingIndex;
	bool m_allowLetterShortcuts;
	QWidget *m_editor;
private Q_SLOTS:
	void itemActivated(QModelIndex index);
	void keySequenceChanged(const QKeySequence &);
	void shapeGestureChanged(const KShapeGesture &);
	void rockerGestureChanged(const KRockerGesture &);
};


class TabConnectedWidget : public QWidget
{
	Q_OBJECT
public:
	TabConnectedWidget(QWidget *parent)
	 : QWidget(parent) {}
protected:
	void paintEvent(QPaintEvent *pe);
};


class ShortcutEditWidget : public TabConnectedWidget
{
	Q_OBJECT
public:
	ShortcutEditWidget(QWidget *viewport, const QKeySequence &defaultSeq, const QKeySequence &activeSeq,
	                   bool allowLetterShortcuts);
Q_SIGNALS:
	void keySequenceChanged(const QKeySequence &);
private Q_SLOTS:
	void defaultToggled(bool);
	void setCustom(const QKeySequence &);
private:
	QKeySequence m_defaultKeySequence;
	QRadioButton *m_defaultRadio;
	QRadioButton *m_customRadio;
	KKeySequenceWidget *m_customEditor;
	bool m_isUpdating;
};

Q_DECLARE_METATYPE(KShapeGesture)
Q_DECLARE_METATYPE(KRockerGesture)


#endif /* KSHORTCUTSDIALOG_P_H */

