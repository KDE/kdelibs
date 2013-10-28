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

#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QString>

class KEmoTest : public QWidget
{
    Q_OBJECT
    public:
        KEmoTest();

    public Q_SLOTS:
        void updateEmoticons();
        void changeTheme(const QString &emoticonTheme);

    private:
        QLineEdit *lineEdit;
        QLabel *label;
        KEmoticons emoticons;
        KEmoticonsTheme emoticonTheme;
        QComboBox *comboBox;
};

KEmoTest::KEmoTest()
{
    lineEdit = new QLineEdit;
    label = new QLabel;
    QLabel *explanation = new QLabel;
    explanation->setText("Please enter text with emoticons. They will be parsed, "
                         "except <b>:-)</b> and <b>:)</b> which are excluded. "
                         "Emoticon theme can be chosen from the combo box.");
    explanation->setWordWrap(true);
    comboBox = new QComboBox;

    emoticonTheme = emoticons.theme();
    // Theme list is repeating three times the same two themes: "Oxygen" and "kde4"
    qDebug() << "Theme list: " << emoticons.themeList();
    // Theme name is empty!!
    qDebug() << "Theme name: " << emoticonTheme.themeName();

    comboBox->addItems(emoticons.themeList());
    comboBox->setCurrentIndex(emoticons.themeList().indexOf(emoticonTheme.themeName()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(explanation);
    layout->addWidget(lineEdit);
    layout->addWidget(comboBox);
    layout->addWidget(label);
    setLayout(layout);

    connect(lineEdit, SIGNAL(textChanged(QString)), this, SLOT(updateEmoticons()));
    connect(comboBox, SIGNAL(activated(QString)), this, SLOT(changeTheme(QString)));
}

void KEmoTest::updateEmoticons()
{
    QStringList excludedEmoticons;
    excludedEmoticons << ":)" << ":-)";
    label->setText(emoticonTheme.parseEmoticons(lineEdit->text().toHtmlEscaped(), KEmoticonsTheme::DefaultParse, excludedEmoticons));
}

void KEmoTest::changeTheme(const QString &theme)
{
    emoticonTheme = emoticons.theme(theme);
    updateEmoticons();
}

int main(int argc, char **argv)
{
    QApplication::setApplicationName("kemoticonstest");
    QApplication app(argc, argv);

    KEmoTest kt;
    
    kt.show();
    
    return app.exec();
}

#include "main.moc"
