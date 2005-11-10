/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kpimagepage.h"
#include "imageposition.h"
#include "imagepreview.h"
#include "driver.h"

#include <qcombobox.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <q3groupbox.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>
#include <knuminput.h>
#include <kseparator.h>
#include <kstandarddirs.h>

KPImagePage::KPImagePage(DrMain *driver, QWidget *parent)
    : KPrintDialogPage(parent)
{
	//WhatsThis strings.... (added by pfeifle@kde.org)
	QString whatsThisBrightnessImagePage = i18n( " <qt> "
			" <p><b>Brightness:</b> Slider to control the brightness value of all colors used.</p>"
			" <p> The brightness value can range from 0 to 200. Values greater than 100 will "
			" lighten the print. Values less than 100 will darken the print. </p> "
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o brightness=...      # use range from \"0\" to \"200\" "
			" </pre> "
			" </p> "
			" </qt>" );

	QString whatsThisHueImagePage = i18n( " <qt> "
			" <p><b>Hue (Tint):</b> Slider to control the hue value for color rotation.</p>"
			" <p> The hue value is a number from -360 to 360 and represents the color hue rotation. "
			" The following table summarizes the change you will see for the base colors: "
			" <center> "
			" <table border=\"1\" width=\"70%\"> "
			" <tr><th><b>Original</b></th> <th><b>hue=-45</b></th> <th><b>hue=45</b></th>   </tr> "
			" <tr><td>Red</td>             <td>Purple</td>         <td>Yellow-orange</td>   </tr> "
			" <tr><td>Green</td>           <td>Yellow-green</td>   <td>Blue-green</td>      </tr> "
			" <tr><td>Yellow</td>          <td>Orange</td>         <td>Green-yellow</td>    </tr> "
			" <tr><td>Blue</td>            <td>Sky-blue</td>       <td>Purple</td>          </tr> "
			" <tr><td>Magenta</td>         <td>Indigo</td>         <td>Crimson</td>         </tr> "
			" <tr><td>Cyan</td>            <td>Blue-green</td>     <td>Light-navy-blue</td> </tr> "
			" </table> "
			" </center> "
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o hue=...     # use range from \"-360\" to \"360\" "
			" </pre>"
			" </p> "
			" </qt>" );

	QString whatsThisSaturationImagePage = i18n( " <qt> "
			" <p><b>Saturation:</b> Slider to control the saturation value for all colors used.</p>"
			" <p> The saturation value adjusts the saturation of the colors in an image, similar to "
			" the color knob on your television. The color saturation value.can range from 0 to 200."
			" On inkjet printers, a higher saturation value uses more ink. On laserjet printers, a "
			" higher saturation uses more toner. "
			" A color saturation of 0 produces a black-and-white print, while a value of 200 will "
			" make the colors extremely intense. </p>"
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o saturation=...      # use range from \"0\" to \"200\" "
			" </pre>"
			" </p> "
			" </qt>" );

	QString whatsThisGammaImagePage = i18n( " <qt> "
			" <p><b>Gamma:</b> Slider to control the gamma value for color correction.</p>"
			" <p> The gamma value can range from 1 to 3000. "
			" A gamma values greater than 1000 lightens the print. A gamma value less than 1000 "
			" darken the print. The default gamma is 1000. </p>"
			" <p><b>Note:</b></p> the gamma value adjustment is not visible in the thumbnail "
			" preview. </p> "
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o gamma=...      # use range from \"1\" to \"3000\" "
			" </pre>"
			" </p> "
			" </qt>" );

	QString whatsThisImagePage = i18n( " <qt> "
			" <p><b>Image Printing Options</b></p> "
			" <p>All options controlled on this page only apply to printing images. "
			" Most image file formats are supported. To name a few: JPEG, TIFF, PNG, GIF, "
			" PNM (PBM/PGM/PNM/PPM), Sun Raster, SGI RGB, Windows BMP."
			" Options to influence color output of image printouts are: "
			" <ul> "
			" <li> Brightness  </li>"
			" <li> Hue         </li>"
			" <li> Saturation  </li>"
			" <li> Gamma       </li>"
			" </ul> "
			" <p>For a more detailed explanation about Brightness, Hue, Saturation and Gamma settings, please "
			" look at the 'WhatsThis' items provided for these controls. "
			" </p> "
			" </p> "
			" </qt>" );

	QString whatsThisColorationPreviewImagePage = i18n( " <qt> "
			" <p><b>Coloration Preview Thumbnail</b></p> "
			" <p>The coloration preview thumbnail indicates change of image coloration by different settings. "
			" Options to influence output are: "
			" <ul> "
			" <li> Brightness  </li>"
			" <li> Hue (Tint)  </li>"
			" <li> Saturation  </li>"
			" <li> Gamma       </li>"
			" </ul> "
			" </p> "
			" <p>For a more detailed explanation about Brightness, Hue, Saturation and Gamma settings, please "
			" look at the 'WhatsThis' items provided for these controls. "
			" </p> "
			" </qt>" );

	QString whatsThisSizeImagePage = i18n( " <qt> "
			" <p><b>Image Size:</b> Dropdown menu to control the image size on the printed paper. Dropdown "
			" works in conjunction with slider below. Dropdown options are:.</p>"
			" <ul> "
			" <li> <b>Natural Image Size:</b> Image prints in its natural image size. If it does not fit onto "
			" one sheet, the printout will be spread across multiple sheets. Note, that the slider is disabled "
			" when selecting 'natural image size' in the dropdown menu. </li>"
			" <li> <b>Resolution (ppi):</b> The resolution value slider covers a number range from 1 to 1200. "
			" It specifies the resolution of the image in Pixels Per Inch (PPI). An image that is 3000x2400 pixels "
			" will print 10x8 inches at 300 pixels per inch, for example, but 5x4 inches at 600 pixels per inch."
			" If the specified resolution makes the image larger than the page, multiple pages will be printed. "
			" Resolution defaults to 72 ppi. "
			" </li>"
			" <li> <b>% of Page Size:</b> The percent value slider covers numbers from 1 to 800. It specifies the size "
			" in relation to the page (not the image). A scaling of 100 percent will fill the page as completely "
			" as the image aspect ratio allows (doing auto-rotation of the image as needed). "
			" A scaling of more than 100 will print the image across multiple "
			" sheets. A scaling of 200 percent will print on up to 4 pages. </li>"
			" Scaling in % of page size defaults to 100 %. "
			" <li> <b>% of Natural Image Size:</b> The percent value slider moves from 1 to 800. It specifies "
			" the printout size in relation "
			" to the natural image size. A scaling of 100 percent will print the image at its natural size, while a "
			" scaling of 50 percent will print the image at half its natural size. If the specified scaling makes "
			" the image larger than the page, multiple pages will be printed. "
			" Scaling in % of natural image size defaults to 100 %. "
			" </ul> "
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"     -o natural-scaling=...     # range in  %  is 1....800 "
			" <br> "
			"    -o scaling=...             # range in  %  is 1....800 "
			" <br> "
			"    -o ppi=...                 # range in ppi is 1...1200 "
			" </pre>"
			" </p> "
			" </qt>" );

	QString whatsThisPreviewPositionImagePage = i18n( " <qt> "
			" <p><b>Position Preview Thumbnail</b></p> "
			" <p>This position preview thumbnail indicates the position of the image on the paper sheet. "
			" <p>Click on horizontal and vertical radio buttons to move image alignment on paper around. Options are: "
			" <ul> "
			" <li> center      </li>"
			" <li> top         </li>"
			" <li> top-left    </li>"
			" <li> left        </li>"
			" <li> bottom-left </li>"
			" <li> bottom      </li>"
			" <li> bottom-right</li>"
			" <li> right       </li>"
			" <li> top-right   </li>"
			" </ul> "
			" </p> "
			" </qt>" );

	QString whatsThisResetButtonImagePage = i18n( " <qt> "
			" <p><b>Reset to Default Values</b> </p> "
			" <p> Reset all coloration settings to default values. Default values are: "
			" <ul> "
			" <li> Brightness: 100 </li>"
			" <li> Hue (Tint).   0 </li>"
			" <li> Saturation: 100 </li>"
			" <li> Gamma:     1000 </li>"
			" </ul> "
			" </p> "
			" </qt>" );

	QString whatsThisPositionImagePage = i18n( " <qt> "
			" <p><b>Image Positioning:</b></p> "
			" <p>Select a pair of radiobuttons to "
			" move image to the position you want on the paper printout. Default "
			" is 'center'. </p> "
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o position=...       # examples: \"top-left\" or \"bottom\" "
			" </pre>"
			" </p> "
			" </qt>" );

	setTitle(i18n("Image"));

	Q3GroupBox	*colorbox = new Q3GroupBox(0, Qt::Vertical, i18n("Color Settings"), this);
	  this->setWhatsThis(whatsThisImagePage);
	Q3GroupBox	*sizebox = new Q3GroupBox(0, Qt::Vertical, i18n("Image Size"), this);
	  sizebox->setWhatsThis(whatsThisSizeImagePage);
	Q3GroupBox	*positionbox = new Q3GroupBox(0, Qt::Vertical, i18n("Image Position"), this);
	  positionbox->setWhatsThis(whatsThisPositionImagePage);

	m_brightness = new KIntNumInput(100,colorbox);
	m_brightness->setLabel(i18n("&Brightness:"));
	m_brightness->setRange(0, 200, 20, true);
	  m_brightness->setWhatsThis(whatsThisBrightnessImagePage);

	m_hue = new KIntNumInput(m_brightness, 0,colorbox);
	m_hue->setLabel(i18n("&Hue (Color rotation):"));
	m_hue->setRange(-360, 360, 36, true);
	  m_hue->setWhatsThis(whatsThisHueImagePage);

	m_saturation = new KIntNumInput(m_brightness, 100,colorbox);
	m_saturation->setLabel(i18n("&Saturation:"));
	m_saturation->setRange(0, 200, 20, true);
	  m_saturation->setWhatsThis(whatsThisSaturationImagePage);

	m_gamma = new KIntNumInput(m_saturation, 1000,colorbox);
	m_gamma->setLabel(i18n("&Gamma (Color correction):"));
	m_gamma->setRange(1, 3000, 100, true);
	  m_gamma->setWhatsThis(whatsThisGammaImagePage);

	connect(m_brightness, SIGNAL(valueChanged(int)), SLOT(slotImageSettingsChanged()));
	connect(m_hue, SIGNAL(valueChanged(int)), SLOT(slotImageSettingsChanged()));
	connect(m_saturation, SIGNAL(valueChanged(int)), SLOT(slotImageSettingsChanged()));
	//connect(m_gamma, SIGNAL(valueChanged(int)), SLOT(slotImageSettingsChanged()));

	m_preview = new ImagePreview(colorbox);
	bool	useColor = (driver ? driver->get("colordevice") == "1" : true);
	m_preview->setBlackAndWhite(!useColor);
	  m_preview->setWhatsThis(whatsThisColorationPreviewImagePage);

	m_hue->setEnabled(useColor);
	m_saturation->setEnabled(useColor);
	QImage	img(locate("data", "kdeprint/preview.png"));
	m_preview->setImage(img);

	KSeparator	*sep = new KSeparator(Qt::Horizontal, colorbox);

	QPushButton	*defbtn = new QPushButton(i18n("&Default Settings"), colorbox);
	  defbtn->setWhatsThis(whatsThisResetButtonImagePage);
	connect(defbtn, SIGNAL(clicked()), SLOT(slotDefaultClicked()));
	slotDefaultClicked();

	m_sizetype = new QComboBox(sizebox);
	m_sizetype->insertItem(i18n("Natural Image Size"));
	m_sizetype->insertItem(i18n("Resolution (ppi)"));
	// xgettext:no-c-format
	m_sizetype->insertItem(i18n("% of Page"));
	// xgettext:no-c-format
	m_sizetype->insertItem(i18n("% of Natural Image Size"));

	m_size = new KIntNumInput(sizebox);
	m_size->setRange(1, 1200, 20, true);
	m_size->setValue(72);

	connect(m_sizetype, SIGNAL(activated(int)), SLOT(slotSizeTypeChanged(int)));
	m_sizetype->setCurrentItem(0);
	slotSizeTypeChanged(0);

	QLabel	*lab = new QLabel(i18n("&Image size type:"), sizebox);
	lab->setBuddy(m_sizetype);

	m_position = new ImagePosition(positionbox);
	  m_position->setWhatsThis(whatsThisPreviewPositionImagePage);

	QRadioButton	*bottom = new QRadioButton(positionbox);
	QRadioButton	*top = new QRadioButton(positionbox);
	QRadioButton	*vcenter = new QRadioButton(positionbox);
	QRadioButton	*left = new QRadioButton(positionbox);
	QRadioButton	*right = new QRadioButton(positionbox);
	QRadioButton	*hcenter = new QRadioButton(positionbox);
	QSize	sz = bottom->sizeHint();
	bottom->setFixedSize(sz);
	vcenter->setFixedSize(sz);
	top->setFixedSize(sz);
	left->setFixedSize(sz);
	hcenter->setFixedSize(sz);
	right->setFixedSize(sz);

	m_vertgrp = new Q3ButtonGroup(positionbox);
	m_vertgrp->hide();

	m_horizgrp = new Q3ButtonGroup(positionbox);
	m_horizgrp->hide();

	m_vertgrp->insert(top, 0);
	m_vertgrp->insert(vcenter, 1);
	m_vertgrp->insert(bottom, 2);
	if ( QApplication::isRightToLeft() )
	{
	    m_horizgrp->insert(left, 2);
	    m_horizgrp->insert(hcenter, 1);
	    m_horizgrp->insert(right, 0);
	}
	else
	{
	    m_horizgrp->insert(left, 0);
	    m_horizgrp->insert(hcenter, 1);
	    m_horizgrp->insert(right, 2);
	}
	connect(m_vertgrp, SIGNAL(clicked(int)), SLOT(slotPositionChanged()));
	connect(m_horizgrp, SIGNAL(clicked(int)), SLOT(slotPositionChanged()));
	m_vertgrp->setButton(1);
	m_horizgrp->setButton(1);
	slotPositionChanged();

	QGridLayout	*l0 = new QGridLayout(this, 2, 2, 0, 10);
	l0->addMultiCellWidget(colorbox, 0, 0, 0, 1);
	l0->addWidget(sizebox, 1, 0);
	l0->addWidget(positionbox, 1, 1);
	l0->setColStretch(0, 1);
	QGridLayout	*l1 = new QGridLayout(colorbox->layout(), 5, 2, 10);
	l1->addWidget(m_brightness, 0, 0);
	l1->addWidget(m_hue, 1, 0);
	l1->addWidget(m_saturation, 2, 0);
	l1->addWidget(sep, 3, 0);
	l1->addWidget(m_gamma, 4, 0);
	l1->addMultiCellWidget(m_preview, 0, 3, 1, 1);
	l1->addWidget(defbtn, 4, 1);
	QVBoxLayout	*l2 = new QVBoxLayout(sizebox->layout(), 3);
	l2->addStretch(1);
	l2->addWidget(lab);
	l2->addWidget(m_sizetype);
	l2->addSpacing(10);
	l2->addWidget(m_size);
	l2->addStretch(1);
	QGridLayout	*l3 = new QGridLayout(positionbox->layout(), 2, 2, 10);
	QHBoxLayout	*l4 = new QHBoxLayout(0, 0, 10);
	QVBoxLayout	*l5 = new QVBoxLayout(0, 0, 10);
	l3->addLayout(l4, 0, 1);
	l3->addLayout(l5, 1, 0);
	l3->addWidget(m_position, 1, 1);
	l4->addWidget(left, Qt::AlignLeft);
	l4->addWidget(hcenter, Qt::AlignCenter);
	l4->addWidget(right, Qt::AlignRight);
	l5->addWidget(top, Qt::AlignTop);
	l5->addWidget(vcenter, Qt::AlignVCenter);
	l5->addWidget(bottom, Qt::AlignBottom);
}

