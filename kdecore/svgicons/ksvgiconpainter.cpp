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
    aint with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qvaluevector.h>
#include <qstringlist.h>
#include <qwmatrix.h>
#include <qregexp.h>
#include <qimage.h>
#include <qdict.h>
#include <qmap.h>
#include <qdom.h>

#include <math.h>

#include <kdebug.h>

#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_rgb.h>
#include <libart_lgpl/art_rgba.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_vpath_dash.h>
#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_svp_ops.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_render_svp.h>
#include <libart_lgpl/art_vpath_bpath.h>
#include <libart_lgpl/art_svp_intersect.h>
#include <libart_lgpl/art_svp_vpath_stroke.h>

#include "ksvgiconpainter.h"

#define ART_END2 10

const double deg2rad = 0.017453292519943295769; // pi/180

class KSVGIconPainterHelper
{
public:
	KSVGIconPainterHelper(int width, int height, KSVGIconPainter *painter)
	{
		m_painter = painter;
		
		m_clipSVP = 0;

		m_fillColor = Qt::black;
		
		m_useFill = true;
		m_useStroke = false;

		m_useFillGradient = false;
		m_useStrokeGradient = false;

		m_worldMatrix = new QWMatrix();
		
		// Create new image with alpha support
		m_image = new QImage(width, height, 32);
		m_image->setAlphaBuffer(true);

		m_strokeWidth = 1.0;
		m_strokeMiterLimit = 4;
		m_dashOffset = 0;
		m_dashes = "";

		m_opacity = 0xff;
		m_fillOpacity = 0xff;
		m_strokeOpacity = 0xff;
		
		m_fillRule = "nonzero";
		
		m_width = width;
		m_height = height;
		
		m_rowstride = m_width * 4;

		// Make internal libart rendering buffer transparent
		m_buffer = art_new(art_u8, m_rowstride * m_height);
		memset(m_buffer, 0, m_rowstride * m_height);

		m_tempBuffer = 0;
	}

	~KSVGIconPainterHelper()
	{
		if(m_clipSVP)
			art_svp_free(m_clipSVP);

		art_free(m_buffer);

		delete m_image;
		delete m_worldMatrix;
	}

	ArtVpath *allocVPath(int number)
	{
		return art_new(ArtVpath, number);
	}
	
	ArtBpath *allocBPath(int number)
	{
		return art_new(ArtBpath, number);
	}

	void ensureSpace(QMemArray<ArtBpath> &vec, int index)
	{
		if(vec.size() == (unsigned int) index)
			vec.resize(index + 1);
	}

	void createBuffer()
	{
		m_tempBuffer = art_new(art_u8, m_rowstride * m_height);
		memset(m_tempBuffer, 0, m_rowstride * m_height);

		// Swap buffers, so we work with the new one internally...
		art_u8 *temp = m_buffer;
		m_buffer = m_tempBuffer;
		m_tempBuffer = temp;
	}

	void mixBuffer(int opacity)
	{
		art_u8 *srcPixel = m_buffer;
		art_u8 *dstPixel = m_tempBuffer;

		for(int y = 0; y < m_height; y++)
		{
			for(int x = 0; x < m_width; x++)
			{
				art_u8 r, g, b, a;

				a = srcPixel[4 * x + 3];

				if(a)
				{
					r = srcPixel[4 * x];
					g = srcPixel[4 * x + 1];
					b = srcPixel[4 * x + 2];

					int temp = a * opacity + 0x80;
					a = (temp + (temp >> 8)) >> 8;
					art_rgba_run_alpha(dstPixel + 4 * x, r, g, b, a, 1);
				}
			}

			srcPixel += m_rowstride;
			dstPixel += m_rowstride;
		}

		// Re-swap again...
		art_u8 *temp = m_buffer;
		m_buffer = m_tempBuffer;
		m_tempBuffer = temp;		
		
		art_free(m_tempBuffer);
		m_tempBuffer = 0;
	}

	Q_UINT32 toArtColor(QColor color)
	{
		// Convert in a libart suitable form
		QString tempName = color.name();
		const char *str = tempName.latin1();

		int result = 0;

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

			result = (result << 4) + hexval;
		}

