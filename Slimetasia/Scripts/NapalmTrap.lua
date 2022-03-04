-- VARIABLES
local owner_Transform    = nil
local owner_meshRenderer = nil
local owner_emitter      = nil
local owner_audioEmitter = nil
local timer              = 0.0 -- general timer
local timer2             = 0.0 -- timer for dealing damage per tick
local timer3             = 0.0 -- timer for dealing final explision

-- Charge settings
local isActivated     = false
local duration_Charge = 2.0
local aIstrue         = false

-- Cooldown settings
local isOnCoolDown      = false
local duration_cooldown = 8.0

-- Flickering
local color_red           = Color(1, 0, 0, 1)
local color_black         = Color(0, 0, 0, 1)
local color_orginal       = Color()
local isRed               = false
local timer_flicker       = 0
local interval_flicker    = 0.5
local interval_flickerMin = 0.1
local interval_reduction  = 0.1

-- Shaking
local shakeOriginal     = Vector3()
local shakeSpeed        = 10
local shakeAcceleration = 25
local shakeAngleX       = 0
local shakeAngleZ       = 0
local shakeAngleY       = 0
local shakeAngleBound   = 5
local shakeDirection    = 1

-- Damage
--NormalSlime
local damageInterval     = 0.1
local damagePerTick      = 0.05
local damageExplosion    = 0.5
--Boss
local bossDmgPerTick     = 0.01
local bossDmgExplosion   = 0.2
--Other Details
local duration_explosion = 0.2
local canDealDamage      = false
local constructed        = false
local AOEDist            = 3.0

-- CONSTRUCTOR / UPDATE / ONCOLLISIONENTER
-- =============================================================================
function Constructor()
  owner_Transform    = owner:GetComponent("Transform")
  owner_meshRenderer = owner:GetComponent("MeshRenderer")
  owner_emitter      = owner:GetComponent("ParticleEmitter_Circle")
  owner_audioEmitter = owner:GetComponent("AudioEmitter")
  color_orginal      = owner_meshRenderer:GetColor()
end


function OnUpdate(dt)
  if(isOnCoolDown) then
    timer = timer + dt
    if (timer >= duration_cooldown) then
      ResetTrap()
    end
  elseif(isActivated)then
    initActiveFlame()
    ActivateFlame(dt)
    UpdateShaking(dt)
    UpdateFlicker(dt)
  end
  
  if (timer3 > 0) then
    timer3 = timer3 - dt
    owner_emitter:SetEmitRate(0)
  end
  DistanceFromEnemy()
end

-- Trap explosion
-- =============================================================================
function ActivateFlame(dt)
  -- If trap finished charging
  timer = timer + dt
  if (timer >= duration_Charge) then
    owner_emitter:SetEmitRate(50)
    
    PlayAudioAtPosition("Firecracker")
    
    explosionParticle = CreatePrefab("FirecrackerAfterburn")
    spawnPos          = owner_Transform:GetWorldPosition()
    spawnPos.y        = spawnPos:y() + 3.75
    explosionParticle:GetComponent("Transform"):SetWorldPosition(spawnPos)
    
    owner_meshRenderer:SetColor(color_black)
    
    isActivated  = false
    aIstrue      = false
    isOnCoolDown = true
    timer        = 0
    timer3       = duration_explosion
    return
  end
  
  -- Update deal damage per tick
  if (canDealDamage) then
    canDealDamage = false
  end
  
  timer2 = timer2 + dt
  if (timer2 >= damageInterval) then
    canDealDamage = true
    timer2 = 0.0
  end
end

-- Shake trap
-- =============================================================================
function UpdateShaking(dt)
  -- Increase shake speed overtime
  shakeSpeed  = shakeSpeed + shakeAcceleration * dt
  
  -- Move shake angle
  change      = shakeDirection * shakeSpeed * dt
  shakeAngleX = shakeAngleX + change
  --shakeAngleZ = shakeAngleZ + change
  shakeAngleY = shakeAngleY + shakeSpeed * dt
  -- Check if reach
  if (shakeDirection > 0 and shakeAngleX >= shakeAngleBound) then
    shakeDirection = -1
    shakeAngleX    = shakeAngleBound
    --shakeAngleZ    = shakeAngleBound
  elseif (shakeDirection < 0 and shakeAngleX <= -(shakeAngleBound+10)) then
    shakeDirection = 1
    shakeAngleX    = -(shakeAngleBound+10)
    --shakeAngleZ    = -shakeAngleBound
  end
  -- Rotate angle
  vector = Vector3(shakeAngleX, 0, 0)
  
  -- Add to rotation
  owner_Transform:SetWorldRotation(shakeOriginal + vector)
