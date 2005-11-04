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

#include <qcheckbox.h>
#include <qlayout.h>
#include <khbox.h>
#include <qsettings.h>
#include <qcolor.h>
#include <q3groupbox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcolorbutton.h>
#include <kdemacros.h>

#include "plastikconf.h"

extern "C"
{
	KDE_EXPORT QWidget* allocate_kstyle_config(QWidget* parent)
	{
		KGlobal::locale()->insertCatalog("kstyle_plastik_config");
		return new PlastikStyleConfig(parent);
	}
}

PlastikStyleConfig::PlastikStyleConfig(QWidget* parent): QWidget(parent)
{
	//Should have no margins here, the dialog provides them
	QVBoxLayout* layout = new QVBoxLayout(this, 0, 0);
	KGlobal::locale()->insertCatalog("kstyle_plastik_config");

// 	scrollBarLines = new QCheckBox(i18n("Scrollbar handle lines"), this);
	animateProgressBar = new QCheckBox(i18n("Animate progress bars"), this);
	drawToolBarSeparator = new QCheckBox(i18n("Draw toolbar separator"), this);
	drawToolBarItemSeparator = new QCheckBox(i18n("Draw toolbar item separators"), this);
// 	drawFocusRect = new QCheckBox(i18n("Draw focus rectangles"), this);
	drawTriangularExpander = new QCheckBox(i18n("Triangular tree expander"), this);
	inputFocusHighlight = new QCheckBox(i18n("Highlight focused text input fields"), this);

    customFocusHighlightColor = new QCheckBox(i18n("Custom text input highlight color:"), this);
    KHBox *hbox1 = new KHBox(this);
    hbox1->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
    focusHighlightColor = new KColorButton(hbox1);

    customOverHighlightColor = new QCheckBox(i18n("Custom mouseover highlight color:"), this);
    KHBox *hbox2 = new KHBox(this);
    hbox2->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
    overHighlightColor = new KColorButton(hbox2);

    customCheckMarkColor = new QCheckBox(i18n("Custom checkmark color:"), this);
    KHBox *hbox3 = new KHBox(this);
    hbox3->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
    checkMarkColor = new KColorButton(hbox3);

// 	layout->add(scrollBarLines);
	layout->add(animateProgressBar);
	layout->add(drawToolBarSeparator);
	layout->add(drawToolBarItemSeparator);
// 	layout->add(drawFocusRect);
	layout->add(drawTriangularExpander);
	layout->add(inputFocusHighlight);
    layout->add(customFocusHighlightColor);
	layout->add(hbox1);
    layout->add(customOverHighlightColor);
    layout->add(hbox2);
	layout->add(customCheckMarkColor);
	layout->add(hbox3);
	layout->addStretch(1);

	QSettings s;
// 	origScrollBarLines = s.readBoolEntry("/plastikstyle/Settings/scrollBarLines", false);
// 	scrollBarLines->setChecked(origScrollBarLines);
	origAnimProgressBar = s.readBoolEntry("/plastikstyle/Settings/animateProgressBar", false);
	animateProgressBar->setChecked(origAnimProgressBar);
	origDrawToolBarSeparator = s.readBoolEntry("/plastikstyle/Settings/drawToolBarSeparator", true);
	drawToolBarSeparator->setChecked(origDrawToolBarSeparator);
	origDrawToolBarItemSeparator = s.readBoolEntry("/plastikstyle/Settings/drawToolBarItemSeparator", true);
	drawToolBarItemSeparator->setChecked(origDrawToolBarItemSeparator);
// 	origDrawFocusRect = s.readBoolEntry("/plastikstyle/Settings/drawFocusRect", true);
// 	drawFocusRect->setChecked(origDrawFocusRect);
	origDrawTriangularExpander = s.readBoolEntry("/plastikstyle/Settings/drawTriangularExpander", false);
	drawTriangularExpander->setChecked(origDrawTriangularExpander);
	origInputFocusHighlight = s.readBoolEntry("/plastikstyle/Settings/inputFocusHighlight", true);
	inputFocusHighlight->setChecked(origInputFocusHighlight);
	origCustomOverHighlightColor = s.readBoolEntry("/plastikstyle/Settings/customOverHighlightColor", false);
	customOverHighlightColor->setChecked(origCustomOverHighlightColor);
	origOverHighlightColor = s.readEntry("/plastikstyle/Settings/overHighlightColor", "black");
	overHighlightColor->setColor(origOverHighlightColor);
	origCustomFocusHighlightColor = s.readBoolEntry("/plastikstyle/Settings/customFocusHighlightColor", false);
	customFocusHighlightColor->setChecked(origCustomFocusHighlightColor);
	origFocusHighlightColor = s.readEntry("/plastikstyle/Settings/focusHighlightColor", "black");
	focusHighlightColor->setColor(origFocusHighlightColor);
	origCustomCheckMarkColor = s.readBoolEntry("/plastikstyle/Settings/customCheckMarkColor", false);
	customCheckMarkColor->setChecked(origCustomCheckMarkColor);
	origCheckMarkColor = s.readEntry("/plastikstyle/Settings/checkMarkColor", "black");
	checkMarkColor->setColor(origCheckMarkColor);

// 	connect(scrollBarLines, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(animateProgressBar, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(drawToolBarSeparator, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(drawToolBarItemSeparator, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
// 	connect(drawFocusRect, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(drawTriangularExpander, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(inputFocusHighlight, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(customOverHighlightColor, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(overHighlightColor, SIGNAL( changed(QColor) ), SLOT( updateChanged() ) );
	connect(customFocusHighlightColor, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(focusHighlightColor, SIGNAL( changed(QColor) ), SLOT( updateChanged() ) );
	connect(customCheckMarkColor, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(checkMarkColor, SIGNAL( changed(QColor) ), SLOT( updateChanged() ) );
	if ( customOverHighlightColor->isChecked() )
	 overHighlightColor->setEnabled(true);
	else
	 overHighlightColor->setEnabled(false);
	if ( customFocusHighlightColor->isChecked() )
	 focusHighlightColor->setEnabled(true);
	else
	 focusHighlightColor->setEnabled(false);
	if ( customCheckMarkColor->isChecked() )
	 checkMarkColor->setEnabled(true);
	else
	 checkMarkColor->setEnabled(false);
}

PlastikStyleConfig::~PlastikStyleConfig()
{
}


void PlastikStyleConfig::save()
{
	QSettings s;
// 	s.writeEntry("/plastikstyle/Settings/scrollBarLines", scrollBarLines->isChecked());
	s.writeEntry("/plastikstyle/Settings/animateProgressBar", animateProgressBar->isChecked());
	s.writeEntry("/plastikstyle/Settings/drawToolBarSeparator", drawToolBarSeparator->isChecked());
	s.writeEntry("/plastikstyle/Settings/drawToolBarItemSeparator", drawToolBarItemSeparator->isChecked());
// 	s.writeEntry("/plastikstyle/Settings/drawFocusRect", drawFocusRect->isChecked());
	s.writeEntry("/plastikstyle/Settings/drawTriangularExpander", drawTriangularExpander->isChecked());
	s.writeEntry("/plastikstyle/Settings/inputFocusHighlight", inputFocusHighlight->isChecked());
	s.writeEntry("/plastikstyle/Settings/customOverHighlightColor", customOverHighlightColor->isChecked());
	s.writeEntry("/plastikstyle/Settings/overHighlightColor", QColor(overHighlightColor->color()).name());
	s.writeEntry("/plastikstyle/Settings/customFocusHighlightColor", customFocusHighlightColor->isChecked());
	s.writeEntry("/plastikstyle/Settings/focusHighlightColor", QColor(focusHighlightColor->color()).name());
	s.writeEntry("/plastikstyle/Settings/customCheckMarkColor", customCheckMarkColor->isChecked());
	s.writeEntry("/plastikstyle/Settings/checkMarkColor", QColor(checkMarkColor->color()).name());
}

void PlastikStyleConfig::defaults()
{
// 	scrollBarLines->setChecked(false);
	animateProgressBar->setChecked(false);
	drawToolBarSeparator->setChecked(true);
	drawToolBarItemSeparator->setChecked(true);
// 	drawFocusRect->setChecked(true);
	drawTriangularExpander->setChecked(false);
	inputFocusHighlight->setChecked(true);
	customOverHighlightColor->setChecked(false);
	overHighlightColor->setColor("black");
	customFocusHighlightColor->setChecked(false);
	focusHighlightColor->setColor("black");
	customCheckMarkColor->setChecked(false);
	checkMarkColor->setColor("black");
	//updateChanged would be done by setChecked already
}

void PlastikStyleConfig::updateChanged()
{
	if ( customOverHighlightColor->isChecked() )
	 overHighlightColor->setEnabled(true);
	else
	 overHighlightColor->setEnabled(false);
	if ( customFocusHighlightColor->isChecked() )
	 focusHighlightColor->setEnabled(true);
	else
	 focusHighlightColor->setEnabled(false);
	if ( customCheckMarkColor->isChecked() )
	 checkMarkColor->setEnabled(true);
	else
	 checkMarkColor->setEnabled(false);

	if (/*(scrollBarLines->isChecked() == origScrollBarLines) &&*/
	     (animateProgressBar->isChecked() == origAnimProgressBar) &&
	      (drawToolBarSeparator->isChecked() == origDrawToolBarSeparator) &&
	       (drawToolBarItemSeparator->isChecked() == origDrawToolBarItemSeparator) &&
// 	        (drawFocusRect->isChecked() == origDrawFocusRect) &&
		  (drawTriangularExpander->isChecked() == origDrawTriangularExpander) &&
		   (inputFocusHighlight->isChecked() == origInputFocusHighlight) &&
		    (customOverHighlightColor->isChecked() == origCustomOverHighlightColor) &&
		     (overHighlightColor->color() == origOverHighlightColor) &&
		      (customFocusHighlightColor->isChecked() == origCustomFocusHighlightColor) &&
		       (focusHighlightColor->color() == origFocusHighlightColor) &&
			(customCheckMarkColor->isChecked() == origCustomCheckMarkColor) &&
			 (checkMarkColor->color() == origCheckMarkColor)
		    )
		emit changed(false);
	else
		emit changed(true);
}

#include "plastikconf.moc"
