function isObjectOrVariant(foo)
{
    if (isVariant(foo))
    {
        println(' is a variant.');
    }
    else if (isObject(foo))
    {
        println(' is an object.');
    }
    else
    {
        println(' is a yahoo?');
    }
}

var color = new QColor("red");
println('color ');
isObjectOrVariant(color);

var frame = new QWidget("QFrame", this );
println('frame ');
isObjectOrVariant(frame);

var settings = new QSettings("test.ini", QSettings.IniFormat, this);
println('settings ');
isObjectOrVariant(settings);

