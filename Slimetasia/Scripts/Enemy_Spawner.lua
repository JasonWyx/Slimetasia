-- MyConstructor
local isConstruct              = false
-- Components
local spawnPoints              = nil
local bossPoints               = nil
local owner_healthScript       = nil
local owner_transform          = nil
local owner_meshAnimator       = nil
local owner_audioEmitter       = nil
-- Animations
local switchAnimationStart     = false
local currentAnimationName     = ""
local animation_changePosition = "ChangePosition"
local animation_struggle       = "Struggle"
local animation_struggleLoop   = "StruggleLoop"
local animation_summonOneHand  = "SummonOneHand"
local animation_summonTwoHand  = "SummonTwoHand"
local animation_taiji          = "Taiji"
local animation_flinch         = "Flinch"
-- SpawnPoints
local sp                       = nil
-- Timers
local spawnTimer               = 0
local spawnInterval            = 0
local minSpawnInterval         = 0
local timerIdle                = 0
--Type of enemies to spawn first
      idleSpawner              = false
local typeAvail                = 2
local mySpawnPos               = Vector3()
local portal                   = nil
local idleCounter              = 0.0
local idleTime                 = 90.0
local it                       = 1
-- Game logic
local GameLogicScript          = nil
local enemPerspwn              = 3
local MAXenemies               = 0
--Dynamic Difficulty timer
local ddTimer                  = 0.0
local ddMaxTime                = 3.0
local ddIsTrue                 = false
local MAXhp                    = 40.0
local currHp                   = MAXhp
local maxDmgD                  = 1.0
local minDmgD                  = 0.15
local afkTimer                 = 0.0
local maxAfkTime               = 20.0
--Resting for n amt of enemies spawn
local enemyCounter             = 1
local enemPerRnd               = 6
local enemStartRest            = false
local restTimer                = 0.0
local restMaxTime              = 5.0
-- AudioClips
local audio_SlimeSplosh = "SFX_SlimeSplosh.ogg"
local audio_SlimeDefaultNoise = 
{
  "Zombie07_Growl02",
  "Zombie07_Growl03",
  "Zombie07_Growl04",
  "Zombie07_Growl05",
  "Zombie07_Growl06",
  "Zombie07_Growl07",
  "Zombie07_Growl08",
  "Zombie07_Growl09",
  "Zombie07_Growl10",
  "Zombie07_Growl11",
  "Zombie07_Growl12",
  "Zombie07_Growl13"
}
local audio_timer = 3.0
local track_selector = 1

local dead       = false
local myOrigPath = true
-- MISC
local CoreGO = nil
local moveFromIndex = 1
local NoMoreEnemies = false
local noEnemTimer = 1.0
local UpdateNoEnemTime = noEnemTimer
local levelName = nil
local isFlinch = false
local spawnEnemies = false
local eachObjSpawnTimer = 0.5
local EachEnemiesInterval = 0.5
local beginSpawn = false
local onFire = nil
local isBurning = false
onFlameTimer = -1.0
toFlame = false

function Constructor()
end

