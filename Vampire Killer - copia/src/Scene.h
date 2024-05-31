#pragma once
#include <raylib.h>
#include "Player.h"
#include "TileMap.h"
#include "Object.h"
#include "AudioPlayer.h"
#include "Text.h"

enum class DebugMode { OFF, SPRITES_AND_HITBOXES, ONLY_HITBOXES, SIZE };

class Scene
{
public:
    Scene();
    ~Scene();

    AppStatus Init();
    void Update();
    void Render();
    void Release();
    bool PlayerIsDead() const;
    bool PlayerHasWon() const;

private:
    AppStatus LoadLevel(int stage,int floor);
    
    void CheckCollisions(); 
    void ClearLevel();
    void RenderGameOver() const;
    void RenderObjects() const;
    void RenderObjectsDebug(const Color& col) const;

    void RenderGUI() const;

    Player *player;
    TileMap *level;
    std::vector<Object*> objects;
    int currentLevel;
    int currentFloor;
    const Texture2D* game_over;
    const Texture2D* hud;
    const Texture2D* chest_animation;
    const Texture2D* hud_items;
    int chest_time;
    int enemy_delay_time;

    bool gotBoots = false;
    int boot_time = 60;
    bool gotWings = false;
    int wings_time = 60;
    bool gotShield = false;
    int shield_time = 60;
    bool gotHeart = false;
    int heart_time = 60;

    bool deathExecuted;
    bool renderingGameOver;

    //broken walls booleans
    bool level6WallBroken;
    bool level7_1WallBroken;
    
    bool chestOpening;
    float currentChestX;
    float currentChestY;
    ObjectType currentChestType;

    bool lootOpening;
    float currentLootX;
    float currentLootY;
    ObjectType currentLootType;

    Camera2D camera;
    DebugMode debug;

    Text* font;
};

