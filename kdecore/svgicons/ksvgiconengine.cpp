/*
    Copyright (C) 2002 Nikolas Zimmermann <wildfox@kde.org>
    This file is part of the KDE project

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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qdom.h>
#include <qfile.h>
#include <qcolor.h>
#include <qimage.h>
#include <qmatrix.h>
#include <QVector>
#include <QPolygon>
#include <q3ptrlist.h>

#include <kcodecs.h>

#include <zlib.h>

#include "ksvgiconpainter.h"
#include "ksvgiconengine.h"

class KSVGIconEngineHelper
{
public:
	KSVGIconEngineHelper(KSVGIconEngine *engine)
	{
		m_engine = engine;
	}

	~KSVGIconEngineHelper()
	{
	}

	double toPixel(const QString &s, bool hmode)
	{
		return m_engine->painter()->toPixel(s, hmode);
	}

	ArtGradientStop *parseGradientStops(QDomElement element, int &offsets)
	{
		QVector<ArtGradientStop> *stopArray = new QVector<ArtGradientStop>();

		float oldOffset = -1, newOffset = -1;
		for(QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling())
		{
			QDomElement element = node.toElement();

			oldOffset = newOffset;
			QString temp = element.attribute("offset");

			if(temp.contains("%"))
			{
				temp = temp.left(temp.length() - 1);
				newOffset = temp.toFloat() / 100.0;
			}
			else
				newOffset = temp.toFloat();

			// Spec  skip double offset specifications
			if(oldOffset == newOffset)
				continue;

			offsets++;
			stopArray->resize(offsets + 1);

			(*stopArray)[offsets].offset = newOffset;

			QString parseOpacity;
			QString parseColor;

			if(element.hasAttribute("stop-opacity"))
				parseOpacity = element.attribute("stop-opacity");

			if(element.hasAttribute("stop-color"))
				parseColor = element.attribute("stop-color");

			if(parseOpacity.isEmpty() || parseColor.isEmpty())
			{
				QString style = element.attribute("style");

				QStringList substyles = style.split(';', QString::SkipEmptyParts);
				for(QStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it)
				{
					QStringList substyle = (*it).split(':', QString::SkipEmptyParts);
					QString command = substyle[0];
					QString params = substyle[1];
					command = command.trimmed();
					params = params.trimmed();

					if(command == "stop-color")
					{
						parseColor = params;

						if(!parseOpacity.isEmpty())
							break;
					}
					else if(command == "stop-opacity")
					{
						parseOpacity = params;

						if(!parseColor.isEmpty())
							break;
					}
				}
			}

			// Parse color using KSVGIconPainter (which uses Qt)
			// Supports all svg-needed color formats
			QColor qStopColor = m_engine->painter()->parseColor(parseColor);

			// Convert in a libart suitable form
			quint32 stopColor = m_engine->painter()->toArtColor(qStopColor);

			int opacity = m_engine->painter()->parseOpacity(parseOpacity);

			quint32 rgba = (stopColor << 8) | opacity;
			quint32 r, g, b, a;

			// Convert from separated to premultiplied alpha
			a = rgba & 0xff;
			r = (rgba >> 24) * a + 0x80;
			r = (r + (r >> 8)) >> 8;
			g = ((rgba >> 16) & 0xff) * a + 0x80;
			g = (g + (g >> 8)) >> 8;
			b = ((rgba >> 8) & 0xff) * a + 0x80;
			b = (b + (b >> 8)) >> 8;

			(*stopArray)[offsets].color[0] = ART_PIX_MAX_FROM_8(r);
			(*stopArray)[offsets].color[1] = ART_PIX_MAX_FROM_8(g);
			(*stopArray)[offsets].color[2] = ART_PIX_MAX_FROM_8(b);
			(*stopArray)[offsets].color[3] = ART_PIX_MAX_FROM_8(a);
		}

		return stopArray->data();
	}

	QPolygon parsePoints(QString points)
	{
		if(points.isEmpty())
			return QPolygon();

		points = points.simplified();

		if(points.contains(",,") || points.contains(", ,"))
			return QPolygon();

		points.replace(',', ' ');
		points.replace('\r', QString::null);
		points.replace('\n', QString::null);

		points = points.simplified();

		QStringList pointList = points.split(' ', QString::SkipEmptyParts);

		QPolygon array(pointList.count() / 2);
		int i = 0;

		for(QStringList::Iterator it = pointList.begin(); it != pointList.end(); it++)
		{
			float x = (*(it++)).toFloat();
			float y = (*(it)).toFloat();

			array.setPoint(i, static_cast<int>(x), static_cast<int>(y));
			i++;
		}

		return array;
	}

	void parseTransform(const QString &transform)
	{
		// Combine new and old matrix
		QMatrix matrix = m_engine->painter()->parseTransform(transform);

		QMatrix *current = m_engine->painter()->worldMatrix();
		*current = matrix * *current;
	}

	void parseCommonAttributes(QDomNode &node)
	{
		// Set important default attributes
		m_engine->painter()->setFillColor("black");
		m_engine->painter()->setStrokeColor("none");
		m_engine->painter()->setStrokeDashArray("");
		m_engine->painter()->setStrokeWidth(1);
		m_engine->painter()->setJoinStyle("");
		m_engine->painter()->setCapStyle("");
	//	m_engine->painter()->setFillOpacity(255, true);
	//	m_engine->painter()->setStrokeOpacity(255, true);

		// Collect parent node's attributes
		Q3PtrList<QDomNamedNodeMap> applyList;
		applyList.setAutoDelete(true);

		QDomNode shape = node.parentNode();
		for(; !shape.isNull() ; shape = shape.parentNode())
			applyList.prepend(new QDomNamedNodeMap(shape.attributes()));

		// Apply parent attributes
		for(QDomNamedNodeMap *map = applyList.first(); map != 0; map = applyList.next())
		{
			QDomNamedNodeMap attr = *map;

			for(unsigned int i = 0; i < attr.count(); i++)
			{
				QString name, value;

				name = attr.item(i).nodeName().toLower();
				value = attr.item(i).nodeValue();

				if(name == "transform")
					parseTransform(value);
				else if(name == "style")
					parseStyle(value);
				else
					parsePA(name, value);
			}
		}

		// Apply local attributes
		QDomNamedNodeMap attr = node.attributes();

		for(unsigned int i = 0; i < attr.count(); i++)
		{
			QDomNode current = attr.item(i);

			if(current.nodeName().toLower() == "transform")
				parseTransform(current.nodeValue());
			else if(current.nodeName().toLower() == "style")
				parseStyle(current.nodeValue());
			else
				parsePA(current.nodeName().toLower(), current.nodeValue());
		}
	}

	bool handleTags(QDomElement element, bool paint)
	{
		if(element.attribute("display") == "none")
			return false;
		if(element.tagName() == "linearGradient")
		{
			ArtGradientLinear *gradient = new ArtGradientLinear();

			int offsets = -1;
			gradient->stops = parseGradientStops(element, offsets);
			gradient->n_stops = offsets + 1;

			QString spread = element.attribute("spreadMethod");
			if(spread == "repeat")
				gradient->spread = ART_GRADIENT_REPEAT;
			else if(spread == "reflect")
				gradient->spread = ART_GRADIENT_REFLECT;
			else
				gradient->spread = ART_GRADIENT_PAD;

			m_engine->painter()->addLinearGradient(element.attribute("id"), gradient);
			m_engine->painter()->addLinearGradientElement(gradient, element);
			return true;
		}
		else if(element.tagName() == "radialGradient")
		{
			ArtGradientRadial *gradient = new ArtGradientRadial();

			int offsets = -1;
			gradient->stops = parseGradientStops(element, offsets);
			gradient->n_stops = offsets + 1;

			m_engine->painter()->addRadialGradient(element.attribute("id"), gradient);
			m_engine->painter()->addRadialGradientElement(gradient, element);
			return true;
		}

		if(!paint)
			return true;

		// TODO: Default attribute values
		if(element.tagName() == "rect")
		{
			double x = toPixel(element.attribute("x"), true);
			double y = toPixel(element.attribute("y"), false);
			double w = toPixel(element.attribute("width"), true);
			double h = toPixel(element.attribute("height"), false);

			double rx = 0.0;
			double ry = 0.0;

			if(element.hasAttribute("rx"))
				rx = toPixel(element.attribute("rx"), true);

			if(element.hasAttribute("ry"))
				ry = toPixel(element.attribute("ry"), false);

			m_engine->painter()->drawRectangle(x, y, w, h, rx, ry);
		}
		else if(element.tagName() == "switch")
		{
			QDomNode iterate = element.firstChild();

			while(!iterate.isNull())
			{
				// Reset matrix
				m_engine->painter()->setWorldMatrix(new QMatrix(m_initialMatrix));

				// Parse common attributes, style / transform
				parseCommonAttributes(iterate);

				if(handleTags(iterate.toElement(), true))
					return true;
				iterate = iterate.nextSibling();
			}
			return true;
		}
		else if(element.tagName() == "g" || element.tagName() == "defs")
		{
			QDomNode iterate = element.firstChild();

			while(!iterate.isNull())
			{
				// Reset matrix
				m_engine->painter()->setWorldMatrix(new QMatrix(m_initialMatrix));

				// Parse common attributes, style / transform
				parseCommonAttributes(iterate);

				handleTags(iterate.toElement(), (element.tagName() == "defs") ? false : true);
				iterate = iterate.nextSibling();
			}
			return true;
		}
		else if(element.tagName() == "line")
		{
			double x1 = toPixel(element.attribute("x1"), true);
			double y1 = toPixel(element.attribute("y1"), false);
			double x2 = toPixel(element.attribute("x2"), true);
			double y2 = toPixel(element.attribute("y2"), false);

			m_engine->painter()->drawLine(x1, y1, x2, y2);
			return true;
		}
		else if(element.tagName() == "circle")
		{
			double cx = toPixel(element.attribute("cx"), true);
			double cy = toPixel(element.attribute("cy"), false);

			double r = toPixel(element.attribute("r"), true); // TODO: horiz correct?

			m_engine->painter()->drawEllipse(cx, cy, r, r);
			return true;
		}
		else if(element.tagName() == "ellipse")
		{
			double cx = toPixel(element.attribute("cx"), true);
			double cy = toPixel(element.attribute("cy"), false);

			double rx = toPixel(element.attribute("rx"), true);
			double ry = toPixel(element.attribute("ry"), false);

			m_engine->painter()->drawEllipse(cx, cy, rx, ry);
			return true;
		}
		else if(element.tagName() == "polyline")
		{
			QPolygon polyline = parsePoints(element.attribute("points"));
			m_engine->painter()->drawPolyline(polyline);
			return true;
		}
		else if(element.tagName() == "polygon")
		{
			QPolygon polygon = parsePoints(element.attribute("points"));
			m_engine->painter()->drawPolygon(polygon);
			return true;
		}
		else if(element.tagName() == "path")
		{
			bool filled = true;

			if(element.hasAttribute("fill") && element.attribute("fill").contains("none"))
				filled = false;

			if(element.attribute("style").contains("fill") && element.attribute("style").trimmed().contains("fill:none"))
				filled = false;

			m_engine->painter()->drawPath(element.attribute("d"), filled);
			return true;
		}
		else if(element.tagName() == "image")
		{
			double x = toPixel(element.attribute("x"), true);
			double y = toPixel(element.attribute("y"), false);
			double w = toPixel(element.attribute("width"), true);
			double h = toPixel(element.attribute("height"), false);

			QString href = element.attribute("xlink:href");

			if(href.startsWith("data:"))
			{
				// Get input
				QByteArray input = href.mid(13).toUtf8();

				// Decode into 'output'
				QByteArray output;
				KCodecs::base64Decode(input, output);

				// Display
				QImage image = QImage::fromData(output);

				// Scale, if needed
				if(image.width() != (int) w || image.height() != (int) h)
				{
					QImage show = image.scaled((int) w, (int) h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
					m_engine->painter()->drawImage(x, y, show);
				}

				m_engine->painter()->drawImage(x, y, image);
			}
			return true;
		}
		return false;
	}

	void parseStyle(const QString &style)
	{
		QStringList substyles = style.split(';', QString::SkipEmptyParts);
		for(QStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it)
		{
			QStringList substyle = (*it).split(':', QString::SkipEmptyParts);
			QString command = substyle[0];
			QString params = substyle[1];
			command = command.trimmed();
			params = params.trimmed();

			parsePA(command, params);
		}
	}

	void parsePA(const QString &command, const QString &value)
	{
		if(command == "stroke-width") // TODO: horiz:false correct?
			m_engine->painter()->setStrokeWidth(toPixel(value, false));
		else if(command == "stroke-miterlimit")
			m_engine->painter()->setStrokeMiterLimit(value);
		else if(command == "stroke-linecap")
			m_engine->painter()->setCapStyle(value);
		else if(command == "stroke-linejoin")
			m_engine->painter()->setJoinStyle(value);
		else if(command == "stroke-dashoffset")
			m_engine->painter()->setStrokeDashOffset(value);
		else if(command == "stroke-dasharray" && value != "none")
			m_engine->painter()->setStrokeDashArray(value);
		else if(command == "stroke")
			m_engine->painter()->setStrokeColor(value);
		else if(command == "fill")
			m_engine->painter()->setFillColor(value);
		else if(command == "fill-rule")
			m_engine->painter()->setFillRule(value);
		else if(command == "fill-opacity" || command == "stroke-opacity" || command == "opacity")
		{
			if(command == "fill-opacity")
				m_engine->painter()->setFillOpacity(value);
			else if(command == "stroke-value")
				m_engine->painter()->setStrokeOpacity(value);
			else
			{
				m_engine->painter()->setOpacity(value);
				m_engine->painter()->setFillOpacity(value);
				m_engine->painter()->setStrokeOpacity(value);
			}
		}
	}

private:
	friend class KSVGIconEngine;

	KSVGIconEngine *m_engine;
	QMatrix m_initialMatrix;
};

struct KSVGIconEngine::Private
{
	KSVGIconPainter *painter;
	KSVGIconEngineHelper *helper;

	double width;
	double height;
};

KSVGIconEngine::KSVGIconEngine() : d(new Private())
{
	d->painter = 0;
	d->helper = new KSVGIconEngineHelper(this);

	d->width = 0.0;
	d->height = 0.0;
}

KSVGIconEngine::~KSVGIconEngine()
{
	if(d->painter)
		delete d->painter;

	delete d->helper;

	delete d;
}

bool KSVGIconEngine::load(int width, int height, const QString &path)
{
	QDomDocument svgDocument("svg");
	QFile file(path);

	if(path.right(3).toUpper() == "SVG")
	{
		// Open SVG Icon
		if(!file.open(QIODevice::ReadOnly))
			return false;

		svgDocument.setContent(&file);
	}
	else // SVGZ
	{
		gzFile svgz = gzopen(path.toLatin1().data(), "ro");
		if(!svgz)
			return false;

		QString data;
		bool done = false;

		QByteArray buffer(1024, '\0');
		int length = 0;

		while(!done)
		{
			int ret = gzread(svgz, buffer.data() + length, 1024);
			if(ret == 0)
				done = true;
			else if(ret == -1)
				return false;
			else {
				buffer.resize(buffer.size()+1024);
				length += ret;
			}
		}

		gzclose(svgz);

		svgDocument.setContent(buffer);
	}

	if(svgDocument.isNull())
		return false;

	// Check for root element
	QDomNode rootNode = svgDocument.namedItem("svg");
	if(rootNode.isNull() || !rootNode.isElement())
		return false;

	// Detect width and height
	QDomElement rootElement = rootNode.toElement();

	// Create icon painter
	d->painter = new KSVGIconPainter(width, height);

	d->width = width; // this sets default for no width -> 100% case
	if(rootElement.hasAttribute("width"))
		d->width = d->helper->toPixel(rootElement.attribute("width"), true);

	d->height = height; // this sets default for no height -> 100% case
	if(rootElement.hasAttribute("height"))
		d->height = d->helper->toPixel(rootElement.attribute("height"), false);

	// Create icon painter
	d->painter->setDrawWidth(static_cast<int>(d->width));
	d->painter->setDrawHeight(static_cast<int>(d->height));

	// Set viewport clipping rect
	d->painter->setClippingRect(0, 0, width, height);

	// Apply viewbox
	if(rootElement.hasAttribute("viewBox"))
	{
		QStringList points = rootElement.attribute("viewBox").simplified().split(' ', QString::SkipEmptyParts);

		float w = points[2].toFloat();
		float h = points[3].toFloat();

		double vratiow = width / w;
		double vratioh = height / h;

		d->width = w;
		d->height = h;

		d->painter->worldMatrix()->scale(vratiow, vratioh);
	}
	else
	{
		// Fit into 'width' and 'height'
		// FIXME: Use an aspect ratio
		double ratiow = width / d->width;
		double ratioh = height / d->height;

		d->painter->worldMatrix()->scale(ratiow, ratioh);
	}

	QMatrix initialMatrix = *d->painter->worldMatrix();
	d->helper->m_initialMatrix = initialMatrix;

	// Apply transform
	if(rootElement.hasAttribute("transform"))
		d->helper->parseTransform(rootElement.attribute("transform"));

	// Go through all elements
	QDomNode svgNode = rootElement.firstChild();
	while(!svgNode.isNull())
	{
		QDomElement svgChild = svgNode.toElement();
		if(!svgChild.isNull())
		{
			d->helper->parseCommonAttributes(svgNode);
			d->helper->handleTags(svgChild, true);
		}

		svgNode = svgNode.nextSibling();

		// Reset matrix
		d->painter->setWorldMatrix(new QMatrix(initialMatrix));
	}

	d->painter->finish();

	return true;
}

KSVGIconPainter *KSVGIconEngine::painter()
{
	return d->painter;
}

QImage *KSVGIconEngine::image()
{
	return d->painter->image();
}

double KSVGIconEngine::width()
{
	return d->width;
}

double KSVGIconEngine::height()
{
	return d->height;
}

// vim:ts=4:noet
