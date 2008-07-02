/***************************************************************************
 *   Copyright (C) 2008 by Carlo Segato <brandon.ml@gmail.com>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include <kemoticons.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <qstring.h>
#include <klineedit.h>
#include <qlabel.h>
#include <kcombobox.h>
#include <QVBoxLayout>
#include <QTextDocument>

class KEmoTest : public QWidget
{
    Q_OBJECT
    public:
        KEmoTest();
        
    public slots:
        void changed();
        void changeTheme(const QString&theme);
    
    private:
        KLineEdit kl;
        QLabel lb;
        KEmoticons e;
        KEmoticonsTheme t;
        KComboBox cb;
};

KEmoTest::KEmoTest()
{
    QStringList tl = KEmoticons::themeList();
    kDebug() << "ThemeList:" << tl;
    
    t = e.theme();
    
    cb.addItems(e.themeList());
    cb.setCurrentIndex(e.themeList().indexOf(t.themeName()));
    
    QVBoxLayout *vb = new QVBoxLayout;
    vb->addWidget(&kl);
    vb->addWidget(&cb);
    vb->addWidget(&lb);
    setLayout(vb);
    
    connect(&kl, SIGNAL(textChanged(const QString&)), this, SLOT(changed()));
    connect(&cb, SIGNAL(activated(const QString&)), this, SLOT(changeTheme(const QString&)));
}

void KEmoTest::changed()
{
    QStringList excl;
    excl << ":)" << ":-)";
    lb.setText(t.parseEmoticons(Qt::escape(kl.text()), KEmoticonsTheme::DefaultParse, excl));
}

void KEmoTest::changeTheme(const QString &theme)
{
    t = e.theme(theme);
    changed();
}

int main(int argc, char **argv)
{
    KAboutData aboutData("kemoticonstest", 0, ki18n("KEmoticons Test"), "1.0");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    KEmoTest kt;
    
    kt.show();
    
    return app.exec();
}

#include "main.moc"
