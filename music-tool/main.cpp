
#include "globals.h"
#include "vgmrecorder.h"
#include "FileClasses/music/MusicPlayer.h"
#include "FileClasses/music/ADLPlayer.h"
#include "FileClasses/adl/sound_adlib.h"
#include "FileClasses/FileManager.h"
#include "FileClasses/INIFile.h"
#include <getopt.h>
#include <chrono>
#include <cstdio>
namespace cro = std::chrono;

struct MusicDefinition {
    MUSICTYPE type;
    const char *filename;
    int musicNum;
    const char *title;
};

static const MusicDefinition musicTracks[] = {
    {MUSIC_ATTACK, "DUNE10.ADL", 7, "Attack 1"},
    {MUSIC_ATTACK, "DUNE11.ADL", 7, "Attack 2"},
    {MUSIC_ATTACK, "DUNE12.ADL", 7, "Attack 3"},
    {MUSIC_ATTACK, "DUNE13.ADL", 7, "Attack 4"},
    {MUSIC_ATTACK, "DUNE14.ADL", 7, "Attack 5"},
    {MUSIC_ATTACK, "DUNE15.ADL", 7, "Attack 6"},

    {MUSIC_PEACE, "DUNE1.ADL" , 6, "Peace 1"},
    {MUSIC_PEACE, "DUNE2.ADL" , 6, "Peace 2"},
    {MUSIC_PEACE, "DUNE3.ADL" , 6, "Peace 3"},
    {MUSIC_PEACE, "DUNE4.ADL" , 6, "Peace 4"},
    {MUSIC_PEACE, "DUNE5.ADL" , 6, "Peace 5"},
    {MUSIC_PEACE, "DUNE6.ADL" , 6, "Peace 6"},
    {MUSIC_PEACE, "DUNE9.ADL" , 4, "Peace 7"},
    {MUSIC_PEACE, "DUNE9.ADL" , 5, "Peace 8"},
    {MUSIC_PEACE, "DUNE18.ADL", 6, "Peace 9"},

    {MUSIC_INTRO, "DUNE0.ADL", 2, "Intro"},
    {MUSIC_MENU, "DUNE7.ADL", 6, "Menu"},
    {MUSIC_BRIEFING_H, "DUNE7.ADL", 2, "Briefing H"},
    {MUSIC_BRIEFING_A, "DUNE7.ADL", 3, "Briefing A"},
    {MUSIC_BRIEFING_O, "DUNE7.ADL", 4, "Briefing O"},
    {MUSIC_WIN_H, "DUNE8.ADL", 3, "Win H"},
    {MUSIC_WIN_A, "DUNE8.ADL", 2, "Win A"},
    {MUSIC_WIN_O, "DUNE17.ADL", 4, "Win O"},
    {MUSIC_LOSE_H, "DUNE1.ADL", 4, "Lose H"},
    {MUSIC_LOSE_A, "DUNE1.ADL", 5, "Lose A"},
    {MUSIC_LOSE_O, "DUNE1.ADL", 3, "Lose O"},
    {MUSIC_GAMESTAT, "DUNE20.ADL", 2, "Game stat"},
    {MUSIC_MAPCHOICE, "DUNE16.ADL", 7, "Map choice"},
    {MUSIC_MEANWHILE, "DUNE16.ADL", 8, "Meanwhile"},
    {MUSIC_FINALE_H, "DUNE19.ADL", 4, "Finale H"},
    {MUSIC_FINALE_A, "DUNE19.ADL", 2, "Finale A"},
    {MUSIC_FINALE_O, "DUNE19.ADL", 3, "Finale O"},
};

static constexpr size_t numMusicTracks = sizeof(musicTracks) / sizeof(musicTracks[0]);

static int cmd_list(int argc, char *argv[])
{
    for (int c; (c = getopt(argc, argv, "")) != -1;) {
        switch (c) {
        default:
            return 1;
        }
    }
    if (argc - optind != 0) {
        fprintf(stderr, "Bad number of positional arguments.\n");
        return 1;
    }

    for (size_t i = 0; i < numMusicTracks; ++i)
        printf("%zu - %s\n", i, musicTracks[i].title);

    return 0;
}

static int cmd_play(int argc, char *argv[])
{
    for (int c; (c = getopt(argc, argv, "")) != -1;) {
        switch (c) {
        default:
            return 1;
        }
    }
    if (argc - optind != 1) {
        fprintf(stderr, "Bad number of positional arguments.\n");
        return 1;
    }

    unsigned trackNum = atoi(argv[optind]);
    if (trackNum > numMusicTracks) {
        fprintf(stderr, "Invalid track number.\n");
        return 1;
    }

    Mix_OpenAudio(AUDIO_FREQUENCY, AUDIO_S16SYS, 2, 4096);

    class MyAdlibLogger : public AdlibLogger {
    public:
        VGMrecorder *vgm_ = nullptr;
        bool vgmHaveLoggedFirstReg_ = false;
        cro::steady_clock::time_point vgmTimeLastReg;

        void logOPL(uint8_t reg, uint8_t val) override {
            double timestamp = 0;
            cro::steady_clock::time_point now = cro::steady_clock::now();
            if (vgmHaveLoggedFirstReg_)
                timestamp = cro::duration_cast<cro::duration<double>>(now - vgmTimeLastReg).count();
            // fprintf(stderr, "WriteOPL: %02X %02X @ %f\n", reg, val, timestamp);
            if (vgm_) vgm_->writeReg(timestamp, reg, val);
            vgmHaveLoggedFirstReg_ = true;
            vgmTimeLastReg = now;
        }
    };

    ADLPlayer player;
    VGMrecorder vgm;
    MyAdlibLogger logger;
    if (vgm.openOutputFile("track.vgm"))
        logger.vgm_ = &vgm;
    player.setAdlibLogger(&logger);

    player.changeMusicTrack(musicTracks[trackNum].type, musicTracks[trackNum].filename, musicTracks[trackNum].musicNum);

    SDL_Event event;
    bool quit = false;
    while (!quit && SDL_WaitEvent(&event)) {
        //fprintf(stderr, "Event %d\n", event.type);
        switch (event.type) {
        case SDL_QUIT:
            quit = true;
            break;
        }
    }

    player.setAdlibLogger(nullptr);
    Mix_CloseAudio();

    return 0;
}

