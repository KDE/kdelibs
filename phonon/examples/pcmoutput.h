/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#ifndef PCMOUTPUT_H
#define PCMOUTPUT_H

#include <QWidget>

class QByteArray;
class QLabel;
namespace Phonon { class ByteStream; }

class PcmPlayer : public QWidget
{
	Q_OBJECT
	public:
		PcmPlayer( QWidget* parent = 0 );

	private slots:
		void setFrequency( int f );
		void sendData();

	private:
		QByteArray wavHeader() const;
		QByteArray pcmBlock( int freq ) const;
		int m_frequency;
		Phonon::ByteStream* m_stream;
		QLabel* m_fLabel;
};

#endif // PCMOUTPUT_H