function MyConstructor()
  -- Find Game Logic
  GameLogic_GO     = CurrentLayer():GetObject("GameLogic")
  GameLogicScript  = GameLogic_GO:GetLuaScript("GameLogic_VerticalSlice.lua")
  
  -- Get PlayerPref
  levelName        = PlayerPref_GetString("CurrentLevel")
  spawnPoints      = PlayerPref_GetVector3Array("SpawnPositions", levelName)
  bossPoints       = PlayerPref_GetVector3Array("BossPositions", levelName)
  MAXenemies       = PlayerPref_GetInteger("MaxEnemies", levelName)
  MAXhp            = PlayerPref_GetFloat("SpawnerHealth", levelName)
  enemyPrefabNames = PlayerPref_GetStringArray ("NamesEnemyPrefabs" , "Settings_Spawning")
  spawnInterval    = PlayerPref_GetFloat("IntervalBetweenSpawn", "Settings_Spawning")
  currHp           = MAXhp
  minSpawnInterval = spawnInterval - 0.5
  spawnTimer       = 5.0
  owner_healthScript = owner:GetLuaScript("Health.lua")
  write("[ENEMY SPAWNER] : Curent Level = ", levelName)
  write("[ENEMY SPAWNER] : Max Enemies = ", MAXenemies)
  write("[ENEMY SPAWNER] : Max HP = ", MAXhp)
  
  
  if(levelName == "Level_Changi")then
    idleTime = 35
  end
  
  -- Create spawnPoints
  pathspawnpt = {}
  mySpawnPos  = spawnPoints[1]
  SpawnPortal(mySpawnPos)
 
  for i = 1, #spawnPoints do
    pathspawnpt[i] = false
  end
  PlayerPref_SetBoolArray("isPathFound", pathspawnpt)
  if(levelName == "Level_Tekong")then
    typeAvail = 2
  elseif(levelName == "Level_ChinaTown")then
    typeAvail = 3
  else
    typeAvail = 4
  end
  
  -- Get Components
  owner_transform    = owner:GetComponent("Transform")
  owner_meshAnimator = owner:GetComponent("MeshAnimator")
  owner_audioEmitter = owner:GetComponent("AudioEmitter")
  owner_audioEmitter:SetLoop(true)
  
  -- FaceMerlion
  CoreGO = CurrentLayer():GetObject("Core")
  RotateToFaceObject(CoreGO)
  
  -- Default animation
  owner_meshAnimator:SetTimeScale(2.0)
  PlayAnimation(animation_taiji)
  
  -- Start Merlion ulti
  script = CoreGO:GetLuaScript("MerlionUlti.lua")
  script:CallFunction("StartLiaoLe")
  SpawnFlame()
end

function OnUpdate(dt)
  -- CHEAT CODE
  if(IsKeyPressed(KEY_3)) then
    owner:GetLuaScript("Health.lua"):SetVariable("damage",20)
    owner:GetLuaScript("Health.lua"):CallFunction("DealDamge")
  end
  
  if(IsKeyPressed(KEY_4)) then
    originalState = owner:GetLuaScript("Health.lua"):GetVariable("immune")
    owner:GetLuaScript("Health.lua"):SetVariable("immune",false)
    owner:GetLuaScript("Health.lua"):SetVariable("damage",100)
    owner:GetLuaScript("Health.lua"):CallFunction("DealDamge")
    owner:GetLuaScript("Health.lua"):SetVariable("immune",originalState)
  end
  
  -- My constructor
  if(isConstruct == false)then
    MyConstructor()
    isConstruct = true
  end
  
  -- Update Sound
  PlaySFX(dt)
  if(NoMoreEnemies)then
    noEnemTimer = noEnemTimer - dt
    if(noEnemTimer <= 0.0)then
      noEnemTimer = UpdateNoEnemTime
      enemlist = CurrentLayer():GetObjectsListByTag("Slime")
      numEnemLeft = #enemlist
      if(numEnemLeft == 1)then
        Die()
      end
    end
  end
  -- Update spawning
  if(switchAnimationStart == false)then
    if(idleSpawner == false)then
      if(enemStartRest == false)then
        if(spawnEnemies == false)then
          UpdateEnemySpawning(dt)
        else
          EnemySpawning(dt) -- for spawning enemy per summon
        end
      else
        EnemyRest(dt)
      end
      UpdateDD(dt)
      IdlingTime(dt)
    end
  else
    timerIdle = timerIdle - dt
    if (timerIdle <= 0) then
      if(CoreGO ~= nil)then
        scriptMer = CoreGO:GetLuaScript("MerlionUlti.lua")
        if(scriptMer ~= nil)then
          merUlti = scriptMer:GetVariable("InMerlionUltiMode")
          if(merUlti)then
            return
          end
          RealSwitch()
          switchAnimationStart = false
        end
      end
    end
  end
end

function RandomizeSpawning()
  enemyIndex = 1
  while true do
    myRand = RandomRangeInt(1, 100)
    if(myRand > 0 and myRand <= 35)then
      enemyIndex = 1
    elseif(myRand > 35 and myRand <= 65)then
      enemyIndex = 2
    elseif(myRand > 65 and myRand <= 80)then
      enemyIndex = 3
    else
      enemyIndex = 4
    end
    if(enemyIndex <= typeAvail)then
      return enemyIndex
    end
  end
