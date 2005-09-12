/*
Copyright (c) 2005 Luciano Montanaro <mikelima@cirulla.net>

based on the Keramick configuration dialog 
Copyright (c) 2003 Maksim Orlovich <maksim.orlovich@kdemail.net>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <qcheckbox.h>
#include <qlayout.h>
#include <qsettings.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>

#include "highcontrastconfig.h"

extern "C" KDE_EXPORT QWidget* 
allocate_kstyle_config(QWidget* parent)
{
    return new HighContrastStyleConfig(parent);
}

HighContrastStyleConfig::HighContrastStyleConfig(
        QWidget* parent): QWidget(parent)
{
    // Should have no margins here, the dialog provides them
    QVBoxLayout* layout = new QVBoxLayout(this, 0, 0);
    KGlobal::locale()->insertCatalog("kstyle_highcontrast_config");

    wideLinesBox = new QCheckBox(i18n("Use wider lines"), this);

    layout->add(wideLinesBox);
    layout->addStretch(1);

    QSettings s;

    originalWideLinesState = s.readBoolEntry(
            "/highcontraststyle/Settings/wideLines", false);
    wideLinesBox->setChecked(originalWideLinesState);

    connect(wideLinesBox, SIGNAL(toggled(bool)), SLOT(updateChanged()));
}

HighContrastStyleConfig::~HighContrastStyleConfig()
{
    KGlobal::locale()->removeCatalog("kstyle_keramik_config");
}


void 
HighContrastStyleConfig::save()
{
    QSettings s;
    s.writeEntry("/highcontraststyle/Settings/wideLines", 
            wideLinesBox->isChecked());
}

void 
HighContrastStyleConfig::defaults()
{
    wideLinesBox->setChecked(false);
    // updateChanged would be done by setChecked already
}

void 
HighContrastStyleConfig::updateChanged()
{
    if ((wideLinesBox->isChecked() == originalWideLinesState)) {
        emit changed(false);
    } else {
        emit changed(true);
    }
}

#include "highcontrastconfig.moc"
