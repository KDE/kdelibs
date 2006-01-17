/*
Copyright (C) 2003 Sandro Giessl <ceebx@users.sourceforge.net>

based on the Keramik configuration dialog:
Copyright (c) 2003 Maksim Orlovich <maksim.orlovich@kdemail.net>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#ifndef PLASTIK_CONF_H
#define PLASTIK_CONF_H

class QCheckBox;

class PlastikStyleConfig: public QWidget
{
	Q_OBJECT
public:
	PlastikStyleConfig(QWidget* parent);
	~PlastikStyleConfig();

	//This signal and the next two slots are the plugin
	//page interface
Q_SIGNALS:
	void changed(bool);

public Q_SLOTS:
	void save();
	void defaults();

	//Everything below this is internal.
protected Q_SLOTS:
	void updateChanged();

protected:
	//We store settings directly in widgets to
	//avoid the hassle of sync'ing things
// 	QCheckBox*       scrollBarLines;
	QCheckBox* animateProgressBar;
	QCheckBox* drawToolBarSeparator;
	QCheckBox*       drawToolBarItemSeparator;
// 	QCheckBox*       drawFocusRect;
	QCheckBox*       drawTriangularExpander;
	QCheckBox*       inputFocusHighlight;
	QCheckBox*       customOverHighlightColor;
	KColorButton*    overHighlightColor;
	QCheckBox*       customFocusHighlightColor;
	KColorButton*    focusHighlightColor;
	QCheckBox*       customCheckMarkColor;
	KColorButton*    checkMarkColor;

	//Original settings, for accurate dirtiness tracking
// 	bool       origScrollBarLines;
	bool       origAnimProgressBar;
	bool       origDrawToolBarSeparator;
	bool       origDrawToolBarItemSeparator;
// 	bool       origDrawFocusRect;
	bool       origDrawTriangularExpander;
	bool       origInputFocusHighlight;
	bool       origCustomOverHighlightColor;
	QColor     origOverHighlightColor;
	bool       origCustomFocusHighlightColor;
	QColor     origFocusHighlightColor;
	bool       origCustomCheckMarkColor;
	QColor     origCheckMarkColor;
};

#endif