end

function EnemySpawning(dt)
  if(beginSpawn)then
    eachObjSpawnTimer = eachObjSpawnTimer - dt
    --Timer to wait for summonOneHand to end before spawning
    if(eachObjSpawnTimer <= 0)then
      eachObjSpawnTimer = EachEnemiesInterval
      beginSpawn = false
      enemyIndex = RandomizeSpawning()
      SpawnSlime(mySpawnPos, enemyIndex)
      enemyCounter = enemyCounter + 1
    end
  else
    if(MAXenemies > 0)then
      if(enemyCounter <= enemPerspwn)then
        -- Start spawning one hand animation
        owner_meshAnimator:SetTimeScale(3.0)
        PlayAnimation(animation_summonOneHand)
        beginSpawn = true
        return
      else
        spawnEnemies = false
        enemStartRest = true
        owner_meshAnimator:SetTimeScale(2.0)
        PlayAnimation(animation_taiji)
      end
    else
      spawnEnemies = false
      owner_meshAnimator:SetTimeScale(2.0)
      PlayAnimation(animation_taiji)
      NoMoreEnemies = true
    end
  end  
end

function UpdateEnemySpawning(dt)
  if(not NoMoreEnemies )then
    spawnTimer = spawnTimer - dt
    if (spawnTimer <= 0) then
      spawnTimer =  RandomRange(minSpawnInterval, spawnInterval)
      spawnEnemies = true
    end
  end
end
--This function does the dynamic difficulty scaling
function UpdateDD(dt)
  --When spawner is hit
  if(ddIsTrue)then
    ddTimer = ddTimer + dt
    if(ddTimer >= ddMaxTime)then
      ddTimer = 0.0
      ddIsTrue = false
      finalHp = owner_healthScript:GetVariable("health")
      dmgDealt = currHp - finalHp
      write("How much i dealt? ",dmgDealt)
      if(dmgDealt >= maxDmgD)then
        write("activate inc spawn----------------------")
        if(enemPerspwn < enemPerRnd)then
          enemPerspwn = enemPerspwn + 1
        end
      elseif(dmgDealt <= minDmgD)then
        write("activate dec spawn------------------")
        if(enemPerspwn > 3)then
          enemPerspwn = enemPerspwn - 1
        end
      end
      afkTimer = 0.0
    end
  else -- ddIsTrue is false, player nv hit boss at all(afk)
    afkTimer = afkTimer + dt
    if(afkTimer >= maxAfkTime)then
      if(enemPerspwn < enemPerRnd)then
        enemPerspwn = enemPerspwn + 1
      end
      afkTimer = 0.0
    end
  end
  OnFireTimer(dt)
  if(not dead and toFlame)then
    FlameOn()
    toFlame = false
  end
end

function SwitchNextPlace()
  owner_meshAnimator:SetTimeScale(2.0)
  PlayAnimation(animation_changePosition)
  switchAnimationStart = true
  timerIdle = 2.0
end

function RealSwitch()
  write("Relocate boss\n")
  if(sp ~= nil)then
    sp:Destroy()
    sp = nil
  else
    GameLogicScript:CallFunction("DestroySpwn")
  end
  idleSpawner = true
  portal:GetComponent("Transform"):SetWorldPosition(Vector3(mySpawnPos:x(), mySpawnPos:y() - 10.0, mySpawnPos:z()))
  bosspos = owner_transform:GetWorldPosition()
  owner_transform:SetWorldPosition(Vector3(bosspos:x(), bosspos:y() - 10.0, bosspos:z()))
  moveFromIndex = it
  it = it + 1  
  sizeOfSpwnpt = #spawnPoints
  if(it > sizeOfSpwnpt )then
    it = 1
  end
  idleCounter = 0.0
  SpawnTeleTrail()
end

