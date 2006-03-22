/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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
#ifndef Phonon_VIDEOWIDGET_H
#define Phonon_VIDEOWIDGET_H

#include <QWidget>
#include <kdelibs_export.h>
#include "../abstractvideooutput.h"

class QString;

namespace Phonon
{
class AbstractVideoOutput;

namespace Ui
{
	namespace Ifaces
	{
		class VideoWidget;
	}
	class VideoWidgetPrivate;
	/**
	 * \short Widget to display video.
	 *
	 * This widget shows the video signal and provides an object that can be
	 * plugged into the VideoPath.
	 *
	 * \code
	 * VideoWidget* vwidget = new VideoWidget( this );
	 * videoPath->addOutput( vwidget );
	 * \endcode
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class PHONON_EXPORT VideoWidget : public QWidget, public Phonon::AbstractVideoOutput
	{
		K_DECLARE_PRIVATE( VideoWidget )
		Q_OBJECT
		/**
		 * This property holds whether the video is shown using the complete
		 * screen.
		 *
		 * The property differs from QWidget::fullScreen in that it is
		 * writeable.
		 *
		 * By default the widget is not shown in fullScreen.
		 */
		Q_PROPERTY( bool fullScreen READ isFullScreen WRITE setFullScreen )
		public:
			/**
			 * Constructs a new video widget with a \p parent.
			 */
			VideoWidget( QWidget* parent = 0 );

		public Q_SLOTS:
			void setFullScreen( bool fullscreen );

			/**
			 * Convenience slot, calling setFullScreen( false )
			 */
			void exitFullScreen();

			/**
			 * Convenience slot, calling setFullScreen( true )
			 */
			void enterFullScreen();

		protected:
			/**
			 * \internal
			 *
			 * Constructs a new video widget with private data pointer \p d and
			 * a \p parent.
			 */
			VideoWidget( VideoWidgetPrivate& d, QWidget* parent );
			
			/**
			 * \copydoc Phonon::AbstractVideoOutput::setupIface
			 */
			void setupIface();

		private:
			void init();
			Ui::Ifaces::VideoWidget* iface();
	};
}} //namespace Phonon::Ui

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_VIDEOWIDGET_H
