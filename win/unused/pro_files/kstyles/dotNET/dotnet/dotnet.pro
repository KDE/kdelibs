include( ../../common.pro )

TARGET		= dotnet$$KDEBUG

SOURCES = \
dotnet.cpp

system( moc dotnet.h -o moc/dotnet.moc )


