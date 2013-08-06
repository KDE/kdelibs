// krazy:excludeall=spelling
/**
 * test_textedit.cpp
 *
 * Copyright (C)  2013  Aurélien Gâteau <agateau@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
// Local
#include <dictionarycombobox.h>
#include <highlighter.h>
#include <texteditinstaller.h>

// Qt
#include <QApplication>
#include <QDebug>
#include <QTextEdit>
#include <QVBoxLayout>

class MailTextEditInstaller : public Sonnet::TextEditInstaller
{
public:
    MailTextEditInstaller(QTextEdit *edit)
    : Sonnet::TextEditInstaller(edit)
    {}

protected:
    bool shouldBlockBeSpellChecked(const QString &blockText) const Q_DECL_OVERRIDE
    {
        qDebug() << blockText;
        return !blockText.startsWith(">");
    }
};

int main( int argc, char** argv )
{
    QApplication app(argc, argv);

    QWidget window;

    Sonnet::DictionaryComboBox *comboBox = new Sonnet::DictionaryComboBox;


    QTextEdit *textEdit = new QTextEdit;
    textEdit->setText( "This is a sample buffer. Whih this thingg will "
        "be checkin for misstakes. Whih, Enviroment, govermant. Whih."
        );

    Sonnet::TextEditInstaller *installer = new Sonnet::TextEditInstaller(textEdit);
    installer->highlighter()->setCurrentLanguage("en");
    QObject::connect(comboBox, SIGNAL(dictionaryChanged(QString)), installer->highlighter(), SLOT(setCurrentLanguage(QString)));


    QTextEdit *mailTextEdit = new QTextEdit;
    mailTextEdit->setText(
        "John Doe said:\n"
        "> Hello how aree you?\n"
        "I am ffine thanks");

    installer = new MailTextEditInstaller(mailTextEdit);
    installer->highlighter()->setCurrentLanguage("en");
    QObject::connect(comboBox, SIGNAL(dictionaryChanged(QString)), installer->highlighter(), SLOT(setCurrentLanguage(QString)));


    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->addWidget(comboBox);
    layout->addWidget(textEdit);
    layout->addWidget(mailTextEdit);

    window.show();
    return app.exec();
}
