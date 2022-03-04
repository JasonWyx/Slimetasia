-- VARIABLES ===================================================================

-- [Settings]
enableSelfDestroy         = true
local bulletParticles     = "bulletHitParticle"
local bulletDamage        = 0.2
local BossBulletDmg       = 0.1
local speed               = 5000
local lifespan            = 1
local lifespanTimer       = 0

-- [Components]
local owner_Transform  = nil
local owner_Rigidbody  = nil
local player_GO        = nil
local player_Transform = nil
local GO_Camera        = nil
local Camera_Transform = nil
local Camera_Camera    = nil

-- [Auto aim]
autoAim_enabled        = false
autoAim_targetPosition = Vector3(0, 0, 0)

-- FUNCTIONS ===================================================================
function Constructor()
  -- Get components
  owner_Transform  = owner:GetComponent("Transform")
  owner_Rigidbody  = owner:GetComponent("RigidBody")
  player_GO        = CurrentLayer():GetObject("Player")
  player_Transform = player_GO:GetComponent("Transform")
  GO_Camera        = CurrentLayer():GetObject("Camera")
  Camera_Transform = GO_Camera:GetComponent("Transform")
  Camera_Camera    = GO_Camera:GetComponent("Camera")
end

function OnUpdate(dt)
  UpdateLifeSpan(dt)
end

-- DEATH =======================================================================
function UpdateLifeSpan(dt)
  -- Reduce life
  lifespanTimer = lifespanTimer - dt
  if (lifespanTimer <= 0) then 
    Die()
  end
end

function Die()
  owner:Destroy()
end

function OnCollisionEnter(go)
  -- Ignore collision
  if (go:Name() == "Player" or go:Name() == "Bullet" or go:Name() == "Trap_BarbedWire" or go:Name() == "Slime_TourGuide_AreaEffect") then
    return
  end
  
  if(go:Tag() == "" or go:Tag() == "Coin" or go:Tag() == "Trap") then
    return
  end
  
  -- Particle effect
  particles = CreatePrefab(bulletParticles)
  particlesTransform = particles:GetComponent("Transform")
  particlesTransform:SetWorldPosition(owner_Transform:GetWorldPosition())
    
  -- Collide with enemy
  if (go:Tag() == "Slime") then
    script = go:GetLuaScript("Health.lua")
    if(go:Name() == "Slime_Spawner")then
      bulletDamage = BossBulletDmg
    end
    script:SetVariable("damage", bulletDamage)
    script:CallFunction("DealDamge")
    Die()
  end
  
  Die()
end

-- MOVEMENT ====================================================================
function MoveForward()
  Constructor()
  lifespanTimer = lifespan
  spawnPos      = nil

  -- Find player position and spawn pos
  PlayerPos        = player_Transform:GetWorldPosition()
  spawnPos         = PlayerPos
  spawnPos         = spawnPos + player_Transform:RightVector()   * 0.2
  spawnPos         = spawnPos - player_Transform:ForwardVector() * 0.5
  owner_Transform:SetWorldPosition(spawnPos)
  
  -- Find shoot direction
  currentRotation  = Vector3(90, 0, 0)
  direction        = Vector3(0, 0, 0)
  
  if (not autoAim_enabled) then
    --owner_Transform:SetWorldRotation(currentRotation + player_Transform:GetWorldRotation())
    cameraForward    = Camera_Camera:GetLookAt()
    cameraPos        = Camera_Transform:GetWorldPosition()
    endPos           = cameraPos + cameraForward * 200
    
    -- Offset player body
    cameraToPlayer   = PlayerPos - cameraPos
    offset           = cameraToPlayer:Project(cameraForward)
    offset           = offset + cameraForward * 2 -- Fix angle by moving forward
    
    -- Find a position from range
    hitInfo = RayCast(cameraPos + offset, cameraForward, 200, "Player", "NoBullet")
    if (hitInfo:GameObject() ~= nil) then
      endPos = hitInfo:Point()
    end
    
    -- Compute direction
    direction = endPos - spawnPos
  else
    --owner_Transform:SetWorldRotation(currentRotation + player_Transform:GetWorldRotation())
    direction = autoAim_targetPosition - spawnPos
  end
  
  direction = direction:Normalized()
  
  -- Move bullet
  owner_Rigidbody:AddForce(direction * speed)
  
  -- Rotate bullet according to direction
  angle = VectorAngleWorldAxis(-direction)
  rotationVector = owner_Transform:GetWorldRotation()
  rotationVector.y = angle
  owner_Transform:SetWorldRotation(rotationVector)
  
  -- Shooting sound
  AudioSystem_PlayAudioAtLocation("SFX_Shoot.ogg", spawnPos)
  
  --Go_Sound          = CreatePrefab("PlayAudioAndDie")
  --Sound_Transform   = Go_Sound:GetComponent("Transform")
  --Sound_Transform:SetWorldPosition(spawnPos)
  --Sound_SoundScript = Go_Sound:GetLuaScript("PlayAudioAndDie.lua")
  --Sound_SoundScript:SetVariable("audioClipName", "SFX_Shoot.ogg")
  --Sound_SoundScript:CallFunction("PlayAudio")
end