KPImagePage::~KPImagePage()
{
}

void KPImagePage::setOptions(const QMap<QString,QString>& opts)
{
	QString	value;
	if (!(value=opts["brightness"]).isEmpty())
		m_brightness->setValue(value.toInt());
	if (!(value=opts["hue"]).isEmpty())
		m_hue->setValue(value.toInt());
	if (!(value=opts["saturation"]).isEmpty())
		m_saturation->setValue(value.toInt());
	if (!(value=opts["gamma"]).isEmpty())
		m_gamma->setValue(value.toInt());

	int	type = 0;
	int	ival(0);
	if ((ival = opts["ppi"].toInt()) != 0)
		type = 1;
	else if ((ival = opts["scaling"].toInt()) != 0)
		type = 2;
	else if (!opts["natural-scaling"].isEmpty() && (ival = opts["natural-scaling"].toInt()) != 1)
		type = 3;
	m_sizetype->setCurrentItem(type);
	slotSizeTypeChanged(type);
	if (type != 0)
		m_size->setValue(ival);

	if (!(value=opts["position"]).isEmpty())
	{
		m_position->setPosition(value.latin1());
		int	pos = m_position->position();
		m_vertgrp->setButton(pos/3);
		m_horizgrp->setButton(pos%3);
	}
}

