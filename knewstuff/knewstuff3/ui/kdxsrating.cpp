/*
    This file is part of KNewStuff2.
    Copyright (c) 2006, 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kdxsrating.h"

#include "qstarframe.h"

#include <QtGui/QLayout>
#include <QtGui/QSlider>
#include <QtGui/QLabel>

#include <klocale.h>
#include <kstandarddirs.h>

KDXSRating::KDXSRating(QWidget *parent)
        : KDialog(parent)
{
    setCaption(i18n("Rate this entry"));
    setButtons(KDialog::Ok | KDialog::Cancel);

    QVBoxLayout *vbox;
    QHBoxLayout *hbox;

    QWidget *root = new QWidget(this);
    setMainWidget(root);

    m_slider = new QSlider(root);
    m_slider->setOrientation(Qt::Horizontal);
    m_slider->setTickPosition(QSlider::TicksBelow);
    m_slider->setMinimum(0);
    m_slider->setMaximum(100);

    m_starrating = new QStarFrame(root);
    m_starrating->setMinimumWidth(100);

    m_rating = new QLabel(QString("0/100"), root);
    m_rating->setFixedWidth(100);

    vbox = new QVBoxLayout(root);
    hbox = new QHBoxLayout();
    hbox->addWidget(m_rating);
    hbox->addWidget(m_starrating);
    vbox->addLayout(hbox);
    vbox->addWidget(m_slider);

    connect(m_slider, SIGNAL(valueChanged(int)),
            m_starrating, SLOT(slotRating(int)));
    connect(m_slider, SIGNAL(valueChanged(int)), SLOT(slotRating(int)));
}

int KDXSRating::rating()
{
    return m_slider->value();
}

void KDXSRating::slotRating(int rating)
{
    m_rating->setText(QString("%1/100").arg(rating));
}

/*
void KDXSRating::slotRating(int rating)
{
 QString starpath = locate("data", "kpdf/pics/ghns_star.png");
 QString graystarpath = locate("data", "kpdf/pics/ghns_star_gray.png");

 QPixmap star(starpath);
 QPixmap graystar(graystarpath);

 int wpixels = (int)(m_starrating->width() * (float)rating / 100.0);

 QPainter p;
 p.begin(m_starrating);
 int w = star.width();
 for(int i = 0; i < wpixels; i += star.width())
 {
  w = wpixels - i;
  if(w > star.width()) w = star.width();
  p.drawPixmap(i, 0, star, 0, 0, w, -1);
 }
 p.drawPixmap(wpixels, 0, graystar, w, 0, graystar.width() - w, -1);
 wpixels += graystar.width() - w;
 for(int i = wpixels; i < m_starrating->width(); i += graystar.width())
 {
  w = m_starrating->width() - i;
  if(w > graystar.width()) w = graystar.width();
  p.drawPixmap(i, 0, graystar, 0, 0, w, -1);
 }
 p.end();

 m_rating->setText(QString("%1/100").arg(rating));
}

void KDXSRating::paintEvent(QPaintEvent e)
{
 Q_UNUSED(e);

 kDebug() << "paint event!";

 int rating = m_slider->value();
 slotRating(rating);
}
*/

#include "kdxsrating.moc"