static void initSettings()
{
    SDL_RWops *rwEmpty = SDL_RWFromConstMem("\n", 1);
    INIFile myINIFile(rwEmpty);
    SDL_RWclose(rwEmpty);

    settings.general.playIntro = myINIFile.getBoolValue("General","Play Intro",false);
    settings.general.playerName = myINIFile.getStringValue("General","Player Name","Player");
    settings.general.language = myINIFile.getStringValue("General","Language","en");
    settings.general.scrollSpeed = myINIFile.getIntValue("General","Scroll Speed",50);
    settings.general.showTutorialHints = myINIFile.getBoolValue("General","Show Tutorial Hints",true);
    settings.video.width = myINIFile.getIntValue("Video","Width",640);
    settings.video.height = myINIFile.getIntValue("Video","Height",480);
    settings.video.physicalWidth= myINIFile.getIntValue("Video","Physical Width",640);
    settings.video.physicalHeight = myINIFile.getIntValue("Video","Physical Height",480);
    settings.video.fullscreen = myINIFile.getBoolValue("Video","Fullscreen",false);
    settings.video.frameLimit = myINIFile.getBoolValue("Video","FrameLimit",true);
    settings.video.preferredZoomLevel = myINIFile.getIntValue("Video","Preferred Zoom Level", 0);
    settings.video.scaler = myINIFile.getStringValue("Video","Scaler","ScaleHD");
    settings.video.rotateUnitGraphics = myINIFile.getBoolValue("Video","RotateUnitGraphics",false);
    settings.audio.musicType = myINIFile.getStringValue("Audio","Music Type","adl");
    settings.audio.playMusic = myINIFile.getBoolValue("Audio","Play Music", true);
    settings.audio.musicVolume = myINIFile.getIntValue("Audio","Music Volume", 64);
    settings.audio.playSFX = myINIFile.getBoolValue("Audio","Play SFX", true);
    settings.audio.sfxVolume = myINIFile.getIntValue("Audio","SFX Volume", 64);

    settings.network.serverPort = myINIFile.getIntValue("Network","ServerPort",DEFAULT_PORT);
    settings.network.metaServer = myINIFile.getStringValue("Network","MetaServer",DEFAULT_METASERVER);
    settings.network.debugNetwork = myINIFile.getBoolValue("Network","Debug Network",false);

    settings.ai.campaignAI = myINIFile.getStringValue("AI","Campaign AI",DEFAULTAIPLAYERCLASS);

    settings.gameOptions.gameSpeed = myINIFile.getIntValue("Game Options","Game Speed",GAMESPEED_DEFAULT);
    settings.gameOptions.concreteRequired = myINIFile.getBoolValue("Game Options","Concrete Required",true);
    settings.gameOptions.structuresDegradeOnConcrete = myINIFile.getBoolValue("Game Options","Structures Degrade On Concrete",true);
    settings.gameOptions.fogOfWar = myINIFile.getBoolValue("Game Options","Fog of War",false);
    settings.gameOptions.startWithExploredMap = myINIFile.getBoolValue("Game Options","Start with Explored Map",false);
    settings.gameOptions.instantBuild = myINIFile.getBoolValue("Game Options","Instant Build",false);
    settings.gameOptions.onlyOnePalace = myINIFile.getBoolValue("Game Options","Only One Palace",false);
    settings.gameOptions.rocketTurretsNeedPower = myINIFile.getBoolValue("Game Options","Rocket-Turrets Need Power",false);
    settings.gameOptions.sandwormsRespawn = myINIFile.getBoolValue("Game Options","Sandworms Respawn",false);
    settings.gameOptions.killedSandwormsDropSpice = myINIFile.getBoolValue("Game Options","Killed Sandworms Drop Spice",false);
    settings.gameOptions.manualCarryallDrops = myINIFile.getBoolValue("Game Options","Manual Carryall Drops",false);
    settings.gameOptions.maximumNumberOfUnitsOverride = myINIFile.getIntValue("Game Options","Maximum Number of Units Override",-1);
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);
    Mix_Init(MIX_INIT_MID|MIX_INIT_FLAC|MIX_INIT_MP3|MIX_INIT_OGG);

    initSettings();
    pFileManager.reset(new FileManager);

    if (argc < 2) {
        fprintf(stderr, "Please indicate a subcommand: list, play\n");
        return 1;
    }

    int ret = 1;

    if (!strcmp(argv[1], "list"))
        ret = cmd_list(argc - 1, argv + 1);
    else if (!strcmp(argv[1], "play"))
        ret = cmd_play(argc - 1, argv + 1);
    else
        fprintf(stderr, "Unknown subcommand: %s\n", argv[1]);

    return ret;
}
