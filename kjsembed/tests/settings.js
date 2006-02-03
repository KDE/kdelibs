var settings = new QSettings("test.ini", QSettings.IniFormat, this);

settings.setValue("Main/value1", 10);
settings.setValue("Main/value2", new QColor("blue") );
settings.setValue("Main/value3", "Text");
settings.setValue("Main/value4", new QPoint(10,10) );
settings.sync();
var keys = settings.allKeys();
for( x in keys )
	println( keys[x] + ":\t" + settings.value(keys[x]));