		return result;
	}
	
	void drawSVP(ArtSVP *svp, Q_UINT32 rgb, int opacity)
	{
		if(!svp)
			return;

		ArtRender *render = art_render_new(0, 0, m_width, m_height, m_buffer, m_rowstride, 3, 8, ART_ALPHA_SEPARATE, 0);
		art_render_svp(render, svp);
		
		art_render_mask_solid(render, (opacity << 8) + opacity + (opacity >> 7));

		ArtPixMaxDepth color[3];
		color[0] = ART_PIX_MAX_FROM_8(rgb >> 16);
		color[1] = ART_PIX_MAX_FROM_8((rgb >> 8) & 0xff);
		color[2] = ART_PIX_MAX_FROM_8(rgb & 0xff);
		
		art_render_image_solid(render, color);
		art_render_invoke(render);
	}

	void drawVPath(ArtVpath *vec)
	{
		ArtSVP *svp;

		double affine[6];
		affine[0] = m_worldMatrix->m11();
		affine[1] = m_worldMatrix->m12();
		affine[2] = m_worldMatrix->m21();
		affine[3] = m_worldMatrix->m22();
		affine[4] = m_worldMatrix->dx();
		affine[5] = m_worldMatrix->dy();

		ArtVpath *temp = art_vpath_affine_transform(vec, affine);
		art_free(vec);
		vec = temp;

		ArtSVP *fillSVP = 0, *strokeSVP = 0;

		Q_UINT32 fillColor = 0, strokeColor = 0;

		// Filling
		{
			int index = -1;
			QValueVector<int> toCorrect;
			while(vec[++index].code != ART_END)
			{
				if(vec[index].code == ART_END2)
				{
					vec[index].code = ART_LINETO;
					toCorrect.push_back(index);
				}
			}

			fillColor = toArtColor(m_fillColor);

			ArtSvpWriter *swr;
			ArtSVP *temp;
			temp = art_svp_from_vpath(vec);

			if(m_fillRule == "evenodd")
				swr = art_svp_writer_rewind_new(ART_WIND_RULE_ODDEVEN);
			else
				swr = art_svp_writer_rewind_new(ART_WIND_RULE_NONZERO);

			art_svp_intersector(temp, swr);
			svp = art_svp_writer_rewind_reap(swr);

			fillSVP = svp;

			art_svp_free(temp);

			QValueVector<int>::iterator it;
			for(it = toCorrect.begin(); it != toCorrect.end(); ++it)
				vec[(*it)].code = (ArtPathcode)ART_END2;
		}

		// There seems to be a problem when stroke width is zero, this is a quick
		// fix (Rob).
		if(m_strokeWidth <= 0)
			m_useStroke = m_useStrokeGradient = false;
		
		// Stroking
		if(m_useStroke || m_useStrokeGradient)
		{
			strokeColor = toArtColor(m_strokeColor);

			double ratio = sqrt(pow(affine[0], 2) + pow(affine[3], 2)) / sqrt(2.0);
			double strokeWidth = m_strokeWidth * ratio;

		    ArtPathStrokeJoinType joinStyle = ART_PATH_STROKE_JOIN_MITER;
			ArtPathStrokeCapType capStyle = ART_PATH_STROKE_CAP_BUTT;
			
			if(m_joinStyle == "miter")
				joinStyle = ART_PATH_STROKE_JOIN_MITER;
			else if(m_joinStyle == "round")
				joinStyle = ART_PATH_STROKE_JOIN_ROUND;
			else if(m_joinStyle == "bevel")
				joinStyle = ART_PATH_STROKE_JOIN_BEVEL;

			if(m_capStyle == "butt")
				capStyle = ART_PATH_STROKE_CAP_BUTT;
			else if(m_capStyle == "round")
				capStyle = ART_PATH_STROKE_CAP_ROUND;
			else if(m_capStyle == "square")
				capStyle = ART_PATH_STROKE_CAP_SQUARE;

			if(m_dashes.length() > 0)
			{
				QRegExp reg("[a-zA-Z,(; ]");
				QStringList dashList = QStringList::split(reg, m_dashes);

				double *dashes = new double[dashList.count()];
				for(unsigned int i = 0; i < dashList.count(); i++)
					dashes[i] = dashList[i].toDouble();

				ArtVpathDash dash;
				dash.offset = m_dashOffset;
				dash.n_dash = dashList.count();

				dash.dash = dashes;

				ArtVpath *vec2 = art_vpath_dash(vec, &dash);
				art_free(vec);

				delete dashes;

				vec = vec2;
			}

			svp = art_svp_vpath_stroke(vec, joinStyle, capStyle, strokeWidth, m_strokeMiterLimit, 0.25);

			strokeSVP = svp;
		}

		// Apply opacity
		int fillOpacity = static_cast<int>(m_fillOpacity);
		int strokeOpacity = static_cast<int>(m_strokeOpacity);
		int opacity = static_cast<int>(m_opacity);
		
		// Needed hack, to support both transparent
		// paths and transparent gradients
		if(fillOpacity == strokeOpacity && fillOpacity == opacity && !m_useFillGradient && !m_useStrokeGradient)
			opacity = 255;

		if(fillOpacity != 255)
		{
			int temp = fillOpacity * opacity + 0x80;
			fillOpacity = (temp + (temp >> 8)) >> 8;
		}
		
		if(strokeOpacity != 255)
		{
			int temp = strokeOpacity * opacity + 0x80;
			strokeOpacity = (temp + (temp >> 8)) >> 8;
		}
		
		// Create temporary buffer if necessary
		bool tempDone = false;
		if(m_opacity != 0xff)
		{
			tempDone = true;
			createBuffer();
		}

		// Apply Gradients on fill/stroke
		if(m_useFillGradient)
			applyGradient(fillSVP, true);
		else if(m_useFill)
			drawSVP(fillSVP, fillColor, fillOpacity);

		if(m_useStrokeGradient)
			applyGradient(strokeSVP, false);
		else if(m_useStroke)
			drawSVP(strokeSVP, strokeColor, strokeOpacity);

		// Mix in temporary buffer, if possible
		if(tempDone)
			mixBuffer(opacity);
	
		if(m_clipSVP)
		{
			art_svp_free(m_clipSVP);
			m_clipSVP = 0;
		}

		if(fillSVP)
			art_svp_free(fillSVP);

		if(strokeSVP)
			art_svp_free(strokeSVP);

		// Reset opacity values
		m_opacity = 255.0;
		m_fillOpacity = 255.0;
		m_strokeOpacity = 255.0;
		
		art_free(vec);
	}

	void applyLinearGradient(ArtSVP *svp, const QString &ref)
	{
		ArtGradientLinear *linear = m_linearGradientMap[ref];
		if(linear)
		{
			QDomElement element = m_linearGradientElementMap[linear];
			
			double x1, y1, x2, y2;
			if(element.hasAttribute("x1"))
				x1 = m_painter->toPixel(element.attribute("x1"), true);
			else
				x1 = 0;

			if(element.hasAttribute("y1"))
				y1 = m_painter->toPixel(element.attribute("y1"), false);
			else
				y1 = 0;

			if(element.hasAttribute("x2"))
				x2 = m_painter->toPixel(element.attribute("x2"), true);
			else
				x2 = 100;

			if(element.hasAttribute("y2"))
				y2 = m_painter->toPixel(element.attribute("y2"), false);
			else
				y2 = 0;

			// Adjust to gradientTransform
			QWMatrix m = m_painter->parseTransform(element.attribute("gradientTransform"));
			m.map(x1, y1, &x1, &y1);
			m.map(x2, y2, &x2, &y2);

			double x1n = x1 * m_worldMatrix->m11() + y1 * m_worldMatrix->m21() + m_worldMatrix->dx();
			double y1n = x1 * m_worldMatrix->m12() + y1 * m_worldMatrix->m22() + m_worldMatrix->dy();
			double x2n = x2 * m_worldMatrix->m11() + y2 * m_worldMatrix->m21() + m_worldMatrix->dx();
			double y2n = x2 * m_worldMatrix->m12() + y2 * m_worldMatrix->m22() + m_worldMatrix->dy();

			double dx = x2n - x1n;
			double dy = y2n - y1n;
			double scale = 1.0 / (dx * dx + dy * dy);

			linear->a = dx * scale;
			linear->b = dy * scale;
			linear->c = -(x1n * linear->a + y1n * linear->b);

			ArtRender *render = art_render_new(0, 0, m_width, m_height, m_buffer, m_rowstride, 3, 8, ART_ALPHA_SEPARATE, 0);
			art_render_svp(render, svp);

			art_render_gradient_linear(render, linear, ART_FILTER_HYPER);
			art_render_invoke(render);		
		}
	}

	void applyRadialGradient(ArtSVP *svp, const QString &ref)
	{
		ArtGradientRadial *radial = m_radialGradientMap[ref];
		if(radial)
		{
			QDomElement element = m_radialGradientElementMap[radial];

			double cx, cy, r, fx, fy;				
			if(element.hasAttribute("cx"))
				cx = m_painter->toPixel(element.attribute("cx"), true);
			else
				cx = 50;

			if(element.hasAttribute("cy"))
				cy = m_painter->toPixel(element.attribute("cy"), false);
			else
				cy = 50;

			if(element.hasAttribute("r"))
				r = m_painter->toPixel(element.attribute("r"), true);
			else
				r = 50;

			if(element.hasAttribute("fx"))
				fx = m_painter->toPixel(element.attribute("fx"), false);
			else
				fx = cx;

			if(element.hasAttribute("fy"))
				fy = m_painter->toPixel(element.attribute("fy"), false);
			else
				fy = cy;

			radial->affine[0] = m_worldMatrix->m11();
			radial->affine[1] = m_worldMatrix->m12();
			radial->affine[2] = m_worldMatrix->m21();
			radial->affine[3] = m_worldMatrix->m22();
			radial->affine[4] = m_worldMatrix->dx();
			radial->affine[5] = m_worldMatrix->dy();

			radial->fx = (fx - cx) / r;
			radial->fy = (fy - cy) / r;

			double aff1[6], aff2[6], gradTransform[6];

			// Respect gradientTransform
			QWMatrix m = m_painter->parseTransform(element.attribute("gradientTransform"));

			gradTransform[0] = m.m11();
			gradTransform[1] = m.m12();
			gradTransform[2] = m.m21();
			gradTransform[3] = m.m22();
			gradTransform[4] = m.dx();
			gradTransform[5] = m.dy();

			art_affine_scale(aff1, r, r);
			art_affine_translate(aff2, cx, cy);

			art_affine_multiply(aff1, aff1, aff2);
			art_affine_multiply(aff1, aff1, gradTransform);
			art_affine_multiply(aff1, aff1, radial->affine);
			art_affine_invert(radial->affine, aff1);

			ArtRender *render = art_render_new(0, 0, m_width, m_height, m_buffer, m_rowstride, 3, 8, ART_ALPHA_SEPARATE, 0);
			art_render_svp(render, svp);

			art_render_gradient_radial(render, radial, ART_FILTER_HYPER);
			art_render_invoke(render);		
		}
	}

	void applyGradient(ArtSVP *svp, const QString &ref)
	{
		ArtGradientLinear *linear = m_linearGradientMap[ref];
		if(linear)
		{
			QDomElement element = m_linearGradientElementMap[linear];

			if(!element.hasAttribute("xlink:href"))
			{
				applyLinearGradient(svp, ref);
				return;
			}
			else
			{
				ArtGradientLinear *linear = m_linearGradientMap[element.attribute("xlink:href").mid(1)];
				QDomElement newElement = m_linearGradientElementMap[linear];

				// Saved 'old' attributes
				QDict<QString> refattrs;
				refattrs.setAutoDelete(true);

				for(unsigned int i = 0; i < newElement.attributes().length(); ++i)
					refattrs.insert(newElement.attributes().item(i).nodeName(), new QString(newElement.attributes().item(i).nodeValue()));

				// Copy attributes
				if(!newElement.isNull())
				{
					QDomNamedNodeMap attr = element.attributes();

					for(unsigned int i = 0; i < attr.length(); i++)
					{
						QString name = attr.item(i).nodeName();
						if(name != "xlink:href" && name != "id")
							newElement.setAttribute(name, attr.item(i).nodeValue());
					}
				}
				
				applyGradient(svp, element.attribute("xlink:href").mid(1));

				// Restore attributes
				QDictIterator<QString> itr(refattrs);
				for(; itr.current(); ++itr)
					newElement.setAttribute(itr.currentKey(), *(itr.current()));
				
				return;
			}
		}
		
		ArtGradientRadial *radial = m_radialGradientMap[ref];
		if(radial)
		{
			QDomElement element = m_radialGradientElementMap[radial];

			if(!element.hasAttribute("xlink:href"))
			{
				applyRadialGradient(svp, ref);
				return;
			}
			else
			{
				ArtGradientRadial *radial = m_radialGradientMap[element.attribute("xlink:href").mid(1)];
				QDomElement newElement = m_radialGradientElementMap[radial];

				// Saved 'old' attributes
				QDict<QString> refattrs;
				refattrs.setAutoDelete(true);

				for(unsigned int i = 0; i < newElement.attributes().length(); ++i)
					refattrs.insert(newElement.attributes().item(i).nodeName(), new QString(newElement.attributes().item(i).nodeValue()));

				// Copy attributes
				if(!newElement.isNull())
				{
					QDomNamedNodeMap attr = element.attributes();

					for(unsigned int i = 0; i < attr.length(); i++)
					{
						QString name = attr.item(i).nodeName();
						if(name != "xlink:href" && name != "id")
							newElement.setAttribute(name, attr.item(i).nodeValue());
					}
				}
				
				applyGradient(svp, element.attribute("xlink:href").mid(1));

				// Restore attributes
				QDictIterator<QString> itr(refattrs);
				for(; itr.current(); ++itr)
					newElement.setAttribute(itr.currentKey(), *(itr.current()));

				return;
			}
		}
	}

	void applyGradient(ArtSVP *svp, bool fill)
	{
		QString ref;
		
		if(fill)
		{
			m_useFillGradient = false;
			ref = m_fillGradientReference;
		}
		else
		{
			m_useStrokeGradient = false;
			ref = m_strokeGradientReference;
		}

		applyGradient(svp, ref);
	}

	void blit()
	{
		  unsigned char *line = m_buffer;
		  
		  for(int y = 0; y < m_height; y++)
		  {
			  QRgb *sl = reinterpret_cast<QRgb *>(m_image->scanLine(y));
			  for(int x = 0; x < m_width; x++)
				  sl[x] = qRgba(line[x * 4], line[x * 4 + 1], line[x * 4 + 2], line[x * 4 + 3]);

			  line += m_rowstride;
		  }
	}

	void calculateArc(bool relative, QMemArray<ArtBpath> &vec, int &index, double &curx, double &cury, double angle, double x, double y, double r1, double r2, bool largeArcFlag, bool sweepFlag)
	{
		double sin_th, cos_th;
		double a00, a01, a10, a11;
		double x0, y0, x1, y1, xc, yc;
		double d, sfactor, sfactor_sq;
		double th0, th1, th_arc;
		int i, n_segs;

		sin_th = sin(angle * (M_PI / 180.0));
		cos_th = cos(angle * (M_PI / 180.0));

		double dx;

		if(!relative)
			dx = (curx - x) / 2.0;
		else
			dx = -x / 2.0;

		double dy;
		
		if(!relative)
			dy = (cury - y) / 2.0;
		else
			dy = -y / 2.0;
		
		double _x1 =  cos_th * dx + sin_th * dy;
		double _y1 = -sin_th * dx + cos_th * dy;
		double Pr1 = r1 * r1;
		double Pr2 = r2 * r2;
		double Px = _x1 * _x1;
		double Py = _y1 * _y1;

		// Spec : check if radii are large enough
		double check = Px / Pr1 + Py / Pr2;
		if(check > 1)
		{
			r1 = r1 * sqrt(check);
			r2 = r2 * sqrt(check);
		}

		a00 = cos_th / r1;
		a01 = sin_th / r1;
		a10 = -sin_th / r2;
		a11 = cos_th / r2;

		x0 = a00 * curx + a01 * cury;
		y0 = a10 * curx + a11 * cury;

		if(!relative)
			x1 = a00 * x + a01 * y;
		else
			x1 = a00 * (curx + x) + a01 * (cury + y);
		
		if(!relative)
			y1 = a10 * x + a11 * y;
		else
			y1 = a10 * (curx + x) + a11 * (cury + y);

		/* (x0, y0) is current point in transformed coordinate space.
		   (x1, y1) is new point in transformed coordinate space.

		   The arc fits a unit-radius circle in this space.
	     */

		d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);

		sfactor_sq = 1.0 / d - 0.25;

		if(sfactor_sq < 0)
			sfactor_sq = 0;

		sfactor = sqrt(sfactor_sq);

		if(sweepFlag == largeArcFlag)
			sfactor = -sfactor;

		xc = 0.5 * (x0 + x1) - sfactor * (y1 - y0);
		yc = 0.5 * (y0 + y1) + sfactor * (x1 - x0);

		/* (xc, yc) is center of the circle. */
		th0 = atan2(y0 - yc, x0 - xc);
		th1 = atan2(y1 - yc, x1 - xc);

		th_arc = th1 - th0;
		if(th_arc < 0 && sweepFlag)
			th_arc += 2 * M_PI;
		else if(th_arc > 0 && !sweepFlag)
			th_arc -= 2 * M_PI;

		n_segs = (int) (int) ceil(fabs(th_arc / (M_PI * 0.5 + 0.001)));

		for(i = 0; i < n_segs; i++)
		{
			index++;

			ensureSpace(vec, index);

			{
				double sin_th, cos_th;
				double a00, a01, a10, a11;
				double x1, y1, x2, y2, x3, y3;
				double t;
				double th_half;

				double _th0 = th0 + i * th_arc / n_segs;
				double _th1 = th0 + (i + 1) * th_arc / n_segs;

				sin_th = sin(angle * (M_PI / 180.0));
				cos_th = cos(angle * (M_PI / 180.0));

				/* inverse transform compared with rsvg_path_arc */
				a00 = cos_th * r1;
				a01 = -sin_th * r2;
				a10 = sin_th * r1;
				a11 = cos_th * r2;

				th_half = 0.5 * (_th1 - _th0);
				t = (8.0 / 3.0) * sin(th_half * 0.5) * sin(th_half * 0.5) / sin(th_half);
				x1 = xc + cos(_th0) - t * sin(_th0);
				y1 = yc + sin(_th0) + t * cos(_th0);
				x3 = xc + cos(_th1);
				y3 = yc + sin(_th1);
				x2 = x3 + t * sin(_th1);
				y2 = y3 - t * cos(_th1);

				ensureSpace(vec, index);

				vec[index].code = ART_CURVETO;
				vec[index].x1 = a00 * x1 + a01 * y1;
				vec[index].y1 = a10 * x1 + a11 * y1;
				vec[index].x2 = a00 * x2 + a01 * y2;
				vec[index].y2 = a10 * x2 + a11 * y2;
				vec[index].x3 = a00 * x3 + a01 * y3;
				vec[index].y3 = a10 * x3 + a11 * y3;
			}
		}

		if(!relative)
			curx = x;
		else
			curx += x;

		if(!relative)
			cury = y;
		else
			cury += y;	
	}

	// For any docs, see the libart library
	static void art_vpath_render_bez(ArtVpath **p_vpath, int *pn, int *pn_max,
									 double x0, double y0,
									 double x1, double y1,
									 double x2, double y2,
									 double x3, double y3,
									 double flatness)
	{
		double x3_0, y3_0, z3_0_dot, z1_dot, z2_dot;
		double z1_perp, z2_perp, max_perp_sq;

		double x_m, y_m, xa1, ya1, xa2, ya2, xb1, yb1, xb2, yb2;

		x3_0 = x3 - x0;
		y3_0 = y3 - y0;

		z3_0_dot = x3_0 * x3_0 + y3_0 * y3_0;

		if (z3_0_dot < 0.001)
			goto nosubdivide;

		max_perp_sq = flatness * flatness * z3_0_dot;

		z1_perp = (y1 - y0) * x3_0 - (x1 - x0) * y3_0;
		if (z1_perp * z1_perp > max_perp_sq)
			goto subdivide;

		z2_perp = (y3 - y2) * x3_0 - (x3 - x2) * y3_0;
		if (z2_perp * z2_perp > max_perp_sq)
			goto subdivide;

		z1_dot = (x1 - x0) * x3_0 + (y1 - y0) * y3_0;
		if (z1_dot < 0 && z1_dot * z1_dot > max_perp_sq)
			goto subdivide;

		z2_dot = (x3 - x2) * x3_0 + (y3 - y2) * y3_0;
		if (z2_dot < 0 && z2_dot * z2_dot > max_perp_sq)
			goto subdivide;

		if (z1_dot + z1_dot > z3_0_dot)
			goto subdivide;

		if (z2_dot + z2_dot > z3_0_dot)
			goto subdivide;

	nosubdivide:
		art_vpath_add_point (p_vpath, pn, pn_max, ART_LINETO, x3, y3);
		return;

	subdivide:
		xa1 = (x0 + x1) * 0.5;
		ya1 = (y0 + y1) * 0.5;
		xa2 = (x0 + 2 * x1 + x2) * 0.25;
		ya2 = (y0 + 2 * y1 + y2) * 0.25;
		xb1 = (x1 + 2 * x2 + x3) * 0.25;
		yb1 = (y1 + 2 * y2 + y3) * 0.25;
		xb2 = (x2 + x3) * 0.5;
		yb2 = (y2 + y3) * 0.5;
		x_m = (xa2 + xb1) * 0.5;
		y_m = (ya2 + yb1) * 0.5;
		art_vpath_render_bez (p_vpath, pn, pn_max, x0, y0, xa1, ya1, xa2, ya2, x_m, y_m, flatness);
		art_vpath_render_bez (p_vpath, pn, pn_max, x_m, y_m, xb1, yb1, xb2, yb2, x3, y3, flatness);
	}

	ArtVpath *art_bez_path_to_vec(const ArtBpath *bez, double flatness)
	{
		ArtVpath *vec;
		int vec_n, vec_n_max;
		int bez_index;
		double x, y;

		vec_n = 0;
		vec_n_max = (1 << 4);
		vec = art_new (ArtVpath, vec_n_max);

		x = 0;
		y = 0;

		bez_index = 0;
		do
		{
			if(vec_n >= vec_n_max)
				art_expand (vec, ArtVpath, vec_n_max);
			
			switch (bez[bez_index].code)
			{
				case ART_MOVETO_OPEN:
				case ART_MOVETO:
				case ART_LINETO:
					x = bez[bez_index].x3;
					y = bez[bez_index].y3;
					vec[vec_n].code = bez[bez_index].code;
					vec[vec_n].x = x;
					vec[vec_n].y = y;
					vec_n++;
					break;
				case ART_END:
					vec[vec_n].code = ART_END;
					vec[vec_n].x = 0;
					vec[vec_n].y = 0;
					vec_n++;
					break;
				case ART_END2:
					vec[vec_n].code = (ArtPathcode)ART_END2;
					vec[vec_n].x = bez[bez_index].x3;
					vec[vec_n].y = bez[bez_index].y3;
					vec_n++;
					break;
				case ART_CURVETO:
					art_vpath_render_bez (&vec, &vec_n, &vec_n_max,
							x, y,
							bez[bez_index].x1, bez[bez_index].y1,
							bez[bez_index].x2, bez[bez_index].y2,
							bez[bez_index].x3, bez[bez_index].y3,
							flatness);
					x = bez[bez_index].x3;
					y = bez[bez_index].y3;
					break;
			}
		}
		
		while (bez[bez_index++].code != ART_END);
		return vec;
	}

	static void art_rgb_affine_run(int *p_x0, int *p_x1, int y,
								   int src_width, int src_height,
								   const double affine[6])
	{
		int x0, x1;
		double z;
		double x_intercept;
		int xi;

		x0 = *p_x0;
		x1 = *p_x1;

		if (affine[0] > 1e-6)
		{
			z = affine[2] * (y + 0.5) + affine[4];
			x_intercept = -z / affine[0];
			xi = (int) (int) ceil (x_intercept + 1e-6 - 0.5);
			if (xi > x0)
				x0 = xi;
			x_intercept = (-z + src_width) / affine[0];
			xi = (int) ceil (x_intercept - 1e-6 - 0.5);
			if (xi < x1)
				x1 = xi;
		}
		else if (affine[0] < -1e-6)
		{
			z = affine[2] * (y + 0.5) + affine[4];
			x_intercept = (-z + src_width) / affine[0];
			xi = (int) ceil (x_intercept + 1e-6 - 0.5);
			if (xi > x0)
				x0 = xi;
			x_intercept = -z / affine[0];
			xi = (int) ceil (x_intercept - 1e-6 - 0.5);
			if (xi < x1)
				x1 = xi;
		}
		else
		{
			z = affine[2] * (y + 0.5) + affine[4];
			if (z < 0 || z >= src_width)
			{
				*p_x1 = *p_x0;
				return;
			}
		}
		if (affine[1] > 1e-6)
		{
			z = affine[3] * (y + 0.5) + affine[5];
			x_intercept = -z / affine[1];
			xi = (int) ceil (x_intercept + 1e-6 - 0.5);
			if (xi > x0)
				x0 = xi;
			x_intercept = (-z + src_height) / affine[1];
			xi = (int) ceil (x_intercept - 1e-6 - 0.5);
			if (xi < x1)
				x1 = xi;
		}
		else if (affine[1] < -1e-6)
		{
			z = affine[3] * (y + 0.5) + affine[5];
			x_intercept = (-z + src_height) / affine[1];
			xi = (int) ceil (x_intercept + 1e-6 - 0.5);
			if (xi > x0)
				x0 = xi;
			x_intercept = -z / affine[1];
			xi = (int) ceil (x_intercept - 1e-6 - 0.5);
			if (xi < x1)
				x1 = xi;
		}
		else
		{
			z = affine[3] * (y + 0.5) + affine[5];
			if (z < 0 || z >= src_height)
			{
				*p_x1 = *p_x0;
				return;
			}
		}

		*p_x0 = x0;
		*p_x1 = x1;
	}

	// Slightly modified version to support RGBA buffers, copied from gnome-print
	static void art_rgba_rgba_affine(art_u8 *dst,
									 int x0, int y0, int x1, int y1, int dst_rowstride,
									 const art_u8 *src,
									 int src_width, int src_height, int src_rowstride,
									 const double affine[6])
	{
		int x, y;
		double inv[6];
		art_u8 *dst_p, *dst_linestart;
		const art_u8 *src_p;
		ArtPoint pt, src_pt;
		int src_x, src_y;
		int alpha;
		art_u8 bg_r, bg_g, bg_b, bg_a, cr, cg, cb;
		art_u8 fg_r, fg_g, fg_b;
		int tmp;
		int run_x0, run_x1;

		dst_linestart = dst;
		art_affine_invert (inv, affine);
		for (y = y0; y < y1; y++)
		{
			pt.y = y + 0.5;
			run_x0 = x0;
			run_x1 = x1;
			art_rgb_affine_run (&run_x0, &run_x1, y, src_width, src_height,
					inv);
			dst_p = dst_linestart + (run_x0 - x0) * 4;
			for (x = run_x0; x < run_x1; x++)
			{
				pt.x = x + 0.5;
				art_affine_point (&src_pt, &pt, inv);
				src_x = (int) floor (src_pt.x);
				src_y = (int) floor (src_pt.y);
				src_p = src + (src_y * src_rowstride) + src_x * 4;
				if (src_x >= 0 && src_x < src_width &&
						src_y >= 0 && src_y < src_height)
				{

					alpha = src_p[3];
					if (alpha)
					{
						if (alpha == 255)
						{
							dst_p[0] = src_p[0];
							dst_p[1] = src_p[1];
							dst_p[2] = src_p[2];
							dst_p[3] = 255;
						}
						else
						{
							bg_r = dst_p[0];
							bg_g = dst_p[1];
							bg_b = dst_p[2];
							bg_a = dst_p[3];

							cr = (bg_r * bg_a + 0x80) >> 8;
							cg = (bg_g * bg_g + 0x80) >> 8;
							cb = (bg_b * bg_b + 0x80) >> 8;

							tmp = (src_p[0] - bg_r) * alpha;
							fg_r = bg_r + ((tmp + (tmp >> 8) + 0x80) >> 8);
							tmp = (src_p[1] - bg_g) * alpha;
							fg_g = bg_g + ((tmp + (tmp >> 8) + 0x80) >> 8);
							tmp = (src_p[2] - bg_b) * alpha;
							fg_b = bg_b + ((tmp + (tmp >> 8) + 0x80) >> 8);

							dst_p[0] = fg_r;
							dst_p[1] = fg_g;
							dst_p[2] = fg_b;
							dst_p[3] = bg_a + (((255 - bg_a) * alpha + 0x80) >> 8);
						}
					}
				} else { dst_p[0] = 255; dst_p[1] = 0; dst_p[2] = 0; dst_p[3] = 255;}
				dst_p += 4;
			}
			dst_linestart += dst_rowstride;
		}
	}

