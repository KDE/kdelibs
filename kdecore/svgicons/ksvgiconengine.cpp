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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qdom.h>
#include <qfile.h>
#include <qcolor.h>
#include <qimage.h>
#include <qregexp.h>
#include <qwmatrix.h>

#include <kdebug.h>
#include <kmdcodec.h>

#include <zlib.h>
#include <math.h>

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

	double dpi()
	{
	    return 90.0; // TODO: make modal?
	}
	
	double toPixel(const QString &s, bool hmode)
	{
		if(s.isEmpty())
			return 0.0;

		QString check = s;

		double ret = 0.0;

		bool ok = false;

		double value = check.toDouble(&ok);

		if(!ok)
		{
			QRegExp reg("[0-9 .-]");
			check.replace(reg, "");

			if(check.compare("px") == 0)
				ret = value;
			else if(check.compare("cm") == 0)
				ret = (value / 2.54) * dpi();
			else if(check.compare("pc") == 0)
				ret = (value / 6.0) * dpi();
			else if(check.compare("mm") == 0)
				ret = (value / 25.4) * dpi();
			else if(check.compare("in") == 0)
				ret = value * dpi();
			else if(check.compare("pt") == 0)
				ret = (value / 72.0) * dpi();
			else if(check.compare("%") == 0)
			{
				ret = value / 100.0;

				if(hmode)
					ret *= m_engine->width();
				else
					ret *= m_engine->height();
			}
			else if(check.compare("em") == 0)
			{
				ret = (value / 72.0) * dpi() * 12.0; // TODO make this depend on actual font size
			}
		}
		else
			ret = value;

		return ret;
	}
	
	ArtGradientStop *parseGradientStops(QDomElement element, int &offsets)
	{
		QMemArray<ArtGradientStop> *stopArray = new QMemArray<ArtGradientStop>();

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
					
				QStringList substyles = QStringList::split(';', style);
				for(QStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it)
				{
					QStringList substyle = QStringList::split(':', (*it));
					QString command = substyle[0];
					QString params = substyle[1];
					command = command.stripWhiteSpace();
					params = params.stripWhiteSpace();

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
			QString tempName = qStopColor.name();
			const char *str = tempName.latin1();

			int opacity = 0xff;
			int stopColor = 0;
				
			for(int i = 1; str[i]; i++)
			{
				int hexval;
				if(str[i] >= '0' && str[i] <= '9')
					hexval = str[i] - '0';
				else if (str[i] >= 'A' && str[i] <= 'F')
					hexval = str[i] - 'A' + 10;
				else if (str[i] >= 'a' && str[i] <= 'f')
					hexval = str[i] - 'a' + 10;
				else
					break;
				
				stopColor = (stopColor << 4) + hexval;
			}
			
			if(!parseOpacity.isEmpty())
			{
				double temp;
		
				if(parseOpacity.contains("%"))
				{
					QString tempString = parseOpacity.left(parseOpacity.length() - 1);
					temp = double(255 * tempString.toDouble()) / 100.0;
				}
				else
					temp = parseOpacity.toDouble();

				opacity = (int) floor(temp * 255 + 0.5);
			}

			Q_UINT32 rgba = (stopColor << 8) | opacity;
			Q_UINT32 r, g, b, a;				
				
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

	QPointArray parsePoints(QString points)
	{
		if(points.isEmpty())
			return QPointArray();

		points = points.simplifyWhiteSpace();

		if(points.contains(",,") || points.contains(", ,"))
			return QPointArray();

		points.replace(QRegExp(","), " ");
		points.replace(QRegExp("\r"), "");
		points.replace(QRegExp("\n"), "");

		points = points.simplifyWhiteSpace();

		QStringList pointList = QStringList::split(' ', points);

		QPointArray array(pointList.count() / 2);
		int i = 0;		
		
		for(QStringList::Iterator it = pointList.begin(); it != pointList.end(); it++)
		{
			float x = (*(it++)).toFloat();
			float y = (*(it)).toFloat();
			
			array.setPoint(i, x, y);
	        i++;
		}

		return array;
	}

	void parseTransform(const QString &transform)
	{
		// Combine new and old matrix
		QWMatrix matrix = m_engine->painter()->parseTransform(transform);
		
		QWMatrix *current = m_engine->painter()->worldMatrix();
		*current *= matrix;
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
		QPtrList<QDomNamedNodeMap> applyList;
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

				name = attr.item(i).nodeName().lower();
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

			if(current.nodeName().lower() == "transform")
				parseTransform(current.nodeValue());
			else if(current.nodeName().lower() == "style")
				parseStyle(current.nodeValue());
			else
				parsePA(current.nodeName().lower(), current.nodeValue());
		}
	}

	void handleTags(QDomElement element, bool paint)
	{
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
			return;
		}
		else if(element.tagName() == "radialGradient")
		{
			ArtGradientRadial *gradient = new ArtGradientRadial();

			int offsets = -1;
			gradient->stops = parseGradientStops(element, offsets);
			gradient->n_stops = offsets + 1;
				
			m_engine->painter()->addRadialGradient(element.attribute("id"), gradient);	
			m_engine->painter()->addRadialGradientElement(gradient, element);	
			return;
		}
		
		if(!paint)
			return;

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
		else if(element.tagName() == "g" || element.tagName() == "defs")
		{
			QDomNode iterate = element.firstChild();

			while(!iterate.isNull())
			{
				// Reset matrix
				m_engine->painter()->setWorldMatrix(new QWMatrix(m_initialMatrix));

				// Parse common attributes, style / transform
				parseCommonAttributes(iterate);
				
				handleTags(iterate.toElement(), (element.tagName() == "defs") ? false : true);
				iterate = iterate.nextSibling();
			}
		}
		else if(element.tagName() == "line")
		{
			double x1 = toPixel(element.attribute("x1"), true);
			double y1 = toPixel(element.attribute("y1"), false);
			double x2 = toPixel(element.attribute("x2"), true);
			double y2 = toPixel(element.attribute("y2"), false);

			m_engine->painter()->drawLine(x1, y1, x2, y2);
		}
		else if(element.tagName() == "circle")
		{
			double cx = toPixel(element.attribute("cx"), true);
			double cy = toPixel(element.attribute("cy"), false);

			double r = toPixel(element.attribute("r"), true); // TODO: horiz correct?

			m_engine->painter()->drawEllipse(cx, cy, r, r);
		}
		else if(element.tagName() == "ellipse")
		{
			double cx = toPixel(element.attribute("cx"), true);
			double cy = toPixel(element.attribute("cy"), false);

			double rx = toPixel(element.attribute("rx"), true);
			double ry = toPixel(element.attribute("ry"), false);
				
			m_engine->painter()->drawEllipse(cx, cy, rx, ry);
		}
		else if(element.tagName() == "polyline")
		{
			QPointArray polyline = parsePoints(element.attribute("points"));
			m_engine->painter()->drawPolyline(polyline);
		}
		else if(element.tagName() == "polygon")
		{
			QPointArray polygon = parsePoints(element.attribute("points"));
			m_engine->painter()->drawPolygon(polygon);
		}
		else if(element.tagName() == "path")
		{
			bool filled = true;

			if(element.hasAttribute("fill") && element.attribute("fill").contains("none"))
				filled = false;

			if(element.attribute("style").contains("fill") && element.attribute("style").stripWhiteSpace().contains("fill:none"))
				filled = false;

			m_engine->painter()->drawPath(element.attribute("d"), filled);
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
				QCString input = href.mid(13).utf8();

				// Decode into 'output'
				QByteArray output;
				KCodecs::base64Decode(input, output);

				// Display
				QImage *image = new QImage(output);

				// Scale, if needed
				if(image->width() != (int) w || image->height() != (int) h)
				{
					QImage show = image->smoothScale((int) w, (int) h, QImage::ScaleMin);
					m_engine->painter()->drawImage(x, y, show);
				}

				m_engine->painter()->drawImage(x, y, *image);
			}
		}
	}
	
	void parseStyle(const QString &style)
	{
		QStringList substyles = QStringList::split(';', style);
		for(QStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it)
		{
			QStringList substyle = QStringList::split(':', (*it));
			QString command = substyle[0];
			QString params = substyle[1];
			command = command.stripWhiteSpace();
			params = params.stripWhiteSpace();
			
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
		else if(command == "stroke-dasharray")
			m_engine->painter()->setStrokeDashArray(value);
		else if(command == "stroke")
			m_engine->painter()->setStrokeColor(value);
		else if(command == "fill")
			m_engine->painter()->setFillColor(value);
		else if(command == "fill-rule")
			m_engine->painter()->setFillRule(value);
		else if(command == "fill-opacity" || command == "stroke-opacity" || command == "opacity")
		{
			double opacity;
			
			if(value.contains("%"))
			{
				QString temp = value.left(value.length() - 1);
				opacity = double(255 * temp.toDouble()) / 100.0;
			}
			else
				opacity = value.toDouble();
	
			if(command == "fill-opacity")
				m_engine->painter()->setFillOpacity(opacity);
			else if(command == "stroke-opacity")
				m_engine->painter()->setStrokeOpacity(opacity);
			else
			{
				m_engine->painter()->setFillOpacity(opacity);
				m_engine->painter()->setStrokeOpacity(opacity);
			}
		}
	}

private:
	friend class KSVGIconEngine;
	
	KSVGIconEngine *m_engine;
	QWMatrix m_initialMatrix;
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

	if(path.right(3).upper() == "SVG")
	{
		// Open SVG Icon
		if(!file.open(IO_ReadOnly))
			return false;

		svgDocument.setContent(&file);
	}
	else // SVGZ
	{
		gzFile svgz = gzopen(path.latin1(), "ro");
		if(!svgz)
			return false;

		QString data;
		bool done = false;
		
		char *buffer = new char[1024];

		while(!done)
		{
			memset(buffer, 0, 1024);
			
			int ret = gzread(svgz, buffer, 1024);
			if(ret == 0)
				done = true;
			else if(ret == -1)
				return false;
			
			data += QString::fromUtf8(buffer);
		}
		
		gzclose(svgz);

		svgDocument.setContent(data);
	}

	if(svgDocument.isNull())
		return false;

	// Check for root element
	QDomNode rootNode = svgDocument.namedItem("svg");
	if(rootNode.isNull() || !rootNode.isElement())
		return false;

	// Detect width and height
	QDomElement rootElement = rootNode.toElement();

	d->width = width;

	if(rootElement.hasAttribute("width"))
		d->width = d->helper->toPixel(rootElement.attribute("width"), true);
	else
		d->width = d->helper->toPixel("100%", true);

	d->height = height;

	if(rootElement.hasAttribute("height"))
		d->height = d->helper->toPixel(rootElement.attribute("height"), false);
	else
		d->height = d->helper->toPixel("100%", false);

	// Create icon painter
	d->painter = new KSVGIconPainter(width, height, d->width, d->height);

	// Set viewport clipping rect
	d->painter->setClippingRect(0, 0, width, height);

	// Apply viewbox
	if(rootElement.hasAttribute("viewBox"))
	{
		QStringList points = QStringList::split(' ', rootElement.attribute("viewBox").simplifyWhiteSpace());

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

	QWMatrix initialMatrix = *d->painter->worldMatrix();
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
		d->painter->setWorldMatrix(new QWMatrix(initialMatrix));
	}

	d->painter->finish();

	return true;
}

KSVGIconPainter *KSVGIconEngine::painter()
{
	return d->painter;
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
