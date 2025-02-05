#include "Game.h"
#include "Globals.h"
#include "ResourceManager.h"
#include <stdio.h>

Game::Game()
{
    state = GameState::INTRO_UPC;
    scene = nullptr;
    img_menu = nullptr;
    img_menu_play = nullptr;
    img_menu_empty = nullptr;
    img_intro_upc = nullptr;
    img_intro_background = nullptr;
    img_game_win = nullptr;

    img_introduction = nullptr;
    img_intro_bat = nullptr;
    img_intro_cloud = nullptr;
    img_intro_simon = nullptr;
    img_intro_gui = nullptr;

    timerWin = 1800;
    timerLose = 300;
    timerPlay = 120;
    timerIntroduction = 360;

    panAnimation = 200;

    playerAnim = 240;

    target = {};
    src = {};
    dst = {};
}
Game::~Game()
{
    if (scene != nullptr)
    {
        scene->Release();
        delete scene;
        scene = nullptr;
    }
}
AppStatus Game::Initialise(float scale)
{
    float w, h;
    w = WINDOW_WIDTH * scale;
    h = WINDOW_HEIGHT * scale;

    SetConfigFlags(FLAG_FULLSCREEN_MODE);

    //Initialise window
    InitWindow((int)w, (int)h, "Vampire killer");

    InitAudioDevice();

    //Render texture initialisation, used to hold the rendering result so we can easily resize it
    target = LoadRenderTexture(WINDOW_WIDTH, WINDOW_HEIGHT);
    if (target.id == 0)
    {
        LOG("Failed to create render texture");
        return AppStatus::ERROR;
    }
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);
    src = { 0, 0, WINDOW_WIDTH, -WINDOW_HEIGHT };
    dst = { 0, 0, w, h };

    //Load resources
    if (LoadResources() != AppStatus::OK)
    {
        LOG("Failed to load resources");
        return AppStatus::ERROR;
    }

    //Set the target frame rate for the application
    SetTargetFPS(60);
    SetMasterVolume(0.6);

    //Disable the escape key to quit functionality
    SetExitKey(0);

    AudioPlayer::Instance().CreateMusic("audio/Music/15 Unused.ogg", "Unused");
    AudioPlayer::Instance().SetMusicLoopStatus("Unused", false);
    AudioPlayer::Instance().CreateMusic("audio/Music/10 Game Over.ogg", "GameOver");
    AudioPlayer::Instance().SetMusicLoopStatus("GameOver", false);
    AudioPlayer::Instance().CreateMusic("audio/Music/01 Prologue.ogg", "Prologue");
    AudioPlayer::Instance().SetMusicLoopStatus("Prologue", false);


    return AppStatus::OK;
}
AppStatus Game::LoadResources()
{
    ResourceManager& data = ResourceManager::Instance();
    
    if (data.LoadTexture(Resource::IMG_MENU, "images/Spritesheets/Introduction/Intro Space.png") != AppStatus::OK)
    {
        return AppStatus::ERROR;
    }
    img_menu = data.GetTexture(Resource::IMG_MENU);
    if (data.LoadTexture(Resource::IMG_MENU_PLAY, "images/Spritesheets/Introduction/IntroPlay.png") != AppStatus::OK)
    {
        return AppStatus::ERROR;
    }
    img_menu_play = data.GetTexture(Resource::IMG_MENU_PLAY);
    if (data.LoadTexture(Resource::IMG_MENU_EMPTY, "images/Spritesheets/Introduction/Intro Empty.png") != AppStatus::OK)
    {
        return AppStatus::ERROR;
    }
    img_menu_empty = data.GetTexture(Resource::IMG_MENU_EMPTY);
    if (data.LoadTexture(Resource::IMG_INTRO_UPC, "images/Spritesheets/Introduction/IntroUPC.png") != AppStatus::OK)
    {
        return AppStatus::ERROR;
    }
    img_intro_upc = data.GetTexture(Resource::IMG_INTRO_UPC);
    if (data.LoadTexture(Resource::IMG_INTRO_BACKGROUND, "images/Spritesheets/Introduction/IntroBackground.png") != AppStatus::OK)
    {
        return AppStatus::ERROR;
    }
    img_intro_background = data.GetTexture(Resource::IMG_INTRO_BACKGROUND);
    if (data.LoadTexture(Resource::IMG_GAME_WIN, "images/Spritesheets/Introduction/GameWin.png") != AppStatus::OK)
    {
        return AppStatus::ERROR;
    }
    img_game_win = data.GetTexture(Resource::IMG_GAME_WIN);


    if (data.LoadTexture(Resource::IMG_INTRODUCTION, "images/Spritesheets/Introduction/IntroOutside.png") != AppStatus::OK)
    {
        return AppStatus::ERROR;
    }
    img_introduction = data.GetTexture(Resource::IMG_INTRODUCTION);
    if (data.LoadTexture(Resource::IMG_INTRO_BAT, "images/Spritesheets/Introduction/IntroBat.png") != AppStatus::OK)
    {
        return AppStatus::ERROR;
    }
    img_intro_bat = data.GetTexture(Resource::IMG_INTRO_BAT);
    if (data.LoadTexture(Resource::IMG_INTRO_CLOUD, "images/Spritesheets/Introduction/IntroCloud.png") != AppStatus::OK)
    {
        return AppStatus::ERROR;
    }
    img_intro_cloud = data.GetTexture(Resource::IMG_INTRO_CLOUD);
    if (data.LoadTexture(Resource::IMG_PLAYER, "images/Spritesheets/Simon/Simon Spritesheet.png") != AppStatus::OK)
    {
        return AppStatus::ERROR;
    }
    img_intro_simon = data.GetTexture(Resource::IMG_PLAYER);
    if (data.LoadTexture(Resource::IMG_HUD_INTRO, "images/Spritesheets/Introduction/IntroHud.png") != AppStatus::OK)
    {
        return AppStatus::ERROR;
    }
    img_intro_gui = data.GetTexture(Resource::IMG_HUD_INTRO);
    
    return AppStatus::OK;
}
AppStatus Game::BeginPlay()
{
    scene = new Scene();
    if (scene == nullptr)
    {
        LOG("Failed to allocate memory for Scene");
        return AppStatus::ERROR;
    }
    if (scene->Init() != AppStatus::OK)
    {
        LOG("Failed to initialise Scene");
        return AppStatus::ERROR;
    }

    return AppStatus::OK;
}
void Game::FinishPlay()
{
    scene->Release();
    delete scene;
    scene = nullptr;
}
AppStatus Game::Update()
{
    //Check if user attempts to close the window, either by clicking the close button or by pressing Alt+F4
    if(WindowShouldClose()) return AppStatus::QUIT;

    AudioPlayer::Instance().Update();

    if (IsKeyPressed(KEY_F)) {
        ToggleFullscreen();  // modifies window size when scaling!
    }

    switch (state)
    {
        case GameState::INTRO_UPC:
            if (IsKeyPressed(KEY_ESCAPE)) return AppStatus::QUIT;
            if (IsKeyPressed(KEY_SPACE))
            {
                state = GameState::TRANSITION_1;
            }
            if (IsKeyPressed(KEY_ZERO))
            {
                if (BeginPlay() != AppStatus::OK) return AppStatus::ERROR;
                state = GameState::PLAYING;
            }
            break;
        case GameState::TRANSITION_1:
            if (IsKeyPressed(KEY_ESCAPE)) return AppStatus::QUIT;
            WaitTime(0.5);
            state = GameState::INTRO_BACKGROUND;
            break;
        case GameState::INTRO_BACKGROUND:
            if (IsKeyPressed(KEY_ESCAPE)) return AppStatus::QUIT;
            WaitTime(2);
            state = GameState::TRANSITION_2;
            break;
        case GameState::TRANSITION_2:
            if (IsKeyPressed(KEY_ESCAPE)) return AppStatus::QUIT;
            WaitTime(0.5);
            state = GameState::MAIN_MENU;
            break;
        case GameState::MAIN_MENU: 
            if (IsKeyPressed(KEY_ESCAPE)) return AppStatus::QUIT;
            if (IsKeyPressed(KEY_SPACE))
            {
                    state = GameState::MENU_PLAY;
            }
            break;
        case GameState::MENU_PLAY:
            if (IsKeyPressed(KEY_ESCAPE)) return AppStatus::QUIT;
            AudioPlayer::Instance().PlayMusicByName("Prologue");
            timerPlay--;
            if (timerPlay == 0) {
                timerPlay = 120;
                state = GameState::INTRODUCTION;
            }
            break;
        case GameState::INTRODUCTION:
            if (IsKeyPressed(KEY_ESCAPE)) return AppStatus::QUIT;
            timerIntroduction--;
            if (timerIntroduction == 0) {
                AudioPlayer::Instance().StopMusicByName("Prologue");
                if (BeginPlay() != AppStatus::OK) return AppStatus::ERROR;
                state = GameState::PLAYING;
                timerIntroduction = 300;
                panAnimation = 200;
                playerAnim = 240;
            }
            break;
        case GameState::GAME_WIN:
            if (IsKeyPressed(KEY_ESCAPE)) return AppStatus::QUIT;
            if (AudioPlayer::Instance().IsMusicPlaying("Prologue")) AudioPlayer::Instance().StopMusicByName("Prologue");
            AudioPlayer::Instance().PlayMusicByName("Unused");
            timerWin--;
            if (timerWin == 0) {
                AudioPlayer::Instance().StopMusicByName("Unused");
                state = GameState::MAIN_MENU;
                panAnimation = 200;
                timerWin = 1800;
            }
            break;

        case GameState::PLAYING:  
            if (IsKeyPressed(KEY_ESCAPE))
            {
                FinishPlay();
                AudioPlayer::Instance().StopMusicByName("VampireKiller");
                state = GameState::MAIN_MENU;
            }
            else if (scene->PlayerIsDead())
            {
                AudioPlayer::Instance().StopMusicByName("VampireKiller");
                AudioPlayer::Instance().PlayMusicByName("GameOver");
                timerLose--;
                if (timerLose == 0) {
                    FinishPlay();
                    AudioPlayer::Instance().StopMusicByName("GameOver");
                    state = GameState::MAIN_MENU;
                    timerLose = 300;
                }
            }
            else if (scene->PlayerHasWon())
            {
                FinishPlay();
                WaitTime(1);
                state = GameState::GAME_WIN;
            }
            else
            {
                //Game logic
                scene->Update();
            }
            break;
    }
    return AppStatus::OK;
}
void Game::Render()
{
    //Draw everything in the render texture, note this will not be rendered on screen, yet
    BeginTextureMode(target);
    ClearBackground({6,6,6,255});
    
    switch (state)
    {
        case GameState::MAIN_MENU:
            DrawTexture(*img_menu, 0, 0, WHITE);
            break;

        case GameState::INTRO_UPC:
            DrawTexture(*img_intro_upc, 0, 0, WHITE);
            break;

        case GameState::INTRO_BACKGROUND:
            DrawTexture(*img_intro_background, 0, 0, WHITE);
            
            break;

        case GameState::MENU_PLAY:
            DrawTexture(*img_menu_play, 0, 0, WHITE);
            if (timerPlay % 6 == 0 || timerPlay % 6 == 1 || timerPlay % 6 == 2) {
                DrawTexture(*img_menu_empty, 0, 0, WHITE);
            }
            break;

        case GameState::INTRODUCTION:
            DrawTexture(*img_introduction, 0, 0, WHITE);
            if (timerIntroduction % 12 == 0) {
                panAnimation--;
            }
            DrawTexture(*img_intro_cloud, panAnimation, 72, WHITE);

            if (timerIntroduction % 6 == 0 || timerIntroduction % 6 == 1 || timerIntroduction % 6 == 2) {
                DrawTextureRec(*img_intro_bat, { 0,0,16,16 }, { (float)panAnimation - 80, 60 }, WHITE);
            }
            else {
                DrawTextureRec(*img_intro_bat, { 16,16,16,16 }, { (float)panAnimation - 80, 60 }, WHITE);
            }
            
            if (timerIntroduction % 6 == 0 || timerIntroduction % 6 == 1 || timerIntroduction % 6 == 2) {
                DrawTextureRec(*img_intro_bat, { 0,0,16,16 }, { -(float)panAnimation + 250, (float)panAnimation - 92}, WHITE);
            }
            else {
                DrawTextureRec(*img_intro_bat, { 16,16,16,16 }, { -(float)panAnimation + 250, (float)panAnimation - 92 }, WHITE);
            }

            if (timerIntroduction % 2 == 0) {
                playerAnim--;
            }

            if (timerIntroduction < 121) {
                DrawTextureRec(*img_intro_simon, { 0,32 * 7,-32,32 }, { 124, 175 }, WHITE);
            }
            else {
                if (timerIntroduction % 30 < 8) {
                    DrawTextureRec(*img_intro_simon, { 0,0,-32,32 }, { (float)playerAnim, 175 }, WHITE);
                }
                else if (timerIntroduction % 30 < 16) {
                    DrawTextureRec(*img_intro_simon, { 32*1,0,-32,32 }, { (float)playerAnim, 175 }, WHITE);
                }
                else if (timerIntroduction % 30 < 23) {
                    DrawTextureRec(*img_intro_simon, { 32*2,0,-32,32 }, { (float)playerAnim, 175 }, WHITE);
                }
                else if (timerIntroduction % 30 < 30) {
                    DrawTextureRec(*img_intro_simon, { 32*3,0,-32,32 }, { (float)playerAnim, 175 }, WHITE);
                }
            }

            DrawTexture(*img_intro_gui, 0, 0, WHITE);


            break;

        case GameState::GAME_WIN:
            if (timerWin % 4 == 0) {
                panAnimation--;
            }
            DrawTexture(*img_game_win, 0, panAnimation, WHITE);
            break;

        case GameState::PLAYING:
            scene->Render();
            break;
    }
    
    EndTextureMode();

    //Draw render texture to screen, properly scaled
    BeginDrawing();
    DrawTexturePro(target.texture, src, dst, { 0, 0 }, 0.0f, WHITE);
    EndDrawing();
}
void Game::Cleanup()
{
    UnloadResources();
    CloseAudioDevice();
    CloseWindow();
}
void Game::UnloadResources()
{
    ResourceManager& data = ResourceManager::Instance();
    data.ReleaseTexture(Resource::IMG_MENU);
    data.ReleaseTexture(Resource::IMG_MENU_PLAY);
    data.ReleaseTexture(Resource::IMG_MENU_EMPTY);
    data.ReleaseTexture(Resource::IMG_INTRO_UPC);
    data.ReleaseTexture(Resource::IMG_INTRO_BACKGROUND);
    data.ReleaseTexture(Resource::IMG_GAME_WIN);

    data.ReleaseTexture(Resource::IMG_INTRODUCTION);
    data.ReleaseTexture(Resource::IMG_INTRO_BAT);
    data.ReleaseTexture(Resource::IMG_INTRO_CLOUD);
    data.ReleaseTexture(Resource::IMG_HUD_INTRO);
    data.ReleaseTexture(Resource::IMG_PLAYER);

    UnloadRenderTexture(target);
}