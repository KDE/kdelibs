/*  This file is part of the KDE libraries

    Copyright (C) 2008 Chusslove Illich <caslav.ilic@gmx.net>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kfontcombobox.h"
#include "fonthelpers_p.h"

#include "kdebug.h"
#include "klocale.h"
#include "kcolorscheme.h"
#include "kglobalsettings.h"
#include "kfontchooser.h"
#include "kcompletion.h"

#include <QEvent>
#include <QListView>
#include <QFontDatabase>
#include <QIcon>
#include <QAbstractItemDelegate>
#include <QStringListModel>
#include <QPainter>
#include <QList>
#include <QHash>
#include <QScrollBar>

static QString alphabetSample ()
{
    return i18nc("short",
    // i18n: A shorter version of the alphabet test phrase translated in
    // another message. It is displayed in the dropdown list of font previews
    // (the font selection combo box), so keep it under the length equivalent
    // to 60 or so proportional Latin characters.
                 "The Quick Brown Fox Jumps Over The Lazy Dog");
}

class KFontFamilyDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    explicit KFontFamilyDelegate (QObject *parent);

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const;

    QSize sizeHint (const QStyleOptionViewItem &option,
                    const QModelIndex &index) const;

    QIcon truetype;
    QIcon bitmap;
    double sizeFactFamily;
    double sizeFactSample;

    QHash<QString, QString> fontFamilyTrMap;
};

KFontFamilyDelegate::KFontFamilyDelegate (QObject *parent)
: QAbstractItemDelegate(parent)
{
    truetype = QIcon(QLatin1String(":/trolltech/styles/commonstyle/images/fonttruetype-16.png"));
    bitmap = QIcon(QLatin1String(":/trolltech/styles/commonstyle/images/fontbitmap-16.png"));

    // Font size factors for family name and text sample in font previes,
    // multiplies normal font size.
    sizeFactFamily = 1.0;
    sizeFactSample = 1.0; // better leave at 1, so that user can relate sizes to default
}

void KFontFamilyDelegate::paint (QPainter *painter,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    QBrush sampleBrush;
    if (option.state & QStyle::State_Selected) {
        painter->save();
        painter->setBrush(option.palette.highlight());
        painter->setPen(Qt::NoPen);
        painter->drawRect(option.rect);
        painter->setPen(QPen(option.palette.highlightedText(), 0));
        sampleBrush = option.palette.highlightedText();
    } else {
        sampleBrush = KColorScheme(QPalette::Normal).foreground(KColorScheme::InactiveText);
    }

    QFont baseFont = KGlobalSettings::generalFont();
    QString trFontFamily = index.data(Qt::DisplayRole).toString();
    QString fontFamily = fontFamilyTrMap[trFontFamily];

    // Writing systems provided by the font.
    QList<QFontDatabase::WritingSystem> availableSystems = QFontDatabase().writingSystems(fontFamily);

    // Intersect font's writing systems with that specified for
    // the language's sample text, to see if the sample can be shown.
    // If the font reports no writing systems, assume it can show the sample.
    bool canShowLanguageSample = true;
    if (availableSystems.count() > 0) {
        canShowLanguageSample = false;
        QString scriptsSpec = i18nc("Numeric IDs of scripts for font previews",
        // i18n: Integer which indicates the script you used in the sample text
        // for font previews in your language. For the possible values, see
        // http://doc.trolltech.com/qfontdatabase.html#WritingSystem-enum
        // If the sample text contains several scripts, their IDs can be given
        // as a comma-separated list (e.g. for Japanese it is "1,27").
                                    "1");
        QStringList scriptStrIds = scriptsSpec.split(',');
        foreach (const QString &scriptStrId, scriptStrIds) {
            bool convOk;
            int ws = scriptStrId.toInt(&convOk);
            if (   convOk && ws > 0 && ws < QFontDatabase::WritingSystemsCount
                && availableSystems.contains(static_cast<QFontDatabase::WritingSystem>(ws))) {
                canShowLanguageSample = true;
                break;
            }
        }
    }

    // Choose and paint an icon according to the font type, scalable or bitmat.
    const QIcon *icon = &bitmap;
    if (QFontDatabase().isSmoothlyScalable(fontFamily)) {
        icon = &truetype;
    }
    QRect r = option.rect;
    icon->paint(painter, r, Qt::AlignLeft|Qt::AlignTop);

    // Claim space taken up by the icon.
    QSize actualSize = icon->actualSize(r.size());
    if (option.direction == Qt::RightToLeft) {
        r.setRight(r.right() - actualSize.width() - 4);
    } else {
        r.setLeft(r.left() + actualSize.width() + 4);
    }

    // Draw the font family.
    QFont oldPainterFont = painter->font();
    QFont familyFont = baseFont;
    familyFont.setPointSizeF(familyFont.pointSizeF() * sizeFactFamily);
    painter->setFont(familyFont);
    painter->drawText(r, Qt::AlignTop|Qt::AlignLeading|Qt::TextSingleLine, trFontFamily);

    // Claim space taken up by the font family name.
    int h = painter->fontMetrics().lineSpacing();
    r.setTop(r.top() + h);

    // Show text sample in user's language if the writing system is supported,
    // otherwise show a collage of generic script samples provided by Qt.
    // If the font does not report what it supports, assume all.
    QString sample;
    if (canShowLanguageSample) {
        sample = alphabetSample();
    } else {
        foreach (const QFontDatabase::WritingSystem &ws, availableSystems) {
            sample += QFontDatabase::writingSystemSample(ws) + "  ";
            if (sample.length() > 40) { // do not let the sample be too long
                break;
            }
        }
        sample = sample.trimmed();
    }
    QFont sampleFont;
    sampleFont.setFamily(fontFamily);
    sampleFont.setPointSizeF(sampleFont.pointSizeF() * sizeFactSample);
    painter->setFont(sampleFont);
    QPen oldPen = painter->pen();
    painter->setPen(sampleBrush.color());
    painter->drawText(r, Qt::AlignTop|Qt::AlignLeading|Qt::TextSingleLine, sample);
    painter->setFont(oldPainterFont);
    painter->setPen(oldPen);

    if (option.state & QStyle::State_Selected) {
        painter->restore();
    }
}

QSize KFontFamilyDelegate::sizeHint (const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    Q_UNUSED(option);

    QFont baseFont = KGlobalSettings::generalFont();
    QString trFontFamily = index.data(Qt::DisplayRole).toString();
    QString fontFamily = fontFamilyTrMap[trFontFamily];

    QFont familyFont = baseFont;
    familyFont.setPointSizeF(familyFont.pointSizeF() * sizeFactFamily);
    QFontMetrics familyMetrics(familyFont);

    QFont sampleFont = baseFont;
    sampleFont.setFamily(fontFamily);
    sampleFont.setPointSizeF(sampleFont.pointSizeF() * sizeFactSample);
    QFontMetrics sampleMetrics(familyFont);
    QString sample = alphabetSample();

    // Only the hight matters here, the width is mandated by KFontComboBox::event()
    return QSize(qMax(familyMetrics.width(trFontFamily), sampleMetrics.width(sample)),
                 qRound(familyMetrics.lineSpacing() + sampleMetrics.lineSpacing() * 1.2));
}

class KFontComboBoxPrivate
{
public:
    KFontComboBoxPrivate (KFontComboBox *parent);
    void updateDatabase ();
    void updateIndexToFont ();
    void _k_currentFontChanged (int index);

    KFontComboBox *k;
    QFont currentFont;
    bool onlyFixed;
    bool signalsAllowed;
    KFontFamilyDelegate *delegate;
    QStringListModel *model;
};

KFontComboBoxPrivate::KFontComboBoxPrivate (KFontComboBox *parent)
{
    k = parent;
    currentFont = KGlobalSettings::generalFont();
    onlyFixed = false;
    signalsAllowed = true;
}

void KFontComboBoxPrivate::updateDatabase ()
{
    QStringList fontFamilies;
    KFontChooser::getFontList(fontFamilies,
                              onlyFixed ? KFontChooser::FixedWidthFonts : 0);

    // Translate font families for the list model.
    delegate->fontFamilyTrMap.clear();
    QStringList trFontFamilies =
        translateFontNameList(fontFamilies, &(delegate->fontFamilyTrMap));

    // Add families to the list model and completion.
    model->setStringList(trFontFamilies);
    KCompletion *completion = k->completionObject();
    if (completion) {
        completion->insertItems(trFontFamilies);
        completion->setIgnoreCase(true);
    }
}

void KFontComboBoxPrivate::updateIndexToFont ()
{
    // QFontInfo necessary to return the family with proper casing.
    QString selectedFontFamily = QFontInfo(currentFont).family();
    QString trSelectedFontFamily = translateFontName(selectedFontFamily);
    const QStringList trFontFamilies = model->stringList();
    if (!trFontFamilies.count()) {
        return;
    }

    // Match the font's family with an item in the list.
    int index = 0;
    foreach (const QString &trFontFamily, trFontFamilies) {
        if (trSelectedFontFamily == trFontFamily) {
            break;
        }
        ++index;
    }
    if (index == trFontFamilies.count()) {
        // If no family matched, change font to first on the list.
        index = 0;
        currentFont = QFont(delegate->fontFamilyTrMap[trFontFamilies[0]]);
        emit k->currentFontChanged(currentFont);
    }

    // Set the new list item.
    signalsAllowed = false;
    k->setCurrentIndex(index);
    signalsAllowed = true;
}

void KFontComboBoxPrivate::_k_currentFontChanged (int index)
{
    if (!signalsAllowed) {
        return;
    }

    QString trFontFamily = k->itemText(index);
    QString fontFamily = delegate->fontFamilyTrMap[trFontFamily];
    if (!fontFamily.isEmpty()) {
        currentFont = QFont(fontFamily);
        emit k->currentFontChanged(currentFont);
    } else {
        // Unknown font family given. Just remove from the list.
        // This should not happen, as adding arbitrary font names is prevented.
        QStringList lst = model->stringList();
        lst.removeAll(trFontFamily);
        model->setStringList(lst);
    }
}

KFontComboBox::KFontComboBox (QWidget *parent)
: KComboBox(true, parent), d(new KFontComboBoxPrivate(this))
{
    // Inputing arbitrary font names does not make sense.
    setInsertPolicy(QComboBox::NoInsert);

    // Special list item painter showing font previews and its list model.
    d->delegate = new KFontFamilyDelegate(this);
    setItemDelegate(d->delegate);
    d->model = new QStringListModel(this);
    setModel(d->model);

    // Set current font when a new family has been chosen in the combo.
    connect(this, SIGNAL(currentIndexChanged(int)),
            this, SLOT(_k_currentFontChanged(int)));

    // Initialize font selection and list of available fonts.
    d->updateDatabase();
    d->updateIndexToFont();
}

KFontComboBox::~KFontComboBox ()
{
    delete d;
}

void KFontComboBox::setOnlyFixed (bool onlyFixed)
{
    if (onlyFixed != d->onlyFixed) {
        d->onlyFixed = onlyFixed;
        d->updateDatabase();
    }
}

QFont KFontComboBox::currentFont () const
{
    return d->currentFont;
}

void KFontComboBox::setCurrentFont (const QFont &font)
{
    if (font != d->currentFont) {
        d->currentFont = font;
        emit currentFontChanged(d->currentFont);
        d->updateIndexToFont();
    }
}

bool KFontComboBox::event (QEvent *e)
{
    if (e->type() == QEvent::Resize) {
        QListView *lview = qobject_cast<QListView*>(view());
        if (lview) {
            QString sample = alphabetSample();
            // Limit text sample length to avoid too wide list view.
            if (sample.length() > 60) {
                sample = sample.left(57) + "...";
            }
            QFont approxFont = KGlobalSettings::generalFont();
            approxFont.setPointSizeF(approxFont.pointSizeF()
                                           * d->delegate->sizeFactSample);
            int widgetWidth = width();
            int sampleWidth = QFontMetrics(approxFont).width(sample);
            sampleWidth = qRound(sampleWidth * 1.1); // extra for wider fonts
            int iconWidth = d->delegate->truetype.actualSize(size()).width();
            int vsbarWidth = 0;
            if (lview->verticalScrollBar()) {
                vsbarWidth = lview->verticalScrollBar()->width();
            }
            lview->window()->setFixedWidth(  qMax(widgetWidth, sampleWidth)
                                           + iconWidth + vsbarWidth);
        }
    }
    return KComboBox::event(e);
}

QSize KFontComboBox::sizeHint() const
{
    QSize sz = KComboBox::sizeHint();
    QFontMetrics fm(KGlobalSettings::generalFont());
    sz.setWidth(fm.width("m") * 14);
    return sz;
}

#include "kfontcombobox.moc"
#include "moc_kfontcombobox.moc"

