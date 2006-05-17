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

#ifndef PHONON_VISUALIZATION_H
#define PHONON_VISUALIZATION_H

#include "base.h"
#include "phonondefs.h"
#include <QObject>
#include "basedestructionhandler.h"

namespace Phonon
{
	class VisualizationPrivate;
	class AudioPath;
	class AbstractVideoOutput;
	class VisualizationDescription;

	namespace Ifaces
	{
		class Visualization;
	}

/**
 * \short A class to create visual effects from an audio signal.
 *
 * This class is used to define how an audio signal from an AudioPath object
 * should be visualized. What visualization effects are available depends solely
 * on the backend. You can list the available visualization effects using
 * BackendCapabilities::availableVisualizationEffects().
 *
 * The following example code takes the first effect from the list and uses that
 * to display a visualization on a new VideoWidget.
 * \code
 * QList<VisualizationEffect> list = BackendCapabilities::availableVisualizationEffects();
 * if( list.size() > 0 )
 * {
 *   VideoWidget* visWidget = new VideoWidget( parent );
 *   Visualization* vis = new Visualization( visWidget );
 *   vis->setAudioPath( audioPath );
 *   vis->setVideoOutput( visWidget );
 *   vis->setVisualization( list.first() );
 * }
 * \endcode
 *
 * \author Matthias Kretz <kretz@kde.org>
 * \see AudioDataOutput
 * \see BackendCapabilities::availableVisualizationEffects()
 */
class PHONONCORE_EXPORT Visualization : public QObject, public Base, private BaseDestructionHandler
{
	Q_OBJECT
	K_DECLARE_PRIVATE( Visualization )
	PHONON_OBJECT( Visualization )
	Q_PROPERTY( VisualizationDescription visualization READ visualization WRITE setVisualization )

	public:
		~Visualization();

		AudioPath* audioPath() const;
		void setAudioPath( AudioPath* audioPath );

		AbstractVideoOutput* videoOutput() const;
		void setVideoOutput( AbstractVideoOutput* output );

		VisualizationDescription visualization() const;
		void setVisualization( const VisualizationDescription& newVisualization );

	private:
		/**
		 * \internal
		 */
		void phononObjectDestroyed( Base* );
};

} // namespace Phonon

#endif // PHONON_VISUALIZATION_H
// vim: sw=4 ts=4 noet tw=80
