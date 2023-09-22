#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

constexpr float DISPLAY_WIDTH{ 1280 };
constexpr float DISPLAY_HEIGHT{ 720 };
constexpr int DISPLAY_SCALE{ 1 };
constexpr int AGENT_RADIUS{ 40 };
constexpr int MAX_ASTEROIDS{ 2 };
constexpr int ASTEROID_RADIUS{ 48 };
constexpr int MAX_METEOR{ 2 };
constexpr int METEOR_RADIUS{ 30 };

int SCORE{ 0 };


const Vector2D AGENT_VELOCITY_JUMP(.0f, .0f);
float PlayerRotation{ .0f };
float PlayerSpeed{ .0f };
int Timer{ 0 };

float AsteroidSpeed{ 2.0f };
float AsteroidRotation{ 0.1f};

float MeteorSpeed{ 10.0f };
float MeteorRotation{ 0.1f };

enum GameObjectType
{
	TYPE_AGENT = 0,
	TYPE_GEM,
	TYPE_ASTEROID,
	TYPE_METEOR,
	TYPE_ASTEROID_PIECES,
	TYPE_BLUE_RING,
	TYPE_PARTICLE,
	TYPE_SHIP,
	TYPE_LASER,
};

enum agentState
{
	STATE_BEGIN_GAME,
	STATE_FLYING,
	STATE_ASTEROID,
	STATE_DEAD,
	STATE_FINISHED,
	STATE_GET_TO_SHIP,
	STATE_DESTROY_METEORS,
	STATE_MISSION_COMPLETE,
};

struct GameState
{
	int agentState{ STATE_BEGIN_GAME };
	int attachedId{ -1 };
};

GameState gamestate;

void Draw();
void BorderLoop(GameObject& object);
void PlayerMovement();
void WelcomeScreen();
void TrailPath();
void UpdateTrail();
void DestroyTrail();
void PlayerMovementOnAsteroid();
void AsteroidMovement();
void MeteorMovement();
void UpdateGem();
void UpdateRing();
void GemOffScreen();
void PiecesMovement();
void UpdateAsteroidPieces();
void MeteorAngleRandomise();
void AsteroidAngleRandomise();
void PlayerAsteroidCollision();
void PlayerMeteorCollision();
void PlayerDead();
void GemsCollected();
void GameCompleted();
void CreateShip();
void RocketMovement();
void MakeLaser();
void FireLaser();
void LaserMeteorCollision();
void MeteorsCleared();
void MissionComplete();
bool HasCollided(Point2f pos1, Point2f pos2);
bool GemHasCollided(Point2f pos1, Point2f pos2);

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::CentreAllSpriteOrigins();

	Play::CreateGameObject(TYPE_AGENT, { DISPLAY_WIDTH / 2, 650 }, AGENT_RADIUS, "agent8_fly");
	Play::CreateGameObject(TYPE_SHIP, { DISPLAY_WIDTH / 2, 650 }, 10, "rocket");

	GameObject& asteroidObj{ Play::GetGameObjectByType(TYPE_ASTEROID) };
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		int a = Play::RandomRollRange(0, DISPLAY_WIDTH);
		Play::CreateGameObject(TYPE_ASTEROID, { a , -40 }, 10, "asteroid_2");
	}

	GameObject& meteorObj{ Play::GetGameObjectByType(TYPE_METEOR) };
	for (int j = 0; j < MAX_METEOR; j++)
	{
		int b = Play::RandomRollRange(0, DISPLAY_WIDTH);
		Play::CreateGameObject(TYPE_METEOR, { b , -40 }, 10, "meteor_2");
	} 

	MeteorAngleRandomise();
	AsteroidAngleRandomise();

	

	GameObject& playerObj(Play::GetGameObjectByType(TYPE_AGENT));
	playerObj.velocity = AGENT_VELOCITY_JUMP;
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	GameObject& playerObj{ Play::GetGameObjectByType(TYPE_AGENT) };

	// applies resistance to player movement
	PlayerSpeed *= 0.99;

	Draw();
	UpdateGem(); 
	UpdateRing();

	switch (gamestate.agentState)
	{
	case STATE_BEGIN_GAME:
		AsteroidMovement(); 
		MeteorMovement(); 
		WelcomeScreen();
	break;

	case STATE_FLYING:
		--Timer;
		UpdateTrail();
		UpdateAsteroidPieces();
 		PlayerMovement();
	 	PlayerAsteroidCollision();
 		PlayerMeteorCollision();
		TrailPath();
		GemsCollected();
		AsteroidMovement();
		MeteorMovement();
		PiecesMovement();
	break;

	case STATE_ASTEROID:
		UpdateTrail();
		AsteroidMovement();
		MeteorMovement();
		PlayerMovementOnAsteroid();
		PlayerAsteroidCollision();
		PiecesMovement();
		UpdateAsteroidPieces();
	break;

	case STATE_FINISHED:
		PlayerMovement();
		GameCompleted();
		DestroyTrail();
	break;

	case STATE_GET_TO_SHIP:
		MeteorMovement();
		PlayerMovement();
		CreateShip();
	break;

	case STATE_DESTROY_METEORS:
		MeteorMovement();
		MakeLaser();
		RocketMovement();
		FireLaser();
		LaserMeteorCollision();
		MeteorsCleared();
	break;

	case STATE_MISSION_COMPLETE:
		MissionComplete();
	break; 

	case STATE_DEAD:
		AsteroidMovement();
		MeteorMovement();
		PlayerDead();
		DestroyTrail();
	break;
	}
	Play::DrawFontText("105px", "SCORE: " + std::to_string(SCORE), { DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);
	Play::PresentDrawingBuffer();

	return Play::KeyDown( VK_ESCAPE );
}

// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

void Draw()
{
	Play::ClearDrawingBuffer(Play::cWhite);
	
	Play::DrawBackground();
}


void BorderLoop(GameObject& object)
{
	if (object.pos.y + 50 < 0)
	{
		object.pos.y = DISPLAY_HEIGHT + 49;
	}
	if (object.pos.y - 50 > DISPLAY_HEIGHT)
	{
		object.pos.y = -49;
	}
	if (object.pos.x + 50 < 0)
	{
		object.pos.x = DISPLAY_WIDTH + 49;
	}
	if (object.pos.x - 50 > DISPLAY_WIDTH)
	{
		object.pos.x = -49;
	}
}

void WelcomeScreen()
{
	Play::DrawFontText("151px", "Welcome Agent8", { DISPLAY_WIDTH / 2, 200 }, Play::CENTRE);
	Play::DrawFontText("105px", "Your Mission is to destoy these meteros", { DISPLAY_WIDTH / 2, 300 }, Play::CENTRE);
	Play::DrawFontText("105px", "and collect all the treasure you find.", { DISPLAY_WIDTH / 2, 400 }, Play::CENTRE);
	Play::DrawFontText("64px", "Press Space to begin you Mission.", { DISPLAY_WIDTH / 2, 550 }, Play::CENTRE);
	GameObject& playerObj{ Play::GetGameObjectByType(TYPE_AGENT) };
	Play::DrawObjectRotated(playerObj);

	if (Play::KeyPressed(VK_SPACE))
	{
		gamestate.agentState = STATE_FLYING;
	}
}

bool HasCollided(Point2f pos1, Point2f pos2)
{
	const float DISTANCE_TEST = 90.0f;

	Vector2f separation = pos2 - pos1;
	float dist = sqrt((separation.x * separation.x) + (separation.y * separation.y));
	if (dist < DISTANCE_TEST)
	{
		return true;
	}
	else
		return false;
}

