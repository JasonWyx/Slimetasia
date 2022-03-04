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
local audio_SlimeAttack = "SFX_EnemyAttack"
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

-- [attacking]
      isRanged           = false
local color_range        = Color(0,1,1,1)
local color_melee        = Color(1,1,1,1)
local range_melee        = 6
local radius_melee       = 3
local range_range        = 15
local radius_range       = 10
local interval_attack    = 2.0
local timer_attack       = 0
      damage_melee       = 2
local coinSpawnOffset    = Vector3(0, 1, 0)

-- [Animation]
local animation_attack_1         = "Attack02"
local animation_attack_2         = "Attack02"
local animation_death            = "Death"
local animation_flinch_1         = "Flinch"
local animation_flinch_2         = "Flinch02"
local animation_hop_1            = "Hop01"
local animation_hop_2            = "Hop02"
local animation_hop_idle         = "HopIdle"
local animation_idle             = "NormalIdle"
local animation_range_attack_1   = "ThrowAttack01"
local animation_range_attack_2   = "ThrowAttack02"
local animation_walk_1           = "walk"
local animation_walk_2           = "walk"
local animation_buff             = "Buff"
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
local PathFindScript = owner:GetLuaScript("PathFindLogic.lua")

-- [Enemy Details]
moveVec = Vector3()
local EnemySpeed    = PathFindScript:GetVariable("EnemySpeed")
local maxVelocity   = PathFindScript:GetVariable("maxVelocity")
resource_amount = 5

hitWaypoint = false
local playerWRange = false

-- [Tour guide buffs]
local effect = nil

local myConstructor = true
local noEntry = false
local isFlinch = false
local DeathAnim = false
local deathTimer = 2.0
local AOE = nil
local AOECast = false
local AOE_CastTimer = 5.0

--Burning from firecracker
local onFire = nil
local isBurning = false
onFlameTimer = -1.0
toFlame = false
-- FUNCTIONS ===================================================================
function Constructor()
  trueEnemySpeed = EnemySpeed
  trueEnemyVelocity = maxVelocity
  cannotMoveTime = trueCannotMoveTime
  -- Find components
  owner_transform    = owner:GetComponent("Transform")
  owner_meshRenderer = owner:GetComponent("MeshRenderer")
  owner_audioEmitter = owner:GetComponent("AudioEmitter")
  owner_rigidBody    = owner:GetComponent("RigidBody")
  owner_meshAnimator = owner:GetComponent("MeshAnimator")
  owner_healthScript = owner:GetLuaScript("Health.lua")
  
  -- Find other gameobjects
  GO_gameLogic = CurrentLayer():GetObject("GameLogic")
  GO_player    = CurrentLayer():GetObject("Player")
  GO_Core      = CurrentLayer():GetObject("Core")
  -- Find other components
  gameLogic_script = GO_gameLogic:GetLuaScript("GameLogic_VerticalSlice.lua")

  -- Set up slime settings
  color_OriginalColor  = owner_meshRenderer:GetColor()
  SetTarget(GO_player)
  owner_audioEmitter:SetLoop(true)
  
  -- set up stun UI script
  stunScript = owner:GetLuaScript("StunPopup.lua")

end

function Destructor()
	Die()
end

function CreateAOE()
--Instantiate effect sphere collider and parent it
    AOE = CreatePrefab("Slime_TourGuide_AreaEffect", owner)
    AOE:GetComponent("Transform"):SetWorldPosition(owner:GetComponent("Transform"):GetWorldPosition())
    AOE:SetParent(owner:GetID())
end

function DestroyAOE()
  AOE:UnParent()
  AOE:Destroy()
end

