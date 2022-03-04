-- VARIABLES ===================================================================

-- [components]
local owner_transform     = nil
local owner_meshRenderer  = nil
local owner_audioEmitter  = nil
local owner_rigidBody     = nil
local owner_meshAnimator  = nil
local owner_healthScript  = nil
local dead = false

-- [game logic]
local GO_gameLogic     = nil
local gameLogic_script = nil
-- [current target]
--local distance_detectTarget = 7.0
local GO_player             = nil
local GO_target             = nil
local target_transform      = nil
local targetBlock           = nil

-- [Settings]
local color_OriginalColor = Color()
local canJump             = false

-- [audio clips]
local audio_SlimeSplosh = "SFX_SlimeSplosh.ogg"
local audio_Explosion = "zombie_suicide_explo"
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
local track_selector = 4

-- [attacking]
local color_range        = Color(0,1,1,1)
local color_melee        = Color(1,1,1,1)
local range_melee        = 11

local timer_attack       = 0
local prefab_slimeBullet = "EnemyBullet"
local damage_slimeBullet = 2
local bulletSpawnOffset  = Vector3(0, 0.5, 0)
local coinSpawnOffset    = Vector3(0, 1, 0)

-- [Animation]
local animation_attack_1         = "Kamikaze Run"
local animation_attack_2         = "Kamikaze Run"
local animation_death            = "Kamikaze Run"
local animation_flinch_1         = "Kamikaze Run"
local animation_flinch_2         = "Kamikaze Run"
local animation_hop_1            = "Kamikaze Run"
local animation_hop_2            = "Kamikaze Run"
local animation_hop_idle         = "Kamikaze Run"
local animation_idle             = "Kamikaze Run"
local animation_range_attack_1   = "Kamikaze Run"
local animation_range_attack_2   = "Kamikaze Run"
local animation_walk_1           = "Kamikaze Run"
local animation_walk_2           = "Kamikaze Run"
local isPlayingWalkAnimation     = false 
local isAtacking                 = false
local isIdle                     = false
local interval_idle              = 0.2
local timer_idle                 = 0

-- [Damage from playerBullet]
local damage_playerBullet = 1
local timer_flicker       = 0
local interval_flicker    = 0.1
local color_flicker       = Color(1, 0, 0, 1)

-- [Trap : barbedWire]
local interval_barbedWireDamage = 0.2
local damage_barbedWire         = 0.05
local timer_barbedWire          = 0
local slow_barbedWire           = 0.5   -- current speed * this amount
local isCollidingWithBarbedWire = false
local trueEnemySpeed            = 0.0
local trueEnemyVelocity         = 0.0
local cannotMoveTime            = 0.0
local trueCannotMoveTime        = 3.0

-- [A* Search]
GO_Core        = nil
targetBlockExist = false
local PathFindScript = nil

-- [Enemy Details]
moveVec = Vector3()
EnemySpeed    = 0.0
maxVelocity   = 0.0
origCapVelocity   = maxVelocity
hitWaypoint = false
resource_amount = 4

local playerWRange = false
local blockInRange = false
local coreInRange = false

local myConstructor = true
--Change this to set how long to buff the enemy if out of range
buffUpTime  = 5.0

--Extras
local buffTimer        = 0.0
local effectBuff       = nil

local onFire = nil
local isBurning = false
onFlameTimer = -1.0
toFlame = false
-- FUNCTIONS ===================================================================
function Constructor()
  
end

function ConstructorInit()
  trueEnemySpeed = 7
  trueEnemyVelocity = 7
  cannotMoveTime = trueCannotMoveTime
  -- Find components
  owner_transform    = owner:GetComponent("Transform")
  owner_meshRenderer = owner:GetComponent("MeshRenderer")
  owner_audioEmitter = owner:GetComponent("AudioEmitter")
  owner_rigidBody    = owner:GetComponent("RigidBody")
  owner_meshAnimator = owner:GetComponent("MeshAnimator")
  owner_healthScript = owner:GetLuaScript("Health.lua")
  PathFindScript     = owner:GetLuaScript("PathFindLogic.lua")
  -- Find other gameobjects
  GO_gameLogic = CurrentLayer():GetObject("GameLogic")
  GO_player    = CurrentLayer():GetObject("Player")
  GO_Core      = CurrentLayer():GetObject("Core")
  Core_transform   = GO_Core:GetComponent("Transform")
  -- Find other components
  gameLogic_script = GO_gameLogic:GetLuaScript("GameLogic_VerticalSlice.lua")
  
  -- Set some slimes to be range
  currentLevel = gameLogic_script:GetVariable("currentWave")

  -- Set up slime settings
  color_OriginalColor  = owner_meshRenderer:GetColor()
  SetTarget(GO_Core)
  owner_audioEmitter:SetLoop(true)
  if(PathFindScript ~= nil)then
    EnemySpeed    = PathFindScript:GetVariable("EnemySpeed")
    maxVelocity   = PathFindScript:GetVariable("maxVelocity")
  end
  -- set up stun UI script
  stunScript = owner:GetLuaScript("StunPopup.lua")
  levelName        = PlayerPref_GetString("CurrentLevel")
  if(levelName == "Level_Tekong")then
    GL = CurrentLayer():GetObject("GameLogic")
    if(GL~=nil)then
      glScript = GL:GetLuaScript("Event_Tekong.lua")
      if(glScript~=nil)then
        myCurrEvt = glScript:GetVariable("currentEvent")
        if(myCurrEvt == 6 or myCurrEvt == 7)then
          range_melee = 15
        end
      end
    end
  end
  owner_healthScript:SetVariable("destroyOndeath", false)
  owner_healthScript:SetVariable("health", 0.5)
  EnemySpeed    = 7
  maxVelocity   = 7
  SpawnFlame()
