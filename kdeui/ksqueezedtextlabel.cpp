// (c) 2000 Ronny Standtke <Ronny.Standtke@gmx.de>

#include "ksqueezedtextlabel.h"

KSqueezedTextLabel::KSqueezedTextLabel( const QString &text , QWidget *parent)
 : QLabel ( parent) {
  fontMetrics = new QFontMetrics(font());
  QSizePolicy myLabelSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  setSizePolicy(myLabelSizePolicy);
  fullText = text;
  squeezeTextToLabel();
}

KSqueezedTextLabel::KSqueezedTextLabel( QWidget *parent)
 : QLabel ( parent) {
  fontMetrics = new QFontMetrics(font());
  QSizePolicy myLabelSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  setSizePolicy(myLabelSizePolicy);
}

void KSqueezedTextLabel::resizeEvent( QResizeEvent * ) {
  squeezeTextToLabel();
}

void KSqueezedTextLabel::setText( const QString &text ) {
  fullText = text;
  squeezeTextToLabel();
}

void KSqueezedTextLabel::squeezeTextToLabel() {
  int labelWidth = size().width();
  int textWidth = fontMetrics->width(fullText);
  if (textWidth > labelWidth) {
    // start with the dots only
    QString squeezedText = "...";
    int squeezedWidth = fontMetrics->width(squeezedText);

    // estimate how many letters we can add to the dots on both sides
    int letters = fullText.length() * (labelWidth - squeezedWidth) / textWidth / 2;
    squeezedText = fullText.left(letters) + "..." + fullText.right(letters);
    squeezedWidth = fontMetrics->width(squeezedText);

    if (squeezedWidth < labelWidth) {
        // we estimated too short
        // add letters while text < label
        do {
                letters++;
                squeezedText = fullText.left(letters) + "..." + fullText.right(letters);
                squeezedWidth = fontMetrics->width(squeezedText);
        } while (squeezedWidth < labelWidth);
        letters--;
        squeezedText = fullText.left(letters) + "..." + fullText.right(letters);
    } else if (squeezedWidth > labelWidth) {
        // we estimated too long
        // remove letters while text > label
        do {
            letters--;
            squeezedText = fullText.left(letters) + "..." + fullText.right(letters);
            squeezedWidth = fontMetrics->width(squeezedText);
        } while (squeezedWidth > labelWidth);
    }

    if (letters < 5) {
    	// too few letters added -> we give up squeezing
    	QLabel::setText(fullText);
    } else {
	QLabel::setText(squeezedText);
    }

    QToolTip::remove( this );
    QToolTip::add( this, fullText );

  } else {
    QLabel::setText(fullText);

    QToolTip::remove( this );
    QToolTip::hide();

  };
}

#include "ksqueezedtextlabel.moc"