end

-- Update flckering from red - white
-- =============================================================================
function UpdateFlicker(dt)
  if (timer_flicker > 0) then
    timer_flicker = timer_flicker - dt
  else
    if (isRed) then
      owner_meshRenderer:SetColor(color_orginal)
    else
      owner_meshRenderer:SetColor(color_red)
    end
    isRed = not isRed
    timer_flicker    = interval_flicker
    interval_flicker = interval_flicker - interval_reduction
    if (interval_flicker < interval_flickerMin) then
      interval_flicker = interval_flickerMin
    end
  end
end

-- Reset trap to make it usable again
-- =============================================================================
function ResetTrap()
  timer                     = 0.0
  timer2                    = 0.0
  timer3                    = 0.0
  isActivated               = false
  isOnCoolDown              = false
  isRed                     = false
  owner_meshRenderer:SetColor(color_orginal)
  owner_Transform:SetWorldRotation(shakeOriginal)
  owner_emitter:SetEmitRate(0)
end

-- Play sound
-- =============================================================================
function PlayAudioAtPosition(name)
  --newAduioEmitter = CreatePrefab("PlayAudioAndDie")
  --newAduioEmitter:GetComponent("Transform"):SetWorldPosition(owner_Transform:GetWorldPosition())
  --script = newAduioEmitter:GetLuaScript("PlayAudioAndDie.lua")
  --script:SetVariable("audioClipName", name)
  --script:CallFunction("PlayAudio")
  owner_audioEmitter:SetAndPlayAudioClip(name)
end

function initActiveFlame(other)
  if(aIstrue == false) then
   if(not isActivated and not isOnCoolDown) then
     if(other:Tag() == "Slime" or other:Tag() == "Bullet")then
      isActivated = true
      shakeOriginal = owner_Transform:GetWorldRotation()
      PlayAudioAtPosition("Firecracker_Fuseburn")
      owner_emitter:SetEmitRate(15)
      aIstrue = true
     end
   end
  end
end
--Damage Over Time
function DOT(other)
---- Normal damage
  if(isActivated and other:Tag() == "Slime")then
      if (canDealDamage) then
        script = other:GetLuaScript("Health.lua")
        if(other:Name() == "Slime_Spawner")then
          damagePerTick = bossDmgPerTick
        end
        --Allow the fire to be on the enemies until dot is over
        enemScript = ObtainScript(other)
        if(enemScript ~= nil)then
          enemScript:SetVariable("toFlame", true)
          enemScript:SetVariable("onFlameTimer",0.5)
        end
        --Dmg over time as long as they are near the firecracker
        script:SetVariable("damage", damagePerTick)
        script:CallFunction("DealDamge")
      end
  end
  -- explosion damage
  if (timer3 > 0) then
    if(other:Tag() == "Slime")then
      script = other:GetLuaScript("Health.lua")
      if(other:Name() == "Slime_Spawner")then
        damageExplosion = bossDmgExplosion
      end
      --Create on fire particle on enemies
      enemScript = ObtainScript(other)
      if(enemScript ~= nil)then
        enemScript:SetVariable("toFlame", true)
        enemScript:SetVariable("onFlameTimer",0.5)
      end
      script:SetVariable("damage", damageExplosion)
      script:CallFunction("DealDamge")
    end
  end
end

function DistanceFromEnemy()
  slimeList = CurrentLayer():GetObjectsListByTag("Slime")
  slimeSize = #slimeList
  for i = 1, slimeSize
  do
    slimepos = slimeList[i]:GetComponent("Transform"):GetWorldPosition()
    if(slimepos:DistanceTo(owner_Transform:GetWorldPosition()) < AOEDist)then
      initActiveFlame(slimeList[i])
      DOT(slimeList[i])
    end
  end
end

function ObtainScript(other)
  enemyScript = other:GetLuaScript("Enemy_Spawner.lua")
  if(enemyScript ~= nil)then
    return enemyScript
  end
  enemyScript = other:GetLuaScript("Enemy_TourGuideBehaviour.lua")
  if(enemyScript ~= nil)then
    return enemyScript
  end
  enemyScript = other:GetLuaScript("Enemy_Kamikaze.lua")
  if(enemyScript ~= nil)then
    return enemyScript
  end
  enemyScript = other:GetLuaScript("EnemyBehavior.lua")
  if(enemyScript ~= nil)then
    return enemyScript
  end
  return nil  
end