end

function Destructor()
	Die()
end

function OnUpdate(dt)
  if (myConstructor) then
    myConstructor = false
    ConstructorInit()
    return
  end
  
  core = owner:GetLayer():GetObject("Core")
  if(core ~= nil) then
    script = core:GetLuaScript("MerlionUlti.lua")
    if(script ~= nil) then
      death = script:GetVariable("die")
      if(death == true) then
        Die()
      end
    end
  end 
  
  audio_timer = audio_timer - dt
  if(audio_timer < 0) then
    --write(audio_SlimeDefaultNoise[track_selector])
    owner_audioEmitter:SetAndPlayAudioClip(audio_SlimeDefaultNoise[track_selector])
    audio_timer = 3.0
    track_selector =  track_selector + 1
    if(track_selector > 6) then 
      track_selector = 4
    end
  end
  
  currentHealth = owner_healthScript:GetVariable("health")
  if(IsKeyPressed(KEY_1) or currentHealth <= 0.0) then
     Die()
  end
  
  --getting buffed
  if(effectBuff ~= nil)then
   buffTimer = buffTimer - dt
   if(buffTimer <= 0)then
     TourGuideBuffEnd()
   end
  end
  
  if (isAtacking) then
    timer_attack = timer_attack - dt
    if (owner_meshAnimator:IsPlaying() == false) then
      isAtacking = false
      isIdle     = true
      owner_meshAnimator:Play(animation_idle)
      timer_idle = interval_idle
    end
  end
  
  if (isIdle) then
    timer_attack = timer_attack - dt
    if (timer_attack <= 0) then
      isIdle                 = false
      isPlayingWalkAnimation = false
    end
  end
  
  if (not isPlayingWalkAnimation) then
    isPlayingWalkAnimation = true
    owner_meshAnimator:Play(animation_walk_1)
  end
  
  currentPositionAtTime = owner_transform:GetWorldPosition()
  if(currentPositionAtTime:y() <= -1.0)then
    Die()
    return
  end
  --color_Acl = Color(1.0, 0.0, 0.0, 1.0)
  --s         = owner_transform:GetWorldPosition()
  --DebugDrawLine(s, s + owner_transform:ForwardVector() * 5 , color_Acl)
  
  -- Damage flicker
  if (timer_flicker > 0) then
    timer_flicker = timer_flicker - dt
    if (timer_flicker <= 0) then
      owner_meshRenderer:SetColor(color_OriginalColor)
    end
  end
  --Do block and core check----
  blockInRange = BlockWithinRange()
  coreInRange = CoreWithinRange()  
  --------end of check---------
  --No path found, hitting blockade --if u want blockade priority than 
  --core dont edit otherwise, change btm line to 
  --if(targetBlockExist and coreInRange == false)then
  if(targetBlockExist )then
    SetTarget(targetBlock)
  else
    SetTarget(GO_Core)
  end
  
  --Enemy moving
  if(coreInRange or blockInRange)then
    MoveToTarget()
  else
    if(PathFindScript ~= nil)then
      PathFindScript:CallFunction("AStarMove")
    end
  end

  
  --BarbedWire
  if (isCollidingWithBarbedWire) then
    if(cannotMoveTime > 0) then
      cannotMoveTime = cannotMoveTime - dt
      if (timer_barbedWire > 0) then
        timer_barbedWire = timer_barbedWire - dt
      else
        timer_barbedWire = interval_barbedWireDamage
        TakeDamage(damage_barbedWire)
      end
    else
      if(stunScript ~= nil) then stunScript:CallFunction("End") end
      EnemySpeed = trueEnemySpeed
      maxVelocity = trueEnemyVelocity
    end
  else
    if(stunScript ~= nil) then stunScript:CallFunction("End") end
    EnemySpeed = trueEnemySpeed
    maxVelocity = trueEnemyVelocity
  end
  
  -- Caps velocity
  currVelocity   = owner_rigidBody:GetVelocity()
  currVelocityY  = currVelocity:y()
  currVelocity.y = 0
  

  if (currVelocity:Length() > maxVelocity) then
    idealVelocity   = currVelocity:Normalize() * maxVelocity
    idealVelocity.y = currVelocityY
    currVelocity    = idealVelocity
    owner_rigidBody:SetVelocity(currVelocity)
  end
      --write("1")
  -- Prevent slimes from jumping
  if (not canJump and currVelocityY > 0) then
    currVelocity.y = 0;
    owner_rigidBody:SetVelocity(currVelocity)
  end
  
  -- Rotate slime to face it's moving velocity
  if (currVelocity:Length() > 0.5) then
    currVelocity.y = 0
    angle = VectorAngleWorldAxis(-currVelocity)
    rotationVector = owner_transform:GetWorldRotation()
    rotationVector.y = angle
    owner_transform:SetWorldRotation(rotationVector)
    
    if (isIdle) then
      currVelocity.y = 0
      angle = VectorAngleWorldAxis(currVelocity)
      rotationVector = owner_transform:GetWorldRotation()
      rotationVector.y = angle
      owner_transform:SetWorldRotation(rotationVector)
    end
  end
  --OnFire by firecracker
  OnFireTimer(dt)
  if(not dead and toFlame)then
    FlameOn()
    toFlame = false
  end
