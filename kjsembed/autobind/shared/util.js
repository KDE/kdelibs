
function contains( var string, var part )
{
    if (string.indexOf(part) != -1)
        return true;
    return false;
}

// An array of primitive Qt types, this is annoying but seems to be necessary
var variant_types = [
    'QBitArray', 'QBitmap', 'bool', 'QBrush',
    'QByteArray', 'QChar', 'QColor', 'QCursor',
    'QDate', 'QDateTime', 'double', 'QFont',
    'QIcon', 'QImage', 'int', 'QKeySequence',
    'QLine', 'QLineF', 'QVariantList', 'QLocale',
    'qlonglong', 'QVariantMap', 'QPalette', 'QPen',
    'QPixmap', 'QPoint', 'QPointArray', 'QPointF',
    'QPolygon', 'QRect', 'QRectF', 'QRegExp',
    'QRegion', 'QSize', 'QSizeF', 'QSizePolicy',
    'QString', 'QStringList', 'QTextFormat',
    'QTextLength', 'QTime', 'uint', 'qulonglong',
    'QUrl'
];

function isVariant( variable )
{
    for (var i in variant_types)
    {
        if (variable.indexOf(variant_types[i]) != -1)
            return true;
    }
    return false;
}


