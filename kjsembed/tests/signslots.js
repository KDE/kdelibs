
var slider1 = new Widget("QSlider", this);
var slider2 = new Widget("QSlider", this);

slider1.show();
slider2.show();

connect( slider1, "valueChanged(int)", slider2, "setValue(int)");
if( slider1.isWidgetType() )
{
	print("This is a widget");
}

exec();