end

function OnCollisionEnter(go)
  --Blockade
  if(go:Name() == "Trap_Blockade")then
    AudioSystem_PlayAudioAtLocation(audio_Explosion, owner:GetComponent("Transform"):GetWorldPosition(), 1, 500, 1000)
    blockscript = go:GetLuaScript("Blockade.lua")
    blockscript:CallFunction("DestroyBlockade")
    effect = CreatePrefab("KamikazeEffect")
    effect:GetComponent("Transform"):SetWorldPosition(owner_transform:GetWorldPosition())
    Die()
  -- Trap : BarbedWire
  elseif(go:Name() == "Trap_BarbedWire") then  
    write("Trap_BarbedWire collision Enter!")
    EnemySpeed  = 0.0
    maxVelocity = 0.0
    isCollidingWithBarbedWire = true
    go:GetLuaScript("BarbWire.lua"):CallFunction("StartTimer")
    if(stunScript ~= nil) then stunScript:CallFunction("Start") end
  elseif(go:Tag() == "Merlion") then  
  -- Core
    script = GO_Core:GetLuaScript("CoreLogic.lua")
    script:CallFunction("DealDamage")
    effect = CreatePrefab("KamikazeEffect")
    effect:GetComponent("Transform"):SetWorldPosition(owner_transform:GetWorldPosition())
    Die()
  end
end

function OnCollisionExit(other)
  if(other:Name() == "Trap_BarbedWire") then
    write("Trap_BarbedWire collision Exit!")
    isCollidingWithBarbedWire = false
    timer_barbedWire          = interval_barbedWireDamage
    cannotMoveTime            = trueCannotMoveTime
    -- EnemySpeed                = EnemySpeed  / slow_barbedWire
    -- maxVelocity               = maxVelocity / slow_barbedWire
  end
end

-- FUNCTIONS RELATED TO Death ==================================================
function Die ()
  GO_gameLogic = CurrentLayer():GetObject("GameLogic")
  
  if (not dead and GO_gameLogic ~= nil) then
    gameLogic_script = GO_gameLogic:GetLuaScript("GameLogic_VerticalSlice.lua")
    AudioSystem_PlayAudioAtLocation(audio_SlimeSplosh, owner:GetComponent("Transform"):GetWorldPosition())
    gameLogic_script:CallFunction("IncreaseWaveKillCount")
        
    DropResource()
    if(effectBuff ~= nil)then
      TourGuideBuffEnd()
    end
    --owner_transform:SetWorldPosition(Vector3(0, -100, 0))
    deathParticle = CreatePrefab("EnemyDeathParticle")
    deathParticle:GetComponent("Transform"):SetWorldPosition(owner_transform:GetWorldPosition())
    dead = true
    owner:Destroy()
  end
end

-- FUNCTIONS RELATED TO TARGET =================================================
function SetTarget(newTarget)
  GO_target = newTarget
  target_transform = GO_target:GetComponent("Transform")
end

-- FUNCTIONS RELATED TO ATTACKING ==============================================


-- FUNCTIONS RELATED TO MOVEMENT ===============================================
function MoveToTarget()
  moveVec= target_transform:GetWorldPosition()
  moveVec.y = owner_transform:GetWorldPosition():y()
  moveVec = moveVec - owner_transform:GetWorldPosition()
 -- write("start")
  owner_rigidBody:AddForce(moveVec:Normalize() * EnemySpeed * 200)
  --write("end")