private:
	friend class KSVGIconPainter;
	ArtSVP *m_clipSVP;

	QImage *m_image;
	QWMatrix *m_worldMatrix;

	QString m_fillRule;
	QString m_joinStyle;
	QString m_capStyle;
	
	int m_strokeMiterLimit;
	
	QString m_dashes;
	unsigned short m_dashOffset;

	QColor m_fillColor;
	QColor m_strokeColor;
	
	art_u8 *m_buffer;
	art_u8 *m_tempBuffer;
	
	int m_width;
	int m_height;

	int m_rowstride;

	double m_opacity;
	double m_fillOpacity;
	double m_strokeOpacity;
	
	bool m_useFill;
	bool m_useStroke;

	bool m_useFillGradient;
	bool m_useStrokeGradient;	
	
	QString m_fillGradientReference;
	QString m_strokeGradientReference;	
	
	QMap<QString, ArtGradientLinear *> m_linearGradientMap;
	QMap<ArtGradientLinear *, QDomElement> m_linearGradientElementMap;

	QMap<QString, ArtGradientRadial *> m_radialGradientMap;
	QMap<ArtGradientRadial *, QDomElement> m_radialGradientElementMap;

	KSVGIconPainter *m_painter;

	double m_strokeWidth;
};

struct KSVGIconPainter::Private
{
	KSVGIconPainterHelper *helper;
	