function OnUpdate(dt)
  if (myConstructor) then
    myConstructor = false
    owner_healthScript:SetVariable("destroyOndeath", false)
    owner_meshAnimator:PlayOnce(animation_buff)
    SpawnFlame()
    BossObj      = CurrentLayer():GetObject("Core")
  end
  
  if(not DeathAnim)then
    if(not AOECast)then
      AOE_CastTimer = AOE_CastTimer + dt
      if(AOE_CastTimer >= 10.0)then
        AOECast = true
        AOE_CastTimer = 0.0
        owner_meshAnimator:PlayOnce(animation_buff)
        CreateAOE()
        owner_rigidBody:SetVelocity(Vector3())
        return
      end
      -- Check for merlion
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
      --Zombie noise
      audio_timer = audio_timer - dt
      if(audio_timer < 0) then
        --write(audio_SlimeDefaultNoise[track_selector])
        owner_audioEmitter:SetAndPlayAudioClip(audio_SlimeDefaultNoise[track_selector])
        audio_timer = 3.0
        track_selector =  track_selector + 1
        if(track_selector > 3) then 
          track_selector = 1
        end
      end
      --Check enemy health
      currentHealth = owner_healthScript:GetVariable("health")
      if(IsKeyPressed(KEY_1) or currentHealth <= 0.0) then
         Die()
      end
      
      if(isFlinch)then
        owner_rigidBody:SetVelocity(Vector3())
        if (owner_meshAnimator:IsPlaying() == false) then
          isFlinch = false
          isPlayingWalkAnimation = false
          owner_meshAnimator:PlayOnce(animation_walk_1)
        end
        return
      else
        if (isAtacking) then
         -- timer_attack = timer_attack - dt
          if (owner_meshAnimator:IsPlaying() == false) then
            isAtacking = false
            isIdle     = true
            owner_meshAnimator:Play(animation_walk_1)
            timer_attack = interval_idle
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
      end
      
      -- Damage flicker
      if (timer_flicker > 0) then
        timer_flicker = timer_flicker - dt
        if (timer_flicker <= 0) then
          owner_meshRenderer:SetColor(color_OriginalColor)
        end
      end
      
      --if target is nil, find target. This is here as a safeguard
      if (GO_target == nil) then
        SetTarget(GO_player)
      end
      
     --No path found, hitting blockade 
     if(targetBlockExist)then
      SetTarget(targetBlock)
     else
      SetTarget(GO_player)
     end
     -- distance check to core
      rc = PathFindScript:GetVariable("reachedCore")
      if (rc) then
          SetTarget(GO_Core)
      end
      currentPositionAtTime = owner_transform:GetWorldPosition()
      if(currentPositionAtTime:y() <= -1.0)then
        Die()
        return
      end
      -- Attack target if close enough. If not continue moving
      vectorToTarget   = target_transform:GetWorldPosition() - owner_transform:GetWorldPosition()
      distanceToTarget = vectorToTarget:Length()
      if (not isAtacking and not isIdle) then
        if (TargetInAttackRange(distanceToTarget)) then
          ----write("WIthin range")
          if (TargetInAttackRadius(distanceToTarget)) then
              --  --write("Within radius")
            AttackTarget(dt, vectorToTarget:Normalize())
          else
            MoveToTarget()
          end
        else
          --Attack when no path found
          ablk = PathFindScript:GetVariable("attackBlock")
          if(ablk)then
            if(not targetBlockExist)then
              if(BlockWithinRange())then
                --write("Target found")
                targetBlockExist = true
              end
            end
          end  
          PathFindScript:CallFunction("AStarMove")
          --MoveLocation()
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
          --write("Please")
      -- Rotate slime to face it's moving velocity
      if (currVelocity:Length() > 0.5) then
        --write("I'm here")
        currVelocity.y = 0
        angle = VectorAngleWorldAxis(currVelocity)
        rotationVector = owner_transform:GetWorldRotation()
        rotationVector.y = angle
        owner_transform:SetWorldRotation(rotationVector)
        
        if (isIdle) then
          --write("Whats this?")
          currVelocity.y = 0
          angle = VectorAngleWorldAxis(-currVelocity)
          rotationVector = owner_transform:GetWorldRotation()
          rotationVector.y = angle
          owner_transform:SetWorldRotation(rotationVector)
        end
      end
    else--AOECasting
      owner_rigidBody:SetVelocity(Vector3())
      if (owner_meshAnimator:IsPlaying() == false) then
          AOECast = false
          DestroyAOE()
          rc = PathFindScript:GetVariable("reachedCore")
          if (rc) then
            owner_meshAnimator:PlayOnce(animation_attack_1)
          else
            owner_meshAnimator:Play(animation_walk_1)
          end
          return
      end
    end
    OnFireTimer(dt)
    if(not dead and toFlame)then
      FlameOn()
      toFlame = false
    end
  else
    deathTimer = deathTimer - dt    
    if(deathTimer <= 0)then
      RealDeath()
    else
      owner_rigidBody:SetVelocity(Vector3())
      owner_meshRenderer:SetColor(Color(deathTimer, deathTimer, deathTimer, 1.0))
      owner_meshRenderer:SetEmissive(Color(deathTimer, deathTimer, deathTimer, 1.0))
    end
  end
end

function OnCollisionEnter(go)
  -- Trap : BarbedWire
  if(go:Name() == "Trap_BarbedWire") then  
    --write("Trap_BarbedWire collision Enter!")
    EnemySpeed  = 0.0
    maxVelocity = 0.0
    isCollidingWithBarbedWire = true
    go:GetLuaScript("BarbWire.lua"):CallFunction("StartTimer")
    if(stunScript ~= nil) then stunScript:CallFunction("Start") end
  end
  --Entered the grass area in changi
  if (go:Name() == "NoEntry") then
    PathFindScript:SetVariable("isFollowPlayer", true)
    noEntry = true
  end
  -- Trap : Blockad
