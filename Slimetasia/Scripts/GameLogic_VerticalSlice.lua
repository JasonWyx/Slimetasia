-- VARIABLES ===================================================================
enableShootToStartWave  = true

-- Wave
levelName               = nil
waveStart               = false
waveRunning             = false
currentWave             = 0
totalWaveCount          = 0
enemiesKilled           = 0
local currentWaveSpawns = nil
local nextWaveTimer     = 0
local waveInterval      = nil
local spawnTimer        = 0
local spawnInterval     = nil
local minSpawnInterval  = 0
local spawnPoints       = nil
local enemyInScene      = 0
local enemyPrefabNames  = nil

-- MISC
local endlessMode       = nil
local cursorShown       = false
local isConstruct       = false

-- RANGE enemy
--local rangeArray     = {}
--local rangeIndex_new = 0
--local rangeIndex_old = 0

local endGameTimer   = 0
gameEnded            = false
quitLevelWhenGameEnd = true
spawnStart           = false
gameLose             = false
winLiao              = false
local audioemitter   = nil
local winSFX         = "SFX_WinEffect"
local cam_transform = nil
spawnerLastPosition = Vector3()
local sp = nil

-- FUNCTIONS ===================================================================
--Note: Constructor might call multiple copies, hence do not create stuff in
--Constructor
function Constructor()
  --PlayerPref_SetString ("CurrentLevel", "Level_Tekong")
  
  levelName        = PlayerPref_GetString      ("CurrentLevel")
  write("LOADING : ", levelName)
  totalWaveCount   = PlayerPref_GetInteger     ("WaveCount"     , levelName)
  spawnPoints      = PlayerPref_GetVector3Array("SpawnPositions", levelName)
  camera           = CurrentLayer():GetObject("Camera")
  if(camera ~= nil)then
    audioemitter     = camera:GetComponent("AudioEmitter")
  end
  waveInterval     = PlayerPref_GetFloat       ("IntervalBetweenWaves", "Settings_Spawning")
  spawnInterval    = PlayerPref_GetFloat       ("IntervalBetweenSpawn", "Settings_Spawning")
  minSpawnInterval = spawnInterval - 1
  enemyPrefabNames = PlayerPref_GetStringArray ("NamesEnemyPrefabs"   , "Settings_Spawning")
                                               
  endlessMode      = PlayerPref_GetBool        ("EndlessMode")
  cam_transform  = camera:GetComponent("Transform")
end

--ONLY CALLS ONCE
function MyConstructor()
  --Bool array for spawnpt path check
  --if(levelName == "Level_Tekong")then
  --  pathspawnpt = {}
  --  for i = 1, #spawnPoints do
      pathspawnpt = {}
      sp = CreatePrefab("Spawn_Point")
      mySpawnPos = spawnPoints[1]
      SpawnTrailPath(sp,mySpawnPos)
      for i = 1, #spawnPoints do
        pathspawnpt[i] = false
      end
      PlayerPref_SetBoolArray("isPathFound", pathspawnpt)
  --    pathspawnpt[i] = false
  --    write("Created portal")
     
  --    SpawnPortal(spawnPoints[i])
  --  end
  --  PlayerPref_SetBoolArray("isPathFound", pathspawnpt)
  --end
  --nextWaveTimer = waveInterval
end

function OnUpdate(dt)
  -- CHEATS
  -- Press U : to spawn next wave
  -- Press C : to show  cursor
  -- Press 3 : to spawn enemy
  if(IsKeyPressed(KEY_U)) then SpawnNextWave()          end
  if(IsKeyUp(KEY_C)) then cursorShown = not cursorShown end 
    ShowMouseCursor(cursorShown)
  --if(IsKeyPressed(KEY_3)) then SpawnRandomSlime()            end
  
  if(isConstruct == false)then
    MyConstructor()
    isConstruct = true
  end
  -- Wait for key input to start the game
  if (not waveStart and enableShootToStartWave) then
   if (ControllerDown("StartWave")) then
      StartSpawn()
   end
  end
  
  --if (rangeIndex_old ~= rangeIndex_new) then
  --  rangeIndex_old = rangeIndex_old + 1
  --  rangeArray[rangeIndex_old]:CallFunction("ChangeToRange")
  --end
  
  -- End game
  if (endGameTimer > 0) then
    endGameTimer = endGameTimer - dt
    if (endGameTimer <= 0) then
      gameEnded = true
      
      if (quitLevelWhenGameEnd) then
        SceneLoad("LevelSelect")
      end
    end
  end
  --if(levelName == "Level_Tekong")then
  --    -- Update spawn 
  --  UpdateNextWaveTimer(dt)
  --  UpdateEnemySpawning(dt)
  --end
  
  --Dont touch. Testing code by cs
 --if (IsKeyPressed(KEY_M)) then
 --  particle = CreatePrefab("SummonerMovement")
 --  particle:GetComponent("Transform"):SetWorldPosition(spawnPoints[1])
 --  script = particle:GetLuaScript("TeleportTrail.lua")
 --  if (script ~= nil) then
 --    --script:SetVariable("isReverse", true)
 --    script:CallFunction("MyConstructor")      
 --  end
 --  
 --end
  