	int drawWidth;
	int drawHeight;
};

KSVGIconPainter::KSVGIconPainter(int width, int height, int dwidth, int dheight) : d(new Private())
{
	d->helper = new KSVGIconPainterHelper(width, height, this);
	
	d->drawWidth = dwidth;
	d->drawHeight = dheight;
}

KSVGIconPainter::~KSVGIconPainter()
{
	delete d->helper;
	delete d;	
}

void KSVGIconPainter::finish()
{
	d->helper->blit();
}

QImage *KSVGIconPainter::image()
{
	return new QImage(*d->helper->m_image);
}

QWMatrix *KSVGIconPainter::worldMatrix()
{
	return d->helper->m_worldMatrix;
}

void KSVGIconPainter::setWorldMatrix(QWMatrix *matrix)
{
	if(d->helper->m_worldMatrix)
		delete d->helper->m_worldMatrix;

	d->helper->m_worldMatrix = matrix;
}

void KSVGIconPainter::setStrokeWidth(double width)
{
	d->helper->m_strokeWidth = width;
}

void KSVGIconPainter::setStrokeMiterLimit(const QString &miter)
{
	d->helper->m_strokeMiterLimit = miter.toInt();
}

void KSVGIconPainter::setStrokeDashOffset(const QString &dashOffset)
{
	d->helper->m_dashOffset = dashOffset.toUInt();
}

void KSVGIconPainter::setStrokeDashArray(const QString &dashes)
{
	d->helper->m_dashes = dashes;
}

void KSVGIconPainter::setCapStyle(const QString &cap)
{
	d->helper->m_capStyle = cap;
}

void KSVGIconPainter::setJoinStyle(const QString &join)
{
	d->helper->m_joinStyle = join;
}

void KSVGIconPainter::setStrokeColor(const QString &stroke)
{
	if(stroke.startsWith("url"))
	{
		d->helper->m_useStroke = false;
		d->helper->m_useStrokeGradient = true;
		
		QString url = stroke;
	
		unsigned int start = url.find("#") + 1;
		unsigned int end = url.findRev(")");

		d->helper->m_strokeGradientReference = url.mid(start, end - start);
	}
	else
	{
		d->helper->m_strokeColor = parseColor(stroke);

		d->helper->m_useStrokeGradient = false;
		d->helper->m_strokeGradientReference = QString::null;
		
		if(stroke.stripWhiteSpace().lower() != "none")
			setUseStroke(true);
		else
			setUseStroke(false);
	}
}

void KSVGIconPainter::setFillColor(const QString &fill)
{
	if(fill.startsWith("url"))
	{
		d->helper->m_useFill = false;
		d->helper->m_useFillGradient = true;

		QString url = fill;
	
		unsigned int start = url.find("#") + 1;
		unsigned int end = url.findRev(")");

		d->helper->m_fillGradientReference = url.mid(start, end - start);
	}
	else
	{
		d->helper->m_fillColor = parseColor(fill);

		d->helper->m_useFillGradient = false;
		d->helper->m_fillGradientReference = QString::null;
		
		if(fill.stripWhiteSpace().lower() != "none")
			setUseFill(true);
		else
			setUseFill(false);
	}
}

void KSVGIconPainter::setFillRule(const QString &fillRule)
{
	d->helper->m_fillRule = fillRule;
}

Q_UINT32 KSVGIconPainter::parseOpacity(QString data)
{
	int opacity = 255;

	if(!data.isEmpty())
	{
		double temp;

		if(data.contains("%"))
		{
			QString tempString = data.left(data.length() - 1);
			temp = double(255 * tempString.toDouble()) / 100.0;
		}
		else
			temp = data.toDouble();

		opacity = (int) floor(temp * 255 + 0.5);
	}

	return opacity;
}

void KSVGIconPainter::setFillOpacity(const QString &fillOpacity)
{
	d->helper->m_fillOpacity = parseOpacity(fillOpacity);
}

void KSVGIconPainter::setStrokeOpacity(const QString &strokeOpacity)
{
	d->helper->m_strokeOpacity = parseOpacity(strokeOpacity);
}

void KSVGIconPainter::setOpacity(const QString &opacity)
{
	d->helper->m_opacity = parseOpacity(opacity);
}

void KSVGIconPainter::setUseFill(bool fill)
{
	d->helper->m_useFill = fill;
}

void KSVGIconPainter::setUseStroke(bool stroke)
{
	d->helper->m_useStroke = stroke;
}

void KSVGIconPainter::setClippingRect(int x, int y, int w, int h)
{
	ArtVpath *vec = d->helper->allocVPath(6);

	vec[0].code = ART_MOVETO;
	vec[0].x = x;
	vec[0].y = y;

	vec[1].code = ART_LINETO;
	vec[1].x = x;
	vec[1].y = y + h;

	vec[2].code = ART_LINETO;
	vec[2].x = x + w;
	vec[2].y = y + h;

	vec[3].code = ART_LINETO;
	vec[3].x = x + w;
	vec[3].y = y;

	vec[4].code = ART_LINETO;
	vec[4].x = x;
	vec[4].y = y;

	vec[5].code = ART_END;

	if(d->helper->m_clipSVP)
		art_svp_free(d->helper->m_clipSVP);

	d->helper->m_clipSVP = art_svp_from_vpath(vec);

	art_free(vec);
}