bool GemHasCollided(Point2f pos1, Point2f pos2)
{
	const float DISTANCE_TEST = 30.0f;

	Vector2f separation = pos2 - pos1;
	float dist = sqrt((separation.x * separation.x) + (separation.y * separation.y));
	if (dist < DISTANCE_TEST)
	{
		return true;
	}
	else
		return false;
}

void GemOffScreen()
{
	GameObject& playerObj{ Play::GetGameObjectByType(TYPE_AGENT) };

	std::vector<int> gemIds{ Play::CollectGameObjectIDsByType(TYPE_GEM) };
	for (int gemId : gemIds)
	{
		GameObject& gemIdObj(Play::GetGameObject(gemId));
		if (gemIdObj.pos.x <= 0)
		{
			gemIdObj.pos.x += 50;
		}
		if (gemIdObj.pos.x >= DISPLAY_WIDTH)
		{
			gemIdObj.pos.x -= 50;
		}
		if (gemIdObj.pos.y <= 0)
		{
			gemIdObj.pos.y += 50;
		}
		if (gemIdObj.pos.y >= DISPLAY_HEIGHT)
		{
			gemIdObj.pos.y -= 50;
		}
	}
} 

void PlayerMovement()
{
	GameObject& playerObj{ Play::GetGameObjectByType(TYPE_AGENT) };
	Play::CentreSpriteOrigin("agent8_fly");
	BorderLoop(playerObj);
	if (Play::KeyDown(VK_RIGHT))
	{
		playerObj.rotation += 0.1;
		Play::SetSprite(playerObj, "agent8_fly", 1.0f);
	}
	else if (Play::KeyDown(VK_LEFT))
	{
		playerObj.rotation -= 0.1;
		Play::SetSprite(playerObj, "agent8_fly", 1.0f);
	}
	else if (Play::KeyDown(VK_UP))
	{
		PlayerSpeed += 1;
		Play::SetSprite(playerObj, "agent8_fly", 1.0f);
	}
	else
	{
		playerObj.animSpeed = 0;
	}

	playerObj.pos.x = playerObj.pos.x + sin(playerObj.rotation) * PlayerSpeed;
	playerObj.pos.y = playerObj.pos.y - cos(playerObj.rotation) * PlayerSpeed;
	Play::UpdateGameObject(playerObj);
	Play::DrawObjectRotated(playerObj);
}

void TrailPath()
{
	GameObject& playerObj{ Play::GetGameObjectByType(TYPE_AGENT) };
	Play::CreateGameObject(TYPE_PARTICLE, playerObj.pos, 10, "particle");

	std::vector<int> particleIds{ Play::CollectGameObjectIDsByType(TYPE_PARTICLE) };

	for (int particleId : particleIds)
	{
		GameObject& particleIdObj{ Play::GetGameObject(particleId) };
		Play::DrawObjectRotated(particleIdObj,1.0f);
	}
}

void UpdateTrail()
{
	std::vector<int> particleIds{ Play::CollectGameObjectIDsByType(TYPE_PARTICLE) };
	for (int particleId : particleIds)
	{
		GameObject& particleIdObj{ Play::GetGameObject(particleId) };
		particleIdObj.scale -= 0.01;
		Play::UpdateGameObject(particleIdObj);
		if (particleIdObj.scale <= 0.4)
		{
			Play::DestroyGameObject(particleId);
		}
	}
}

