#include "Scene.h"
#include <stdio.h>
#include "Globals.h"
#include "EnemyManager.h"

Scene::Scene()
{
	player = nullptr;
    level = nullptr;
	game_over = nullptr;
	hud = nullptr;
	chest_animation = nullptr;

	currentLevel = 0;
	currentFloor = 0;
	camera.target = { 0, 0 };				//Center of the screen
	camera.offset = { SIDE_MARGINS, TOP_MARGIN };	//Offset from the target (center of the screen)
	camera.rotation = 0.0f;					//No rotation
	camera.zoom = 1.0f;						//Default zoom

	deathExecuted = false;
	renderingGameOver = false;

	level6WallBroken = false;
	level7_1WallBroken = false;

	font = nullptr;

	debug = DebugMode::OFF;

	chest_time = 60;
	loot_time = 1;
	zombie_delay_time = 120;
	bat_delay_time = 120*2;

	bossDoor = new Door({ 15 * TILE_SIZE+8, 2 * TILE_SIZE + 48 });
}
Scene::~Scene()
{
	if (player != nullptr)
	{
		player->Release();
		delete player;
		player = nullptr;
	}
	if (bossDoor != nullptr)
	{
		bossDoor->Release();
		delete bossDoor;
		bossDoor = nullptr;
	}
    if (level != nullptr)
    {
		level->Release();
        delete level;
        level = nullptr;
    }
	for (Entity* obj : objects)
	{
		delete obj;
	}
	if (font != nullptr)
	{
		delete font;
		font = nullptr;
	}
}
AppStatus Scene::Init()
{
	ResourceManager& data = ResourceManager::Instance();

	//Create player
	player = new Player({ 20,144 }, State::IDLE, Look::RIGHT);
	EnemyManager::Instance().target = player;
	if (player == nullptr)
	{
		LOG("Failed to allocate memory for Player");
		return AppStatus::ERROR;
	}
	//Initialise player
	if (player->Initialise() != AppStatus::OK)
	{
		LOG("Failed to initialise Player");
		return AppStatus::ERROR;
	}

	//Create level 
    level = new TileMap();
    if (level == nullptr)
    {
        LOG("Failed to allocate memory for Level");
        return AppStatus::ERROR;
    }
	//Initialise level
	if (level->Initialise() != AppStatus::OK)
	{
		LOG("Failed to initialise Level");
		return AppStatus::ERROR;
	}
	//Load level
	if (LoadLevel(1,currentFloor) != AppStatus::OK)
	{
		LOG("Failed to load Level 1");
		return AppStatus::ERROR;
	}
	//Assign the tile map reference to the player to check collisions while navigating
	player->SetTileMap(level);
	EnemyManager::Instance().SetTilemap(level);

	//Add the Game Over image for the end
	if (data.LoadTexture(Resource::IMG_GAME_OVER, "images/Spritesheets/HUD Spritesheet/GameOver.png") != AppStatus::OK)
	{
		return AppStatus::ERROR;
	}
	game_over = data.GetTexture(Resource::IMG_GAME_OVER);

	//Add the Hud image for the top
	if (data.LoadTexture(Resource::IMG_HUD, "images/Spritesheets/HUD Spritesheet/EmptyHUD.png") != AppStatus::OK)
	{
		return AppStatus::ERROR;
	}
	hud = data.GetTexture(Resource::IMG_HUD);
	//Hud items
	if (data.LoadTexture(Resource::IMG_HUD_ITEMS, "images/Spritesheets/HUD Spritesheet/Items.png") != AppStatus::OK)
	{
		return AppStatus::ERROR;
	}
	hud_items = data.GetTexture(Resource::IMG_HUD_ITEMS);

	//Add the chest animation
	if (data.LoadTexture(Resource::IMG_OPEN_CHEST, "images/Spritesheets/FX/OpenChest.png") != AppStatus::OK)
	{
		return AppStatus::ERROR;
	}
	chest_animation = data.GetTexture(Resource::IMG_OPEN_CHEST);

	//Add the loot animation
	if (data.LoadTexture(Resource::IMG_TILES, "images/Levels/LevelsTileset.png") != AppStatus::OK)
	{
		return AppStatus::ERROR;
	}
	loot_heart = data.GetTexture(Resource::IMG_TILES);

	//Add the trader pop up
	if (data.LoadTexture(Resource::IMG_POPUP_TRADER, "images/Spritesheets/Enemies & Characters/PopUp.png") != AppStatus::OK)
	{
		return AppStatus::ERROR;
	}
	popup_trader = data.GetTexture(Resource::IMG_POPUP_TRADER);

	//Add the hit effect image
	if (data.LoadTexture(Resource::IMG_HIT_EFFECT, "images/Spritesheets/FX/HitFx.png") != AppStatus::OK)
	{
		return AppStatus::ERROR;
	}
	hit_effect = data.GetTexture(Resource::IMG_HIT_EFFECT);

	AudioPlayer::Instance().CreateMusic("audio/Music/02 Vampire Killer.ogg", "VampireKiller");
	AudioPlayer::Instance().SetMusicLoopStatus("VampireKiller",true);

	AudioPlayer::Instance().CreateMusic("audio/Music/08 Poison Mind.ogg", "BossMusic");
	AudioPlayer::Instance().SetMusicLoopStatus("BossMusic", true);

	AudioPlayer::Instance().CreateSound("audio/SFX/17.wav", "Collect");
	AudioPlayer::Instance().CreateSound("audio/SFX/25.wav", "OpenChest");
	AudioPlayer::Instance().CreateSound("audio/SFX/02.wav", "BreakWalls");
	AudioPlayer::Instance().CreateSound("audio/SFX/03.wav", "EnterCastle");
	AudioPlayer::Instance().CreateSound("audio/SFX/01.wav", "GetDoorKey");
	AudioPlayer::Instance().CreateSound("audio/SFX/12.wav", "GetHeart");
	AudioPlayer::Instance().CreateSound("audio/SFX/05.wav", "OpenDoor");

	player->weapon->SetWeapon(WeaponType::WHIP);

	//Create text font 1
	font = new Text();
	if (font == nullptr)
	{
		LOG("Failed to allocate memory for font 1");
		return AppStatus::ERROR;
	}
	//Initialise text font 1
	if (font->Initialise(Resource::IMG_FONT, "images/Spritesheets/HUD Spritesheet/Font.png", ' ', 8) != AppStatus::OK)
	{
		LOG("Failed to initialise Level");
		return AppStatus::ERROR;
	}

	Object* obj;

	// Level 1 Objects
	obj = new Object({ 5 * TILE_SIZE, 8 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::FIRE, { 1,0 }, ObjectType::HEART_SMALL, TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 13 * TILE_SIZE, 8 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::FIRE, { 1,0 }, ObjectType::HEART_SMALL, TILE_SIZE);
	objects.push_back(obj);
	
	//Level 2 Objects
	obj = new Object({ 5 * TILE_SIZE, 8 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::FIRE, { 2,0 }, ObjectType::HEART_BIG, TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 13 * TILE_SIZE, 8 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::FIRE, { 2,0 }, ObjectType::HEART_BIG, TILE_SIZE);
	objects.push_back(obj);
	
	//Level 3 Objects
	obj = new Object({ 5 * TILE_SIZE, 8 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::FIRE, { 3,0 }, ObjectType::CHAIN, TILE_SIZE);
	objects.push_back(obj);
	
	//Level 4 Objects
	obj = new Object({ 8 * TILE_SIZE, 8 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 4,0 }, ObjectType::HEART_SMALL, 2 * TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 12 * TILE_SIZE, 8 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 4,0 }, ObjectType::HEART_SMALL, 2 * TILE_SIZE);
	objects.push_back(obj);
	
	//Level 5 Objects
	obj = new Object({ 4 * TILE_SIZE, 8 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 5,0 }, ObjectType::HEART_SMALL, 2 * TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 8 * TILE_SIZE, 8 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 5,0 }, ObjectType::HEART_SMALL, 2 * TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 12 * TILE_SIZE, 7 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 5,0 }, ObjectType::HEART_BIG, 3 * TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 10 * TILE_SIZE, 3 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 5,0 }, ObjectType::HEART_BIG, 3 * TILE_SIZE);
	objects.push_back(obj);
	
	//Level 6 Objects
	obj = new Object({ 7 * TILE_SIZE, 3 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 6,0 }, ObjectType::HEART_SMALL, 3 * TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 12 * TILE_SIZE, 3 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 6,0 }, ObjectType::HEART_SMALL, 3 * TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 14 * TILE_SIZE, 8 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 6,0 }, ObjectType::HEART_BIG, 2 * TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 5 * TILE_SIZE, 10 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::KEY_CHEST, { 6,0 });
	objects.push_back(obj);
	obj = new Object({ 12 * TILE_SIZE, 10 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CHEST_HEART, { 6,0 });
	objects.push_back(obj);
	
	//Level 7 Objects
	obj = new Object({ 12 * TILE_SIZE, 4 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 7,0 }, ObjectType::HEART_BIG, 2 * TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 5 * TILE_SIZE, 5 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 7,0 }, ObjectType::HEART_BIG, TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 3 * TILE_SIZE, 9 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 7,0 }, ObjectType::HEART_SMALL, TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 9 * TILE_SIZE, 9 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 7,0 }, ObjectType::HEART_SMALL, TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 4 * TILE_SIZE, 6 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CHEST_WINGS, { 7,0 });
	objects.push_back(obj);

	//Level 4, floor 1 Objects
	obj = new Object({ 4 * TILE_SIZE, 1 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 4,1 }, ObjectType::HEART_BIG, TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 4 * TILE_SIZE, 5 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 4,1 }, ObjectType::HEART_SMALL, TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 12 * TILE_SIZE, 3 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 4,1 }, ObjectType::HEART_BIG, TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 11 * TILE_SIZE, 8 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 4,1 }, ObjectType::HEART_SMALL, 2 * TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 7 * TILE_SIZE, 8 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 4,1 }, ObjectType::HEART_SMALL, 2 * TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 3 * TILE_SIZE, 2 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CHEST_BOOTS, { 4,1 });
	objects.push_back(obj);
	obj = new Object({ 3 * TILE_SIZE, 6 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CHEST_SHIELD, { 4,1 });
	objects.push_back(obj);
	obj = new Object({ 3 * TILE_SIZE, 10 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::KEY_CHEST, { 4,1 });
	objects.push_back(obj);

	//Level 5, floor 1 Objects
	obj = new Object({ 10 * TILE_SIZE, 7 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 5,1 }, ObjectType::HEART_SMALL, 3 * TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 5 * TILE_SIZE, 8 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 5,1 }, ObjectType::HEART_SMALL, 2 * TILE_SIZE);
	objects.push_back(obj);

	//Level 6, floor 1 Objects
	obj = new Object({ 8 * TILE_SIZE, 7 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 6,1 }, ObjectType::HEART_SMALL, 3 * TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 14 * TILE_SIZE, 4 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 6,1 }, ObjectType::HEART_SMALL, 2 * TILE_SIZE);
	objects.push_back(obj);

	//Level 7, floor 1 Objects
	obj = new Object({ 7 * TILE_SIZE, 9 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 7,1 }, ObjectType::HEART_SMALL, TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 11 * TILE_SIZE, 9 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 7,1 }, ObjectType::HEART_SMALL, TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 9 * TILE_SIZE, 6 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::KEY_CHEST, { 7,1 });
	objects.push_back(obj);

	//Level 8, floor 1 Objects
	obj = new Object({ 8 * TILE_SIZE, 7 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 8,1 }, ObjectType::HEART_BIG, 3 * TILE_SIZE);
	objects.push_back(obj);
	obj = new Object({ 11 * TILE_SIZE, 7 * TILE_SIZE + TILE_SIZE - 1 }, ObjectType::CANDLE, { 8,1 }, ObjectType::HEART_BIG, TILE_SIZE);
	objects.push_back(obj);

    return AppStatus::OK;
}
AppStatus Scene::LoadLevel(int stage,int floor)
{
	int size;
	int x, y, i;
	Tile tile;
	Point pos;
	int *mapBack = nullptr;
	int *map = nullptr;
	int *mapFront = nullptr;
	//Object *obj = nullptr;
	Object* obj;
	

	size = LEVEL_WIDTH * LEVEL_HEIGHT;
	if (stage == 1 && floor == 0)
	{
		currentLevel = 1;
		currentFloor = 0;
		mapBack = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  1,  2,  3,  4,  1,  2,  3,  4,  1,  2,  3,  4,  1,  2,  3,  4,  0,
			  0,  5,  6,  7,  8,  5,  6,  7,  8,  5,  6,  7,  8,  5,  6,  7,  8,  0,
			  0,  9, 10, 11, 12,  9, 10, 11, 12,  9, 10, 11, 12,  9, 10, 11, 12,  0,
			  0, 13, 14, 15, 16, 13, 14, 15, 16, 13, 14, 15, 16, 13, 14, 15, 16,  0,
			  0, 17, 18, 19, 20, 17, 18, 19, 20, 17, 18, 19, 20, 17, 18, 19, 20,  0,
			  0, 21, 22, 23, 24, 25, 26, 27, 28, 21, 26, 27, 28, 25, 26, 27, 28,  0,
			  0, 29, 30, 41, 42, 31, 32, 29, 32, 29, 32, 29, 33, 34, 32, 29, 32,  0,
			  0, 36, 35, 43, 44, 35, 35, 36, 35, 36, 35, 36, 35, 35, 35, 36, 35,  0,
			  0, 37, 37, 45, 46, 38, 37, 37, 37, 37, 37, 37, 37, 38, 37, 37, 37,  0,
			  0, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,  0,
			  0, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
			};
		map = new int[size] {
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			550, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,501,
			  0, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		mapFront = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};

		EnemyManager::Instance().DestroyEnemies();
		if (player->isGUIinit == false) {
			player->InitGUI();
		}
		
	}
	else if (stage == 2 && floor == 0)
	{
		currentLevel = 2;
		currentFloor = 0;
		mapBack = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  1,  2,  3,  4,  1,  2,  3,  4,  1,  2,  3,  4,  1,  2,  3,  4,  0,
			  0,  5,  6,  7,  8,  5,  6,  7,  8,  5,  6,  7,  8,  5,  6,  7,  8,  0,
			  0,  9, 10, 11, 12,  9, 10, 11, 12,  9, 10, 11, 12,  9, 10, 11, 12,  0,
			  0, 13, 14, 15, 16, 13, 14, 15, 16, 13, 14, 15, 16, 13, 14, 15, 16,  0,
			  0, 17, 18, 19, 20, 17, 18, 19, 20, 17, 18, 19, 20, 17, 18, 19, 20,  0,
			  0, 21, 22, 23, 24, 25, 26, 27, 28, 21, 26, 27, 28, 25, 26, 27, 28,  0,
			  0, 29, 30, 41, 42, 31, 32, 29, 32, 29, 32, 29, 33, 34, 32, 29, 32,  0,
			  0, 36, 35, 43, 44, 35, 35, 36, 35, 36, 35, 36, 35, 35, 35, 36, 35,  0,
			  0, 37, 37, 45, 46, 38, 37, 37, 37, 37, 37, 37, 37, 38, 37, 37, 37,  0,
			  0, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,  0,
			  0, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		map = new int[size] {
			 500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			 500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			 500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			 500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			 500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			 500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			 500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			 500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			 500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			 500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			 500, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,501,
			   0, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,  0,
			   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		mapFront = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		EnemyManager::Instance().DestroyEnemies();
	}
	else if (stage == 3 && floor == 0)
	{
		currentLevel = 3;
		currentFloor = 0;
		mapBack = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  1,  2,  3,  4,  1,  2,  3,  4,  1,  2, 47, 48, 49, 48, 49, 48,  0,
			  0,  5,  6,  7,  8,  5,  6,  7,  8,  5,  6, 50, 51, 52, 52, 53, 54,  0,
			  0,  9, 10, 11, 12,  9, 10, 11, 12,  9, 10, 55, 56, 57, 58, 53, 59,  0,
			  0, 13, 14, 15, 16, 13, 14, 15, 16, 13, 14, 50, 60, 61, 40, 62, 63,  0,
			  0, 17, 18, 19, 20, 17, 18, 19, 20, 17, 18, 55, 65, 66, 40, 67, 68,  0,
			  0, 21, 22, 23, 24, 25, 26, 27, 28, 21, 26, 50, 60, 61, 40, 62, 63,  0,
			  0, 29, 30, 41, 42, 31, 32, 29, 32, 29, 32, 55, 65, 66, 40, 67, 68,  0,
			  0, 36, 35, 43, 44, 35, 35, 36, 35, 36, 35, 69, 70, 71, 40, 72, 73,  0,
			  0, 37, 37, 45, 46, 38, 37, 37, 37, 37, 37, 69, 70, 71, 40, 72, 73,  0,
			  0, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,  0,
			  0, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		map = new int[size] {
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,  0,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,  0,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,  0,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,  0,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,  0,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,  0,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,  0,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,  0,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,  0,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,  0,
			500, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,  0,
			  0, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		mapFront = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 67, 68,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 67, 68,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 72, 73,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 72, 73,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		EnemyManager::Instance().DestroyEnemies();
	}
	else if (stage == 4 && floor == 0)
	{
		currentLevel = 4;
		currentFloor = 0;
		mapBack = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0, 74, 75, 82, 83,110,110, 97, 98, 74, 75, 86, 87,110,110,101,102,  0,
			  0, 76, 77, 84, 85,110,110, 99,100, 76, 77, 91,110,110,110,110,110,  0,
			  0, 76, 77, 86, 87,110,110,101,102, 76, 77, 92,110,110,110,110,110,  0,
			  0, 76, 77, 88,110,103,103,110,110, 76, 77, 95, 96,103,103,110,110,  0,
			  0, 76, 77, 89,110,103,103,110,110, 76, 77, 97, 98,103,103,110,110,  0,
			  0, 76, 77, 90,110,103,103,110,110, 76, 77, 99,100,103,103,110,110,  0,
			  0, 76, 77, 90,110,103,103,110,110, 76, 77,101,102,103,103,110,110,  0,
			  0, 76, 77, 91,110,103,103,110,110, 76, 77,110,110,103,103,110,110,  0,
			  0, 76, 77, 92,110,103,103,110,110, 76, 77,110,110,103,103,110,110,  0,
			  0, 80, 81,110,110,110,110,110,110, 80, 81,110,110,110,110,110,110,  0,
			  0,108,109,108,109,108,109,108,109,108,109,108,109,108,109,108,109,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		map = new int[size] {
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			  0,108,109,108,109,108,109,108,109,108,109,108,109,108,109,108,109,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		mapFront = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		EnemyManager::Instance().DestroyEnemies();
	}
	else if (stage == 5 && floor == 0)
	{
		currentLevel = 5;
		currentFloor = 0;
		mapBack = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0, 74, 75, 82, 83,110,110, 97, 98,110, 93,  0,  0,110,110,101,102,  0,
			  0, 76, 77, 84, 85,103,103,103,103,110,110,110,  0,  0,110,110,110,  0,
			  0, 76, 77, 86, 87,103,103,103,103,110,110,110, 93,  0,  0,110,110,  0,
			  0, 76, 77, 88,110,103,103,103,103,110,110,110,110,110,  0,  0,110,  0,
			  0, 76, 77, 89,110,103,103,103,103,110,110,  0,  0,  0,  0,  0,  0,  0,
			  0, 76, 77, 90,110, 99,100,110,110,110,  0,  0,110, 74, 75, 95, 96,  0,
			  0, 76, 77, 90,110,101,102,  0,  0,  0,  0, 94,110, 76, 77, 97, 98,  0,
			  0, 76, 77, 91,110,110,  0,  0,110,110,110,110,110, 76, 77, 99,100,  0,
			  0, 78, 77, 92,110,  0,  0, 94,110,110,110,110,110, 76, 77,101,102,  0,
			  0, 80, 81,110,  0,  0,110,110,110,110,110,110,110, 76, 77,110,110,  0,
			  0,108,109,108,109,108,109,108,109,108,109,108,109,108,109,108,109,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		map = new int[size] {
			500,  0,  0,  0,  0,  0,  0,  0,502,502,502,502,502,502,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,105,106,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,105,106,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,105,106,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,105,106,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,107,108,108,109,108,109,  0,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,107,104,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,107,108,108,109,550,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,107,104,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,107,104,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,107,104,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			  0,108,109,108,109,108,109,108,109,108,109,108,109,108,109,108,109,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		mapFront = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		EnemyManager::Instance().DestroyEnemies();
	}
	else if (stage == 6 && floor == 0)
	{
		currentLevel = 6;
		currentFloor = 0;
		mapBack = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0, 97, 98, 76, 77, 86, 87,110,110,110,110,110,110,101,102,110,110,  0,
			  0, 99,100, 76, 77, 91,110,110,110,103,103,110,110,110,110,110,110,  0,
			  0,101,102, 76, 77, 92,110,110,110,103,103,110,110,110,110,  0,  0,  0,
			  0,110,110, 80, 81,110,110,110,110,103,103,110,110,110,  0,  0,110,  0,
			  0,  0,  0,  0,  0,  0,  0,110,110,103,103,110,110,  0,  0, 94,110,  0,
			  0,110,110, 74, 75,110,  0,  0,110,110,110,110,  0,  0,110, 95, 96,  0,
			  0,110,110, 76, 77,  0,  0,  0,  0,  0,  0,  0,  0, 94,110, 97, 98,  0,
			  0,110,110, 76, 77, 99,100,  0,  0,  0,  0,110,110,110,110, 99,100,  0,
			  0,110,110, 76, 77,101,102,  0,  0,101,102,110,110,110,110,101,102,  0,
			  0,110,110, 76, 77,110,110,  0,  0,110,110,110,110,110,110,110,110,  0,
			  0,108,109,108,109,108,109,108,109,108,109,108,109,108,109,108,109,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		if (!level6WallBroken) {
			map = new int[size] {
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,107,108,  0,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,107,104,  0,501,
				  0,108,109,108,109,108,106,  0,  0,  0,  0,  0,  0,107,104,  0,  0,501,
				500,  0,  0,  0,  0,  0,105,106,  0,  0,  0,  0,107,104,  0,  0,  0,501,
				500,  0,  0,  0,  0,108,109,108,109,108,109,108,109,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,108,109,108,109,  0,  0,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,108,109,131,132,  0,  0,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,108,109,131,132,  0,  0,  0,  0,  0,  0,501,
				  0,108,109,108,109,108,109,108,109,108,109,108,109,108,109,108,109,  0,
				  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
			};
		}
		else if(level6WallBroken){
				map = new int[size] {
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,107,108,  0,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,107,104,  0,501,
				  0,108,109,108,109,108,106,  0,  0,  0,  0,  0,  0,107,104,  0,  0,501,
				500,  0,  0,  0,  0,  0,105,106,  0,  0,  0,  0,107,104,  0,  0,  0,501,
				500,  0,  0,  0,  0,108,109,108,109,108,109,108,109,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,108,109,108,109,  0,  0,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,108,109,  0,  0,  0,  0,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,108,109,  0,  0,  0,  0,  0,  0,  0,  0,501,
				  0,108,109,108,109,108,109,108,109,108,109,108,109,108,109,108,109,  0,
				  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
			};
		}
		mapFront = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		EnemyManager::Instance().DestroyEnemies();
	}
	else if (stage == 7 && floor == 0)
	{
		currentLevel = 7;
		currentFloor = 0;
		mapBack = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,101,102,110,110,  0,  0, 93,110,110,110, 97, 98, 74, 75, 82, 83,  0,
			  0,110,110,110,  0,  0,110,110,110,110,110, 99,100, 76, 77, 84, 85,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,110,110,101,102, 76, 77, 86, 87,  0,
			  0, 74, 75, 99,100,110,110,110,  0,  0,110,110,110, 76, 77, 88,110,  0,
			  0, 76, 77,101,102,110,110,110, 94,  0,  0,110,110, 76, 77, 89,110,  0,
			  0, 76, 77,110,110,110,110,110,110,110,  0,  0,110, 78, 77, 90,110,  0,
			  0, 76, 77,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 79, 77, 90,110,  0,
			  0, 76, 77,110,110,110,  0,  0,110,110,110, 99,100, 76, 77, 91,110,  0,
			  0, 78, 77,110,110,  0,  0, 94,110,110,110,101,102, 76, 77, 92,110,  0,
			  0, 80, 81,110,  0,  0,110,110,110,110,110,110,110, 80, 81,110,110,  0,
			  0,108,109,108,109,108,109,108,109,108,109,108,109,108,109,108,109,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		map = new int[size] {
			500,  0,  0,  0,502,502,502,502,502,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,107,104,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,107,104,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			  0,108,109,108,109,108,109,108,106,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,105,106,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,105,106,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,105,106,  0,  0,  0,  0,  0,501,
			500,  0,  0,108,109,108,109,107,108,108,109,108,109,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,107,104,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,107,104,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,107,104,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			  0,108,109,108,109,108,109,108,109,108,109,108,109,108,109,108,109,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		mapFront = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		EnemyManager::Instance().DestroyEnemies();
	}
	else if (stage == 4 && floor == 1)
	{
		currentLevel = 4;
		currentFloor = 1;
		mapBack = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,110,110, 74, 75,110,110,110,110,110,110,101,102, 74, 75,  0,
			  0,  0,  0,110,110, 76, 77,110,110,110,110,110,110, 95, 96, 76, 77,  0,
			  0,  0,  0,  0,  0, 76, 77,  0,  0,  0,  0,110,110, 97, 98, 76, 77,  0,
			  0,  0,  0,110,110, 76, 77, 99,100,110,  0,  0,110, 99,100, 76, 77,  0,
			  0,  0,  0,110,110, 76, 77,101,102,110, 93,  0,  0,101,102, 76, 77,  0,
			  0,  0,  0,110,110, 76, 77,110,110, 95, 96,110,  0,  0,110, 76, 77,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0, 97, 98,110, 93,  0,  0,  0,  0,  0,
			  0,  0,  0, 99,100, 74, 75,110,110, 99,100,110,110,110,110, 74, 75,  0,
			  0,  0,  0,101,102, 76, 77,110,110,101,102,110,110,110,110, 76, 77,  0,
			  0,  0,  0,110,110, 80, 81,110,110,110,110,110,110,110,110, 80, 81,  0,
			  0,108,109,108,109,108,109,108,109,108,109,108,109,108,109,108,109,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		map = new int[size] {
			  0,550,550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			  0,108,109,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			  0,108,109,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			  0,108,109,108,109,  0,  0,108,109,108,106,  0,  0,  0,  0,  0,  0,501,
			  0,108,109,  0,  0,  0,  0,  0,  0,  0,105,106,  0,  0,  0,  0,  0,501,
			  0,108,109,  0,  0,  0,  0,  0,  0,  0,  0,105,106,  0,  0,  0,  0,501,
			  0,108,109,  0,  0,  0,  0,  0,  0,  0,  0,  0,105,106,  0,  0,  0,501,
			  0,108,109,108,109,108,109,108,109,  0,  0,  0,  0,108,109,108,109,501,
			  0,108,109,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			  0,108,109,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			  0,108,109,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			  0,108,109,108,109,108,109,108,109,108,109,108,109,108,109,108,109,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		mapFront = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		EnemyManager::Instance().DestroyEnemies();
	}
	else if (stage == 5 && floor == 1)
	{
		currentLevel = 5;
		currentFloor = 1;
		mapBack = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,110,110, 74, 75, 86, 87,110,110, 97, 98,110,110, 74, 75, 82, 83,  0,
			  0, 95, 96, 76, 77, 88,110,103,103,103,103,110,110, 76, 77, 84, 85,  0,
			  0, 97, 98, 76, 77, 90,110,103,103,103,103,110,110, 76, 77, 86, 87,  0,
			  0, 99,100, 76, 77, 91,110,103,103,103,103,110,110, 76, 77, 88,110,  0,
			  0,101,102, 78, 77, 92,110,103,103,103,103,110,110, 76, 77, 89,110,  0,
			  0,110,110, 80, 81,110,110, 99,100,110,110, 95, 96, 76, 77, 91,110,  0,
			  0,  0,  0,  0,  0,  0,  0,101,102,110,110, 97, 98, 76, 77, 92,110,  0,
			  0,110,110, 99,100,110,  0,  0,110,110,110, 99,100, 78, 77,110,110,  0,
			  0,110,110,101,102,110, 93,  0,  0,110,110,101,102, 79, 77,110,110,  0,
			  0,110,110,110,110,110,110,110,  0,  0,110,110,110, 80, 81,110,110,  0,
			  0,108,109,108,109,108,109,108,109,108,109,108,  0,108,109,108,109,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		map = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,108,109,108,109,108,106,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,105,106,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,105,106,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,105,106,  0,  0,  0,  0,  0,  0,  0,501,
			500,108,109,108,109,108,109,108,109,108,109,108,106,108,109,108,109,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,503,503,503,  0,  0,  0,  0
		};
		mapFront = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,551,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		EnemyManager::Instance().DestroyEnemies();
	}
	else if (stage == 6 && floor == 1)
	{
		currentLevel = 6;
		currentFloor = 1;
		mapBack = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,110,110,101,102, 74, 75, 82, 83,110,110, 97, 98,110,110, 97, 98,  0,
			  0, 95, 96,110,110, 76, 77, 84, 85,110,110,103,103,110,110,103,103,  0,
			  0, 97, 98,110,110, 76, 77, 86, 87,110,110,103,103,110,110,103,103,  0,
			  0,103,103,110,110, 76, 77, 88,110, 95, 96,103,103,110,110,103,103,  0,
			  0,103,103,110,110, 76, 77, 89,110, 97, 98,103,103,110,110,103,103,  0,
			  0,103,103, 95, 96, 76, 77, 90,110, 99,100,110,110,110,110,110,110,  0,
			  0,103,103, 97, 98, 76, 77, 90,110,101,102,  0,  0,  0,  0,  0,  0,  0,
			  0,103,103, 99,100, 76, 77, 91,110,110,  0,  0,110, 99,100,110,110,  0,
			  0,103,103,101,102, 76, 77, 92,110,  0,  0, 94,110,101,102,110,110,  0,
			  0,110,110,110,110, 80, 81,110,  0,  0,110,110,110,110,110,110,110,  0,
			  0,108,109,108,109,108,109,108,109,108,109,108,109,108,109,108,109,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		map = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,107,109,108,109,108,109,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,  0,107,104,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,  0,107,104,  0,  0,  0,  0,  0,  0,501,
			500,  0,  0,  0,  0,  0,  0,  0,107,104,  0,  0,  0,  0,  0,  0,  0,501,
			500,108,109,108,109,108,109,108,109,108,109,108,109,108,109,108,109,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		mapFront = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		EnemyManager::Instance().DestroyEnemies();
	}
	else if (stage == 7 && floor == 1)
	{
		currentLevel = 7;
		currentFloor = 1;
		mapBack = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0, 74, 75, 82, 83,110,110, 97, 98,110,110,110,110, 97, 98,  0,  0,  0,
			  0, 76, 77, 84, 85, 95, 96,103,103,110,110,110,110, 99,100,  0,  0,  0,
			  0, 76, 77, 86, 87, 97, 98,103,103,110,110,110,110,101,102,  0,  0,  0,
			  0, 78, 77, 91,110, 99,100,103,103,110,110,110,110,110,110,  0,  0,  0,
			  0, 79, 77, 92,110,101,102,103,103,110,110,  0,  0,  0,  0,  0,  0,  0,
			  0, 76, 77,110,110,110,110,110,110,110,  0,  0,110, 95, 96,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 94,110, 97, 98,  0,  0,  0,
			  0, 74, 75, 99,100,110,110,110,110, 99,100,110,110, 99,100,  0,  0,  0,
			  0, 76, 77,101,102,110,110,110,110,101,102,110,110,101,102,  0,  0,  0,
			  0, 80, 81,110,110,110,110,110,110,110,110,110,110,110,110,  0,  0,  0,
			  0,108,109,108,109,  0,109,108,109,108,109,108,109,108,109,108,109,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		EnemyManager::Instance().DestroyEnemies();
		
		if (!level7_1WallBroken) {
			map = new int[size] {
				  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,108,109,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,107,109,108,109,108,109,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,107,104,  0,  0,  0,108,109,501,
				500,108,109,108,109,108,109,108,109,108,109,  0,  0,  0,  0,108,109,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,108,109,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,131,132,108,109,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,131,132,108,109,501,
				500,108,109,108,109,107,109,108,109,108,109,108,109,108,109,108,109,  0,
				  0,  0,  0,  0,503,503,503,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
			};
		}
		else if (level7_1WallBroken) {
			map = new int[size] {
				  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,108,109,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,107,109,108,109,108,109,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,107,104,  0,  0,  0,108,109,501,
				500,108,109,108,109,108,109,108,109,108,109,  0,  0,  0,  0,108,109,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,108,109,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,108,109,501,
				500,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,108,109,501,
				500,108,109,108,109,107,109,108,109,108,109,108,109,108,109,108,109,  0,
				  0,  0,  0,  0,503,503,503,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
			};
			
		}
		mapFront = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,108,109,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		EnemyManager::Instance().DestroyEnemies();
		
	}
	else if (stage == 8 && floor == 1)
	{
		currentLevel = 8;
		currentFloor = 1;
		mapBack = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,111,111, 74, 75,111,113,111,111, 74, 75,111,111,111,111, 74, 75,  0,
			  0,123,123, 78, 77,123,123,123,123, 76, 77,123,123,123,123, 76, 77,  0,
			  0,123,123, 79, 77,123,123,123,123, 76, 77,123,123,123,123, 76, 77,  0,
			  0,123,123, 76, 77,123,123,123,123, 78, 77,123,123,123,123, 76, 77,  0,
			  0,123,123, 76, 77,123,123,123,123, 79, 77,123,123,123,123, 76, 77,  0,
			  0,111,112, 76, 77,111,112,111,118, 76, 77,117,111,111,118, 76, 77,  0,
			  0,111,113, 76, 77,111,113,111,111, 76, 77,111,111,  0,  0,  0,  0,  0,
			  0,114,115, 76, 77,114,115,114,115, 76, 77,111,  0,  0,111, 76, 77,  0,
			  0,  0,116, 78, 77,  0,116,  0,116, 76, 77,  0,  0,111,111, 78, 77,  0,
			  0,111,118, 80, 81,117,111,111,111, 80, 81,117,111,111,118, 80, 81,  0,
			  0,108,109,108,109,108,109,108,109,108,109,108,109,108,109,108,109,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		map = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,550,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,550,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,550,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,550,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,550,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,550,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,119,109,108,109,550,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,119,121,  0,  0,  0,550,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,108,109,550,  0,  0,  0,550,
			550,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,550,
			550,108,109,108,109,108,109,108,109,108,109,108,109,108,109,108,109,550,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		mapFront = new int[size] {
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
			  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
		};
		EnemyManager::Instance().DestroyEnemies();
	}
	else
	{
		//Error level doesn't exist or incorrect level number
		LOG("Failed to load level, stage %d doesn't exist", stage);
		return AppStatus::ERROR;	
	}

	//Player
	i = 0;
	for (y = 0; y < LEVEL_HEIGHT; ++y)
	{
		for (x = 0; x < LEVEL_WIDTH; ++x)
		{
			tile = (Tile)mapBack[i];
			if (tile == Tile::EMPTY)
			{
				mapBack[i] = 0;
			}
			else if (tile == Tile::PLAYER)
			{
				pos.x = x * TILE_SIZE;
				pos.y = y * TILE_SIZE + TILE_SIZE - 1;
				player->SetPos(pos);
				mapBack[i] = 0;
			}
			

			++i;
		}
	}
	i = 0;
	for (y = 0; y < LEVEL_HEIGHT; ++y)
	{
		for (x = 0; x < LEVEL_WIDTH; ++x)
		{
			tile = (Tile)map[i];
			if (tile == Tile::EMPTY)
			{
				map[i] = 0;
			}
			else if (tile == Tile::PLAYER)
			{
				pos.x = x * TILE_SIZE;
				pos.y = y * TILE_SIZE + TILE_SIZE - 1;
				player->SetPos(pos);
				map[i] = 0;
			}
			++i;
		}
	}
	i = 0;
	for (y = 0; y < LEVEL_HEIGHT; ++y)
	{
		for (x = 0; x < LEVEL_WIDTH; ++x)
		{
			tile = (Tile)mapFront[i];
			if (tile == Tile::EMPTY)
			{
				mapFront[i] = 0;
			}
			else if (tile == Tile::PLAYER)
			{
				pos.x = x * TILE_SIZE;
				pos.y = y * TILE_SIZE + TILE_SIZE - 1;
				player->SetPos(pos);
				mapFront[i] = 0;
			}
			++i;
		}
	}
	//Tile map
	level->Load(map, mapFront, mapBack,LEVEL_WIDTH, LEVEL_HEIGHT);
	delete[] map;
	delete[] mapFront;
	delete[] mapBack;
	return AppStatus::OK;
}
void Scene::Update()
{
	Point p1, p2;
	AABB box;
	Point left_position(16+3, player->GetPos().y);
	Point right_position(256-3, player->GetPos().y);
	Point top_position(player->GetPos().x, 48);
	Point bottom_position(player->GetPos().x, 150);

	Point boss_position(16 + 3, 170);

	EnemyManager::Instance().SetTilemap(level);

	//Switch between the different debug modes: off, on (sprites & hitboxes), on (hitboxes) 
	if (IsKeyPressed(KEY_F2))
	{
		debug = (DebugMode)(((int)debug + 1) % (int)DebugMode::SIZE);
	}
	//Debug levels instantly

	AudioPlayer::Instance().PlayMusicByName("VampireKiller");

	if (debug == DebugMode::SPRITES_AND_HITBOXES || debug == DebugMode::ONLY_HITBOXES) {
		if (IsKeyPressed(KEY_ONE)) {
			player->SetPos({ player->GetPos().x , player->GetPos().y - 16 });
			LoadLevel(1, 0);
		}
		else if (IsKeyPressed(KEY_TWO)) {
			player->SetPos({ player->GetPos().x , player->GetPos().y - 16 });
			LoadLevel(2, 0);
		}
		else if (IsKeyPressed(KEY_THREE)) {
			player->SetPos({ player->GetPos().x , player->GetPos().y - 16 });
			LoadLevel(3, 0);
		}
		else if (IsKeyPressed(KEY_FOUR))	LoadLevel(4,0);
		else if (IsKeyPressed(KEY_FIVE))	LoadLevel(5,0);
		else if (IsKeyPressed(KEY_SIX))		LoadLevel(6,0);
		else if (IsKeyPressed(KEY_SEVEN))	LoadLevel(7,0);
		else if (IsKeyPressed(KEY_EIGHT))	LoadLevel(4,1);
		else if (IsKeyPressed(KEY_NINE))	LoadLevel(5,1);
		else if (IsKeyPressed(KEY_ZERO))	LoadLevel(6,1);
		else if (IsKeyPressed(KEY_O))	LoadLevel(7,1);
		else if (IsKeyPressed(KEY_P))	LoadLevel(8,1);

		else if (IsKeyPressed(KEY_C))		player->weapon->SetWeapon(WeaponType::CHAIN);
		else if (IsKeyPressed(KEY_F3)) {
			AudioPlayer::Instance().StopMusicByName("VampireKiller");
			AudioPlayer::Instance().StopMusicByName("BossMusic");
			player->Win();
		}
		else if (IsKeyPressed(KEY_F1))	    player->GodModeSwitch();
		else if (IsKeyPressed(KEY_F4) || player->GetLife() == 0)
		{
			if (!player->IsGodMode() && player->GetState() != State::DYING) {
				player->StartDying();
			}
		}
		else if (IsKeyPressed(KEY_F5))
		{
			if (!player->IsGodMode()) {
				player->DecrLife(8);
			}
		}
		else if (IsKeyPressed(KEY_F6)) player->IncrHearts(99);
		else if (IsKeyPressed(KEY_G)) {
			EnemyManager::Instance().SpawnZombie(player->GetPos());
		}
		else if (IsKeyPressed(KEY_H)) {
			EnemyManager::Instance().SpawnBat(player->GetPos());
		}
		else if (IsKeyPressed(KEY_J)) {
			EnemyManager::Instance().SpawnBoss(player->GetPos());
		}
		else if (IsKeyPressed(KEY_K)) {
			EnemyManager::Instance().SpawnTrader(player->GetPos());
		}
		else if (IsKeyPressed(KEY_L)) {
			Object* obj;
			obj = new Object(player->GetPos(), ObjectType::SHIELD, {(float)currentLevel, (float)currentFloor});
			objects.push_back(obj);
		}
	}

	box = player->GetHitbox();

	AudioPlayer::Instance().Update();

	if (level->TestCollisionRight(box))
	{
		if (currentLevel == 7 && currentFloor == 1)
		{
			LoadLevel(8, currentFloor);
			player->SetPos(boss_position);
		}
		else if (currentLevel == 7) {
			LoadLevel(4,currentFloor);
			player->SetPos(left_position);
		}
		else if (currentLevel == 3) {
			AudioPlayer::Instance().PlaySoundByName("EnterCastle");
			LoadLevel(currentLevel + 1, currentFloor);
			player->SetPos(left_position);
		}
		else {
			LoadLevel(currentLevel + 1,currentFloor);
			player->SetPos(left_position);
		}
	}
	else if (level->TestCollisionLeft(box)) {
		if (currentLevel <= 3) {
			LoadLevel(currentLevel - 1,currentFloor);
			player->SetPos(right_position);
		}
		else if (currentLevel == 4) {
			LoadLevel(7,currentFloor);
			player->SetPos(right_position);
		}
		else if (currentLevel > 4) {
			LoadLevel(currentLevel - 1,currentFloor);
			player->SetPos(right_position);
		}
	}

	else if (level->TestCollisionTop(box)) {
		LoadLevel(currentLevel, currentFloor + 1);
		player->SetPos(bottom_position);
	}
	else if (level->TestCollisionBottom(box)) {
		LoadLevel(currentLevel, currentFloor - 1);
		player->SetPos(top_position);
	}

	if (level->TestCollisionWin(box)) {
		AudioPlayer::Instance().StopMusicByName("VampireKiller");
		AudioPlayer::Instance().StopMusicByName("BossMusic");
		player->Win();
	}

	EnemyManager::Instance().SetTilemap(level);
	
	Object* obj;
	if (chestOpening) {
		chest_time--;
		// PLAY ANIMATION
		if (chest_time == 0) {
			if (currentChestType == ObjectType::CHEST_CHAIN) {
				obj = new Object({ (int)currentChestX,(int)currentChestY }, ObjectType::CHAIN, { (float)currentLevel, (float)currentFloor });
				objects.push_back(obj);
				chest_time = 60;
				chestOpening = false;
			}
			if (currentChestType == ObjectType::CHEST_SHIELD) {

				obj = new Object({ (int)currentChestX,(int)currentChestY }, ObjectType::SHIELD, { (float)currentLevel, (float)currentFloor });
				objects.push_back(obj);
				chest_time = 60;
				chestOpening = false;
			}
			if (currentChestType == ObjectType::CHEST_BOOTS) {

				obj = new Object({ (int)currentChestX,(int)currentChestY }, ObjectType::BOOTS, { (float)currentLevel, (float)currentFloor });
				objects.push_back(obj);
				chest_time = 60;
				chestOpening = false;
			}
			if (currentChestType == ObjectType::CHEST_WINGS) {

				obj = new Object({ (int)currentChestX,(int)currentChestY }, ObjectType::WINGS, { (float)currentLevel, (float)currentFloor });
				objects.push_back(obj);
				chest_time = 60;
				chestOpening = false;
			}
			if (currentChestType == ObjectType::CHEST_HEART) {

				obj = new Object({ (int)currentChestX,(int)currentChestY }, ObjectType::HEART_BIG, { (float)currentLevel, (float)currentFloor });
				objects.push_back(obj);
				chest_time = 60;
				chestOpening = false;
			}
		}
	}

	if (lootOpening) {
		loot_time--;
		spawnY++;

		if (loot_time < 0) {
			if (currentLootType == ObjectType::CHAIN) {
				obj = new Object({ (int)currentLootX,(int)spawnY }, ObjectType::CHAIN, { (float)currentLevel, (float)currentFloor });
				objects.push_back(obj);
				lootOpening = false;
				loot_time = 40;
			}
			if (currentLootType == ObjectType::HEART_BIG) {
				obj = new Object({ (int)currentLootX,(int)spawnY }, ObjectType::HEART_BIG, { (float)currentLevel, (float)currentFloor });
				objects.push_back(obj);
				lootOpening = false;
				loot_time = 40;
			}
			if (currentLootType == ObjectType::HEART_SMALL) {
				obj = new Object({ (int)currentLootX,(int)spawnY }, ObjectType::HEART_SMALL, { (float)currentLevel, (float)currentFloor });
				objects.push_back(obj);
				lootOpening = false;
				loot_time = 40;
			}
		}
	}

	zombie_delay_time--;
	if (zombie_delay_time < 0) {
		if (currentFloor == 0) {
			if (currentLevel == 4 || currentLevel == 5 ) {
				EnemyManager::Instance().SpawnZombie({ 236,175 });
				zombie_delay_time = 120;
			}
		}
		if (currentFloor == 1) {
			if (currentLevel == 5 || currentLevel == 6) {
				EnemyManager::Instance().SpawnZombie({ 236,175 });
				zombie_delay_time = 120;
			}
		}
	}

	bat_delay_time--;
	if (bat_delay_time < 0) {
		if (currentFloor == 1 && currentLevel == 4) {
			EnemyManager::Instance().SpawnBat({ 236,player->GetPos().y });
			bat_delay_time = 120*2;
		}
	}

	if (player->weapon->GetFrame() == 2) {
		if (currentLevel == 6 && currentFloor == 0) {
			if (level->TestCollisionBreakableBrick(player->weapon->HitboxOnAttack())) {	
				level->TurnIntoAir();
				obj = new Object({ 144,160 }, ObjectType::KEY_DOOR, { 6, 0 });
				objects.push_back(obj);
				level6WallBroken = true;
				AudioPlayer::Instance().PlaySoundByName("BreakWalls");
			}
		}
		else if (currentLevel == 7 && currentFloor == 1) {
			if (level->TestCollisionBreakableBrick(player->weapon->HitboxOnAttack())) {
				level->TurnIntoAir();
				level7_1WallBroken = true;
				if (!traderSpawned) {
					EnemyManager::Instance().SpawnTrader({ 205,175 });
					traderSpawned = true;
				}
				AudioPlayer::Instance().PlaySoundByName("BreakWalls");
			}
		}
	}

	if (currentLevel == 8 && EnemyManager::Instance().IsBossDead()) {
		boss_loot_time--;
		bossSpawnY++;
		if (boss_loot_time < 0) {
			if (!boss_loot_spawned) {
				obj = new Object({ WINDOW_WIDTH/2,(int)bossSpawnY }, ObjectType::BOSS_BALL, { (float)currentLevel, (float)currentFloor });
				objects.push_back(obj);
				boss_loot_spawned = true;
			}
		}
	}

	if (currentLevel == 8 && !boss_spawned) {
		EnemyManager::Instance().SpawnBoss({ 100, 100 });
		boss_spawned = true;
	}

	if (gotBoots == true) {
		boot_time--;
		if (boot_time <= 0) {
			gotBoots = false;
			boot_time = 60;
		}
	}
	if (gotWings == true) {
		wings_time--;
		if (wings_time <= 0) {
			gotWings = false;
			wings_time = 60;
		}
	}
	if (gotShield == true) {
		shield_time--;
		if (shield_time <= 0) {
			gotShield = false;
			shield_time = 60;
		}
	}
	if (gotHeart == true) {
		heart_time--;
		if (heart_time <= 0) {
			gotHeart = false;
			heart_time = 60;
		}
	}

	if (currentLevel == 7 && currentFloor == 1) {
		if (player->GetHitbox().TestAABB(bossDoor->GetHitbox())) {
			if (player->HasDoorKey()) {
				bossDoor->Open();
				AudioPlayer::Instance().PlaySoundByName("OpenDoor");
			}
			else {
				player->SetPos({ player->GetPos().x-1, player->GetPos().y });
			}
		}
	}

	if (currentLevel == 8 && currentFloor == 1) {
		if (AudioPlayer::Instance().IsMusicPlaying("VampireKiller")) {
			AudioPlayer::Instance().StopMusicByName("VampireKiller");
			AudioPlayer::Instance().PlayMusicByName("BossMusic");
		}
	}
	else {
		if (AudioPlayer::Instance().IsMusicPlaying("BossMusic")) {
			AudioPlayer::Instance().StopMusicByName("BossMusic");
			AudioPlayer::Instance().PlayMusicByName("VampireKiller");
		}
	}

	bossDoor->Update();
	level->Update();
	EnemyManager::Instance().SetTilemap(level);
	EnemyManager::Instance().Update();
	player->Update();
	for (Object* obj : objects)
	{
		obj->Update();
	}
	CheckCollisions();
}
void Scene::Render()
{
	BeginMode2D(camera);
	if (!player->IsDead()) {
		level->RenderEarly();

		level->Render();
		if (debug == DebugMode::OFF || debug == DebugMode::SPRITES_AND_HITBOXES)
		{
			RenderObjects(); 
			EnemyManager::Instance().Render();
			if (chestOpening) {
				if (chest_time % 30 < 8) {
					DrawTextureRec(*chest_animation, { 0,0,16,16 }, { currentChestX, currentChestY-16 }, WHITE);
				}
				else if (chest_time % 30 < 16) {
					DrawTextureRec(*chest_animation, { 16 * 1,0,16,16 }, { currentChestX, currentChestY-16  }, WHITE);
				}
				else if (chest_time % 30 < 23) {
					DrawTextureRec(*chest_animation, { 16 * 2,0,16,16 }, { currentChestX, currentChestY-16  }, WHITE);
				}
				else if (chest_time % 30 < 30) {
					DrawTextureRec(*chest_animation, { 16 * 3,0,16,16 }, { currentChestX, currentChestY-16  }, WHITE);
				}
			}

			if (currentLevel == 7 && currentFloor == 1) {
				bossDoor->Draw();
			}

			if (lootOpening) {		
				if (currentLootType == ObjectType::CHAIN) {
					if ((int)loot_time % 30 < 8) {
						DrawTextureRec(*chest_animation, { 0,0,16,16 }, { currentLootX, spawnY - 16 }, WHITE);
					}
					else if ((int)loot_time % 30 < 16) {
						DrawTextureRec(*chest_animation, { 16 * 1,0,16,16 }, { currentLootX, spawnY - 16 }, WHITE);
					}
					else if ((int)loot_time % 30 < 23) {
						DrawTextureRec(*chest_animation, { 16 * 2,0,16,16 }, { currentLootX, spawnY - 16 }, WHITE);
					}
					else if ((int)loot_time % 30 < 30) {
						DrawTextureRec(*chest_animation, { 16 * 3,0,16,16 }, { currentLootX, spawnY - 16 }, WHITE);
					}
				}
				else {
					DrawTextureRec(*loot_heart, { 14*16,4*16,16,16 }, { currentLootX, spawnY - 16 }, WHITE);
				}
			}

			if (currentLevel == 8) {
				if (EnemyManager::Instance().IsBossDead()) {
					if (boss_loot_time % 30 > 15 && boss_loot_time > 0) {
						DrawTextureRec(*loot_heart, { 1 * 16,8 * 16,16,16 }, { WINDOW_WIDTH / 2, bossSpawnY - 16 }, WHITE);
					}
					else if (boss_loot_time % 30 < 15 && boss_loot_time > 0) {
						DrawTextureRec(*loot_heart, { 2 * 16,8 * 16,16,16 }, { WINDOW_WIDTH / 2, bossSpawnY - 16 }, WHITE);
					}
				}
			}

			//enemy_killed = EnemyManager::Instance().GetKilled();

			//if (player->weapon->GetFrame() == 2 && enemy_killed) {
			//	if (!got_enemy_pos) {
			//		enemy_killed_pos = EnemyManager::Instance().GetKilledPos();
			//		got_enemy_pos = true;
			//	}
			//}

			//if (enemy_killed) {
			//	hit_effect_time--;
			//	if (hit_effect_time < 0) {
			//		hit_effect_time = 30;
			//		enemy_killed = false;
			//		got_enemy_pos = false;
			//	}
			//	else if (hit_effect_time % 30 > 15) {
			//		DrawTextureRec(*hit_effect, { 0,0,16,16 }, enemy_killed_pos, WHITE);
			//	}
			//	else if (hit_effect_time % 30 < 15) {
			//		DrawTextureRec(*hit_effect, { 16,0,16,16 }, enemy_killed_pos, WHITE);
			//	}
			//}

			if (EnemyManager::Instance().DeleteTraderPopUp()) {
				DeletePopUp();
			}

			if (EnemyManager::Instance().GetTraderPopUp() && popUpDisplayed == false)
			{
				DrawTextureRec(*popup_trader, { 0,0,128,32 }, { 176,120 }, WHITE);
			}
			

			if(player->GetDamagedDelay() > 0){
				if (player->GetDamagedDelay() % 12 == 0 || player->GetDamagedDelay() % 12 == 1 || player->GetDamagedDelay() % 12 == 2 ||
					player->GetDamagedDelay() % 12 == 3 || player->GetDamagedDelay() % 12 == 4 || player->GetDamagedDelay() % 12 == 5) {
					player->Draw();
				}
			}
			else {
				player->Draw();
			}
			if (player->GetState() != State::DAMAGED) {
				player->weapon->Draw();
			}
		}
		if (debug == DebugMode::SPRITES_AND_HITBOXES || debug == DebugMode::ONLY_HITBOXES)
		{
			if (currentLevel == 7 && currentFloor == 1) {
				bossDoor->DrawDebug(PURPLE);
			}
			RenderObjectsDebug(YELLOW);
			EnemyManager::Instance().RenderDebug();
			player->DrawDebug(GREEN);
			if (player->GetState() != State::DAMAGED) {
				player->weapon->DrawDebug(RED);
			}

		}
		level->RenderLate();


		deathExecuted = false;
	}
	else if(player->IsDead() && deathExecuted == false){
		if (player->GetLives() <= 0) {
			AudioPlayer::Instance().StopMusicByName("VampireKiller");
			AudioPlayer::Instance().StopMusicByName("BossMusic");
			renderingGameOver = true;
			deathExecuted = true;
		}
		else {
			player->DecrLives(1);
			deathExecuted = true;
			AudioPlayer::Instance().StopMusicByName("VampireKiller");
			AudioPlayer::Instance().StopMusicByName("BossMusic");
			
			player->SetPos({ 20,140 });
			player->SetState(State::IDLE);
			player->weapon->SetWeapon(WeaponType::WHIP);
			player->SetLook(Look::RIGHT);
			player->InitLife();
			player->InitHearts();
			if (player->HasShield()) {
				player->SwitchShield();
			}
			if (player->HasChestKey()) {
				player->SwitchChestKey();
			}
			boss_spawned = false;

			if (currentLevel > 3) {
				LoadLevel(4, 0);
			}
			else {
				LoadLevel(1, 0);
			}
		}
	}

	if (renderingGameOver) {
		RenderGameOver();
	}

	EndMode2D();

	if (debug == DebugMode::SPRITES_AND_HITBOXES || debug == DebugMode::ONLY_HITBOXES) {
		if (player->IsGodMode()) {
			DrawText("GOD MODE : ON", 10, 50, 8, LIGHTGRAY);
		}
		else {
			DrawText("GOD MODE : OFF", 10, 50, 8, LIGHTGRAY);
		}
	}

	RenderGUI();

	DrawTexture(*hud, 0, 0, WHITE);

}
void Scene::Release()
{
	ResourceManager& data = ResourceManager::Instance();
	data.ReleaseTexture(Resource::IMG_GAME_OVER);
	data.ReleaseTexture(Resource::IMG_HUD);
	data.ReleaseTexture(Resource::IMG_OPEN_CHEST);
	data.ReleaseTexture(Resource::IMG_POPUP_TRADER);
	data.ReleaseTexture(Resource::IMG_HIT_EFFECT);

    level->Release();
	player->Release();
	ClearLevel();
}
bool Scene::PlayerIsDead() const {
	if (player->GetLives() <= 0) {
		return player->IsDead();
	}
	else {
		return false;
	}
}
bool Scene::PlayerHasWon() const {
	return player->HasWon();
}
void Scene::DeletePopUp()
{
	popUpDisplayed = true;
}
void Scene::CheckCollisions()
{
	AABB player_box, obj_box;
	
	player_box = player->GetHitbox();
	auto it = objects.begin();
	while (it != objects.end())
	{
		Vector2 currentObjLevel = (*it)->GetObjectLevel();
		if (currentObjLevel.x != currentLevel || currentObjLevel.y != currentFloor)
		{
			++it;
			continue;
		}
		obj_box = (*it)->GetHitbox();
		if ((*it)->GetType() == ObjectType::FIRE || (*it)->GetType() == ObjectType::CANDLE) {
			if (player->weapon->GetFrame() == 2) {
				if (player->weapon->HitboxOnAttack().TestAABB(obj_box)) {
					AudioPlayer::Instance().PlaySoundByName("Attack");
					lootOpening = true;
					currentLootType = (*it)->GetLoot();
					currentLootX = (*it)->GetPos().x;
					currentLootY = (*it)->GetPos().y;
					spawnY = currentLootY;
					loot_time = (*it)->GetDistanceToFloor();
					//Delete the object
					delete* it;
					//Erase the object from the vector and get the iterator to the next valid element
					it = objects.erase(it);
				}
				else {
					++it;
				}
			}
			else {
				++it;
			}
		}
		else if(player_box.TestAABB(obj_box))
		{

			if ((*it)->GetType() == ObjectType::CHAIN) {
				AudioPlayer::Instance().PlaySoundByName("Collect");
				player->weapon->SetWeapon(WeaponType::CHAIN);
				//Delete the object
				delete* it;
				//Erase the object from the vector and get the iterator to the next valid element
				it = objects.erase(it);
			}
			else if ((*it)->GetType() == ObjectType::BOSS_BALL) {
				AudioPlayer::Instance().PlaySoundByName("Collect");
				AudioPlayer::Instance().StopMusicByName("VampireKiller");
				AudioPlayer::Instance().StopMusicByName("BossMusic");
				player->Win();
				//Delete the object
				delete* it;
				//Erase the object from the vector and get the iterator to the next valid element
				it = objects.erase(it);
			}
			else if ((*it)->GetType() == ObjectType::HEART_BIG) {
				AudioPlayer::Instance().PlaySoundByName("GetHeart");
				player->IncrHearts(5);
				gotHeart = true;
				//Delete the object
				delete* it;
				//Erase the object from the vector and get the iterator to the next valid element
				it = objects.erase(it);
			}
			else if ((*it)->GetType() == ObjectType::HEART_SMALL) {
				AudioPlayer::Instance().PlaySoundByName("GetHeart");
				player->IncrHearts(1);
				//Delete the object
				delete* it;
				//Erase the object from the vector and get the iterator to the next valid element
				it = objects.erase(it);
			}
			else if ((*it)->GetType() == ObjectType::BOOTS) {
				AudioPlayer::Instance().PlaySoundByName("Collect");
				gotBoots = true;
				//Delete the object
				delete* it;
				//Erase the object from the vector and get the iterator to the next valid element
				it = objects.erase(it);
			}
			else if ((*it)->GetType() == ObjectType::WINGS) {
				AudioPlayer::Instance().PlaySoundByName("Collect");
				gotWings = true;
				//Delete the object
				delete* it;
				//Erase the object from the vector and get the iterator to the next valid element
				it = objects.erase(it);
			}
			else if ((*it)->GetType() == ObjectType::SHIELD) {
				AudioPlayer::Instance().PlaySoundByName("Collect");
				if (!player->HasShield()) {
					player->SwitchShield();
				}
				gotShield = true;
				//Delete the object
				delete* it;
				//Erase the object from the vector and get the iterator to the next valid element
				it = objects.erase(it);
			}
			else if ((*it)->GetType() == ObjectType::KEY_CHEST) {
				if (!player->HasChestKey()) {
					AudioPlayer::Instance().PlaySoundByName("Collect");
					player->SwitchChestKey();
					//Delete the object
					delete* it;
					//Erase the object from the vector and get the iterator to the next valid element
					it = objects.erase(it);
				}
				else {
					++it;
				}
			}
			else if ((*it)->GetType() == ObjectType::KEY_DOOR) {
				if (!player->HasDoorKey()) {
					AudioPlayer::Instance().PlaySoundByName("GetDoorKey");
					player->SwitchDoorKey();
					//Delete the object
					delete* it;
					//Erase the object from the vector and get the iterator to the next valid element
					it = objects.erase(it);
				}
				else {
					++it;
				}
			}
			else if ((*it)->GetType() == ObjectType::CHEST_CHAIN) {
				if (player->HasChestKey()) {
					chestOpening = true;
					currentChestType = ObjectType::CHEST_CHAIN;
					currentChestX = (float)(*it)->GetPos().x;
					currentChestY = (*it)->GetPos().y;
					player->SwitchChestKey();
					AudioPlayer::Instance().PlaySoundByName("OpenChest");
					//Delete the object
					delete* it;
					//Erase the object from the vector and get the iterator to the next valid element
					it = objects.erase(it);
				}
				else {
					++it;
				}
			}
			else if ((*it)->GetType() == ObjectType::CHEST_SHIELD) {
				if (player->HasChestKey()) {
					chestOpening = true;
					currentChestType = ObjectType::CHEST_SHIELD;
					currentChestX = (float)(*it)->GetPos().x;
					currentChestY = (*it)->GetPos().y;
					player->SwitchChestKey();
					AudioPlayer::Instance().PlaySoundByName("OpenChest");
					//Delete the object
					delete* it;
					//Erase the object from the vector and get the iterator to the next valid element
					it = objects.erase(it);
				}
				else {
					++it;
				}
			}
			else if ((*it)->GetType() == ObjectType::CHEST_BOOTS) {
				if (player->HasChestKey()) {
					chestOpening = true;
					currentChestType = ObjectType::CHEST_BOOTS;
					currentChestX = (float)(*it)->GetPos().x;
					currentChestY = (*it)->GetPos().y;
					player->SwitchChestKey();
					AudioPlayer::Instance().PlaySoundByName("OpenChest");
					//Delete the object
					delete* it;
					//Erase the object from the vector and get the iterator to the next valid element
					it = objects.erase(it);
				}
				else {
					++it;
				}
			}
			else if ((*it)->GetType() == ObjectType::CHEST_WINGS) {
				if (player->HasChestKey()) {
					chestOpening = true;
					currentChestType = ObjectType::CHEST_WINGS;
					currentChestX = (float)(*it)->GetPos().x;
					currentChestY = (*it)->GetPos().y;
					player->SwitchChestKey();
					AudioPlayer::Instance().PlaySoundByName("OpenChest");
					//Delete the object
					delete* it;
					//Erase the object from the vector and get the iterator to the next valid element
					it = objects.erase(it);
				}
				else {
					++it;
				}
			}
			else if ((*it)->GetType() == ObjectType::CHEST_HEART) {
				if (player->HasChestKey()) {
					chestOpening = true;
					currentChestType = ObjectType::CHEST_HEART;
					currentChestX = (float)(*it)->GetPos().x;
					currentChestY = (*it)->GetPos().y;
					player->SwitchChestKey();
					AudioPlayer::Instance().PlaySoundByName("OpenChest");
					//Delete the object
					delete* it;
					//Erase the object from the vector and get the iterator to the next valid element
					it = objects.erase(it);
				}
				else {
					++it;
				}
			}
			else {
				//Delete the object
				delete* it;
				//Erase the object from the vector and get the iterator to the next valid element
				it = objects.erase(it);
			}
		}
		else
		{
			//Move to the next object
			++it; 
		}
	}
	//CheckCollisionsStairs();
}
void Scene::CheckCollisionsStairs()
{
	//bool hasOnStair = false;
	//AABB player_box;

	//player_box = player->GetHitbox();

	//for (/*ir por todas las escaleras*/) {
	//	if (stairs[i].checkcollision(player_box) {
	//		hasOnStair = true;
	//		if (inFirstStairCheck) {
	//			if (isStairMode) {
	//				isStairMode = false;
	//			}
	//			else {
	//				if (isStairTop) {
	//					if (IsKeyPressed(KEY_DOWN)) {
	//						start going down;
	//					}
	//				}
	//				if (isStairBottom) {
	//					if(IsKeyPressed(KEY_UP)) {
	//						start going up;
	//					}
	//				}
	//			}
	//			inFirstStairCheck = false;
	//		}
	//		else {
	//			if (!isStairMode) {
	//				if (isStairTop) {
	//					if (IsKeyPressed(KEY_DOWN)) {
	//						start going down;
	//					}
	//				}
	//				if (isStairBottom) {
	//					if (IsKeyPressed(KEY_UP)) {
	//						start going up;
	//					}
	//				}
	//			}
	//		}
	//	}
	//	else {
	//		inFirstStairCheck = true;
	//	}
	//}
}
void Scene::ClearLevel()
{
	for (Object* obj : objects)
	{
		delete obj;
	}
	objects.clear();
}
void Scene::RenderObjects() const
{
	for (Object* obj : objects)
	{
		Vector2 currentObjLevel = obj->GetObjectLevel();
		if (currentObjLevel.x != currentLevel || currentObjLevel.y != currentFloor)	continue;
		if (obj->GetType() == ObjectType::FIRE || obj->GetType() == ObjectType::CANDLE) {
			obj->DrawAnimation();
			continue;
		}
		obj->Draw();
	}
}
void Scene::RenderObjectsDebug(const Color& col) const
{
	for (Object* obj : objects)
	{
		Vector2 currentObjLevel = obj->GetObjectLevel();
		if (currentObjLevel.x != currentLevel || currentObjLevel.y != currentFloor)
		{
			continue;
		}
		obj->DrawDebug(col);
	}
}
void Scene::RenderGUI() const
{
	DrawRectangle(0, 0, WINDOW_WIDTH, 46, { 6, 6, 6, 255 });

	font->Draw(65, 14, TextFormat("%06d", player->GetScore()), WHITE);
	
	if (currentLevel <= 3) {
		font->Draw(165, 14, "00", WHITE);
	}
	else if (currentLevel > 3) {
		font->Draw(165, 14, "01", WHITE);
	}

	if (player->HasChestKey()) {
		DrawTextureRec(*hud_items, { 3 * 16,0,16,16 }, { 156 ,26 }, WHITE);
	}
	if (player->HasDoorKey()) {
		DrawTextureRec(*hud_items, { 4 * 16,0,16,16 }, { 172 ,26 }, WHITE);
	}

	if (player->weapon->GetWeaponType() == WeaponType::CHAIN) {
		DrawTextureRec(*hud_items, { 0,0,16,16 }, { 136 ,26 }, WHITE);
	}	

	if (player->GetLives() >= 0) {
		font->Draw(237, 14, TextFormat("%02d", player->GetLives()), WHITE);
	}
	else {
		font->Draw(237, 14, "00", WHITE);
	}

	if (player->GetHearts() >= 0) {
		font->Draw(201, 14, TextFormat("%02d", player->GetHearts()), WHITE);
	}
	else {
		font->Draw(201, 14, "00", WHITE);
	}

	if (player->GetLife() > 0) {
		DrawRectangle(68, 28, player->GetLife() * 2, 4, { 247, 176, 144, 255 });
	}

	if (currentLevel == 8) {
		DrawRectangle(68, 37, EnemyManager::Instance().GetBossLife()*4, 4, { 176, 6, 6, 255 });
	}
	else {
		DrawRectangle(68, 37, 64, 4, { 176, 6, 6, 255 });
	}

	if (gotBoots == true) {
		DrawTextureRec(*hud_items, { 5 * 16,0,16,16 }, { 216 ,26 }, WHITE);
	}
	else if (gotWings == true) {
		DrawTextureRec(*hud_items, { 6 * 16,0,16,16 }, { 216 ,26 }, WHITE);
	}
	else if (gotShield == true) {
		DrawTextureRec(*hud_items, { 2 * 16,0,16,16 }, { 216 ,26 }, WHITE);
	}
	else if (gotHeart == true) {
		DrawTextureRec(*hud_items, { 7 * 16,0,16,16 }, { 216 ,26 }, WHITE);
	}
	else {
		if (player->HasShield()) {
			DrawTextureRec(*hud_items, { 2 * 16,0,16,16 }, { 208 ,26 }, WHITE);
		}
	}
}
void Scene::RenderGameOver() const
{
	DrawTexture(*game_over, 16, 0, WHITE);
}