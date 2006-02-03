var pix = new QPixmap(120,120);
var col = new QColor("blue");
pix.fill("blue");
if( pix.height() != 120 )
	println("Wrong height:" + pix.height());
println( "Size: " + pix.width() + "x" + pix.height());
if ( pix.save("test.png", "PNG", 100 ) )
{
	println("Saved.");
}
else
{
	println("Save Error");
}