void KPImagePage::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	if (incldef || m_brightness->value() != 100)
		opts["brightness"] = QString::number(m_brightness->value());
	if (m_hue->isEnabled())
	{
		if (incldef || m_hue->value() != 0)
			opts["hue"] = QString::number(m_hue->value());
		if (incldef || m_saturation->value() != 100)
			opts["saturation"] = QString::number(m_saturation->value());
	}
	if (incldef || m_gamma->value() != 1000)
		opts["gamma"] = QString::number(m_gamma->value());

	QString	name;
	if (incldef)
	{
		opts["ppi"] = "0";
		opts["scaling"] = "0";
		opts["natural-scaling"] = "1";
	}
	switch (m_sizetype->currentItem())
	{
		case 0: break;
		case 1: name = "ppi"; break;
		case 2: name = "scaling"; break;
		case 3: name = "natural-scaling"; break;
	}
	if (!name.isEmpty())
		opts[name] = QString::number(m_size->value());

	if (incldef || m_position->position() != ImagePosition::Center)
		opts["position"] = m_position->positionString();
}

void KPImagePage::slotSizeTypeChanged(int t)
{
	m_size->setEnabled(t > 0);
	if (t > 0)
	{
		int	minval, maxval, defval;
		if (t == 1)
		{
			minval = 1;
			maxval = 1200;
			defval = 72;
		}
		else
		{
			minval = 1;
			maxval = 800;
			defval = 100;
		}
		m_size->setRange(minval, maxval);
		m_size->setValue(defval);
	}
}

void KPImagePage::slotPositionChanged()
{
	int	h = m_horizgrp->id(m_horizgrp->selected()), v = m_vertgrp->id(m_vertgrp->selected());
	m_position->setPosition(h, v);
}

void KPImagePage::slotImageSettingsChanged()
{
	int	b = m_brightness->value(), h = m_hue->value(), s = m_saturation->value();
	//int g = m_gamma->value();
	int g = 1000;
	m_preview->setParameters(b, h, s, g);
}

void KPImagePage::slotDefaultClicked()
{
	m_brightness->setValue(100);
	m_hue->setValue(0);
	m_saturation->setValue(100);
	m_gamma->setValue(1000);
}

#include "kpimagepage.moc"
