    /*

    Copyright (C) 2001 Stefan Westerfeld
                       stefan@space.twc.de

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

#include "common.h"
#include <qobject.h>

class KArtsFloatWatchPrivate;
namespace Arts { class KFloatWatchProxy_impl; }

/**
 * KArtsFloatWatch offers an easy way to watch aRts streams via Qt signals.
 * For instance, if you have an object of the following type:
 *
 * <pre>
 * interface StereoVolumeControl : StereoEffect {
 *   attribute float scaleFactor;
 *   readonly attribute float currentVolumeLeft;
 *   readonly attribute float currentVolumeRight;
 * };
 * </pre>
 *
 * and you want to get notified when scaleFactor changes, you could do it
 * like this:
 *
 * <pre>
 *   StereoVolumeControl stereoVolumeControl = ...;
 *   KArtsFloatWatch *w = new KArtsFloatWatch(stereoVolumeControl, "scaleFactor_changed", this);
 *   connect(w, SIGNAL(valueChanged(float)), this, SLOT(setValue(float)));
 * </pre>
 */
class KArtsFloatWatch : public QObject {
	Q_OBJECT
private:
	KArtsFloatWatchPrivate *d;
	friend class Arts::KFloatWatchProxy_impl;

	/**
	 * called by the proxy (internal)
	 */
	void change(float newValue);

public:
	/**
	 * Constructor.
	 *
	 * @param object  the aRts object that should be watched
	 * @param stream  the aRts stream that should be watched
	 * @param parent  the parent Qt object
	 * @param name    the Qt object name of this object
	 */
	KArtsFloatWatch(Arts::Object object, const char *stream, QObject *parent = 0, const char *name = 0);

	/**
	 * Destructor
	 */
	~KArtsFloatWatch();

signals:
	/**
	 * this signal will be emitted with values of the aRts stream
	 */
	void valueChanged(float newValue);
};
