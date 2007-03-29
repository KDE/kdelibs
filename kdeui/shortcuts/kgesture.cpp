/* This file is part of the KDE libraries
    Copyright (C) 2006,2007 Andreas Hartmetz (ahartmetz@gmail.com)

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kgesture.h"
#include <klocalizedstring.h>
#include <kdebug.h>
#include <math.h>
#include <QStringList>

KShapeGesture::KShapeGesture()
{
}


KShapeGesture::KShapeGesture(const QPolygon &shape)
{
    setShape(shape);
}


KShapeGesture::KShapeGesture(const QString &description)
{
    QStringList sl = description.split(',');
    m_friendlyName = sl.takeFirst();

    bool ok = true;
    QPolygon poly;
    int x, y;
    QStringList::const_iterator it = sl.constBegin();
    while (it != sl.constEnd()) {
        x = (*it).toInt(&ok);
        ++it;
        if (!ok || it == sl.constEnd())
            break;
        y = (*it).toInt(&ok);
        if (!ok)
            break;
        ++it;
        poly.append(QPoint(x, y));
    }
    if (!ok) {
        m_friendlyName = QString();
        return;
    }

    setShape(poly);
}


KShapeGesture::KShapeGesture(const KShapeGesture &other)
 : m_shape(other.m_shape),
   m_lengthTo(other.m_lengthTo),
   m_curveLength(other.m_curveLength)
{
}


KShapeGesture::~KShapeGesture()
{
}


inline float KShapeGesture::metric(float dx, float dy) const
{
    //square root of that or not? - not square root has possible advantages
    return (dx*dx + dy*dy);
}


void KShapeGesture::setShape(const QPolygon &shape)
{
    //Scale and translate into a 100x100 square with its
    //upper left corner at origin.
    m_shape = shape;
    QRect bounding = shape.boundingRect();
    //TODO: don't change aspect ratio "too much" to avoid problems with straight lines
    //TODO: catch all bad input, like null height/width

    //compensate for QRect weirdness
    bounding.setWidth(bounding.width() - 1);
    bounding.setHeight(bounding.height() - 1);

    float xScale = bounding.width() ? 100.0 / bounding.width() : 1.0;
    float yScale = bounding.height() ? 100.0 / bounding.height() : 1.0;
    m_shape.translate(-bounding.left(), -bounding.top());
    for (int i=0; i < m_shape.size(); i++) {
        m_shape[i].setX((int)(xScale * (float)m_shape[i].x()));
        m_shape[i].setY((int)(yScale * (float)m_shape[i].y()));
    }

    //calculate accumulated lengths of lines making up the polygon
    Q_ASSERT(m_shape.size() > 1);
    m_curveLength = 0.0;
    m_lengthTo.clear();
    m_lengthTo.reserve(m_shape.size());
    m_lengthTo.append(m_curveLength);

    int prevX = m_shape[0].x();
    int prevY = m_shape[0].y();
    for (int i=1; i < m_shape.size(); i++) {
        int curX = m_shape[i].x();
        int curY = m_shape[i].y();
        m_curveLength += metric(curX-prevX, curY - prevY);
        m_lengthTo.append(m_curveLength);
        prevX = curX;
        prevY = curY;
    }
}


void KShapeGesture::setShapeName(const QString &friendlyName)
{
    m_friendlyName = friendlyName;
}


QString KShapeGesture::shapeName() const
{
    return m_friendlyName;
}


bool KShapeGesture::isValid() const
{
    return !m_shape.isEmpty();
}


QString KShapeGesture::toString() const
{
    if (!isValid())
        return QString();

    //TODO: what if the name contains a "," or ";"? Limit the name to letters?
    QString ret = m_friendlyName;
    ret.append(',');

    int i;
    for (i = 0; i < m_shape.size() - 1; i++) {
        ret.append(QString::number(m_shape[i].x()));
        ret.append(',');
        ret.append(QString::number(m_shape[i].y()));
        ret.append(',');
    }
    i++;
    ret.append(QString::number(m_shape[i].x()));
    ret.append(',');
    ret.append(QString::number(m_shape[i].y()));

    return ret;
}


#include <kdebug.h>
QByteArray KShapeGesture::toSvg(const QString &attributes) const
{
    if (!isValid()) {
        return QByteArray();
        //TODO: KDE standard debug output
    }
    const char *prolog = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
                         "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
                         "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">"
                         "<svg width=\"100\" height=\"100\" version=\"1.1\" "
                         "xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M";
    const char *epilog1 = "\" fill=\"none\" ";
    const char *epilog2 = " /></svg>";
    QByteArray ret(prolog);

    ret.append(QString::number(m_shape[0].x()).toUtf8());
    ret.append(",");
    ret.append(QString::number(m_shape[0].y()).toUtf8());

    for (int i=1; i < m_shape.size(); i++) {
        ret.append("L");
        ret.append(QString::number(m_shape[i].x()).toUtf8());
        ret.append(",");
        ret.append(QString::number(m_shape[i].y()).toUtf8());
    }

    ret.append(epilog1);
    ret.append(attributes.toUtf8());
    ret.append(epilog2);
    return ret;
}


/*
  algorithm: iterate in order over 30 points on our shape and measure the
  minimum distance to any point on the other shape. never go backwards on
  the other shape to also check direction of movement.
  This algorithm is best applied like a->distance(b) + b->distance(a).
  fabs(a->distance(b) - b->distance(a)) might turn out to be very interesting,
  too. in fact, i think it's the most interesting value.
 */
