var svg = new QSvgWidget( this );
var renderer = svg.renderer();
renderer.load("test.svg");
svg.show();
exec();