function NextPhase()
  mySpawnPos = spawnPoints[it]
  myBossPos  = bossPoints[it]
  sizeOfSpwnpt = #spawnPoints
  idleCounter = 0.0
  owner_transform:SetWorldPosition(myBossPos)
  portal:GetComponent("Transform"):SetWorldPosition(Vector3(mySpawnPos:x(), mySpawnPos:y() + 3, mySpawnPos:z())) 
  CoreGO = CurrentLayer():GetObject("Core")
  RotateToFaceObject(CoreGO)
  sp = CreatePrefab("Spawn_Point")
  if(levelName == "Level_Changi")then
    if(it == 3 or it == 4)then
      bossRot = owner_transform:GetWorldRotation()
      owner_transform:SetWorldRotation(Vector3(bossRot:x(), 180.0, bossRot:z()))
    end
  end
  SpawnTrailPath(sp,mySpawnPos) 
  for i = 1, #spawnPoints do
    pathspawnpt[i] = false
  end
  PlayerPref_SetBoolArray("isPathFound", pathspawnpt)
  --limit the enemy type
  write("Next phas stasrting e")
  slimeList = CurrentLayer():GetObjectsListByTag("Slime")
  slimeSize = #slimeList
  
  for i = 1, slimeSize
  do
    enemyScript = slimeList[i]:GetLuaScript("PathFindLogic.lua")
    if(enemyScript ~= nil)then
      enemyScript:CallFunction("ComputePath_Indv")
    end
  end
  write("called function to path find recompute")
end

function Destructor()
	Die()
end

-- FUNCTIONS RELATED TO Death ==================================================
function Die ()
  if (not dead and GameLogicScript ~= nil) then
    AudioSystem_PlayAudioAtLocation(audio_SlimeSplosh, owner:GetComponent("Transform"):GetWorldPosition())
    write("DEADS, enemyspawner")

    write("[SPAWNER] CALLING WIN")
    GameLogicScript:CallFunction("Win")
    dead = true
    portal:Destroy()
    CurrentLayer():DestroyObjectsWithTag("Slime")
    
    GameLogicScript:SetVariable("spawnerLastPosition", owner_transform:GetWorldPosition())
  end
end

-- SPAWN SLIME =================================================================
function SpawnSlime(position, enemyIndex)
  MAXenemies = MAXenemies - 1
  randomfX = RandomRange(-0.25, 0.25)
  randomfZ = RandomRange(0.0, 0.5)
  if(levelName == "Level_Changi")then
    randomfX = RandomRange(-1.5, 1.5)
  end
  if(levelName == "Level_ChinaTown")then
    randomfX = RandomRange(-2.0, 2.0)
    randomfZ = RandomRange(0.0, 2.0)
  end
  if(levelName == "Level_Tekong")then
    randomfZ = RandomRange(-1.5, 1.5)
  end
  
  mypos = Vector3(position:x(), position:y(), position:z())
  mypos.x = position:x() + randomfX
  mypos.z = position:z() + randomfZ
  newSlime = CreatePrefab(enemyPrefabNames[enemyIndex])
  newSlime:GetComponent("Transform"):SetWorldPosition(mypos)
  -- MELEE
      if (enemyIndex == 1)then newSlime:AddComponent("PathFinding")
                               newSlime:AddComponent("LuaScript", "PathFindLogic.lua")
                               newSlime:AddComponent("LuaScript", "EnemyBehavior.lua")
  -- KAMAIKAZE
  elseif (enemyIndex == 2)then newSlime:AddComponent("PathFinding")
                               newSlime:AddComponent("LuaScript", "PathFindLogic.lua")
                               newSlime:AddComponent("LuaScript", "Enemy_Kamikaze.lua")
  -- RANGE
  elseif (enemyIndex == 3)then newSlime:AddComponent("PathFinding")
                               newSlime:AddComponent("LuaScript", "PathFindLogic.lua")
                               newSlime:AddComponent("LuaScript", "EnemyBehavior.lua")
                               PlayerPref_SetBool("IsRanged", true, "Settings_Spawning")  
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

function TurnOnDD()
  if(ddIsTrue == false and dead == false)then
    currHp = owner_healthScript:GetVariable("health")
    ddIsTrue = true
  end
end

function EnemyRest(dt)
  restTimer = restTimer + dt
  if(restTimer >= restMaxTime)then
    restTimer = 0.0
    enemStartRest = false
    enemyCounter = 1
  end