void PlayerMovementOnAsteroid()
{
	GameObject& playerObj{ Play::GetGameObjectByType(TYPE_AGENT) };
	BorderLoop(playerObj);
	Play::SetSprite(playerObj, "agent8_right_7", 1.0f);


	if (Play::KeyDown(VK_RIGHT))
	{
		playerObj.rotation += 0.1;
		Play::SetSprite(playerObj, "agent8_right_7", 1.0f);
		Play::SetSpriteOrigin("agent8_right_7", 64, 100);
	}
	else if (Play::KeyDown(VK_LEFT))
	{
		playerObj.rotation -= 0.1; 
		Play::SetSprite(playerObj, "agent8_left_7", 1.0f); 
		Play::SetSpriteOrigin("agent8_left_7", 64, 100);
	}
	else
	{
		playerObj.animSpeed = 0;
		Play::SetSpriteOrigin("agent8_left_7", 64, 100);
	}

	playerObj.pos.x = playerObj.pos.x + sin(playerObj.rotation) * PlayerSpeed;
	playerObj.pos.y = playerObj.pos.y - cos(playerObj.rotation) * PlayerSpeed;
	Play::UpdateGameObject(playerObj);
	Play::DrawObjectRotated(playerObj);
}

void AsteroidAngleRandomise()
{
	std::vector<int> asteroidIds{ Play::CollectGameObjectIDsByType(TYPE_ASTEROID) }; 
	for (int asteroidId : asteroidIds)
	{
		GameObject& asteroidIdObj = Play::GetGameObject(asteroidId);
		asteroidIdObj.rotation = Play::DegToRad(Play::RandomRoll(360));
	}
}

void AsteroidMovement()
{
	std::vector<int> asteroidIds{ Play::CollectGameObjectIDsByType(TYPE_ASTEROID) };
	for (int asteroidId : asteroidIds)
	{
		GameObject& asteroidIdObj = Play::GetGameObject(asteroidId);
		asteroidIdObj.animSpeed = 1;
		Play::SetSprite(asteroidIdObj, "asteroid_2", 0.1f);
		Play::UpdateGameObject(asteroidIdObj);
		Play::DrawObjectRotated(asteroidIdObj); 
		asteroidIdObj.pos.x = asteroidIdObj.pos.x + sin(asteroidIdObj.rotation) * AsteroidSpeed;
		asteroidIdObj.pos.y = asteroidIdObj.pos.y - cos(asteroidIdObj.rotation) * AsteroidSpeed;

		BorderLoop(asteroidIdObj);
	}
}

void MeteorAngleRandomise()
{
	std::vector<int> meteorIds{ Play::CollectGameObjectIDsByType(TYPE_METEOR) };
	for (int meteorId : meteorIds)
	{
		GameObject& meteorIdObj = Play::GetGameObject(meteorId);
		meteorIdObj.rotation = Play::DegToRad(Play::RandomRoll(360));
	}
}

void MeteorMovement()
{
	std::vector<int> meteorIds{ Play::CollectGameObjectIDsByType(TYPE_METEOR) };
	for (int meteorId : meteorIds)
	{
		GameObject& meteorIdObj = Play::GetGameObject(meteorId);
		meteorIdObj.animSpeed = 1;
		Play::SetSprite(meteorIdObj, "meteor_2", 0.4f);
		Play::UpdateGameObject(meteorIdObj);
		Play::DrawObjectRotated(meteorIdObj);
		meteorIdObj.pos.x = meteorIdObj.pos.x + sin(meteorIdObj.rotation) * MeteorSpeed;
		meteorIdObj.pos.y = meteorIdObj.pos.y - cos(meteorIdObj.rotation) * MeteorSpeed;

		BorderLoop(meteorIdObj);
	}
}

void UpdateGem() 
{
	GameObject& playerObj{ Play::GetGameObjectByType(TYPE_AGENT) };

	std::vector<int> gemIds{ Play::CollectGameObjectIDsByType(TYPE_GEM) }; 
	for (int gemId : gemIds) 
	{
		GameObject& gemIdObj(Play::GetGameObject(gemId)); 
		Play::DrawObject(gemIdObj); 
		if ((GemHasCollided(gemIdObj.pos, playerObj.pos)) && (Timer<=0) )
		{
			Play::CreateGameObject(TYPE_BLUE_RING, { gemIdObj.pos }, 10, "blue_ring"); 
			GameObject& ringObj(Play::GetGameObject(TYPE_BLUE_RING));
			gemIdObj.pos = ringObj.pos;

			Play::DestroyGameObject(gemId); 
			Play::PlayAudio("reward");
			SCORE += 100;
		}
	} 
}

