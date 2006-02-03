var button = new Widget("QPushButton",this);
button.text = "Push Me";
button.checkable = true;
button.show();

connect( button, "clicked()", this, "pushed()" );
connect( button, "toggled(bool)", this, "toggled(bool)");
button.animateClick(1000);

exec();

function pushed()
{
	println("press");
}

function toggled( state )
{
	if( state == true )
		println("Down");
	else
		println("Up");
}
