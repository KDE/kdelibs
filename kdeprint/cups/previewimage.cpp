/***************************************************************************
                          previewimage.cpp  -  description
                             -------------------
    begin                : Fri Jul 7 2000
    copyright            : (C) 2000 by Michael Goffioul
    email                : gofioul@emic.ucl.ac.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "previewimage.h"

#include <qpainter.h>

// forward definition
QImage convertImage(const QImage& image, int hue, int saturation, int brightness, int gamma);

PreviewImage::PreviewImage(QWidget *parent, const char *name ) : QWidget(parent,name) {
	brightness_ = 100;
	hue_ = 0;
	saturation_ = 100;
	gamma_ = 1000;
	bw_ = false;

	setBackgroundMode(NoBackground);
}
PreviewImage::~PreviewImage(){
}
/**  */
void PreviewImage::setImage(const QImage& image){
	image_ = image.convertDepth(32);
	image_.detach();
	resize(image_.size());
	update();
}
/**  */
void PreviewImage::setParameters(int brightness, int hue, int saturation, int gamma){
	brightness_ = brightness;
	hue_ = hue;
	saturation_ = saturation;
	gamma_ = gamma;
	repaint();
}
/**  */
void PreviewImage::paintEvent(QPaintEvent*){
	QImage	tmpImage = convertImage(image_,hue_,(bw_ ? 0 : saturation_),brightness_,gamma_);
	int	x=(width()-tmpImage.width())/2, y=(height()-tmpImage.height())/2;

	QPixmap	pix(width(),height());
	pix.fill(colorGroup().background());
	QPainter	painter(&pix);
	painter.drawImage(x,y,tmpImage);
	painter.end();

	QPainter	p(this);
	p.drawPixmap(0,0,pix);
	p.end();
}
/**  */
void PreviewImage::setBlackAndWhite(bool on){
	bw_ = on;
	update();
}

void PreviewImage::setBrightness(int b){
	setParameters(b, hue_, saturation_, gamma_);
}

void PreviewImage::setHue(int h){
	setParameters(brightness_, h, saturation_, gamma_);
}

void PreviewImage::setSaturation(int s){
	setParameters(brightness_, hue_, s, gamma_);
}

void PreviewImage::setGamma(int g){
	setParameters(brightness_, hue_, saturation_, g);
}
