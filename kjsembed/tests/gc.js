var arr = new Array();
var frame = new Widget("QFrame", this );
for( var idx = 0; idx < 500; ++idx )
{
	arr[idx] = new Widget("QLabel", 0);
	arr[idx].text = "foo " + idx;
}

for( var idx = 0; idx < 500; ++idx )
{
	//println( arr[idx].text );
	arr[idx] = new Widget("QLabel", frame);
	arr[idx].text = "bar " + idx;
}

for( var idx = 0; idx < 500; ++idx )
{
	//println( arr[idx].text );
	arr[idx] = new Widget("QLabel", frame);
	arr[idx].text = "baz " + idx;
}

for( var idx = 0; idx < 500; ++idx )
{
	println( arr[idx].text );
}