float KShapeGesture::distance(const KShapeGesture &other, float abortThreshold) const
{
    Q_UNUSED(abortThreshold); //for optimizations, later
    const QPolygon &o_shape = other.m_shape;
    const QVector<float> &o_lengthTo = other.m_lengthTo;
    float x, y, mx, my, position;
    float ox, oy, omx, omy, oposition;
    float oxB, oyB, omxB, omyB, opositionB;
    float dist, distB;
    float desiredPosition, strokeLength;
    float retval = 0.0;
    int pointIndex = 0, opointIndex = 0, opointIndexB = 0;

    //set up starting point on our shape
    x = m_shape[0].x();
    y = m_shape[0].y();
    strokeLength = m_lengthTo[1];
    mx = (m_shape[1].x() - x) / strokeLength;
    my = (m_shape[1].y() - y) / strokeLength;
    position = 0.0;

    //set up lower bound of search interval on other shape
    ox = o_shape[0].x();
    oy = o_shape[0].y();
    strokeLength = o_lengthTo[1];
    omx = (o_shape[1].x() - ox) / strokeLength;
    omy = (o_shape[1].y() - oy) / strokeLength;
    oposition = 0.0;
    dist = metric(ox-x, oy-y);

    for (int i = 0; i <= 30; i++) {
        //go to comparison point on our own polygon
        //30.0001 to prevent getting out-of-bounds pointIndex
        desiredPosition = m_curveLength / 30.0001 * (float)i;
        if (desiredPosition > m_lengthTo[pointIndex+1]) {

            while (desiredPosition > m_lengthTo[pointIndex+1])
                pointIndex++;

            x = m_shape[pointIndex].x();
            y = m_shape[pointIndex].y();
            position = m_lengthTo[pointIndex];
            strokeLength = m_lengthTo[pointIndex+1] - position;
            mx = (m_shape[pointIndex+1].x() - x) / strokeLength;
            my = (m_shape[pointIndex+1].y() - y) / strokeLength;
        }
        x += mx * (desiredPosition - position);
        y += my * (desiredPosition - position);
        position = desiredPosition;

        //set up upper bound of search interval on other shape
        desiredPosition = qMin(oposition + other.m_curveLength / 15.00005,
                               other.m_curveLength - 0.0001);
        if (i == 0 || desiredPosition > o_lengthTo[opointIndexB+1]) {

            while (desiredPosition > o_lengthTo[opointIndexB+1])
                opointIndexB++;

            oxB = o_shape[opointIndexB].x();
            oyB = o_shape[opointIndexB].y();
            opositionB = o_lengthTo[opointIndexB];
            strokeLength = o_lengthTo[opointIndexB+1] - opositionB;
            omxB = (o_shape[opointIndexB+1].x() - oxB) / strokeLength;
            omyB = (o_shape[opointIndexB+1].y() - oyB) / strokeLength;
        }
        oxB += omxB * (desiredPosition - opositionB);
        oyB += omyB * (desiredPosition - opositionB);
        opositionB = desiredPosition;
        distB = metric(oxB-x, oyB-y);

        //binary search for nearest point on other shape
        for (int j = 0; j < 6; j++) {
            desiredPosition = (oposition + opositionB) * 0.5;
            if (dist < distB) {
                //retract upper bound to desiredPosition
                //copy state of lower bound to upper bound, advance it from there
                oxB = ox; oyB = oy;
                omxB = omx; omyB = omy;
                opointIndexB = opointIndex; opositionB = oposition;

                if (desiredPosition > o_lengthTo[opointIndexB+1]) {

                    while (desiredPosition > o_lengthTo[opointIndexB+1])
                        opointIndexB++;

                    oxB = o_shape[opointIndexB].x();
                    oyB = o_shape[opointIndexB].y();
                    opositionB = o_lengthTo[opointIndexB];
                    strokeLength = o_lengthTo[opointIndexB+1] - opositionB;
                    omxB = (o_shape[opointIndexB+1].x() - oxB) / strokeLength;
                    omyB = (o_shape[opointIndexB+1].y() - oyB) / strokeLength;
                }
                oxB += omxB * (desiredPosition - opositionB);
                oyB += omyB * (desiredPosition - opositionB);
                opositionB = desiredPosition;
                distB = metric(oxB-x, oyB-y);
            } else {
                //advance lower bound to desiredPosition
                if (desiredPosition > o_lengthTo[opointIndex+1]) {

                    while (desiredPosition > o_lengthTo[opointIndex+1])
                        opointIndex++;

                    ox = o_shape[opointIndex].x();
                    oy = o_shape[opointIndex].y();
                    oposition = o_lengthTo[opointIndex];
                    strokeLength = o_lengthTo[opointIndex+1] - oposition;
                    omx = (o_shape[opointIndex+1].x() - ox) / strokeLength;
                    omy = (o_shape[opointIndex+1].y() - oy) / strokeLength;
                }
                ox += omx * (desiredPosition - oposition);
                oy += omy * (desiredPosition - oposition);
                oposition = desiredPosition;
                dist = metric(ox-x, oy-y);
            }
        }
        retval += qMin(dist, distB);
    }
    //scale value to make it roughly invariant against step width
    return retval / 30.0;
}


