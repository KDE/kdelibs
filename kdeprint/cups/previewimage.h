/***************************************************************************
                          previewimage.h  -  description
                             -------------------
    begin                : Fri Jul 7 2000
    copyright            : (C) 2000 by Michael Goffioul
    email                : gofioul@imec.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PREVIEWIMAGE_H
#define PREVIEWIMAGE_H

#include <qwidget.h>
#include <qimage.h>

/**
  *@author Michael Goffioul
  */

class PreviewImage : public QWidget  {
public:
	PreviewImage(QWidget *parent=0, const char *name=0);
	~PreviewImage();
  /**  */
  void setParameters(int brightness, int hue, int saturation, int gamma);
  /**  */
  void setImage(const QImage& image);
  /**  */
  void setBlackAndWhite(bool on);
public slots:
	void setBrightness(int b);
	void setHue(int h);
	void setSaturation(int s);
	void setGamma(int g);
private: // Private attributes
  /**  */
  int brightness_;
  int hue_;
  int saturation_;
  int gamma_;
  bool bw_;
  /**  */
  QImage image_;
protected: // Protected methods
  /**  */
  void paintEvent(QPaintEvent*);
};

#endif
