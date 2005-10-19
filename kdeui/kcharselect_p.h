/* This file is part of the KDE libraries

   Copyright (C) 2005 Joseph Wenninger <jowenn@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#ifndef KCHARSELECTITEMMODEL
#define KCHARSELECTITEMMODEL

#include <QAbstractTableModel>
#include <QAbstractItemView>
#include <QFont>
#include <QApplication>
#include <klocale.h>

// NO D-Pointer needed, private internal class, no public API

class KCharSelectItemModel: public QAbstractTableModel {
	Q_OBJECT
public: 
	KCharSelectItemModel(int tableNum,const QString& fontName, QObject *parent):QAbstractTableModel(parent),m_tableNum(tableNum),m_fontName(fontName){}

	enum internalRoles {CharacterRole=Qt::UserRole};	
	int rowCount(const QModelIndex &parent) const { return 8;}
	int columnCount(const QModelIndex &parent) const { return 32;}
	
	void setFont(const QString& fontName) {m_fontName=fontName;m_font=QFont(fontName); reset();}
	Qt::ItemFlags flags(const QModelIndex &) const {
		return (Qt::ItemIsSelectable|Qt::ItemIsEnabled);
	}
	QVariant data(const QModelIndex &index, int role) const {
		if (!index.isValid())
			return QVariant();
		else if (role == Qt::ToolTipRole) {
			const ushort uni = m_tableNum * 256 + columnCount(QModelIndex())*index.row() + index.column();;
			QString s;
			s.sprintf( "%04X", uint( uni ) );
			QString result=i18n( "Character","<qt><font size=\"+4\" face=\"%1\">%2</font><br>Unicode code point: U+%3<br>(In decimal: %4)<br>(Character: %5)</qt>" ).arg( m_fontName ).arg( QChar( uni ) ).arg( s ).arg( uni ).arg( QChar( uni ) );
			return QVariant(result);
		} else if (role == Qt::TextAlignmentRole)
			return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
		else if (role == Qt::DisplayRole)
			return QVariant(QChar((unsigned short) (m_tableNum * 256 + index.row() * columnCount(QModelIndex())+index.column())));
		else if (role == Qt::BackgroundColorRole) {
			QFontMetrics fm = QFontMetrics( m_font );
			if( fm.inFont(QChar((const ushort)(m_tableNum * 256 + columnCount(QModelIndex())*index.row() + index.column()))))
				return QVariant(qApp->palette().base().color());
			else
				return QVariant(qApp->palette().button().color());
		} else if (role == Qt::FontRole) return QVariant(m_font);
		else if (role==CharacterRole) return QVariant(QChar((const ushort)(m_tableNum * 256 + columnCount(QModelIndex())*index.row() + index.column())));
		return QVariant();
	}
private:
	int m_tableNum;
	QString m_fontName;
	QFont m_font;
};
#endif