end

-- FUNCTIONS RELATED TO TRAP ===================================================
function FindClosestSlime()
  slimeList = CurrentLayer():GetObjectsListByName("Slime")
  listSize = #slimeList
  returnTarget = nil
  
  if (listSize > 0) then
    returnTarget = slimeList[1]
    for i = 2, listSize
    do
      otherSlime = slimeList[i]
      if (otherSlime:GetID() ~= owner:GetID()) then
          currPos = owner_transform:GetWorldPosition()
          otherSlimeToCurrentDist   = otherSlime:GetComponent("Transform"):GetWorldPosition() - currPos
          returnTargetToCurrentDist = target_transform:GetWorldPosition()     - currPos
          if (otherSlimeToCurrentDist:Length() < returnTargetToCurrentDist:Length()) then
            returnTarget = slimeList[i]
          end
      end    
    end  
  end
  return returnTarget
end

function BlockWithinRange()
  blockList = CurrentLayer():GetObjectsListByName("Trap_Blockade")
  blockSize = #blockList
  for i = 1, blockSize
  do
    blockVec = blockList[i]:GetComponent("Transform"):GetWorldPosition() - owner_transform:GetWorldPosition()
    blockDist = blockVec:Length()
    attackRange = range_melee
    if(blockDist < attackRange)then
      targetBlock = blockList[i]
      targetBlockExist = true
      maxVelocity = 8.0
      return true
    end
  end
  -- maxVelocity = origCapVelocity
  return false
end

function CoreWithinRange()
  coreVec = Vector3()
  coreVec.x = Core_transform:GetWorldPosition():x()
  coreVec.y = 0.0
  coreVec.z = Core_transform:GetWorldPosition():z()
  kamiTransform = owner_transform:GetWorldPosition()
  kamiTransform.y = 0.0
  coreDist = coreVec:DistanceTo(kamiTransform)
  attackRange = range_melee
  if(coreDist < attackRange)then
    maxVelocity = 10.0
    --When its close to the core, enemy "explodes"
    if(coreDist < 7.5)then
      -- sounds here
      --playAudio = CreatePrefab("PlayAudioAndDie")
      --pos = owner:GetComponent("Transform"):GetWorldPosition()
      --playAudio:GetComponent("Transform"):SetWorldPosition(pos)
      --playAudioScript = playAudio:GetLuaScript("PlayAudioAndDie.lua")
      --playAudioScript:SetVariable("audioClipName", audio_Explosion)
      --playAudioScript:CallFunction("PlayAudio")
      write(" went in")
      AudioSystem_PlayAudioAtLocation(audio_Explosion, owner:GetComponent("Transform"):GetWorldPosition())
      
      -- end of sounds
      script = GO_Core:GetLuaScript("CoreLogic.lua")
      script:CallFunction("KamiKazeDealDamage")
      effect = CreatePrefab("KamikazeEffect")
      effect:GetComponent("Transform"):SetWorldPosition(owner_transform:GetWorldPosition())
      Die()
    end
    return true
  end
  return false
end

function TakeDamage (damage)
  owner_healthScript:SetVariable("damage", damage)
  owner_healthScript:CallFunction("DealDamge")
end

-- Tour guide buff functions
function TourGuideBuff()
  --write("Tour guide buffing!")
  buffTimer = buffUpTime
  if(effectBuff == nil)then
    effectBuff = CreatePrefab("TourGuideBuffEffect")
    effectBuff:SetParent(owner:GetID())
    effect_transform = effectBuff:GetComponent("Transform")
    effect_transform:SetWorldPosition(owner_transform:GetWorldPosition())
  end
end

function TourGuideBuffEnd()
  --write("Tour guide buff finish!")
  if(effectBuff ~= nil)then
    script = GO_Core:GetLuaScript("CoreLogic.lua")
    origKDmg = script:GetVariable("truekamikaze_Damage")
    script:SetVariable("kamikaze_Damage", origKDmg)
    effectBuff:UnParent()
    effectBuff:Destroy()
    effectBuff = nil
  end
end

function DropResource()
  drops = CreatePrefab("PrepResourceDrop")
  
  script = drops:GetLuaScript("PrepResourceDrop.lua")
  script:SetVariable("resourceAmount", resource_amount)
  script:SetVariable("position", owner_transform:GetWorldPosition() + coinSpawnOffset)
  
  if (resource_amount < 4) then
    script:SetVariable("prefabName", "Coin")
  elseif (resource_amount < 5) then
    script:SetVariable("prefabName", "Cash")
  else
    script:SetVariable("prefabName", "CoinBundle")
  end
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