void UpdateRing()
{
	std::vector<int> ringIds{Play::CollectGameObjectIDsByType(TYPE_BLUE_RING)};
	for (int ringId : ringIds)
	{
		GameObject& ringIdObj(Play::GetGameObject(ringId));
		Play::DrawSpriteRotated(Play::GetSpriteId("blue_ring"), ringIdObj.pos, 0, ringIdObj.rotation, ringIdObj.scale, 0.3f);
		ringIdObj.scale += 0.05 ;
		if (ringIdObj.scale >= 6)
		{
			Play::DestroyGameObject(ringId);
		}
	}
}

void PiecesMovement()
{
	std::vector<int> piecesIds{Play::CollectGameObjectIDsByType(TYPE_ASTEROID_PIECES)};
	for (int piecesId : piecesIds)
	{
		GameObject& piecesIdObj(Play::GetGameObject(piecesId));
		piecesIdObj.pos.x = piecesIdObj.pos.x + sin(piecesIdObj.rotation) * 10;  
		piecesIdObj.pos.y = piecesIdObj.pos.y - cos(piecesIdObj.rotation) * 10;  
	}
}

void UpdateAsteroidPieces()
{ 
	std::vector<int> piecesIds{Play::CollectGameObjectIDsByType(TYPE_ASTEROID_PIECES)}; 
	for (int piecesId : piecesIds)
	{
		GameObject& piecesIdObj(Play::GetGameObject(piecesId));
		Play::DrawObject(piecesIdObj);
	}
}

void PlayerAsteroidCollision()
{
	std::vector<int> asteroidIds{ Play::CollectGameObjectIDsByType(TYPE_ASTEROID) };
	for (int asteroidId : asteroidIds) 
	{
		GameObject& playerObj{ Play::GetGameObjectByType(TYPE_AGENT) }; 
		GameObject& asteroidIdObj = Play::GetGameObject(asteroidId);

		std::vector<int> gemIds{ Play::CollectGameObjectIDsByType(TYPE_GEM) };
 		for (int gemId : gemIds)
		{
			GameObject& gemIdObj(Play::GetGameObject(gemId));
		}

		std::vector<int> piecesIds{Play::CollectGameObjectIDsByType(TYPE_ASTEROID_PIECES)};
		for (int piecesId : piecesIds)
		{
			GameObject& piecesIdObj(Play::GetGameObject(piecesId)); 
		}

		if ((gamestate.agentState == STATE_FLYING) && (HasCollided(asteroidIdObj.pos, playerObj.pos)) && (Timer < 0))
		{
			playerObj.rotation += Play::DegToRad(180); 
			gamestate.attachedId = asteroidIdObj.GetId();
			gamestate.agentState = STATE_ASTEROID;
		}
		if (gamestate.agentState == STATE_ASTEROID)
		{
			GameObject& attachedObj = Play::GetGameObject(gamestate.attachedId);
			Play::GetSpriteId("agent8_right_7");
			Play::SetSpriteOrigin("agent8_right_7", 64, 100);
			playerObj.pos.x = attachedObj.pos.x + sin(playerObj.rotation) * AsteroidSpeed;
			playerObj.pos.y = attachedObj.pos.y - cos(playerObj.rotation) * AsteroidSpeed;
			Play::UpdateGameObject(playerObj);
		}

		if ((gamestate.agentState == STATE_ASTEROID) && (Play::KeyPressed(VK_SPACE)))
		{
			GameObject& attachedObj = Play::GetGameObject(gamestate.attachedId);
			Timer = 20;
			PlayerSpeed += 5;
			gamestate.agentState = STATE_FLYING;

			Play::PlayAudio("explode"); 
					
			Play::CreateGameObject(TYPE_GEM, { attachedObj.pos }, 10, "gem");
			GameObject& gemObj(Play::GetGameObject(TYPE_GEM));
			gemObj.pos = attachedObj.pos; 

			for (int n = 0; n <3; n++)
			{
				int id = Play::CreateGameObject(TYPE_ASTEROID_PIECES, { attachedObj.pos }, 10, "asteroid_pieces_3"); 
				GameObject& piecesObj(Play::GetGameObject(id));
				piecesObj.frame = n;
				piecesObj.pos = gemObj.pos;
				piecesObj.rotation = -n * Play::DegToRad(120);
			}

			GemOffScreen();

			Play::UpdateGameObject(playerObj);
			Play::DestroyGameObject(gamestate.attachedId);
		}
	}
}

