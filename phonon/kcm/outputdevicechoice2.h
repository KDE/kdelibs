/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

*/

#ifndef OUTPUTDEVICECHOICE2_H_STUPID_UIC
#define OUTPUTDEVICECHOICE2_H_STUPID_UIC

#include "ui_outputdevicechoice2.h"
#include <phonon/namedescriptiontuplemodel.h>

class OutputDeviceChoice : public QWidget, private Ui::OutputDeviceChoice
{
	Q_OBJECT
	public:
		OutputDeviceChoice( QWidget* parent = 0 );
		void load();
		void save();
		void defaults();

	private Q_SLOTS:
		void on_notificationCombo_activated();
		void on_musicCombo_activated();
		//void on_videoCombo_activated();
		void on_communicationCombo_activated();

	private:
		Phonon::NameDescriptionTupleModel m_notificationModel, m_musicModel,
			m_videoModel, m_communicationModel;
		Phonon::NameDescriptionTupleModel m_notificationModel2, m_musicModel2,
			m_videoModel2, m_communicationModel2;
};

#endif // OUTPUTDEVICECHOICE2_H_STUPID_UIC
