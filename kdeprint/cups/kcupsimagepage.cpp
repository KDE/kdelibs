#include "kcupsimagepage.h"
#include "previewimage.h"
#include "position.h"

#include <qimage.h>
#include <qscrollbar.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <klocale.h>
#include <kiconloader.h>

KCupsImagePage::KCupsImagePage(global_ppd_file_t *ppd, IppRequest *req, QWidget *parent, const char *name)
: KCupsImagePageBase(parent,name), ppd_(ppd), request_(req)
{
	initialize();
}

KCupsImagePage::~KCupsImagePage()
{
}

void KCupsImagePage::initialize()
{
	preview_->setImage(UserIcon("kdeprint_preview").convertToImage());

	hue_->setSteps(1,72);
	hue_->setRange(-360,360);
	hue_->setValue(0);
	hue_->setOrientation(QScrollBar::Horizontal);
	saturation_->setSteps(1,20);
	saturation_->setRange(0,200);
	saturation_->setValue(100);
	saturation_->setOrientation(QScrollBar::Horizontal);
	brightness_->setSteps(1,20);
	brightness_->setRange(0,200);
	brightness_->setValue(100);
	brightness_->setOrientation(QScrollBar::Horizontal);
	gamma_->setSteps(1,100);
	gamma_->setRange(1,3000);
	gamma_->setValue(1000);
	gamma_->setOrientation(QScrollBar::Horizontal);
	scaling_->setSteps(1,80);
	scaling_->setRange(1,800);
	scaling_->setValue(100);
	scaling_->setOrientation(QScrollBar::Horizontal);
	ppi_->setSteps(1,120);
	ppi_->setRange(1,1200);
	ppi_->setValue(72);
	ppi_->setOrientation(QScrollBar::Horizontal);

	connect(hue_,SIGNAL(valueChanged(int)),SLOT(colorSettingsChanged(int)));
	connect(saturation_,SIGNAL(valueChanged(int)),SLOT(colorSettingsChanged(int)));
	connect(brightness_,SIGNAL(valueChanged(int)),SLOT(colorSettingsChanged(int)));
	connect(gamma_,SIGNAL(valueChanged(int)),SLOT(colorSettingsChanged(int)));
	connect(default_,SIGNAL(clicked()),SLOT(defaultClicked()));
	connect(ppicheck_,SIGNAL(clicked()),SLOT(ppiCheckClicked()));
	connect(scalingcheck_,SIGNAL(clicked()),SLOT(scalingCheckClicked()));
	connect(ppi_,SIGNAL(valueChanged(int)),SLOT(ppiChanged(int)));
	connect(scaling_,SIGNAL(valueChanged(int)),SLOT(scalingChanged(int)));

	colorSettingsChanged(0);
	ppiCheckClicked();
	scalingCheckClicked();

	horizgrp_ = new QButtonGroup(this);
	horizgrp_->hide();
	vertgrp_ = new QButtonGroup(this);
	vertgrp_->hide();
	connect(horizgrp_, SIGNAL(clicked(int)), SLOT(horizChanged(int)));
	connect(vertgrp_, SIGNAL(clicked(int)), SLOT(vertChanged(int)));

	horizpos_ = vertpos_ = 1;
	pospix_->setPosition(horizpos_,vertpos_);
	wcenter_->setChecked(true);
	hcenter_->setChecked(true);

	horizgrp_->insert(left_,0);
	horizgrp_->insert(wcenter_,1);
	horizgrp_->insert(right_,2);
	vertgrp_->insert(top_,0);
	vertgrp_->insert(hcenter_,1);
	vertgrp_->insert(bottom_,2);
}