void DestroyTrail()
{
	std::vector<int> particleIds{ Play::CollectGameObjectIDsByType(TYPE_PARTICLE) };
	for (int particleId : particleIds)
	{
		Play::DestroyGameObject(particleId);
	}
}

void PlayerMeteorCollision()
{
	std::vector<int> meteorIds{ Play::CollectGameObjectIDsByType(TYPE_METEOR) };
	for (int meteorId : meteorIds)
	{
		GameObject& playerObj{ Play::GetGameObjectByType(TYPE_AGENT) };
		GameObject& meteorIdObj = Play::GetGameObject(meteorId);

		if (HasCollided(meteorIdObj.pos, playerObj.pos))
		{
			Timer = 240;
			Play::PlayAudio("combust");

			gamestate.agentState = STATE_DEAD;
		}
	}
}

void PlayerDead()
{
	--Timer;
	GameObject& playerObj{ Play::GetGameObjectByType(TYPE_AGENT) };
	Play::SetSprite(playerObj, "agent8_dead_2", 1.0f);
	Play::UpdateGameObject(playerObj);
	BorderLoop(playerObj);
	PlayerSpeed = 10;
	playerObj.pos.x = playerObj.pos.x + sin(playerObj.rotation) * PlayerSpeed;
	playerObj.pos.y = playerObj.pos.y - cos(playerObj.rotation) * PlayerSpeed;
	Play::DrawObjectRotated(playerObj);

	if (Timer <= 0)
	{
		playerObj.pos = { -100,-100 };
		Play::DrawFontText("105px", "OH NO,", { 600, 200 }, Play::CENTRE);
		Play::DrawFontText("105px", "Youve been hit by a meteor!", { 600, 300 }, Play::CENTRE);
		Play::DrawFontText("64px", "Press Space To Respawn", { 640, 400 }, Play::CENTRE);

		if (Play::KeyPressed(VK_SPACE))
		{
			Play::SetSprite(playerObj, "agent8_fly", 1.0f);
			playerObj.pos = { DISPLAY_WIDTH / 2 , DISPLAY_HEIGHT / 2 };
			PlayerSpeed = 0;
			gamestate.agentState = STATE_FLYING;
		}
	}
} 

void GemsCollected()
{
	std::vector<int> asteroidIds{ Play::CollectGameObjectIDsByType(TYPE_ASTEROID) };
	std::vector<int> gemIds{ Play::CollectGameObjectIDsByType(TYPE_GEM) };
	if ( (asteroidIds.size() == 0) && (gemIds.size() == 0) )
	{
		gamestate.agentState = STATE_FINISHED;
		Timer = 240;
	}
}

void GameCompleted()
{
	--Timer;
	Play::DrawFontText("151px", "You did it!", { 600, 300 }, Play::CENTRE);
	Play::DrawFontText("105px", "You have collected all the gems.", { 640, 400 }, Play::CENTRE);
	Play::DrawFontText("105px", "But its not safe to leave with these meteors around!", { 640, 500 }, Play::CENTRE);
	if (Timer == 0)
	{
		gamestate.agentState = STATE_GET_TO_SHIP;
	}
}

