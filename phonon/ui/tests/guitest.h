/*  This file is part of the KDE project
    Copyright (C) 2004-2006 Matthias Kretz <kretz@kde.org>

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

#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <QWidget>
#include <phonon/phononnamespace.h>
#include <QList>
#include <QFrame>
#include <QPoint>
#include <QLine>

class QLabel;
class QString;
class QPushButton;
class QFrame;
namespace Phonon
{
	class MediaObject;
	class AudioPath;
	class AudioOutput;
	class VideoPath;
	class VideoOutput;
	class VideoWidget;
	class SeekSlider;
	class VolumeSlider;
}

using namespace Phonon;

class PathWidget;
class OutputWidget;

class ProducerWidget : public QFrame
{
	Q_OBJECT
	public:
		ProducerWidget( QWidget *parent = 0 );
		bool connectPath( PathWidget* w );

	private Q_SLOTS:
		void tick( qint64 );
		void stateChanged( Phonon::State );
		void length( qint64 );
		void loadFile( const QString& );
		void slotFinished();
		void slotAboutToFinish( qint32 remaining );
		void updateMetaData();

	private:
		SeekSlider *m_seekslider;
		QLabel *m_statelabel, *m_totaltime, *m_currenttime, *m_remainingtime;
		QLabel *m_metaDataLabel;
		QPushButton *m_pause, *m_play, *m_stop;
		MediaObject *m_media;
		qint64 m_length;
		QList<AudioPath*> m_audioPaths;
};

class PathWidget : public QFrame
{
	Q_OBJECT
	public:
		PathWidget( QWidget *parent = 0 );

		AudioPath *path() const { return m_path; }
		bool connectOutput( OutputWidget* w );

	private:
		AudioPath *m_path;
};

class OutputWidget : public QFrame
{
	Q_OBJECT
	public:
		OutputWidget( QWidget *parent = 0 );

		AudioOutput *output() const { return m_output; }

	private:
		VolumeSlider *m_volslider;
		AudioOutput *m_output;
};

class ConnectionWidget : public QFrame
{
	Q_OBJECT
	public:
		ConnectionWidget( QWidget *parent = 0 );

		void addConnection( QWidget *, QWidget * );

	signals:
		void madeConnection( const QPoint &, const QPoint & );

	protected:
		virtual void mousePressEvent( QMouseEvent * );
		virtual void mouseReleaseEvent( QMouseEvent * );
		virtual void mouseMoveEvent( QMouseEvent * );
		virtual void paintEvent( QPaintEvent *pe );

	private:
		QPoint m_pressPos;
		QLine m_currentLine;
		struct WidgetConnection
		{
			WidgetConnection( QWidget *_a, QWidget *_b ) : a( _a ), b( _b ) {}
			QWidget *a;
			QWidget *b;
		};
		QList<WidgetConnection> m_lines;
};

class MainWidget : public QWidget
{
	Q_OBJECT
	public:
		MainWidget( QWidget *parent = 0 );

	private slots:
		void addProducer();
		void addPath();
		void addOutput();
		void madeConnection1( const QPoint &, const QPoint & );
		void madeConnection2( const QPoint &, const QPoint & );

	private:
		QList<ProducerWidget*> m_producers;
		QList<PathWidget*> m_paths;
		QList<OutputWidget*> m_outputs;

		QFrame *m_producerFrame;
		QFrame *m_pathFrame;
		QFrame *m_outputFrame;
		ConnectionWidget *m_connectionFrame1;
		ConnectionWidget *m_connectionFrame2;
};

#endif // TESTWIDGET_H
// vim: sw=4 ts=4 noet