void KSVGIconPainter::drawRectangle(double x, double y, double w, double h, double rx, double ry)
{
	if((int) rx != 0 && (int) ry != 0)
	{
		ArtVpath *res;
		ArtBpath *vec = d->helper->allocBPath(10);

		int i = 0;

		if(rx > w / 2)
			rx = w / 2;

		if(ry > h / 2)
			ry = h / 2;

		vec[i].code = ART_MOVETO_OPEN;
		vec[i].x3 = x + rx;
		vec[i].y3 = y;

		i++;

		vec[i].code = ART_CURVETO;
		vec[i].x1 = x + rx * (1 - 0.552);
		vec[i].y1 = y;
		vec[i].x2 = x;
		vec[i].y2 = y + ry * (1 - 0.552);
		vec[i].x3 = x;
		vec[i].y3 = y + ry;

		i++;

		if(ry < h / 2)
		{
			vec[i].code = ART_LINETO;
			vec[i].x3 = x;
			vec[i].y3 = y + h - ry;

			i++;
		}

		vec[i].code = ART_CURVETO;
		vec[i].x1 = x;
		vec[i].y1 = y + h - ry * (1 - 0.552);
		vec[i].x2 = x + rx * (1 - 0.552);
		vec[i].y2 = y + h;
		vec[i].x3 = x + rx;
		vec[i].y3 = y + h;

		i++;

		if(rx < w / 2)
		{
			vec[i].code = ART_LINETO;
			vec[i].x3 = x + w - rx;
			vec[i].y3 = y + h;

			i++;
		}

		vec[i].code = ART_CURVETO;
		vec[i].x1 = x + w - rx * (1 - 0.552);
		vec[i].y1 = y + h;
		vec[i].x2 = x + w;
		vec[i].y2 = y + h - ry * (1 - 0.552);
		vec[i].x3 = x + w;

		vec[i].y3 = y + h - ry;

		i++;

		if(ry < h / 2)
		{
			vec[i].code = ART_LINETO;
			vec[i].x3 = x + w;
			vec[i].y3 = y + ry;

			i++;
		}

		vec[i].code = ART_CURVETO;
		vec[i].x1 = x + w;
		vec[i].y1 = y + ry * (1 - 0.552);
		vec[i].x2 = x + w - rx * (1 - 0.552);
		vec[i].y2 = y;
		vec[i].x3 = x + w - rx;
		vec[i].y3 = y;

		i++;

		if(rx < w / 2)
		{
			vec[i].code = ART_LINETO;
			vec[i].x3 = x + rx;
			vec[i].y3 = y;

			i++;
		}

		vec[i].code = ART_END;

		res = d->helper->art_bez_path_to_vec(vec, 0.25);
		art_free(vec);
		d->helper->drawVPath(res);
	}
	else
	{
		ArtVpath *vec = d->helper->allocVPath(6);

		vec[0].code = ART_MOVETO;
		vec[0].x = x;
		vec[0].y = y;

		vec[1].code = ART_LINETO;
		vec[1].x = x;
		vec[1].y = y + h;

		vec[2].code = ART_LINETO;
		vec[2].x = x + w;
		vec[2].y = y + h;

		vec[3].code = ART_LINETO;
		vec[3].x = x + w;
		vec[3].y = y;

		vec[4].code = ART_LINETO;
		vec[4].x = x;
		vec[4].y = y;

		vec[5].code = ART_END;

		d->helper->drawVPath(vec);
	}
}

void KSVGIconPainter::drawEllipse(double cx, double cy, double rx, double ry)
{
	ArtVpath *vec, *vec2;
	ArtBpath *temp, *abp;
	
	temp = d->helper->allocBPath(6);
	
	double x0, y0, x1, y1, x2, y2, x3, y3, len, s, e;
	double affine[6];
	int i = 0;

	// Use a blowup factor of 10 to make ellipses with small radii look good
	art_affine_scale(affine, rx * 10.0, ry * 10.0);

	temp[i].code = ART_MOVETO;
	temp[i].x3 = cos(0.0);
	temp[i].y3 = sin(0.0);

	i++;
	
	for(s = 0; s < 2 * M_PI; s += M_PI_2)
	{
		e = s + M_PI_2;
		if(e > 2 * M_PI)
			e = 2 * M_PI;

		len = 0.552 * (e - s) / M_PI_2;
		x0 = cos (s);
		y0 = sin (s);
		x1 = x0 + len * cos (s + M_PI_2);
		y1 = y0 + len * sin (s + M_PI_2);
		x3 = cos (e);
		y3 = sin (e);
		x2 = x3 + len * cos (e - M_PI_2);
		y2 = y3 + len * sin (e - M_PI_2);

		temp[i].code = ART_CURVETO;
		temp[i].x1 = x1;
		temp[i].y1 = y1;
		temp[i].x2 = x2;
		temp[i].y2 = y2;
		temp[i].x3 = x3;
		temp[i].y3 = y3;

		i++;
	}

	temp[i].code = ART_END;

	abp = art_bpath_affine_transform(temp, affine);
	vec = d->helper->art_bez_path_to_vec(abp, 0.25);
	art_free(abp);
	// undo blowup
	art_affine_scale(affine, 0.1, 0.1);
	affine[4] = cx;
	affine[5] = cy;
	vec2 = art_vpath_affine_transform(vec, affine);

	art_free(vec);
	
	d->helper->drawVPath(vec2); 
}

void KSVGIconPainter::drawLine(double x1, double y1, double x2, double y2)
{
	ArtVpath *vec;
	
	vec = d->helper->allocVPath(3);

	vec[0].code = ART_MOVETO_OPEN;
	vec[0].x = x1;
	vec[0].y = y1;

	vec[1].code = ART_LINETO;
	vec[1].x = x2;
	vec[1].y = y2;

	vec[2].code = ART_END;

	d->helper->drawVPath(vec);
}

void KSVGIconPainter::drawPolyline(QPointArray polyArray, int points)
{
	if(polyArray.point(0).x() == -1 || polyArray.point(0).y() == -1)
		return;
	
	ArtVpath *polyline;

	if(points == -1)
		points = polyArray.count();

	polyline = d->helper->allocVPath(3 + points);
	polyline[0].code = ART_MOVETO;
    polyline[0].x = polyArray.point(0).x();
    polyline[0].y = polyArray.point(0).y();

	int index;
	for(index = 1; index < points; index++)
	{
		QPoint point = polyArray.point(index);
		polyline[index].code = ART_LINETO;
		polyline[index].x = point.x();
		polyline[index].y = point.y();
	}
	
	if(d->helper->m_useFill) // if the polyline must be filled, inform libart that it should not be closed.
	{
		polyline[index].code = (ArtPathcode)ART_END2;
		polyline[index].x = polyArray.point(0).x();
		polyline[index++].y = polyArray.point(0).y();
	}
	
	polyline[index].code = ART_END;

	d->helper->drawVPath(polyline);
}

void KSVGIconPainter::drawPolygon(QPointArray polyArray)
{
	ArtVpath *polygon;
	
	polygon = d->helper->allocVPath(3 + polyArray.count());
	polygon[0].code = ART_MOVETO;
    polygon[0].x = polyArray.point(0).x();
    polygon[0].y = polyArray.point(0).y();

	unsigned int index;
	for(index = 1; index < polyArray.count(); index++)
	{
		QPoint point = polyArray.point(index);
		polygon[index].code = ART_LINETO;
		polygon[index].x = point.x();
		polygon[index].y = point.y();
	}

	polygon[index].code = ART_LINETO;
	polygon[index].x = polyArray.point(0).x();
	polygon[index].y = polyArray.point(0).y();
	
	index++;
	polygon[index].code = ART_END;

	d->helper->drawVPath(polygon);
}

