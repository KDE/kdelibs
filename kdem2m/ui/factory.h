/*  This file is part of the KDE project
    Copyright (C) 2004-2005 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef Kdem2m_Ui_FACTORY_H
#define Kdem2m_Ui_FACTORY_H

#include <QObject>

class KURL;

namespace Kdem2m
{
namespace Ifaces
{
	namespace Ui
	{
		class Backend;
	}
}
namespace Ui
{

/**
 * \brief Factory to access the GUI dependend part of the Backend.
 *
 * This class is used internally to map the backends functionality to the API.
 *
 * \remarks
 * Extensions to the existing functionality can be added by adding a new interface e.g.
 * BackendV2 and creating a BackendV2 instance when the Backend instance is
 * created.
 *
 * \author Matthias Kretz <kretz@kde.org>
 * \since 4.0
 * \internal
 */
class Factory : public QObject
{
	Q_OBJECT
	public:
		/**
		 * Returns a pointer to the factory.
		 * Use this function to get an instance of Factory.
		 *
		 * @return a pointer to the factory. If no factory exists until now then
		 * one is created
		 */
		static Factory* self();

		Ifaces::Ui::VideoWidget* createVideoWidget( QWidget* parent = 0 );

		const Ifaces::Ui::Backend* Factory::backend() const;

	signals:
		/**
		 * This signal is emitted when the user changes the backend. At that
		 * point all references to Ifaces objects need to be freed.
		 */
		void deleteYourObjects();

		/**
		 * After you got a deleteYourObjects() signal the backend is changed
		 * internally. Then you will receive this signal, and only then should
		 * you reconstruct all your objects again. This time they will
		 * internally use the new backend.
		 */
		void recreateObjects();

		void backendChanged();

	protected:
		Factory();
		~Factory();

	protected:
		template<class T> T* Factory::registerObject( T* o );

	private slots:
		void deleteNow();

	private:
		static Factory * m_self;
		class Private;
		Private * d;
};
}} // namespace Kdem2m::Ui

#endif // Kdem2m_Ui_FACTORY_H
// vim: sw=4 ts=4 tw=80 noet