void KCupsImagePage::setOptions(const OptionSet& opts)
{
	if (ppd_ && ppd_->ppd && !ppd_->ppd->color_device)
	{
		hue_->setEnabled(false);
		saturation_->setEnabled(false);
		huevalue_->hide();
		saturationvalue_->hide();
		preview_->setBlackAndWhite(true);
	}
	else
	{
		hue_->setEnabled(true);
		saturation_->setEnabled(true);
		huevalue_->show();
		saturationvalue_->show();
		preview_->setBlackAndWhite(false);
	}

	// init "hue"
	if (!opts["hue"].isEmpty()) hue_->setValue(opts["hue"].toInt());
	// init "saturation"
	if (!opts["saturation"].isEmpty()) saturation_->setValue(opts["saturation"].toInt());
	// init "brightness"
	if (!opts["brightness"].isEmpty()) brightness_->setValue(opts["brightness"].toInt());
	// init "gamma"
	if (!opts["gamma"].isEmpty()) gamma_->setValue(opts["gamma"].toInt());

	// init "ppi" / "scaling"
	if (!opts["ppi"].isEmpty())
	{
		ppi_->setValue(opts["ppi"].toInt());
		ppicheck_->setChecked(opts["ppi-flag"].isEmpty() || opts["ppi-flag"] == "on");
		ppiCheckClicked();
	}
	else if (!opts["scaling"].isEmpty())
	{
		scaling_->setValue(opts["scaling"].toInt());
		scalingcheck_->setChecked(opts["scaling-flag"].isEmpty() || opts["scaling-flag"] == "on");
		scalingCheckClicked();
	}

	// init "image position"
	if (!opts["position"].isEmpty())
	{
		pospix_->setPosition(opts["position"].latin1());
		int	pos = pospix_->position();
		vertgrp_->setButton((vertpos_=pos/3));
		horizgrp_->setButton((horizpos_=pos%3));
	}
}

void KCupsImagePage::getOptions(OptionSet& opts, bool incldef)
{
	// Brightness
	if (incldef || brightness_->value() != 100)
		opts["brightness"] = QString::number(brightness_->value());
	// Gamma
	if (incldef || gamma_->value() != 1000)
		opts["gamma"] = QString::number(gamma_->value());

	// Hue and saturation
	if (hue_->isEnabled())
	{
		if (incldef || hue_->value() != 0)
			opts["hue"] = QString::number(hue_->value());
		if (incldef || saturation_->value() != 100)
			opts["saturation"] = QString::number(saturation_->value());
	}

	// Resolution / Scaling
	if (ppicheck_->isChecked())
	{
		opts["ppi"] = QString::number(ppi_->value());
		opts.remove("scaling");
	}
	else if (scalingcheck_->isChecked())
	{
		opts["scaling"] = QString::number(scaling_->value());
		opts.remove("ppi");
	}
	else
	{
		opts.remove("ppi");
		opts.remove("scaling");
	}

	if (incldef)
	{
		opts["ppi-flag"] = (ppicheck_->isChecked() ? "on" : "off");
		opts["scaling-flag"] = (scalingcheck_->isChecked() ? "on" : "off");
	}

	// Image position
	if (incldef || pospix_->position() != ImagePosition::Center)
		opts["position"] = pospix_->positionString();
}

QString KCupsImagePage::pageTitle()
{
	return i18n("Image");
}

void KCupsImagePage::colorSettingsChanged(int)
{
	int	h(hue_->value()), b(brightness_->value()), s(saturation_->value()), g(gamma_->value());

	huevalue_->setText(QString::number(h));
	saturationvalue_->setText(QString::number(s));
	brightnessvalue_->setText(QString::number(b));
	gammavalue_->setText(QString::number(double(g)/1000,'f',3));

	preview_->setParameters(b,h,s,1000);
}

void KCupsImagePage::defaultClicked()
{
	hue_->setValue(0);
	brightness_->setValue(100);
	saturation_->setValue(100);
	gamma_->setValue(1000);
}

void KCupsImagePage::ppiChanged(int value)
{
	ppivalue_->setText(QString::number(value));
}

void KCupsImagePage::ppiCheckClicked()
{
	bool	val = ppicheck_->isChecked();
	ppi_->setEnabled(val);
	if (val)
	{
		ppiChanged(ppi_->value());
		if (scalingcheck_->isChecked())
		{
			scalingcheck_->setChecked(false);
			scalingCheckClicked();
		}
	}
	else
		ppivalue_->setText("");
}

void KCupsImagePage::scalingChanged(int value)
{
	scalingvalue_->setText(QString::number(value));
}

void KCupsImagePage::scalingCheckClicked()
{
	bool	val = scalingcheck_->isChecked();
	scaling_->setEnabled(val);
	if (val)
	{
		scalingChanged(scaling_->value());
		if (ppicheck_->isChecked())
		{
			ppicheck_->setChecked(false);
			ppiCheckClicked();
		}
	}
	else
		scalingvalue_->setText("");
}

void KCupsImagePage::vertChanged(int ID)
{
	vertpos_ = ID;
	pospix_->setPosition(horizpos_,vertpos_);
}

void KCupsImagePage::horizChanged(int ID)
{
	horizpos_ = ID;
	pospix_->setPosition(horizpos_,vertpos_);
}
