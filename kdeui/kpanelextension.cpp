/*****************************************************************

Copyright (c) 2000 Matthias Elter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <kconfig.h>

#include "kpanelextension.h"
#include "kpanelextension.moc"

KPanelExtension::KPanelExtension(const QString& configFile, Type type,
				 int actions, QWidget *parent, const char *name)
  : QFrame(parent, name)
  , _type(type)
  , _pos( Top )
  , _config(0)
  , _actions(actions)
{
    setFrameStyle(NoFrame);
    _config = new KConfig(configFile);
}

KPanelExtension::~KPanelExtension()
{
    delete _config;
}

void KPanelExtension::slotSetPosition(Position p)
{
    _pos = p;
    positionChange(p);
    QResizeEvent e(size(), size());
    resizeEvent(&e);
}

void KPanelExtension::action( Action a )
{
    if ( (a & About) != 0 )
	about();
    if ( (a & Help) != 0 )
	help();
    if ( (a & Preferences) != 0 )
	preferences();
    if ( (a & ReportBug) != 0 )
	reportBug();
}

Qt::Orientation KPanelExtension::orientation()
{
    if (_pos == Left || _pos == Right)
	return Vertical;
    else
	return Horizontal;
}