end

function OnCollisionExit(other)
  if(other:Name() == "Trap_BarbedWire") then
    --write("Trap_BarbedWire collision Exit!")
    isCollidingWithBarbedWire = false
    timer_barbedWire          = interval_barbedWireDamage
    cannotMoveTime            = trueCannotMoveTime
    -- EnemySpeed                = EnemySpeed  / slow_barbedWire
    -- maxVelocity               = maxVelocity / slow_barbedWire
  end
  if(other:Name() == "NoEntry")then
    noEntry = false
  end
end

-- FUNCTIONS RELATED TO Death ==================================================
function Die ()
  DeathAnim = true
  deathTimer = 1.0
  owner_meshAnimator:PlayOnce(animation_death)
  owner_rigidBody:SetVelocity(Vector3())
end

function RealDeath()
  GO_gameLogic = CurrentLayer():GetObject("GameLogic")
  if (not dead and GO_gameLogic ~= nil) then
    gameLogic_script = GO_gameLogic:GetLuaScript("GameLogic_VerticalSlice.lua")  
    AudioSystem_PlayAudioAtLocation(audio_SlimeSplosh, owner_transform:GetWorldPosition())
    gameLogic_script:CallFunction("IncreaseWaveKillCount")
    DropResource()
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
function AttackTarget(dt, dir)
  timer_attack = interval_attack
  isAtacking   = true
  owner_audioEmitter:SetAndPlayAudioClip(audio_SlimeAttack)
  owner_rigidBody:SetVelocity(Vector3(0,0,0))
  --write("Melee attacking!")
  angle = VectorAngleWorldAxis(dir)
  rotationVector = owner_transform:GetWorldRotation()
  rotationVector.y = angle
  owner_transform:SetWorldRotation(rotationVector)

  owner_meshAnimator:PlayOnce(animation_attack_1)
  script = nil
  if (GO_target:Name() == "Player")then
    script = GO_target:GetLuaScript("PlayerScript.lua")
    script:SetVariable("damage", damage_melee)
  elseif (GO_target:Name() == "Core")then--Core Script
    --write("Core logic")
    script = GO_target:GetLuaScript("CoreLogic.lua")
  else--Blockade
    --write("Attack Blockade logic")
    script = GO_target:GetLuaScript("Blockade.lua")
  end
  if (script ~= nil) then
    script:CallFunction("DealDamage")
  end
end

function TargetInAttackRange(dist)
  -- Check if melee or isRanged detection range
  rc = PathFindScript:GetVariable("reachedCore")
  if (rc) then return true end
  attackRange = range_melee
  if(dist < attackRange)then
    if (playerWRange == false) then
      if (GO_target:Name() == "Player" ) then
        PathFindScript:SetVariable("isFollowPlayer", true)
      end
    end
    playerWRange = true
  else
    playerWRange = false
  end
  --Dont let it follow player
  if(dist< attackRange and noEntry)then return false end
  --Enable following player
  if(dist < attackRange == false and noEntry)then noEntry = false end
  -- Check if current dist is lesser than attack range
  return dist < attackRange
end

function TargetInAttackRadius(dist)
  rc = PathFindScript:GetVariable("reachedCore")
  if (rc) then return true end
  
  attackRadius = radius_melee
  if(GO_target:Name() == "Trap_Blockade")then
    attackRadius = 4
  end  
  -- Check if current dist is lesser than attack radius
  return dist < attackRadius
end

-- FUNCTIONS RELATED TO MOVEMENT ===============================================
function MoveToTarget()
  moveVec = target_transform:GetWorldPosition() - owner_transform:GetWorldPosition()
  owner_rigidBody:SetVelocity(moveVec:Normalize() * EnemySpeed)
end

-- FUNCTIONS RELATED TO TRAP ===================================================

function BlockWithinRange()
  blockList = CurrentLayer():GetObjectsListByName("Trap_Blockade")
  blockSize = #blockList
  for i = 1, blockSize
  do
    blockVec = blockList[i]:GetComponent("Transform"):GetWorldPosition() - owner_transform:GetWorldPosition()
    blockDist = blockVec:Length()
    attackRange = range_melee
    if (isRanged) then
      attackRange = range_range
    end
    if(blockDist < attackRange)then
      targetBlock = blockList[i]
      targetBlockExist = true
      return true
    end
  end
  return false
end

function TakeDamage (damage)
  owner_healthScript:SetVariable("damage", damage)
  owner_healthScript:CallFunction("DealDamge")
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

function PlayFlinch()
  owner_meshAnimator:PlayOnce(animation_flinch_1)
  owner_rigidBody:SetVelocity(Vector3())
  isFlinch = true
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
