
if (PreStatus != gamephase) {
    switch (gamephase) {
        case WAIT_FOR_PLAYER:
            // chang to wait screen
            PreStatus = WAIT_FOR_PLAYER
            break;
        case WAIT_TO_PLAY:
            // change color from gray to RED
            PreStatus = WAIT_TO_PLAY;
            break;
        case READY_TO_PLAY:
            // Enable start button
            PreStatus = READY_TO_PLAY;
            break;
    }
}

// Player an start => gamephase = PLAYING