end

-- Teleport ===================================================================
function IdlingTime(dt)
  idleCounter = idleCounter + dt
  if(idleCounter >= idleTime)then
    idleCounter = 0.0 
    if(#spawnPoints > 1)then
      myHealth = owner:GetLuaScript("Health.lua")
      myHealth:SetVariable("ignoreMilestone", true)
      myHealth:SetVariable("immune", true)
      SwitchNextPlace()
    end
  end
end

function IdleEnd()
  idleSpawner = false
  NextPhase()
  myHealth = owner:GetLuaScript("Health.lua")
  myHealth:SetVariable("immune", false)
end

function PlaySFX(dt)
  audio_timer = audio_timer - dt
  if(audio_timer < 0) then
    owner_audioEmitter:SetAndPlayAudioClip(audio_SlimeDefaultNoise[track_selector])
    audio_timer = 3.0
    track_selector =  track_selector + 1
    if(track_selector > 6) then 
      track_selector = 4
    end
  end
end

function SpawnTeleTrail()
  particle = CreatePrefab("SummonerMovement")
  --myOrigPath will swap by itself after initialize
  if(myOrigPath)then
    if(#spawnPoints < 3)then
      particle:GetComponent("Transform"):SetWorldPosition(spawnPoints[1] + Vector3(0,2,0))
    else
      telePathName = PlayerPref_GetStringArray("NamesWaypoints", levelName)
      nextPos = telePathName[ToInt(moveFromIndex)]
      telePoints       = PlayerPref_GetVector3Array(nextPos, levelName)
      particle:GetComponent("Transform"):SetWorldPosition(telePoints[1])
    end
    script = particle:GetLuaScript("TeleportTrail.lua")
    if (script ~= nil) then
      --do a reverse path iff number of spawn points is 2
      if(#spawnPoints < 3)then
        myOrigPath = false
      else
        --Change to next Set of Waypoints depending on moveFromIndex
        script:SetVariable("moveIndex", moveFromIndex)
      end
      script:CallFunction("MyConstructor")      
    end
  else
    particle:GetComponent("Transform"):SetWorldPosition(spawnPoints[2] + Vector3(0,2,0))
    script = particle:GetLuaScript("TeleportTrail.lua")
    if (script ~= nil) then
      script:SetVariable("isReverse", true)
      script:CallFunction("MyConstructor")      
    end
    --do a reverse path iff number of spawn points is 2
    if(#spawnPoints < 3) then
      myOrigPath = true
    end
  end
end

-- PLAY ANIMATION
function PlayAnimation(animationName)
  if (currentAnimationName ~= animationName) then
    currentAnimationName = animationName
    owner_meshAnimator:Play(animationName)
    
    write("[ENEMY SPAWNER] : playing animation = ", animationName)
  end
end

-- FACE TARGET ================================================================
function RotateToFaceObject (object)
  targetPos        = object:GetComponent("Transform"):GetWorldPosition()
  myPos            = owner_transform:GetWorldPosition()
  angle            = VectorAngleWorldAxis(targetPos - myPos)
  if(angle < 180) then angle = 0.0 end
  rotationVector   = owner_transform:GetWorldRotation()
  rotationVector.y = angle
  owner_transform:SetWorldRotation(rotationVector)
end

function PlayFlinch()
  owner_meshAnimator:SetTimeScale(2.0)
  owner_meshAnimator:PlayOnce(animation_flinch)
end

function SpawnFlame()
  if(onFire == nil)then
    onFire = owner:GetComponent("ParticleEmitter_Circle")
  end
end

function FlameOn()
  if(onFire ~= nil)then
    SetFlameEmitter(true)
    isBurning = true
    onFlameTimer = 1.0
  end
end

function OnFireTimer(dt)
  if(not dead and isBurning)then
    onFlameTimer = onFlameTimer - dt
    if(onFlameTimer <= 0.0)then
      isBurning = false
      SetFlameEmitter(false)
    end
  end
end

function SetFlameEmitter(isFire)
  if(onFire~= nil)then
    if(isFire)then
      onFire:SetEmitRate(200)
    else
      onFire:SetEmitRate(0)
    end
  end
end