// Path parsing tool
// parses the coord into number and forwards to the next token
const char *getCoord(const char *ptr, double &number)
{
	int integer, exponent;
	double decimal, frac;
	int sign, expsign;

	exponent = 0;
	integer = 0;
	frac = 1.0;
	decimal = 0;
	sign = 1;
	expsign = 1;
	
	// read the sign
	if(*ptr == '+')
		ptr++;
	else if(*ptr == '-')
	{
		ptr++;
		sign = -1;
	}
	// read the integer part
	while(*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
		integer = (integer * 10) + *(ptr++) - '0';
	
	if(*ptr == '.') // read the decimals
    {
		ptr++;
		while(*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
			decimal += (*(ptr++) - '0') * (frac *= 0.1);
    }
	
	if(*ptr == 'e' || *ptr == 'E') // read the exponent part
	{
		ptr++;
		
		// read the sign of the exponent
		if(*ptr == '+')
			ptr++;
		else if(*ptr == '-')
		{
			ptr++;
			expsign = -1;
		}
		
		exponent = 0;
		while(*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
		{
			exponent *= 10;
			exponent += *ptr - '0';
			ptr++;
		}
    }
	
	number = integer + decimal;
	number *= sign * pow(10.0, expsign * exponent);

	// skip the following space
	if(*ptr == ' ')
		ptr++;
	
	return ptr;
}

void KSVGIconPainter::drawPath(const QString &data, bool filled)
{
	QString value = data;

	QMemArray<ArtBpath> vec;
	int index = -1;

	double curx = 0.0, cury = 0.0, contrlx = 0.0, contrly = 0.0, xc, yc;
	unsigned int lastCommand = 0;

	QString _d = value.replace(QRegExp(","), " ");
	_d = _d.simplifyWhiteSpace();
	const char *ptr = _d.latin1();
	const char *end = _d.latin1() + _d.length() + 1;

	double tox, toy, x1, y1, x2, y2, rx, ry, angle;
	bool largeArc, sweep;
	char command = *(ptr++);

	while(ptr < end)
	{
		if(*ptr == ' ')
			ptr++;
		
		switch(command)
		{
			case 'm':
				ptr = getCoord(ptr, tox);
				ptr = getCoord(ptr, toy);

				if(index != -1 && lastCommand != 'z')
				{
					// Find last subpath
					int find = -1;
					for(int i = index; i >= 0; i--)
					{
						if(vec[i].code == ART_MOVETO_OPEN || vec[i].code == ART_MOVETO)
						{
							find = i;
							break;
						}
					}

					index++;

					if(vec.size() == (unsigned int) index)
						vec.resize(index + 1);

					vec[index].code = (ArtPathcode)ART_END2;
					vec[index].x3 = vec[find].x3;
					vec[index].y3 = vec[find].y3;
				}

				curx += tox;
				cury += toy;

				index++;

				d->helper->ensureSpace(vec, index);

				vec[index].code = (index == 0) ? ART_MOVETO : ART_MOVETO_OPEN;
				vec[index].x3 = curx;
				vec[index].y3 = cury;

				lastCommand = 'm';
				break;
			case 'M':
				ptr = getCoord(ptr, tox);
				ptr = getCoord(ptr, toy);
				if(index != -1 && lastCommand != 'z')
				{
					// Find last subpath
					int find = -1;
					for(int i = index; i >= 0; i--)
					{
						if(vec[i].code == ART_MOVETO_OPEN || vec[i].code == ART_MOVETO)
						{
							find = i;
							break;
						}
					}

					index++;

					if(vec.size() == (unsigned int) index)							
						vec.resize(index + 1);

					vec[index].code = (ArtPathcode)ART_END2;
					vec[index].x3 = vec[find].x3;
					vec[index].y3 = vec[find].y3;
				}

				curx = tox;
				cury = toy;

				index++;

				d->helper->ensureSpace(vec, index);

				vec[index].code = (index == 0) ? ART_MOVETO : ART_MOVETO_OPEN;
				vec[index].x3 = curx;
				vec[index].y3 = cury;

				lastCommand = 'M';
				break;
			case 'l':
				ptr = getCoord(ptr, tox);
				ptr = getCoord(ptr, toy);

				index++;

				d->helper->ensureSpace(vec, index);

				vec[index].code = ART_LINETO;
				vec[index].x3 = curx + tox;
				vec[index].y3 = cury + toy;

				curx += tox;
				cury += toy;

				lastCommand = 'l';
				break;
			case 'L':
				ptr = getCoord(ptr, tox);
				ptr = getCoord(ptr, toy);

				index++;

				d->helper->ensureSpace(vec, index);

				vec[index].code = ART_LINETO;
				vec[index].x3 = tox;
				vec[index].y3 = toy;

				curx = tox;
				cury = toy;

				lastCommand = 'L';
				break;
			case 'h':
				ptr = getCoord(ptr, tox);

				index++;

				curx += tox;

				d->helper->ensureSpace(vec, index);

				vec[index].code = ART_LINETO;
				vec[index].x3 = curx;
				vec[index].y3 = cury;

				lastCommand = 'h';
				break;
			case 'H':
				ptr = getCoord(ptr, tox);

				index++;

				curx = tox;

				d->helper->ensureSpace(vec, index);

				vec[index].code = ART_LINETO;
				vec[index].x3 = curx;
				vec[index].y3 = cury;

				lastCommand = 'H';
				break;
			case 'v':
				ptr = getCoord(ptr, toy);

				index++;

				cury += toy;

				d->helper->ensureSpace(vec, index);

				vec[index].code = ART_LINETO;
				vec[index].x3 = curx;
				vec[index].y3 = cury;

				lastCommand = 'v';
				break;
			case 'V':
				ptr = getCoord(ptr, toy);

				index++;

				cury = toy;

				d->helper->ensureSpace(vec, index);

				vec[index].code = ART_LINETO;
				vec[index].x3 = curx;
				vec[index].y3 = cury;

				lastCommand = 'V';
				break;
			case 'c':
				ptr = getCoord(ptr, x1);
				ptr = getCoord(ptr, y1);
				ptr = getCoord(ptr, x2);
				ptr = getCoord(ptr, y2);
				ptr = getCoord(ptr, tox);
				ptr = getCoord(ptr, toy);

				index++;

				d->helper->ensureSpace(vec, index);

				vec[index].code = ART_CURVETO;
				vec[index].x1 = curx + x1;
				vec[index].y1 = cury + y1;
				vec[index].x2 = curx + x2;
				vec[index].y2 = cury + y2;
				vec[index].x3 = curx + tox;
				vec[index].y3 = cury + toy;

				curx += tox;
				cury += toy;

				contrlx = vec[index].x2;
				contrly = vec[index].y2;

				lastCommand = 'c';
				break;
			case 'C':
				ptr = getCoord(ptr, x1);
				ptr = getCoord(ptr, y1);
				ptr = getCoord(ptr, x2);
				ptr = getCoord(ptr, y2);
				ptr = getCoord(ptr, tox);
				ptr = getCoord(ptr, toy);

				index++;

				d->helper->ensureSpace(vec, index);

				vec[index].code = ART_CURVETO;
				vec[index].x1 = x1;
				vec[index].y1 = y1;
				vec[index].x2 = x2;
				vec[index].y2 = y2;
				vec[index].x3 = tox;
				vec[index].y3 = toy;

				curx = vec[index].x3;
				cury = vec[index].y3;
				contrlx = vec[index].x2;
				contrly = vec[index].y2;

				lastCommand = 'C';
				break;
			case 's':
				ptr = getCoord(ptr, x2);
				ptr = getCoord(ptr, y2);
				ptr = getCoord(ptr, tox);
				ptr = getCoord(ptr, toy);

				index++;

				d->helper->ensureSpace(vec, index);

				vec[index].code = ART_CURVETO;
				vec[index].x1 = 2 * curx - contrlx;
				vec[index].y1 = 2 * cury - contrly;
				vec[index].x2 = curx + x2;
				vec[index].y2 = cury + y2;
				vec[index].x3 = curx + tox;
				vec[index].y3 = cury + toy;

				curx += tox;
				cury += toy;

				contrlx = vec[index].x2;
				contrly = vec[index].y2;

				lastCommand = 's';
				break;
			case 'S':
				ptr = getCoord(ptr, x2);
				ptr = getCoord(ptr, y2);
				ptr = getCoord(ptr, tox);
				ptr = getCoord(ptr, toy);

				index++;

				d->helper->ensureSpace(vec, index);

				vec[index].code = ART_CURVETO;
				vec[index].x1 = 2 * curx - contrlx;
				vec[index].y1 = 2 * cury - contrly;
				vec[index].x2 = x2;
				vec[index].y2 = y2;
				vec[index].x3 = tox;
				vec[index].y3 = toy;

				curx = vec[index].x3;
				cury = vec[index].y3;
				contrlx = vec[index].x2;
				contrly = vec[index].y2;

				lastCommand = 'S';
				break;
			case 'q':
				ptr = getCoord(ptr, x1);
				ptr = getCoord(ptr, y1);
				ptr = getCoord(ptr, tox);
				ptr = getCoord(ptr, toy);

				index++;

				d->helper->ensureSpace(vec, index);

				vec[index].code = ART_CURVETO;
				vec[index].x1 = (curx + 2 * (x1 + curx)) * (1.0 / 3.0);
				vec[index].y1 = (cury + 2 * (y1 + cury)) * (1.0 / 3.0);
				vec[index].x2 = ((curx + tox) + 2 * (x1 + curx)) * (1.0 / 3.0);
				vec[index].y2 = ((cury + toy) + 2 * (y1 + cury)) * (1.0 / 3.0);
				vec[index].x3 = curx + tox;
				vec[index].y3 = cury + toy;

				contrlx = curx + x1;
				contrly = cury + y1;
				curx += tox;
				cury += toy;

				lastCommand = 'q';
				break;
			case 'Q':
				ptr = getCoord(ptr, x1);
				ptr = getCoord(ptr, y1);
				ptr = getCoord(ptr, tox);
				ptr = getCoord(ptr, toy);

				index++;

				d->helper->ensureSpace(vec, index);

				// TODO : if this fails make it more like QuadraticRel
				vec[index].code = ART_CURVETO;
				vec[index].x1 = (curx + 2 * x1) * (1.0 / 3.0);
				vec[index].y1 = (cury + 2 * y1) * (1.0 / 3.0);
				vec[index].x2 = (tox + 2 * x1) * (1.0 / 3.0);
				vec[index].y2 = (toy + 2 * y1) * (1.0 / 3.0);
				vec[index].x3 = tox;
				vec[index].y3 = toy;

				curx = vec[index].x3;
				cury = vec[index].y3;
				contrlx = vec[index].x2;
				contrly = vec[index].y2;

				lastCommand = 'Q';
				break;
			case 't':
				ptr = getCoord(ptr, tox);
				ptr = getCoord(ptr, toy);

				xc = 2 * curx - contrlx;
				yc = 2 * cury - contrly;

				index++;

				d->helper->ensureSpace(vec, index);

				vec[index].code = ART_CURVETO;
				vec[index].x1 = (curx + 2 * xc) * (1.0 / 3.0);
				vec[index].y1 = (cury + 2 * yc) * (1.0 / 3.0);
				vec[index].x2 = ((curx + tox) + 2 * xc) * (1.0 / 3.0);
				vec[index].y2 = ((cury + toy) + 2 * yc) * (1.0 / 3.0);

				vec[index].x3 = curx + tox;
				vec[index].y3 = cury + toy;

				curx += tox;
				cury += toy;
				contrlx = xc;
				contrly = yc;

				lastCommand = 't';
				break;
			case 'T':
				ptr = getCoord(ptr, tox);
				ptr = getCoord(ptr, toy);

				xc = 2 * curx - contrlx;
				yc = 2 * cury - contrly;

				index++;

				d->helper->ensureSpace(vec, index);

				vec[index].code = ART_CURVETO;
				vec[index].x1 = (curx + 2 * xc) * (1.0 / 3.0);
				vec[index].y1 = (cury + 2 * yc) * (1.0 / 3.0);
				vec[index].x2 = (tox + 2 * xc) * (1.0 / 3.0);
				vec[index].y2 = (toy + 2 * yc) * (1.0 / 3.0);
				vec[index].x3 = tox;
				vec[index].y3 = toy;

				curx = tox;
				cury = toy;
				contrlx = xc;
				contrly = yc;

				lastCommand = 'T';					
				break;
			case 'z':
			case 'Z':
				int find;
				find = -1;
				for(int i = index; i >= 0; i--)
				{
					if(vec[i].code == ART_MOVETO_OPEN || vec[i].code == ART_MOVETO)
					{
						find = i;
						break;
					}
				}

				if(find != -1)
				{
					if(vec[find].x3 != curx || vec[find].y3 != cury)
					{
						index++;

						d->helper->ensureSpace(vec, index);

						vec[index].code = ART_LINETO;
						vec[index].x3 = vec[find].x3;
						vec[index].y3 = vec[find].y3;
					}
				}

				// reset for next (sub)path
				curx = vec[find].x3;
				cury = vec[find].y3;

				lastCommand = 'z';
				break;
			case 'a':
				ptr = getCoord(ptr, rx);
				ptr = getCoord(ptr, ry);
				ptr = getCoord(ptr, angle);
				ptr = getCoord(ptr, tox);
				largeArc = tox == 1;
				ptr = getCoord(ptr, tox);
				sweep = tox == 1;
				ptr = getCoord(ptr, tox);
				ptr = getCoord(ptr, toy);

				d->helper->calculateArc(true, vec, index, curx, cury, angle, tox, toy, rx, ry, largeArc, sweep);

				lastCommand = 'a';
				break;
			case 'A':
				ptr = getCoord(ptr, rx);
				ptr = getCoord(ptr, ry);
				ptr = getCoord(ptr, angle);
				ptr = getCoord(ptr, tox);
				largeArc = tox == 1;
				ptr = getCoord(ptr, tox);
				sweep = tox == 1;
				ptr = getCoord(ptr, tox);
				ptr = getCoord(ptr, toy);

				d->helper->calculateArc(false, vec, index, curx, cury, angle, tox, toy, rx, ry, largeArc, sweep);

				lastCommand = 'A';
				break;
		}

		if(*ptr == '+' || *ptr == '-' || (*ptr >= '0' && *ptr <= '9'))
		{
			// there are still coords in this command
			if(command == 'M')
				command = 'L';
			else if(command == 'm')
				command = 'l';
		}
		else
			command = *(ptr++);

		// Detect reflection points
		if(lastCommand != 'C' && lastCommand != 'c'
	      && lastCommand != 'S' && lastCommand != 's'
		  && lastCommand != 'Q' && lastCommand != 'q'
		  && lastCommand != 'T' && lastCommand != 't')
		{
			contrlx = curx;
			contrly = cury;
		}			
	}

	// Find last subpath
	int find = -1;
	for(int i = index; i >= 0; i--)
	{
		if(vec[i].code == ART_MOVETO_OPEN || vec[i].code == ART_MOVETO)
		{
			find = i;
			break;
		}
	}

	// Fix a problem where the .svg file used doubles as values... (sofico.svg)
	if(curx != vec[find].x3 && cury != vec[find].y3)
	{
		if((int) curx == (int) vec[find].x3 && (int) cury == (int) vec[find].y3)
		{
			index++;

			if(vec.size() == (unsigned int) index)
				vec.resize(index + 1);

			vec[index].code = ART_LINETO;
			vec[index].x3 = vec[find].x3;
			vec[index].y3 = vec[find].y3;

			curx = vec[find].x3;
			cury = vec[find].y3;
		}
	}

	// Handle filled paths that are not closed explicitly
	if(filled)
	{
		if((int) curx != (int) vec[find].x3 || (int) cury != (int) vec[find].y3)
		{
			index++;

			if(vec.size() == (unsigned int) index)
				vec.resize(index + 1);

			vec[index].code = (ArtPathcode)ART_END2;
			vec[index].x3 = vec[find].x3;
			vec[index].y3 = vec[find].y3;

			curx = vec[find].x3;
			cury = vec[find].y3;
		}
	}
	
	// Close
	index++;

	if(vec.size() == (unsigned int) index)
		vec.resize(index + 1);

	vec[index].code = ART_END;

	// There are pure-moveto paths which reference paint servers *bah*
	// Do NOT render them
	bool render = false;
	for(int i = index; i >= 0; i--)
	{
		if(vec[i].code != ART_MOVETO_OPEN && vec[i].code != ART_MOVETO && !(vec[i].code >= ART_END))
		{
			render = true;
			break;
		}
	}

	if(render)
		d->helper->drawVPath(d->helper->art_bez_path_to_vec(vec.data(), 0.25));
}

void KSVGIconPainter::drawImage(double x, double y, QImage &image)
{
	if(image.depth() != 32)
		image = image.convertDepth(32);

	double affine[6];
	affine[0] = d->helper->m_worldMatrix->m11();
	affine[1] = d->helper->m_worldMatrix->m12();
	affine[2] = d->helper->m_worldMatrix->m21();
	affine[3] = d->helper->m_worldMatrix->m22();
	affine[4] = x;
	affine[5] = y;

	d->helper->art_rgba_rgba_affine(d->helper->m_buffer, 0, 0, d->helper->m_width, d->helper->m_height,
									d->helper->m_rowstride, image.bits(), image.width(), image.height(),
									image.width() * 4, affine);
}

QColor KSVGIconPainter::parseColor(const QString &param)
{
	if(param.stripWhiteSpace().startsWith("#"))
	{
		QColor color;
		color.setNamedColor(param.stripWhiteSpace());
		return color;
	}
	else if(param.stripWhiteSpace().startsWith("rgb("))
	{
		QString parse = param.stripWhiteSpace();
		QStringList colors = QStringList::split(',', parse);
		QString r = colors[0].right((colors[0].length() - 4));
		QString g = colors[1];
		QString b = colors[2].left((colors[2].length() - 1));

		if(r.contains("%"))
		{
			r = r.left(r.length() - 1);
			r = QString::number(int((double(255 * r.toDouble()) / 100.0)));
		}

		if(g.contains("%"))
		{
			g = g.left(g.length() - 1);
			g = QString::number(int((double(255 * g.toDouble()) / 100.0)));
		}

		if(b.contains("%"))
		{
			b = b.left(b.length() - 1);
			b = QString::number(int((double(255 * b.toDouble()) / 100.0)));
		}

		return QColor(r.toInt(), g.toInt(), b.toInt());
	}
	else
	{
		QString rgbColor = param.stripWhiteSpace();

		if(rgbColor == "aliceblue")
			return QColor(240, 248, 255);
		else if(rgbColor == "antiquewhite")
			return QColor(250, 235, 215);
		else if(rgbColor == "aqua")
			return QColor(0, 255, 255);
		else if(rgbColor == "aquamarine")
			return QColor(127, 255, 212);
		else if(rgbColor == "azure")
			return QColor(240, 255, 255);
		else if(rgbColor == "beige")
			return QColor(245, 245, 220);
		else if(rgbColor == "bisque")
			return QColor(255, 228, 196);
		else if(rgbColor == "black")
			return QColor(0, 0, 0);
		else if(rgbColor == "blanchedalmond")
			return QColor(255, 235, 205);
		else if(rgbColor == "blue")
			return QColor(0, 0, 255);
		else if(rgbColor == "blueviolet")
			return QColor(138, 43, 226);
		else if(rgbColor == "brown")
			return QColor(165, 42, 42);
		else if(rgbColor == "burlywood")
			return QColor(222, 184, 135);
		else if(rgbColor == "cadetblue")
			return QColor(95, 158, 160);
		else if(rgbColor == "chartreuse")
			return QColor(127, 255, 0);
		else if(rgbColor == "chocolate")
			return QColor(210, 105, 30);
		else if(rgbColor == "coral")
			return QColor(255, 127, 80);
		else if(rgbColor == "cornflowerblue")
			return QColor(100, 149, 237);
		else if(rgbColor == "cornsilk")
			return QColor(255, 248, 220);
		else if(rgbColor == "crimson")
			return QColor(220, 20, 60);
		else if(rgbColor == "cyan")
			return QColor(0, 255, 255);
		else if(rgbColor == "darkblue")
			return QColor(0, 0, 139);
		else if(rgbColor == "darkcyan")
			return QColor(0, 139, 139);
		else if(rgbColor == "darkgoldenrod")
			return QColor(184, 134, 11);
		else if(rgbColor == "darkgray")
			return QColor(169, 169, 169);
		else if(rgbColor == "darkgrey")
			return QColor(169, 169, 169);
		else if(rgbColor == "darkgreen")
			return QColor(0, 100, 0);
		else if(rgbColor == "darkkhaki")
			return QColor(189, 183, 107);
		else if(rgbColor == "darkmagenta")
			return QColor(139, 0, 139);
		else if(rgbColor == "darkolivegreen")
			return QColor(85, 107, 47);
		else if(rgbColor == "darkorange")
			return QColor(255, 140, 0);
		else if(rgbColor == "darkorchid")
			return QColor(153, 50, 204);
		else if(rgbColor == "darkred")
			return QColor(139, 0, 0);
		else if(rgbColor == "darksalmon")
			return QColor(233, 150, 122);
		else if(rgbColor == "darkseagreen")
			return QColor(143, 188, 143);
		else if(rgbColor == "darkslateblue")
			return QColor(72, 61, 139);
		else if(rgbColor == "darkslategray")
			return QColor(47, 79, 79);
		else if(rgbColor == "darkslategrey")
			return QColor(47, 79, 79);
		else if(rgbColor == "darkturquoise")
			return QColor(0, 206, 209);
		else if(rgbColor == "darkviolet")
			return QColor(148, 0, 211);
		else if(rgbColor == "deeppink")
			return QColor(255, 20, 147);
		else if(rgbColor == "deepskyblue")
			return QColor(0, 191, 255);
		else if(rgbColor == "dimgray")
			return QColor(105, 105, 105);
		else if(rgbColor == "dimgrey")
			return QColor(105, 105, 105);
		else if(rgbColor == "dodgerblue")
			return QColor(30, 144, 255);
		else if(rgbColor == "firebrick")
			return QColor(178, 34, 34);
		else if(rgbColor == "floralwhite")
			return QColor(255, 250, 240);
		else if(rgbColor == "forestgreen")
			return QColor(34, 139, 34);
		else if(rgbColor == "fuchsia")
			return QColor(255, 0, 255);
		else if(rgbColor == "gainsboro")
			return QColor(220, 220, 220);
		else if(rgbColor == "ghostwhite")
			return QColor(248, 248, 255);
		else if(rgbColor == "gold")
			return QColor(255, 215, 0);
		else if(rgbColor == "goldenrod")
			return QColor(218, 165, 32);
		else if(rgbColor == "gray")
			return QColor(128, 128, 128);
		else if(rgbColor == "grey")
			return QColor(128, 128, 128);
		else if(rgbColor == "green")
			return QColor(0, 128, 0);
		else if(rgbColor == "greenyellow")
			return QColor(173, 255, 47);
		else if(rgbColor == "honeydew")
			return QColor(240, 255, 240);
		else if(rgbColor == "hotpink")
			return QColor(255, 105, 180);
		else if(rgbColor == "indianred")
			return QColor(205, 92, 92);
		else if(rgbColor == "indigo")
			return QColor(75, 0, 130);
		else if(rgbColor == "ivory")
			return QColor(255, 255, 240);
		else if(rgbColor == "khaki")
			return QColor(240, 230, 140);
		else if(rgbColor == "lavender")
			return QColor(230, 230, 250);
		else if(rgbColor == "lavenderblush")
			return QColor(255, 240, 245);
		else if(rgbColor == "lawngreen")
			return QColor(124, 252, 0);
		else if(rgbColor == "lemonchiffon")
			return QColor(255, 250, 205);
		else if(rgbColor == "lightblue")
			return QColor(173, 216, 230);
		else if(rgbColor == "lightcoral")
			return QColor(240, 128, 128);
		else if(rgbColor == "lightcyan")
			return QColor(224, 255, 255);
		else if(rgbColor == "lightgoldenrodyellow")
			return QColor(250, 250, 210);
		else if(rgbColor == "lightgray")
			return QColor(211, 211, 211);
		else if(rgbColor == "lightgrey")
			return QColor(211, 211, 211);
		else if(rgbColor == "lightgreen")
			return QColor(144, 238, 144);
		else if(rgbColor == "lightpink")
			return QColor(255, 182, 193);
		else if(rgbColor == "lightsalmon")
			return QColor(255, 160, 122);
		else if(rgbColor == "lightseagreen")
			return QColor(32, 178, 170);
		else if(rgbColor == "lightskyblue")
			return QColor(135, 206, 250);
		else if(rgbColor == "lightslategray")
			return QColor(119, 136, 153);
		else if(rgbColor == "lightslategrey")
			return QColor(119, 136, 153);
		else if(rgbColor == "lightsteelblue")
			return QColor(176, 196, 222);
		else if(rgbColor == "lightyellow")
			return QColor(255, 255, 224);
		else if(rgbColor == "lime")
			return QColor(0, 255, 0);
		else if(rgbColor == "limegreen")
			return QColor(50, 205, 50);
		else if(rgbColor == "linen")
			return QColor(250, 240, 230);
		else if(rgbColor == "magenta")
			return QColor(255, 0, 255);
		else if(rgbColor == "maroon")
			return QColor(128, 0, 0);
		else if(rgbColor == "mediumaquamarine")
			return QColor(102, 205, 170);
		else if(rgbColor == "mediumblue")
			return QColor(0, 0, 205);
		else if(rgbColor == "mediumorchid")
			return QColor(186, 85, 211);
		else if(rgbColor == "mediumpurple")
			return QColor(147, 112, 219);
		else if(rgbColor == "mediumseagreen")
			return QColor(60, 179, 113);
		else if(rgbColor == "mediumslateblue")
			return QColor(123, 104, 238);
		else if(rgbColor == "mediumspringgreen")
			return QColor(0, 250, 154);
		else if(rgbColor == "mediumturquoise")
			return QColor(72, 209, 204);
		else if(rgbColor == "mediumvioletred")
			return QColor(199, 21, 133);
		else if(rgbColor == "midnightblue")
			return QColor(25, 25, 112);
		else if(rgbColor == "mintcream")
			return QColor(245, 255, 250);
		else if(rgbColor == "mistyrose")
			return QColor(255, 228, 225);
		else if(rgbColor == "moccasin")
			return QColor(255, 228, 181);
		else if(rgbColor == "navajowhite")
			return QColor(255, 222, 173);
		else if(rgbColor == "navy")
			return QColor(0, 0, 128);
		else if(rgbColor == "oldlace")
			return QColor(253, 245, 230);
		else if(rgbColor == "olive")
			return QColor(128, 128, 0);
		else if(rgbColor == "olivedrab")
			return QColor(107, 142, 35);
		else if(rgbColor == "orange")
			return QColor(255, 165, 0);
		else if(rgbColor == "orangered")
			return QColor(255, 69, 0);
		else if(rgbColor == "orchid")
			return QColor(218, 112, 214);
		else if(rgbColor == "palegoldenrod")
			return QColor(238, 232, 170);
		else if(rgbColor == "palegreen")
			return QColor(152, 251, 152);
		else if(rgbColor == "paleturquoise")
			return QColor(175, 238, 238);
		else if(rgbColor == "palevioletred")
			return QColor(219, 112, 147);
		else if(rgbColor == "papayawhip")
			return QColor(255, 239, 213);
		else if(rgbColor == "peachpuff")
			return QColor(255, 218, 185);
		else if(rgbColor == "peru")
			return QColor(205, 133, 63);
		else if(rgbColor == "pink")
			return QColor(255, 192, 203);
		else if(rgbColor == "plum")
			return QColor(221, 160, 221);
		else if(rgbColor == "powderblue")
			return QColor(176, 224, 230);
		else if(rgbColor == "purple")
			return QColor(128, 0, 128);
		else if(rgbColor == "red")
			return QColor(255, 0, 0);
		else if(rgbColor == "rosybrown")
			return QColor(188, 143, 143);
		else if(rgbColor == "royalblue")
			return QColor(65, 105, 225);
		else if(rgbColor == "saddlebrown")
			return QColor(139, 69, 19);
		else if(rgbColor == "salmon")
			return QColor(250, 128, 114);
		else if(rgbColor == "sandybrown")
			return QColor(244, 164, 96);
		else if(rgbColor == "seagreen")
			return QColor(46, 139, 87);
		else if(rgbColor == "seashell")
			return QColor(255, 245, 238);
		else if(rgbColor == "sienna")
			return QColor(160, 82, 45);
		else if(rgbColor == "silver")
			return QColor(192, 192, 192);
		else if(rgbColor == "skyblue")
			return QColor(135, 206, 235);
		else if(rgbColor == "slateblue")
			return QColor(106, 90, 205);
		else if(rgbColor == "slategray")
			return QColor(112, 128, 144);
		else if(rgbColor == "slategrey")
			return QColor(112, 128, 144);
		else if(rgbColor == "snow")
			return QColor(255, 250, 250);
		else if(rgbColor == "springgreen")
			return QColor(0, 255, 127);
		else if(rgbColor == "steelblue")
			return QColor(70, 130, 180);
		else if(rgbColor == "tan")
			return QColor(210, 180, 140);
		else if(rgbColor == "teal")
			return QColor(0, 128, 128);
		else if(rgbColor == "thistle")
			return QColor(216, 191, 216);
		else if(rgbColor == "tomato")
			return QColor(255, 99, 71);
		else if(rgbColor == "turquoise")
			return QColor(64, 224, 208);
		else if(rgbColor == "violet")
			return QColor(238, 130, 238);
		else if(rgbColor == "wheat")
			return QColor(245, 222, 179);
		else if(rgbColor == "white")
			return QColor(255, 255, 255);
		else if(rgbColor == "whitesmoke")
			return QColor(245, 245, 245);
		else if(rgbColor == "yellow")
			return QColor(255, 255, 0);
		else if(rgbColor == "yellowgreen")
			return QColor(154, 205, 50);
	}

	return QColor();
}

double KSVGIconPainter::dpi()
{
	return 90.0; // TODO: make modal?
}

double KSVGIconPainter::toPixel(const QString &s, bool hmode)
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
				ret *= d->drawWidth;
			else
				ret *= d->drawHeight;
		}
	}
	else
		ret = value;

	return ret;
}

