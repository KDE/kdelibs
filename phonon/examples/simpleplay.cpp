SimplePlayer *player = new SimplePlayer;
player->play(url);
connect(player, SIGNAL(finished()), player, SLOT(deleteLater()));
