println("Start rect test");

var rect = new QRect(1,2,3,4);
println("Rect: " + rect);
for( x in rect )
{
	println( x );
	for( y in rect[x] )
		println( y );
}
