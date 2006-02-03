
var frame = new Widget("QFrame", this);
frame.show();
frame.onResizeEvent = function ( ev )
{
	println("Got resize event " + ev);
	println("Size: " + ev.size.width() + "x" + ev.size.height() );
	println("QSize: " + ev.size );
}

exec();