ArtGradientLinear *KSVGIconPainter::linearGradient(const QString &id)
{
	return d->helper->m_linearGradientMap[id];
}
 
void KSVGIconPainter::addLinearGradient(const QString &id, ArtGradientLinear *gradient)
{
	d->helper->m_linearGradientMap.insert(id, gradient);
}

QDomElement KSVGIconPainter::linearGradientElement(ArtGradientLinear *linear)
{
	return d->helper->m_linearGradientElementMap[linear];
}
 
void KSVGIconPainter::addLinearGradientElement(ArtGradientLinear *gradient, QDomElement element)
{
	d->helper->m_linearGradientElementMap.insert(gradient, element);
}

ArtGradientRadial *KSVGIconPainter::radialGradient(const QString &id)
{
	return d->helper->m_radialGradientMap[id];
}
 
void KSVGIconPainter::addRadialGradient(const QString &id, ArtGradientRadial *gradient)
{
	d->helper->m_radialGradientMap.insert(id, gradient);
}

QDomElement KSVGIconPainter::radialGradientElement(ArtGradientRadial *radial)
{
	return d->helper->m_radialGradientElementMap[radial];
}
 
void KSVGIconPainter::addRadialGradientElement(ArtGradientRadial *gradient, QDomElement element)
{
	d->helper->m_radialGradientElementMap.insert(gradient, element);
}

