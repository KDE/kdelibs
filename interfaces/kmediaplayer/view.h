// Copyright (C) 2002 Neil Stevens <neil@qualityassistant.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// 
// Except as contained in this notice, the name(s) of the author(s) shall not be
// used in advertising or otherwise to promote the sale, use or other dealings
// in this Software without prior written authorization from the author(s).

#ifndef KMEDIAPLAYERVIEW_H
#define KMEDIAPLAYERVIEW_H

#include <qwidget.h>

namespace KMediaPlayer
{

class View : public QWidget
{
Q_OBJECT

public:
	View(QWidget *parent, const char *name);
	virtual ~View(void);

	enum Button
	{
		Play = 1,
		Stop = 2,
		Pause = 4,
		Seeker = 8,
		All = 255
	};

	int buttons(void);
public slots:
	void setButtons(int);

	bool button(int);
	void showButton(int);
	void hideButton(int);
	void toggleButton(int);

signals:
	void buttonsChanged(int);

private:
	int currentButtons;

	struct Data;
	Data *d;
};

}

#endif
