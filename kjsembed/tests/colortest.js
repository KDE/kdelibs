println("Start color test");
var color = new QColor("red");
println("Color: " + color);
for( x in color )
	println( x );
if( color.red() != 255 )
	println("Error red color is " + color.red() );
else
	println("Red okay");
if( color.green() != 0 )
	println("Error green color is " + color.green() );
else
	println("Green okay");

color.setRed( 155 );
if( color.red() != 155 )
	println("Error red color is " + color.red() );
else
	println("Red okay");

println( "Color " + color );
color.setGreen( 250 );
color.setBlue( 100 );

var darkColor = color.dark();
println( "Dark color " + darkColor );