KShapeGesture &KShapeGesture::operator=(const KShapeGesture &other)
{
    m_lengthTo = other.m_lengthTo;
    m_shape = other.m_shape;
    m_curveLength = other.m_curveLength;
    return *this;
}


bool KShapeGesture::operator==(const KShapeGesture &other) const
{
    //a really fast and workable shortcut
    if (fabs(m_curveLength - other.m_curveLength) > 0.1)
        return false;
    return m_shape == other.m_shape;
}


uint KShapeGesture::hashable() const
{
    uint hash = 0;

    foreach (QPoint point, m_shape)
        hash += qHash(point.x()) + qHash(point.y());

    return hash;
}


/********************************************************
 * KRockerGesture *
 *******************************************************/

KRockerGesture::KRockerGesture()
 : m_hold(Qt::NoButton),
   m_thenPush(Qt::NoButton)
{
}


KRockerGesture::KRockerGesture(Qt::MouseButton hold, Qt::MouseButton thenPush)
{
    setButtons(hold, thenPush);
}


KRockerGesture::KRockerGesture(const QString &description)
 : m_hold(Qt::NoButton),
   m_thenPush(Qt::NoButton)
{
    if (description.length() != 2)
        return;

    Qt::MouseButton hold, thenPush;
    Qt::MouseButton *current = &hold;
    for (int i = 0; i < 2; i++) {
        switch (description[i].toLatin1()) {
        case 'L':
            *current = Qt::LeftButton;
            break;
        case 'R':
            *current = Qt::RightButton;
            break;
        case 'M':
            *current = Qt::MidButton;
            break;
        case '1':
            *current = Qt::XButton1;
            break;
        case '2':
            *current = Qt::XButton2;
            break;
        default:
            return;
        }
        current = &thenPush;
    }
    m_hold = hold;
    m_thenPush = thenPush;
}


