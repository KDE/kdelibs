var pix = new QPixmap(100,100 );
pix.fill("white");
var painter = new QPainter();
if (painter.begin( pix ) )
{
	painter.drawLine( 0,0,100,100 );
	painter.drawLine( 100,0, 0, 100 );
	painter.end();
}
else
	println("Could not paint to " + pix );

var label = new Widget("QLabel", this );
label.pixmap = pix;
label.show();
exec();