Q_UINT32 KSVGIconPainter::toArtColor(QColor color)
{
	return d->helper->toArtColor(color);
}

QWMatrix KSVGIconPainter::parseTransform(const QString &transform)
{
	QWMatrix result;
	
	// Split string for handling 1 transform statement at a time
	QStringList subtransforms = QStringList::split(')', transform);
	QStringList::ConstIterator it = subtransforms.begin();
	QStringList::ConstIterator end = subtransforms.end();
	for(; it != end; ++it)
	{
		QStringList subtransform = QStringList::split('(', (*it));

		subtransform[0] = subtransform[0].stripWhiteSpace().lower();
		subtransform[1] = subtransform[1].simplifyWhiteSpace();
		QRegExp reg("[a-zA-Z,( ]");
		QStringList params = QStringList::split(reg, subtransform[1]);

		if(subtransform[0].startsWith(";") || subtransform[0].startsWith(","))
			subtransform[0] = subtransform[0].right(subtransform[0].length() - 1);

		if(subtransform[0] == "rotate")
		{
			if(params.count() == 3)
			{
				float x = params[1].toFloat();
				float y = params[2].toFloat();

				result.translate(x, y);
				result.rotate(params[0].toFloat());
				result.translate(-x, -y);
			}
			else
				result.rotate(params[0].toFloat());
		}
		else if(subtransform[0] == "translate")
		{
			if(params.count() == 2)
				result.translate(params[0].toFloat(), params[1].toFloat());
			else    // Spec : if only one param given, assume 2nd param to be 0
				result.translate(params[0].toFloat() , 0);
		}
		else if(subtransform[0] == "scale")
		{
			if(params.count() == 2)
				result.scale(params[0].toFloat(), params[1].toFloat());
			else    // Spec : if only one param given, assume uniform scaling
				result.scale(params[0].toFloat(), params[0].toFloat());
		}
		else if(subtransform[0] == "skewx")
			result.shear(tan(params[0].toFloat() * deg2rad), 0.0F);
		else if(subtransform[0] == "skewy")
			result.shear(tan(params[0].toFloat() * deg2rad), 0.0F);
		else if(subtransform[0] == "skewy")
			result.shear(0.0F, tan(params[0].toFloat() * deg2rad));
		else if(subtransform[0] == "matrix")
		{
			if(params.count() >= 6)
				result.setMatrix(params[0].toFloat(), params[1].toFloat(), params[2].toFloat(), params[3].toFloat(), params[4].toFloat(), params[5].toFloat());
		}
	}

	return result;
}