KRockerGesture::KRockerGesture(const KRockerGesture &other)
 : m_hold(other.m_hold),
   m_thenPush(other.m_thenPush)
{
}


KRockerGesture::~KRockerGesture()
{
}


void KRockerGesture::setButtons(Qt::MouseButton hold, Qt::MouseButton thenPush)
{
    if (hold == thenPush) {
        m_hold = Qt::NoButton;
        m_thenPush = Qt::NoButton;
        return;
    }

    int button = hold;
    for (int i = 0; i < 2; i++) {
        switch (button) {
        case Qt::LeftButton:
        case Qt::RightButton:
        case Qt::MidButton:
        case Qt::XButton1:
        case Qt::XButton2:
            break;
        default:
            m_hold = Qt::NoButton;
            m_thenPush = Qt::NoButton;
            return;
        }
        button = thenPush;
    }

    m_hold = hold;
    m_thenPush = thenPush;
}


void KRockerGesture::getButtons(Qt::MouseButton *hold, Qt::MouseButton *thenPush) const
{
    *hold = m_hold;
    *thenPush = m_thenPush;
}


QString KRockerGesture::mouseButtonName(Qt::MouseButton button)
{
    switch (button) {
    case Qt::LeftButton:
        return i18nc("left mouse button", "left button");
        break;
    case Qt::MidButton:
        return i18nc("middle mouse button", "middle button");
        break;
    case Qt::RightButton:
        return i18nc("right mouse button", "right button");
        break;
    default:
        return i18nc("a nonexistent value of mouse button", "invalid button");
        break;
    }
}


QString KRockerGesture::rockerName() const
{
    if (!isValid())
        return i18nc("an invalid mouse gesture of type \"hold down one button, then press another button\"",
                     "invalid rocker gesture");
    else
        return i18nc("a kind of mouse gesture: hold down one mouse button, then press another button",
                     "Hold %1, then push %2", mouseButtonName(m_hold), mouseButtonName(m_thenPush));
}


bool KRockerGesture::isValid() const
{
    //### stricter checking might be in order (maybe done in setButtons)
    return (m_hold != Qt::NoButton);
}


QString KRockerGesture::toString() const
{
    if (!isValid())
        return QString();
    QString ret;
    int button = m_hold;
    char desc;
    for (int i = 0; i < 2; i++) {
        switch (button) {
        case Qt::LeftButton:
            desc = 'L';
            break;
        case Qt::RightButton:
            desc = 'R';
            break;
        case Qt::MidButton:
            desc = 'M';
            break;
        case Qt::XButton1:
            desc = '1';
            break;
        case Qt::XButton2:
            desc = '2';
            break;
        default:
            return QString();
        }
        ret.append(desc);
        button = m_thenPush;
    }
    return ret;
}


KRockerGesture &KRockerGesture::operator=(const KRockerGesture &other)
{
    m_hold = other.m_hold;
    m_thenPush = other.m_thenPush;
    return *this;
}


bool KRockerGesture::operator==(const KRockerGesture &other) const
{
    return m_hold == other.m_hold && m_thenPush == other.m_thenPush;
}


uint KRockerGesture::hashable() const
{
    //make it asymmetric
    return qHash(m_hold) + m_thenPush;
}
