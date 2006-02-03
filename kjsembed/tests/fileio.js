var file = new File("test.txt");
if( file.open( File.WriteOnly ) )
{
	for( var x = 0; x < 100; ++x)
	{
		var line = "";
		for( var y = 0; y < 80; ++y)
			line += "#";
		file.writeln(line);
	}
	file.close();
}

if( file.open( File.ReadOnly ) )
{
	var lines = 0;
	while( !file.atEnd() )
	{
		var line = file.readln();
		++lines;
	}
	println( lines + " lines found");
}	