end

-- WAVE ========================================================================
function UpdateNextWaveTimer(dt)
  -- Reduce wave timer and spawn new wave when timer hits 0
  if (nextWaveTimer > 0) then
    nextWaveTimer = nextWaveTimer - dt
    if (nextWaveTimer <= 0) then
      SpawnNextWave()
    end
  end
end

function UpdateEnemySpawning(dt)
  -- Count remainding enemies
  enemiesRemainding = 0
  enemiesAvailable  = {}
  for i = 1, #currentWaveSpawns do 
    enemiesRemainding = enemiesRemainding + currentWaveSpawns[i]
    
    if (currentWaveSpawns[i] > 0) then
      enemiesAvailable[#enemiesAvailable + 1] = i
    end
  end
  
  -- If have things to spawn
  if (enemiesRemainding > 0) then
    spawnTimer = spawnTimer - dt
    if (spawnTimer <= 0) then
      spawnTimer = RandomRange(minSpawnInterval, spawnInterval)
      
      -- Pick an enemy to spawn
      enemyIndex = RandomRangeInt(1, #enemiesAvailable + 1)
      enemyIndex = enemiesAvailable[enemyIndex]
      currentWaveSpawns[enemyIndex] = currentWaveSpawns[enemyIndex] - 1
      
      -- Pick a random point to spawn at
      spawnIndex = RandomRangeInt(1, #spawnPoints + 1)
      
      -- Spawn slime
      SpawnSlime(spawnPoints[spawnIndex], enemyIndex)
    end
  end
end

function SpawnNextWave()
  -- Set amount of enemies to kill to start the next wave
  currentWave = currentWave + 1
  
  -- Reduce spawn timer
  minSpawnInterval = minSpawnInterval - 0.5
  spawnInterval = spawnInterval - 0.5
  if (minSpawnInterval < 0.5) then
    minSpawnInterval = 0.5
  end
  if (spawnInterval < 1) then
    spawnInterval = 1
  end
                                                                                write("GAME LOG : CURRENT WAVE = ", currentWave)
  if (endlessMode) then
    enemyInScene = enemyInScene + currentWave * 3
    spawnTimer   = 0
    waveRunning = true
    currentWaveSpawns = {1, 1, 1, 1}
  else
    if (currentWave <= totalWaveCount) then
      wave = ToString("Wave", currentWave)
      currentWaveSpawns = PlayerPref_GetIntegerArray(wave, levelName)
      for i = 1, #currentWaveSpawns do
        enemyInScene = enemyInScene + currentWaveSpawns[i]
      end
      write(enemyInScene)
      spawnTimer  = 0
      waveRunning = true
    else
      endGameTimer = 2
                                                                                write("GAME LOG : ALL WAVE COMPLETE")
    end
  end
end

function GameOver ()
  endGameTimer = 2
  gameLose = true
end

function Win ()
  if(gameLose == false)then
    gameEnded = true
    --currentlayer():DestroyObjectsWithTag("Slime")
    write("[GAME LOGIC] WIN GAME")
    waveRunning = false
      gameLose = false
    winLiao = true
    endGameTimer = 2
  end
end

function IncreaseWaveKillCount()
  -- count number of enemies killed
  enemiesKilled = enemiesKilled + 1
  
 -- if (waveStart) then
 --   
 --   -- Decrease enemies
 --   enemyInScene = enemyInScene - 1
 --   
 --   -- If enemies count reaches 0, wave clear
 --   if (enemyInScene <= 0) then
 --     write("GAME LOG : ROUND COMPLETE")
 --     nextWaveTimer = waveInterval
 --     waveRunning   = false
 --   end
 -- end
end


-- SPAWN SLIME =================================================================
function SpawnSlime(position, enemyIndex)
  randomintX = RandomRangeInt(-1, 1)
  randomintZ = RandomRangeInt(-1, 1)
  randomFloat = RandomRangeInt(1, 100)
  randomFloat = randomFloat / 100.0
  --write("My Random x: ",  randomintX * randomFloat )
-- write("mY Random z: ", randomintZ * randomFloat) 
 --position.x = position:x() + randomintX * randomFloat 
 --position.z = position:z() + randomintZ * randomFloat
  --write("Spawning")
  --write("Spawning slime at : ", position, enemyIndex)
  newSlime = CreatePrefab(enemyPrefabNames[enemyIndex])
  newSlime:GetComponent("Transform"):SetWorldPosition(position)
  
  -- MALEE
      if (enemyIndex == 1)then newSlime:AddComponent("PathFinding")
                               newSlime:AddComponent("LuaScript", "PathFindLogic.lua")
                               newSlime:AddComponent("LuaScript", "EnemyBehavior.lua")
  -- RANGE
  elseif (enemyIndex == 2)then newSlime:AddComponent("PathFinding")
                               newSlime:AddComponent("LuaScript", "PathFindLogic.lua")
                               newSlime:AddComponent("LuaScript", "EnemyBehavior.lua")
                               PlayerPref_SetBool("IsRanged", true, "Settings_Spawning")
                               --rangeIndex_new = rangeIndex_new + 1
                               --rangeArray[rangeIndex_new] = newSlime:GetLuaScript("EnemyBehavior.lua")
  -- KAMAIKAZE
  elseif (enemyIndex == 3)then newSlime:AddComponent("PathFinding")
                               newSlime:AddComponent("LuaScript", "PathFindLogic.lua")
                               newSlime:AddComponent("LuaScript", "Enemy_Kamikaze.lua")
  -- TOURGUIDE
  elseif (enemyIndex == 4)then newSlime:AddComponent("PathFinding")
                               newSlime:AddComponent("LuaScript", "PathFindLogic.lua")
                               newSlime:AddComponent("LuaScript", "Enemy_TourGuideBehaviour.lua")
  else end
end

function SpawnTrailPath(newspawnpt,position)
  newspawnpt:GetComponent("Transform"):SetWorldPosition(position)
  newspawnpt:AddComponent("PathFinding")
  newspawnpt:AddComponent("LuaScript", "TrailPath_Logic.lua")
end

function SpawnPortal(position)
  portal = CreatePrefab("SpawnPortal")
  portal:GetComponent("Transform"):SetWorldPosition(position + Vector3(0, 3, 0))
  particle = portal:GetComponent("ParticleEmitter_Circle")
  particle:AddAttractor(portal:GetID())
end

function StartSpawn()
  write("START")  
  --if( levelName == "Level_Tekong")then
  --  nextWaveTimer = waveInterval
  --  waveStart     = true
  --else
    if(spawnStart == false)then
      write("I am starting")
      AudioSystem_PlayAudioAtLocation(winSFX, cam_transform:GetWorldPosition())
      write("I Complete")
      position = spawnPoints[1]
      newSlime = CreatePrefab("Slime_Spawner")
      newSlime:GetComponent("Transform"):SetWorldPosition(position+ Vector3( 0, 0, -3))
      --newSlime:GetComponent("RigidBody"):SetBodyType(0)
      if(levelName ~= "Level_Tekong") then
	  newSlime:GetComponent("Transform"):SetWorldRotation(Vector3(0,180,0))
	  else
	  newSlime:GetComponent("Transform"):SetWorldRotation(Vector3(0,0,0))
	  end
	  
      newSlime:AddComponent("LuaScript", "Enemy_Spawner.lua")
      spawnStart = true
    end
  --end
end

function DestroySpwn()
  if(sp ~= nil)then
    sp:Destroy()
    sp = nil
  end
end