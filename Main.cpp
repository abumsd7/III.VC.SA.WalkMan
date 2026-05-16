#include <plugin.h>
#include <CSprite2d.h>
#include "includes/MusicPlayer.h"
#include "includes/DrawPlayer.h"
#include "includes/WalkManConfig.h"

#ifdef GTASA
#include "includes/MusicPlayerSA.h"
#endif

using namespace plugin;

struct Main
{
    Main()
    {
        // Initialise MusicPlayer once the game is ready
        Events::initRwEvent += [] { 
            WalkmanState::Initialise();
            DrawPlayer::Initialise(); 
#ifdef GTASA
            MusicPlayerSA::Initialise();
#else
            MusicPlayer::Initialise(); 
#endif

            if (config.ReloadKey) {
                Events::gameProcessEvent += [] {
                    if (config.ReloadKey != 0) {
                        if (config.ReloadKey == -1 || KeyPressed(config.ReloadKey))
                            config.Read();
                    }
                };
            }
        };

        // Handle music player updates every frame
        Events::gameProcessEvent += [] { 
#ifdef GTASA
            MusicPlayerSA::Update();
#else
            MusicPlayer::Update(); 
#endif
        };

        // Draw music player UI
        Events::drawHudEvent += [] { DrawPlayer::Draw(); };

        Events::shutdownRwEvent += [] { DrawPlayer::Shutdown(); };
    }
} gInstance;
