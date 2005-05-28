/**
 * This file is part of the HTML rendering engine for KDE.
 *
 * Copyright (C) 2004 Allan Sandfeld Jensen (kde@carewolf.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "rendering/render_generated.h"
#include "rendering/render_style.h"
#include "rendering/enumerate.h"
#include "rendering/counter_tree.h"
#include "css/css_valueimpl.h"

using namespace khtml;
using namespace Enumerate;

// -------------------------------------------------------------------------

RenderCounter::RenderCounter(DOM::NodeImpl* node, const DOM::CounterImpl* counter)
    : RenderText(node,0), m_counter(counter), m_counterNode(0)
{
}

void RenderCounter::layout()
{
    KHTMLAssert( needsLayout() );

    if ( !minMaxKnown() )
        calcMinMaxWidth();

    RenderText::layout();
}

QString RenderCounter::toListStyleType(int value, int total, EListStyleType type)
{
    QString item;
    switch(type)
    {
    case LNONE:
        break;
// Glyphs:
    case LDISC:
        item = QChar(0x2022);
        break;
    case LCIRCLE:
        item = QChar(0x25e6);
        break;
    case LSQUARE:
        item = QChar(0x25a0);
        break;
    case LBOX:
        item = QChar(0x25a1);
        break;
    case LDIAMOND:
        item = QChar(0x25c6);
        break;
// Numeric:
    case LDECIMAL:
        item.setNum ( value );
        break;
    case DECIMAL_LEADING_ZERO: {
        int decimals = 2;
        int t = total/100;
        while (t>0) {
            t = t/10;
            decimals++;
        }
        decimals = kMax(decimals, 2);
        QString num = QString::number(value);
        m_item.fill('0',decimals-num.length());
        m_item.append(num);
        break;
    }
    case ARABIC_INDIC:
        item = toArabicIndic( value );
        break;
    case LAO:
        item = toLao( value );
        break;
    case PERSIAN:
    case URDU:
        item = toPersianUrdu( value );
        break;
    case THAI:
        item = toThai( value );
        break;
    case TIBETAN:
        item = toTibetan( value );
        break;
// Algoritmic:
    case LOWER_ROMAN:
        item = toRoman( value, false );
        break;
    case UPPER_ROMAN:
        item = toRoman( value, true );
        break;
    case HEBREW:
        item = toHebrew( value );
        break;
    case ARMENIAN:
        item = toArmenian( value );
        break;
    case GEORGIAN:
        item = toGeorgian( value );
        break;
// Alphabetic:
    case LOWER_ALPHA:
    case LOWER_LATIN:
        item = toLowerLatin( value );
        break;
    case UPPER_ALPHA:
    case UPPER_LATIN:
        item = toUpperLatin( value );
        break;
    case LOWER_GREEK:
        item = toLowerGreek( value );
        break;
    case UPPER_GREEK:
        item = toUpperGreek( value );
        break;
    case HIRAGANA:
        item = toHiragana( value );
        break;
    case HIRAGANA_IROHA:
        item = toHiraganaIroha( value );
        break;
    case KATAKANA:
        item = toKatakana( value );
        break;
    case KATAKANA_IROHA:
        item = toKatakanaIroha( value );
        break;
// Ideographic:
    case JAPANESE_FORMAL:
        item = toJapaneseFormal( value );
        break;
    case JAPANESE_INFORMAL:
        item = toJapaneseInformal( value );
        break;
    case SIMP_CHINESE_FORMAL:
        item = toSimpChineseFormal( value );
        break;
    case SIMP_CHINESE_INFORMAL:
        item = toSimpChineseInformal( value );
        break;
    case TRAD_CHINESE_FORMAL:
        item = toTradChineseFormal( value );
        break;
    case CJK_IDEOGRAPHIC:
        // CSS 3 List says treat as trad-chinese-informal
    case TRAD_CHINESE_INFORMAL:
        item = toTradChineseInformal( value );
        break;
// Quotes:
    case OPEN_QUOTE:
        item = style()->openQuote( value );
        break;
    case CLOSE_QUOTE:
        item = style()->closeQuote( value );
        break;
    default:
        item.setNum ( value );
        break;
    }
    return item;
}

void RenderCounter::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    if (!style()) return;

    bool counters;
    counters = !m_counter->separator().isEmpty();

    if (!m_counterNode)
        m_counterNode = getCounter(m_counter->identifier().string(), true, counters);

    int value = m_counterNode->count();
    if (m_counterNode->isReset()) value = m_counterNode->value();
    int total = value;
    if (m_counterNode->parent()) total = m_counterNode->parent()->total();
    m_item = toListStyleType(value, total, (EListStyleType)m_counter->listStyle());

    if (counters) {
        CounterNode *counter = m_counterNode->parent();
        // we deliberately do not render the root counter-node
        while(counter->parent() && !(counter->isReset() && counter->parent()->isRoot())) {
            value = counter->count();
            total = counter->parent()->total();
            m_item = toListStyleType(value, total, (EListStyleType)m_counter->listStyle()) + m_counter->separator().string() + m_item;
            counter = counter->parent();
        };
    }

    if (str) str->deref();
    str = new DOM::DOMStringImpl(m_item.unicode(), m_item.length());
    str->ref();

    RenderText::calcMinMaxWidth();
}
