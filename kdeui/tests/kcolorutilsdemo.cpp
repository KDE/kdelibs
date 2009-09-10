/*
 * Copyright 2009 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kcolorutilsdemo.h"
#include "../colors/kcolorspaces.h"
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kcolorutils.h>
#include <kcolorscheme.h>

KColorUtilsDemo::KColorUtilsDemo(QWidget *parent) : QWidget(parent)
{
    setupUi(this);

    inputSpinChanged();
    targetSpinChanged();
}

void KColorUtilsDemo::inputChanged()
{
    KColorSpaces::KHCY c(inColor->color());
    ifHue->setValue(c.h);
    ifChroma->setValue(c.c);
    ifLuma->setValue(c.y);
    ifGray->setValue(qGray(inColor->color().rgb()));

    lumaChanged();
    mixChanged();
    shadeChanged();
}

void KColorUtilsDemo::lumaChanged()
{
}

void KColorUtilsDemo::mixChanged()
{
}

void setBackground(QWidget *widget, const QColor &color)
{
    QPalette palette = widget->palette();
    palette.setColor(widget->backgroundRole(), color);
    widget->setPalette(palette);
}

#define SET_SHADE(_n, _c, _cn, _ch) \
    setBackground(ss##_n, KColorScheme::shade(_c, KColorScheme::_n##Shade, _cn, _ch));

void KColorUtilsDemo::shadeChanged()
{
    qreal cn = ssContrast->value();
    qreal ch = ssChroma->value();

    QColor base = inColor->color();
    setBackground(ssOut, base);
    setBackground(ssBase, base);
    SET_SHADE(Light,    base, cn, ch);
    SET_SHADE(Midlight, base, cn, ch);
    SET_SHADE(Mid,      base, cn, ch);
    SET_SHADE(Dark,     base, cn, ch);
    SET_SHADE(Shadow,   base, cn, ch);
}

void updateSwatch(KColorButton *s, const QSpinBox *r, const QSpinBox *g, const QSpinBox *b)
{
    s->setColor(QColor(r->value(), g->value(), b->value()));
}

void updateSpins(const QColor &c, QSpinBox *r, QSpinBox *g, QSpinBox *b)
{
    r->setValue(c.red());
    g->setValue(c.green());
    b->setValue(c.blue());
}

void KColorUtilsDemo::inputSpinChanged()
{
    if (_noUpdate)
        return;
    _noUpdate = true;

    updateSwatch(inColor, inRed, inGreen, inBlue);
    inputChanged();

    _noUpdate = false;
}

void KColorUtilsDemo::targetSpinChanged()
{
    if (_noUpdate)
        return;
    _noUpdate = true;

    updateSwatch(mtTarget, mtRed, mtGreen, mtBlue);
    mixChanged();

    _noUpdate = false;
}

void KColorUtilsDemo::inputSwatchChanged(const QColor &color)
{
    if (_noUpdate)
        return;
    _noUpdate = true;

    updateSpins(color, inRed, inGreen, inBlue);
    inputChanged();

    _noUpdate = false;
}

void KColorUtilsDemo::targetSwatchChanged(const QColor &color)
{
    if (_noUpdate)
        return;
    _noUpdate = true;

    updateSpins(color, mtRed, mtGreen, mtBlue);
    mixChanged();

    _noUpdate = false;
}

int main(int argc, char* argv[]) {
    KAboutData about("kcolorutilsdemo", 0, ki18n("kcolorutilsdemo"), "0.1",
                     ki18n("KColorUtils demo/test application"),
                     KAboutData::License_GPL, ki18n("Copyright 2009 Matthew Woehlke"),
                     KLocalizedString(), 0, "mw_triad@users.sourceforge.net");
    about.addAuthor(ki18n("Matthew Woehlke"), KLocalizedString(),
                    "mw_triad@users.sourceforge.net");
    KCmdLineArgs::init(argc, argv, &about);

    KApplication app;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    Q_UNUSED(args);

    KColorUtilsDemo *d = new KColorUtilsDemo;
    d->show();
    return app.exec();
}

#include "kcolorutilsdemo.moc"
// kate: hl C++; indent-width 4; replace-tabs on;
