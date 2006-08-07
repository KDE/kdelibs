MediaObject* media = new MediaObject;
media->setUrl( url );
media->play();
connect( media, SIGNAL( finished() ), media, SLOT( deleteLater() ) );