void CreateShip()
{
	Play::DrawFontText("105px", "Quick, Get On Your Ship And Destroy All The Meteors!", { 640, 300 }, Play::CENTRE);
	GameObject& playerObj{ Play::GetGameObjectByType(TYPE_AGENT) };
	GameObject& rocketObj{ Play::GetGameObjectByType(TYPE_SHIP) };
	Play::DrawObjectRotated(rocketObj, 1.0f);
	if (HasCollided(playerObj.pos, rocketObj.pos))
	{
		gamestate.agentState = STATE_DESTROY_METEORS;
	}
} 

void RocketMovement()
{
	GameObject& rocketObj{ Play::GetGameObjectByType(TYPE_SHIP) };
	Play::DrawObjectRotated(rocketObj, 1.0f);
	if (Play::KeyDown(VK_RIGHT))
	{
		rocketObj.rotation += 0.1;
		Play::UpdateGameObject(rocketObj);
	}
	else if (Play::KeyDown(VK_LEFT))
	{
		rocketObj.rotation -= 0.1;
		Play::UpdateGameObject(rocketObj);
	}
}

void MakeLaser()
{
	//GameObject& rocketObj{ Play::GetGameObjectByType(TYPE_SHIP) }; 
	std::vector<int> laserIds{ Play::CollectGameObjectIDsByType(TYPE_LASER) }; 
	for (int laserId : laserIds) 
	{
		GameObject& laserIdObj(Play::GetGameObject(laserId));
		Play::DrawObjectRotated(laserIdObj, 1.0f);
		laserIdObj.pos.x += sin(laserIdObj.rotation) * 10; 
		laserIdObj.pos.y -= cos(laserIdObj.rotation) * 10; 
	} 
}

void FireLaser()
{
	GameObject& rocketObj{ Play::GetGameObjectByType(TYPE_SHIP) };

	if (Play::KeyDown(VK_SPACE))
	{
		Play::CreateGameObject(TYPE_LASER, { rocketObj.pos }, 10, "laser");  

		std::vector<int> laserIds{ Play::CollectGameObjectIDsByType(TYPE_LASER) }; 
		for (int laserId : laserIds) 
		{
			GameObject& laserIdObj(Play::GetGameObject(laserId));
			if (laserIdObj.pos == rocketObj.pos)
			{
				laserIdObj.rotation = rocketObj.rotation;
			}
		}
		Play::PlayAudio("laser"); 
	} 
}

void LaserMeteorCollision()
{
	std::vector<int> laserIds{ Play::CollectGameObjectIDsByType(TYPE_LASER) };
	for (int laserId : laserIds)
	{
		GameObject& laserIdObj(Play::GetGameObject(laserId));
		std::vector<int> meteorIds{ Play::CollectGameObjectIDsByType(TYPE_METEOR) }; 
		for (int meteorId : meteorIds) 
		{
			GameObject& meteorIdObj = Play::GetGameObject(meteorId); 
			if (GemHasCollided(meteorIdObj.pos, laserIdObj.pos))
			{
				Play::DestroyGameObject(meteorId);
				Play::DestroyGameObject(laserId);
			}
		}
	} 
}

void MeteorsCleared()
{
	std::vector<int> meteorIds{ Play::CollectGameObjectIDsByType(TYPE_METEOR) };
	if (meteorIds.size() == 0)
	{
		gamestate.agentState = STATE_MISSION_COMPLETE;
	}
}

void MissionComplete()
{
		Play::DrawFontText("151px", "Well Done Agent8", { 600, 300 }, Play::CENTRE);
		Play::DrawFontText("105px", "It is now safe for you to fly", { 600, 400 }, Play::CENTRE);
		Play::DrawFontText("64px", "Press esc to leave the game", { 600, 600 }, Play::CENTRE);
}