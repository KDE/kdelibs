/*
	Copyright (C) 2004 Nikolas Zimmermann <wildfox@kde.org>
				  2004 Rob Buis <buis@kde.org>

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

#ifndef KSVGAggHelper_H
#define KSVGAggHelper_H

#include <qcolor.h>
#include <qptrlist.h>
#include <qvaluelist.h>

#include <agg2/agg_conv_dash.h>
#include <agg2/agg_scanline_u.h>
#include <agg2/agg_scanline_p.h>
#include <agg2/agg_conv_curve.h>
#include <agg2/agg_conv_stroke.h>
#include <agg2/agg_path_storage.h>
#include <agg2/agg_trans_affine.h>
#include <agg2/agg_pixfmt_gray8.h>
#include <agg2/agg_alpha_mask_u8.h>
#include <agg2/agg_span_gradient.h>
#include <agg2/agg_pixfmt_rgba32.h>
#include <agg2/agg_bounding_rect.h>
#include <agg2/agg_conv_transform.h>
#include <agg2/agg_rendering_buffer.h>
#include <agg2/agg_conv_clip_polygon.h>
#include <agg2/agg_renderer_scanline.h>
#include <agg2/agg_span_pattern_rgba32.h>
#include <agg2/agg_pixfmt_amask_adaptor.h>
#include <agg2/agg_rasterizer_scanline_aa.h>
#include <agg2/agg_span_interpolator_linear.h>
#include <agg2/agg_span_image_filter_rgba32.h>

// Common definitions
typedef agg::pixfmt_bgra32 pixfmt;
typedef agg::renderer_base<pixfmt> renderer_base;
typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;

typedef agg::conv_curve<agg::path_storage> curved;
typedef agg::conv_transform<curved> curved_trans;

// Gradient rendering properties
typedef enum
{
	SPREADMETHOD_PAD = 1,
	SPREADMETHOD_REPEAT = 2,
	SPREADMETHOD_REFLECT = 3
} gradient_spread_method;

struct gradient_offset_pair
{
	float offset;
	QColor color;
};

class sorted_gradient_stop_list : public QPtrList<gradient_offset_pair>
{
public:
	sorted_gradient_stop_list() { setAutoDelete(true); }
	virtual ~sorted_gradient_stop_list() { }
	
	void addStop(float offset, const QColor &color)
	{
		gradient_offset_pair *pair = new gradient_offset_pair();
		pair->offset = offset;
		pair->color = color;

		inSort(pair);
	}

	typedef QPtrListIterator<gradient_offset_pair> iterator;

	// Helper function
	void fill_color_array(agg::rgba8 *array, float opacity)
	{
		sorted_gradient_stop_list::iterator it(*this);
		gradient_offset_pair *pair = it.current();
		double offset = pair->offset * 255.0;
	
		agg::rgba8 c(pair->color.red(), pair->color.green(), pair->color.blue());
		c.opacity(qAlpha(pair->color.rgb()) / 255.0 * opacity);

		// fill the colors until the first stop
		unsigned i;
		for(i = 0; i <= unsigned(offset); ++i)
			array[i] = c;

		++it; // Skip the first
		while((pair = it.current()) != 0)
		{
			++it;

			double offset2 = pair->offset * 255.0;

			agg::rgba8 c2(pair->color.red(), pair->color.green(), pair->color.blue());
			c2.opacity(qAlpha(pair->color.rgb()) / 255.0 * opacity);

			for(; i <= unsigned(offset2); ++i)
				array[i] = c.gradient(c2, (i - offset) / (offset2 - offset));

			c = c2;
			offset = offset2;
		}

		// fill the colors after last stop
		for(; i < 256; ++i)
			array[i] = c;
	}

protected:
	virtual int compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2)
	{
		gradient_offset_pair *pair1 = static_cast<gradient_offset_pair *>(item1);
		gradient_offset_pair *pair2 = static_cast<gradient_offset_pair *>(item2);

		if(pair1->offset == pair2->offset)
			return 0;
		else if(pair1->offset < pair2->offset)
			return -1;

		return 1;
	}
};

class gradient_rendering_properties
{
public:
	gradient_rendering_properties()
	{
		linear = true;
		spreadMethod = SPREADMETHOD_PAD;
	}
	
	~gradient_rendering_properties() { }
	
	bool linear; // or radial?
	
	gradient_spread_method spreadMethod;
	sorted_gradient_stop_list gradientStops;
};

// Stroke rendering properties
class stroke_rendering_properties
{
public:
	stroke_rendering_properties()
	{
		width = 1.0;
		miterLimit = 4;
		dashOffset = 0.0;

		lineCap = agg::vcgen_stroke::butt_cap;
		lineJoin = agg::vcgen_stroke::miter_join;
	}

	~stroke_rendering_properties() { }

	float width;
	unsigned int miterLimit;

	float dashOffset;
	QValueList<float> dashArray;
	
	agg::vcgen_stroke::line_cap_e lineCap;
	agg::vcgen_stroke::line_join_e lineJoin;
};

// Stroke related
class stroke_base
{
public:
	stroke_base() { }
	virtual ~stroke_base() { }

	virtual void rewind(unsigned id) = 0;
	virtual unsigned vertex(double *x, double *y) = 0;
};

template<class Source>
class stroke : public stroke_base
{
public:
	stroke(Source &src, stroke_rendering_properties &props) : stroke_base(), m_s(src)
	{
		m_s.width(props.width);
		m_s.miter_limit(props.miterLimit);

		m_s.line_cap(props.lineCap);
		m_s.line_join(props.lineJoin);
	}

	virtual void rewind(unsigned id) { m_s.rewind(id); }
	virtual unsigned vertex(double* x, double* y) { return m_s.vertex(x, y); }

private:
	typedef agg::conv_stroke<Source> stroke_type;

	stroke_type m_s;
};

template<class Source>
class dash_stroke : public stroke_base
{
public:
	dash_stroke(Source &src, stroke_rendering_properties &props) : stroke_base(), m_d(src), m_ds(m_d)
	{
		unsigned int dashLength = !props.dashArray.isEmpty() ? props.dashArray.count() : 0;

		// there are dashes to be rendered
		unsigned int count = (dashLength % 2) == 0 ? dashLength : dashLength * 2;
		for(unsigned int i = 0; i < count; i += 2)
			m_d.add_dash(*props.dashArray.at(i % dashLength),
						 *props.dashArray.at((i + 1) % dashLength));

		m_d.dash_start(props.dashOffset);

		m_ds.width(props.width);
		m_ds.miter_limit(props.miterLimit);

		m_ds.line_cap(props.lineCap);
		m_ds.line_join(props.lineJoin);
	}

	virtual void rewind(unsigned id) { m_ds.rewind(id); }
	virtual unsigned vertex(double *x, double *y) { return m_ds.vertex(x, y); }

private:
	typedef agg::conv_dash<Source> dash_type;
	typedef agg::conv_stroke<dash_type> dash_stroke_type;

	dash_type m_d;
	dash_stroke_type m_ds;
};

template<class Source>
class dash_stroke_simple
{
public:
	dash_stroke_simple(Source &src) : base(src)
	{
		impl = 0;
	}

	~dash_stroke_simple() { delete impl; }

	void setup(stroke_rendering_properties &props)
	{
		if(impl)
			return;

		unsigned int dashLength = !props.dashArray.isEmpty() ? props.dashArray.count() : 0;
		if(dashLength > 0)
			impl = new dash_stroke<Source>(base, props);
		else
			impl = new stroke<Source>(base, props);
	}

	void rewind(unsigned id) { Q_ASSERT(impl != 0); return impl->rewind(id); }
	unsigned vertex(double *x, double *y) { Q_ASSERT(impl != 0); return impl->vertex(x, y); }

private:
	Source &base;
	stroke_base *impl;
};

typedef dash_stroke_simple<curved> curved_stroked;
typedef agg::conv_transform<curved_stroked> curved_stroked_trans;

// Clipping related
typedef agg::conv_clip_polygon<curved_trans> curved_trans_clipped;
typedef agg::conv_clip_polygon<curved_stroked_trans> curved_stroked_trans_clipped;

// Needed for agg::bounding_rect
class GetIdDummy
{
public:
	GetIdDummy() { }
	~GetIdDummy() { }

	unsigned operator [](unsigned) { return 0; }
};

static GetIdDummy theGetIdDummy;

// Most important datastructure in the agg backend
class KCanvasAggData
{
public:
	KCanvasAggData() : curvedPath(storage), curvedPathTrans(curvedPath, transform),
					   curvedPathTransClipped(curvedPathTrans), curvedPathStroked(curvedPath),
					   curvedPathStrokedTrans(curvedPathStroked, transform),
					   curvedPathStrokedTransClipped(curvedPathStrokedTrans)
	{ }

	~KCanvasAggData() { }

	agg::path_storage storage;
	agg::trans_affine transform;

	curved curvedPath;
	curved_trans curvedPathTrans;
	curved_trans_clipped curvedPathTransClipped;

	curved_stroked curvedPathStroked;
	curved_stroked_trans curvedPathStrokedTrans;
	curved_stroked_trans_clipped curvedPathStrokedTransClipped;
};

// Gradient related
class gradient_polymorphic_wrapper_base
{
public:
	virtual int calculate(int x, int y, int) const = 0;
};

class gradient_linear : public gradient_polymorphic_wrapper_base
{
public:
	virtual int calculate(int x, int y, int d) const
	{
		return m_gradient.calculate(x, y, d);
	}

	private:
		agg::gradient_x m_gradient;
};

class gradient_linear_repeat : public gradient_linear
{
public:
	virtual int calculate(int x, int, int d) const
	{
		return (x < 0) ? (d - (-x % d)) : (x % d);
	}
};

class gradient_linear_reflect : public gradient_linear
{
public:
	virtual int calculate(int x, int, int d) const
	{
		if((abs(x) / d) % 2 == 0)
			return (x < 0) ? (-x % d) : (x % d);
		else
			return (x > 0) ? (d - (x % d)) : (d - (-x % d));
	}
};

class gradient_radial : public gradient_polymorphic_wrapper_base
{
public:
	gradient_radial(int fx = 0, int fy = 0) : m_fx(fx), m_fy(fy) { }

	void setFx(int fx) { m_fx = fx; }
	void setFy(int fy) { m_fy = fy; }

	virtual int calculate(int x, int y, int d) const
	{
		if(m_fx == 0 && m_fy == 0)
			return int(sqrt(x * x + y * y));

		double solutionX, solutionY;
		if(x == m_fx)
		{
			solutionX = m_fx;
			solutionY = (y > m_fy) ? sqrt(d * d - (m_fx * m_fx)) : -sqrt(d * d - (m_fx * m_fx));
		}
		else
		{
			double slope = double(y - m_fy) / double(x - m_fx);
			double yIntCpt = y - (slope * double(x)); // y-intercept of that same line

			// Use the quadratic formula to calculate the intersection point
			double a = (slope * slope) + 1;
			double b = (-2 * slope * (-yIntCpt));
			double c = -(d * d) + yIntCpt * yIntCpt;
			double det = sqrt((b * b) - (4 * a * c));

			solutionX = (-b + ((x < m_fx) ? -det : det)) / (2 * a);
			solutionY = (slope * solutionX) + yIntCpt;
		}

		double deltaXSq = solutionX - m_fx;
		deltaXSq *= deltaXSq; // deltaXSq^2

		double deltaYSq = solutionY - m_fy;
		deltaYSq *= deltaYSq; // deltaYSq^2

		double intersectToFocusSq = deltaXSq + deltaYSq;

		deltaXSq = x - m_fx;
		deltaXSq *= deltaXSq; // deltaXSq^2

		deltaYSq = y - m_fy;
		deltaYSq *= deltaYSq; // deltaYSq^2

		double currentToFocusSq = deltaXSq + deltaYSq;

		// We want the percentage (0-1) of the current point along the focus-circumference line
		double g = sqrt(currentToFocusSq / intersectToFocusSq);
		return int(g * d);
	}

	int m_fx;
	int m_fy;
};

class gradient_radial_repeat : public gradient_radial
{
public:
	virtual int calculate(int x, int y, int d) const
	{
		return gradient_radial::calculate(x, y, d) % d;
	}
};

class gradient_radial_reflect : public gradient_radial
{
public:
	virtual int calculate(int x, int y, int d) const
	{
		int dist = gradient_radial::calculate(x, y, d);
		if((dist / d) % 2 == 0)
			return dist % d;
		else
			return d - (dist % d);
	}
};

static gradient_linear gradientLinearPad;
static gradient_linear_repeat gradientLinearRepeat;
static gradient_linear_reflect gradientLinearReflect;

static gradient_radial gradientRadialPad;
static gradient_radial_repeat gradientRadialRepeat;
static gradient_radial_reflect gradientRadialReflect;

typedef agg::span_interpolator_linear<> interpolator_type;
typedef agg::span_gradient<agg::rgba8, interpolator_type, gradient_polymorphic_wrapper_base, const agg::rgba8 *> gradient_span_gen;
typedef agg::span_allocator<gradient_span_gen::color_type> gradient_span_alloc;
typedef agg::renderer_scanline_aa<renderer_base, gradient_span_gen> renderer_gradient_type;

// Pattern related
typedef agg::span_allocator<agg::rgba8> pattern_span_alloc;
typedef agg::span_pattern_rgba32<agg::order_bgra32> pattern_span_gen;
typedef agg::renderer_scanline_aa<renderer_base, pattern_span_gen> renderer_pattern_type;

// Image related
typedef agg::span_allocator<agg::rgba8> image_span_alloc;
typedef agg::span_image_filter_rgba32_bilinear<agg::order_bgra32, interpolator_type> image_span_gen;
typedef agg::renderer_scanline_aa<renderer_base, image_span_gen> renderer_image_type;

// Clipping related
typedef agg::pixfmt_gray8 clip_pixfmt;
typedef agg::renderer_base<clip_pixfmt> clip_renderer_base;
typedef agg::renderer_scanline_aa_solid<clip_renderer_base> clip_renderer_solid;

typedef agg::alpha_mask_gray8 clip_alpha_mask;
typedef agg::scanline_u8_am<clip_alpha_mask> clip_scanline_type;

#endif

// vim:ts=4:noet
