-- VARIABLES ===================================================================
local disable = false

usesControllerMapping       = true
local usesPhysicsSystem     = true
local currentControllerMode = 0.0
local lookForward           = false
local GameLogic             = nil
local TekongScript          = nil
local ForceSetYVelocity     = true

-- [movement booleans]
local keyPad            = Vector3() -- (0 - 1 decides what keys are pressed)
local prevKeyPad        = Vector3()
local isRunning         = false
local prevGrounded      = false
local isGrounded        = false
--local wallClimbMode     = false
local currentPlayerMode = true -- true = shooting / false = trap building

--  [Audio booleans]
local moving            = false
local playSfx           = false
local walkSfxThreshold  = 0.3
local walkSfxTimer      = 0

-- [gameobjects / components]
local owner_meshRenderer       = nil
local owner_rigidbody          = nil
local owner_transform          = nil
local GO_playerModel           = nil
local playerModel_transform    = nil
local playerModel_meshAnimator = nil
local camera_Camera            = nil
local script_playerCamera      = nil

-- [values]
local playerModel_initialRotation = Vector3()
--local animation_jump   = "WeaponJump_MeshAnimationSet"
--local animation_walk   = "Weapon_Run_Mesh_01AnimationSet"
--local animation_idle   = "Idle_Mesh_03AnimationSet"

local animBookIdle        = "BookIdle"
local animBookStrafe      = "BookStrafe"
local animWeaponIdle      = "WeaponIdle"
local animWeaponJump      = "WeaponJump"
local animWeaponRun01     = "WeaponRun01"
local animWeaponRun02     = "WeaponRun02"
local animWeaponStrafe01  = "WeaponStrafe01"
local animWeaponStrafe02  = "WeaponStrafe02"
local animWeaponSwitch    = "WeaponSwitch"
local currentAnimation    = nil

local sfx_walk            = "SFX_PlayerWalk"
local sfx_walk_concrete   = {"Foot_Conc_ShoesRun_1", "Foot_Conc_ShoesRun_2", "Foot_Conc_ShoesRun_3", "Foot_Conc_ShoesRun_4",}
local sfx_walk_sand       = {"Foot_Sand_Run_1", "Foot_Sand_Run_2", "Foot_Sand_Run_3",}
local sfx_walk_grass      = {"Foot_Grass_Run_1", "Foot_Grass_Run_2", "Foot_Grass_Run_3",}
local sfx_walk_metal      = {"Foot_Steel_ShoesRun_1", "Foot_Steel_ShoesRun_2", "Foot_Steel_ShoesRun_3","Foot_Steel_ShoesRun_4", "Foot_Steel_ShoesRun_5",}
local sfx_jump            = {"SFX_PlayerJump1", "SFX_PlayerJump2","SFX_PlayerJump3","SFX_PlayerJump4",}

-- [If using physics system]
  force_walk    =  100.0
  force_run     =  200.0
  force_jump    = 800.0 -- gravity = -50
  maxSpeed_walk =   10.0
  maxSpeed_run  =   15.0
  halvedMaxSpeed_walk = 10.0
  halvedMaxSpeed_run  = 15.0
  local timer_checkGrounded = 0
  forced_Y_vel  = 12.0

-- [If not using physics system]
  speed_walk         = 10.0
  speed_run          = 30.0
  speed_acceleration = 1000.0
  speed_jump         = 200.0
  maxFallSpeed       = 50.0
  force_gravity      = 980.0
  
  local currVel  = Vector3()
  local currVelF = Vector3()
  local currVelR = Vector3()
  local currAcl  = Vector3()
  
  groundLevel = 1.0
  
-- [For Barbed Wire]
barbedDamageInterval = 0.1
local barbedTimer = 0.0
local barbedWireCollision = false
local damage_barbedwire = 0.02
-- [STUPID REASONS]
local callonce = true

-- [different grounds]
local curr_ground = 0
local prev_ground = -1
-- concrete = 0
-- sand = 1
-- grass = 2
-- metal = 3

-- FUNCTIONS ===================================================================
function Constructor()
  owner_rigidbody = owner:GetComponent("RigidBody")
  owner_transform = owner:GetComponent("Transform")
  
  GO_playerModel              = CurrentLayer():GetObject("PlayerModel")
  playerModel_transform       = GO_playerModel:GetComponent("Transform")
  playerModel_initialRotation = playerModel_transform:GetWorldRotation()
  playerModel_meshAnimator    = GO_playerModel:GetComponent("MeshAnimator")
  
  owner_rigidbody:SetGravityEnabled(usesPhysicsSystem)
  
  camera_GO           = CurrentLayer():GetObject("Camera")
  script_playerCamera = camera_GO:GetLuaScript("PlayerCamera.lua")
  camera_Camera       = camera_GO:GetComponent("Camera")
  GameLogic           = owner:GetLayer():GetObject("GameLogic")
  
  halvedMaxSpeed_walk = maxSpeed_walk * 0.5
  halvedMaxSpeed_run  = maxSpeed_run  * 0.5
  
  walkSfxTimer = walkSfxThreshold
  if(GameLogic ~= nil) then TekongScript = GameLogic:GetLuaScript("Event_Tekong.lua") end
end

function OnUpdate(dt)
  --For jump sfx
  prevGrounded = isGrounded
  
  if (callonce) then
    callonce = false
    
    isKeyboard = (currentControllerMode == 0)
    script_playerCamera:SetVariable("currentControllerMode", isKeyboard)
    script_playerCamera:CallFunction("ChangeCameraSettings")
  end
  
  -- if new input mode detected
  tmp = GetControllerInput()
  if (tmp ~= currentControllerMode) then
    currentControllerMode = tmp
    
    isKeyboard = (currentControllerMode == 0)
    script_playerCamera:SetVariable("currentControllerMode", isKeyboard)
    script_playerCamera:CallFunction("ChangeCameraSettings")
  end
  
  -- manual change in keyboard / controller
  if(IsKeyUp(KEY_I))then
    if (currentControllerMode == 0.0) then
      currentControllerMode = 1.0
    else
      currentControllerMode = 0.0
    end
    
    isKeyboard = (currentControllerMode == 0)
    script_playerCamera:SetVariable("currentControllerMode", isKeyboard)
    script_playerCamera:CallFunction("ChangeCameraSettings")
    ChangeControllerInput(currentControllerMode)
  end
  
  -- timer
  if (timer_checkGrounded > 0) then
    timer_checkGrounded = timer_checkGrounded - dt
  end
  
  -- Update player controller
  if (not disable) then
    if (usesPhysicsSystem) then
      if (usesControllerMapping) then
        UpdateAnimationInput  ()
        UpdatePlayerController()
      else
        UpdateKeypad    (dt     )
        UpdateForces    (dt     )
      end
    else
      isGrounded = owner_transform:GetWorldPosition():y() <= groundLevel
      UpdateKeypad      (dt     )
      UpdateCurrVelocity(dt     )
    end
    
    prevKeyPad = keyPad
  end
  
  -- UPDATE PLAYER MODEL
  if (playerModel_transform ~= nil) then
    rotation             = owner_transform:GetWorldRotation()
    idealrotation        = playerModel_initialRotation
    idealrotation.y      = 180.0 + rotation:y()
    playerModel_transform:SetWorldRotation(idealrotation)
    
    -- UPDATE PLAYER ANIMATION
    if (not isGrounded) then 
        PlayAnimationIfNotPlaying(animWeaponJump)
        moving = false
        walkSfxTimer = walkSfxThreshold
        if (prevGrounded ~= isGrounded) then
          --Go_Sound          = CreatePrefab("PlayAudioAndDie")
          --Sound_Transform   = Go_Sound:GetComponent("Transform")
          --Sound_Transform:SetWorldPosition(owner_transform:GetWorldPosition())
          --Sound_SoundScript = Go_Sound:GetLuaScript("PlayAudioAndDie.lua")
          --Sound_SoundScript:SetVariable("audioClipName", sfx_jump)
          --Sound_SoundScript:CallFunction("PlayAudio")
          
          AudioSystem_PlayAudioAtLocation(sfx_jump[RandomRangeInt(1, #sfx_jump)], owner_transform:GetWorldPosition())
          
        end
    else
      if (keyPad:x() == 0 and keyPad:z() == 0) then
        PlayAnimationIfNotPlaying(animWeaponIdle)
        moving = false
      else
        PlayAnimationIfNotPlaying(animWeaponRun01)
        moving = true        
      end
    end
  end
  
  -- PLAYER MOVEMENT SFX
  if (moving) then
    if (walkSfxTimer < 0) then
      --Go_Sound          = CreatePrefab("PlayAudioAndDie")
      --Sound_Transform   = Go_Sound:GetComponent("Transform")
      --Sound_Transform:SetWorldPosition(owner_transform:GetWorldPosition())
      --Sound_SoundScript = Go_Sound:GetLuaScript("PlayAudioAndDie.lua")
      --Sound_SoundScript:SetVariable("audioClipName", sfx_walk)
      --Sound_SoundScript:CallFunction("PlayAudio")
      
      AudioSystem_PlayAudioAtLocation(sfx_walk, owner_transform:GetWorldPosition())
      
      walkSfxTimer = walkSfxThreshold
    else
      walkSfxTimer = walkSfxTimer - dt
    end
  end
  
  -- Update player looking forward
  if (lookForward) then
    MakePlayerLookForward(dt)
  end
                                                                                -- Debug
                                                                                color_Vel = Color(0.0, 0.0, 1.0, 1.0)
                                                                                color_Acl = Color(1.0, 0.0, 0.0, 1.0)
                                                                                currVel   = owner_rigidbody:GetVelocity()
                                                                                currAcl   = owner_rigidbody:GetAcceleration()
                                                                                s         = owner_transform:GetWorldPosition()
                                                                                DebugDrawLine(s, s + currVel, color_Vel)
                                                                                DebugDrawLine(s, s + currAcl, color_Acl)
  
  -- Barbed wire damage
  if (barbedWireCollision) then
    if (barbedTimer > 0) then
      barbedTimer = barbedTimer - dt
    else
      barbedTimer = barbedDamageInterval
      script = owner:GetLuaScript("PlayerScript.lua")
      script:SetVariable("damage", damage_barbedwire)
      script:CallFunction("DealDamage")
    end
  end
  
  script = owner:GetLuaScript("PlayerScript.lua")
  if (script:GetVariable("IsDead")) then
    PlayAnimationIfNotPlaying("Death")
  end
end

function OnCollisionEnter(other)
  if(other:Name() == "Trap_BarbedWire") then
    --force_walk = force_walk * 0.5
    --force_run  = force_run  * 0.5
    --speed_walk = speed_walk * 0.5
    --speed_run  = speed_run  * 0.5
    maxSpeed_run  = halvedMaxSpeed_run
    maxSpeed_walk = halvedMaxSpeed_walk
    barbedWireCollision = true
    
  end
end

function OnCollisionPersist(other)
  --if (wallClimbMode) then
  --  isGrounded = true
  if (timer_checkGrounded <= 0) then
    -- concrete = 0
    -- sand = 1
    -- grass = 2
    -- metal = 3
    if (other:Tag() == "Ground") then
      isGrounded = true
      if(other:Name() == "concrete") then
        curr_ground = 0
        if(prev_ground ~= curr_ground) then
          sfx_walk = sfx_walk_concrete[RandomRangeInt(1, #sfx_walk_concrete)]
          prev_ground = curr_ground
        end
      elseif (other:Name() == "sand") then
        curr_ground = 1
        if(prev_ground ~= curr_ground) then
          sfx_walk = sfx_walk_sand[RandomRangeInt(1, #sfx_walk_sand)]
          prev_ground = curr_ground
        end
      elseif (other:Name() == "grass") then
        curr_ground = 2
        if(prev_ground ~= curr_ground) then
          sfx_walk = sfx_walk_grass[RandomRangeInt(1, #sfx_walk_grass)]
          prev_ground = curr_ground
        end
		
      end
      
    elseif (other:Tag() == "Jumpable") then
      isGrounded = true
	  if (other:Name() == "metal") then
        curr_ground = 3
        if(prev_ground ~= curr_ground) then
          sfx_walk = sfx_walk_metal[RandomRangeInt(1, #sfx_walk_metal)]
          prev_ground = curr_ground
        end
	  end
    end
  end
end

function OnCollisionExit(other)
  if(other:Name() == "Trap_BarbedWire") then
    ExitBarbedWire()
  end
end

function ExitBarbedWire()
  if (barbedWireCollision) then
    --force_walk = force_walk * 2
    --force_run  = force_run  * 2
    --speed_walk = speed_walk * 2
    --speed_run  = speed_run  * 2
    maxSpeed_run  = halvedMaxSpeed_run  * 2
    maxSpeed_walk = halvedMaxSpeed_walk * 2
    barbedWireCollision = false
    barbedTimer = barbedDamageInterval
  end
end
-- MOVEMENT ver1 (not using physics system) ====================================
function UpdateKeypad(dt)
  walk_U    = IsKeyDown(KEY_W     ) or  IsKeyPressed(KEY_W     )
  walk_D    = IsKeyDown(KEY_S     ) or  IsKeyPressed(KEY_S     )
  walk_L    = IsKeyDown(KEY_A     ) or  IsKeyPressed(KEY_A     )
  walk_R    = IsKeyDown(KEY_D     ) or  IsKeyPressed(KEY_D     )
  isRunning = IsKeyDown(KEY_LSHIFT) or  IsKeyPressed(KEY_LSHIFT)
  isJumping = IsKeyPressed(KEY_SPACE ) and isGrounded
  
  keyPad = Vector3()
  if (walk_U and not walk_D) then keyPad.z = -1.0                    end
  if (walk_D and not walk_U) then keyPad.z =  1.0                    end
  if (walk_L and not walk_R) then keyPad.x = -1.0                    end
  if (walk_R and not walk_L) then keyPad.x =  1.0                    end
  if (isJumping            ) then keyPad.y =  1.0 isGrounded = false end
end

function UpdateCurrVelocity(dt)
  -- Update acceleration in direction
  fAcl = owner_transform:ForwardVector() * keyPad:z() * speed_acceleration
  rAcl = owner_transform:RightVector()   * keyPad:x() * speed_acceleration
  currAclY  = currAcl:y()
  currAcl   = fAcl + rAcl
  currAcl.y = currAclY
  
  if (keyPad:y() == 1.0) then
    currAcl.y  = speed_jump
  end
  
  -- Forecefully change velocity based on key input or grounded
  if (keyPad:z() == 0.0 or currVelF:Dot(fAcl) < 0) then
    currVelF = Vector3()
  end
  
  if (keyPad:x() == 0.0 or currVelR:Dot(rAcl) < 0) then
    currVelR = Vector3()
  end
  
  if (isGrounded) then
    currAcl.y = 0.0
    currVel.y = 0.0
    
    owner_transform:SetWorldPosition("y", groundLevel)
  else
    currAcl.y = currAcl:y() - force_gravity * dt
  end
  
  -- Update velocity
  currVelF = currVelF + currAcl * dt
  currVelR = currVelR + currAcl * dt
                                                                                -- write(currVel)
  currVelY  = currVel:y() + currAcl:y() * dt
  currVel   = currVelR + currVelF
  currVel.y = currVelY
                                                                                -- write(currAcl)
   -- Clamp velocity
  if (keyPad:x() ~= 0.0 or keyPad:z() ~= 0.0) then
    xzSpeed           = Vector3( currVel:x(), 0.0, currVel:z())
    xzSpeedSquaredMag = xzSpeed:SquareLength()
                                                                                -- write(xzSpeedSquaredMag)
    speedToCaps = 0.0
    if (isRunning) then
      speedToCaps = speed_run
    else 
      speedToCaps = speed_walk
    end
                                                                                -- write(speedToCaps * speedToCaps)
    
    if (xzSpeedSquaredMag > (speedToCaps * speedToCaps) ) then
      xzSpeed = xzSpeed:Normalized() * speedToCaps
      currVel.x = xzSpeed:x()
      currVel.z = xzSpeed:z()
    end
  else
    currVel.x = 0
    currVel.z = 0
  end
                                                                                -- write(currVel)
  -- Clamp falling speed
  if (currVel:y() < -maxFallSpeed) then
    currVel.y = -maxFallSpeed
  end
                                                                                -- write(currVel)
  owner_rigidbody:SetVelocity(currVel)
end

-- MOVEMENT ver2 (using physics system) ========================================
function UpdateForces(dt)
  -- JUMP
  if (keyPad:y() == 1) then 
    curr   = owner_rigidbody:GetVelocity()
    curr.y = 0
    owner_rigidbody:SetVelocity(curr)
    
    curr   = owner_rigidbody:GetAcceleration()
    curr.y = 0
    owner_rigidbody:SetAcceleration(curr)
    
    owner_rigidbody:AddForce(owner_transform:UpwardVector() * force_jump)
  end
  
  -- Check if running or walking
  clampSpeed = maxSpeed_walk
  currForce  = force_walk
  if (isRunning) then
    clampSpeed = maxSpeed_run
    currForce  = force_run
  end
  
  -- Check for change in movement
  if (prevKeyPad:z() ~= keyPad:z()) then
    direction = -owner_transform:ForwardVector()
    
    curr   = owner_rigidbody:GetVelocity()
    reduce = curr:Project(direction)
    owner_rigidbody:SetVelocity(curr - reduce)
    
    curr   = owner_rigidbody:GetAcceleration()
    reduce = curr:Project(direction)
    owner_rigidbody:SetAcceleration(curr - reduce)
  end
  
  if (prevKeyPad:x() ~= keyPad:x()) then
    direction = -owner_transform:RightVector()
    
    curr   = owner_rigidbody:GetVelocity()
    reduce = curr:Project(direction)
    owner_rigidbody:SetVelocity(curr - reduce)
    
    curr   = owner_rigidbody:GetAcceleration()
    reduce = curr:Project(direction)
    owner_rigidbody:SetAcceleration(curr - reduce)
  end
  
 if (keyPad:x() == 0 and keyPad:z() == 0) then
    curr   = owner_rigidbody:GetVelocity()
    tmpY   = curr:y()
    curr   = Vector3(0, tmpY, 0)
    owner_rigidbody:SetVelocity(curr)
    
    curr   = owner_rigidbody:GetAcceleration()
    tmpY   = curr:y()
    curr   = Vector3(0, tmpY, 0)
    owner_rigidbody:SetAcceleration(curr)
 end
  
  -- Check for movement
  movement = Vector3(keyPad:x(), 0.0, keyPad:z())
  rotation = owner_transform:GetWorldRotation()
  movement = movement:Rotate("y", rotation:y())
  owner_rigidbody:AddForce(movement * currForce);
  
  write("ASD")
  -- Clamp Speed
  currVel   = owner_rigidbody:GetVelocity()
  currVelY  = currVel:y()
  currVel.y = 0
  if (currVel:Length() >= clampSpeed) then
    currVel = currVel:Normalized() * clampSpeed
    currVel.y = currVelY
    owner_rigidbody:SetVelocity(currVel)
  end
end

-- MOVEMENT ver3 (Using phyics system) =========================================
function UpdateAnimationInput()
  keyPad.z = ControllerAxis("Right") - ControllerAxis("Left")
  keyPad.x = ControllerAxis("Up"   ) - ControllerAxis("Down")
  if (ControllerDown("Jump") and isGrounded) then 
    keyPad.y =  1.0
  end
end

function UpdatePlayerController()
  -- VARIABLES
  cur_Force         = owner_rigidbody:GetForce()
  add_Force         = Vector3()
  max_MovementSpeed = maxSpeed_walk
  cur_MovementForce = force_walk
  isSprinting       = ControllerDown("Sprint")
  isJumping         = ControllerDown("Jump"  ) and isGrounded
  axisUp            = ControllerAxis("Up"    )
  axisDown          = ControllerAxis("Down"  )
  axisLeft          = ControllerAxis("Left"  )
  axisRight         = ControllerAxis("Right" )
  
  -- Determine player direction vectors
  VectorForward     = Vector3(0, 0, 0)
  VectorRight       = Vector3(0, 0, 0)
  if (currentPlayerMode) then
    VectorForward = owner_transform:ForwardVector()
    VectorRight   = owner_transform:RightVector()
  else
    VectorForward   = -camera_Camera:GetLookAt()
    VectorForward.y = 0
    VectorRight     = -VectorForward:Cross(Vector3(0, 1, 0))
  end
  
  -- CHECK RUN OR WALK
  if (isSprinting) then
    max_MovementSpeed = maxSpeed_run
    cur_MovementForce = force_run
  end
  
  -- JUMP
  if (isJumping) then
    if(ForceSetYVelocity) then 
      owner_rigidbody:SetYVelocity(forced_Y_vel)
    else
      jumpForce   = owner_transform:UpwardVector() * force_jump
    --jumpForce.y = jumpForce:y() - cur_Force:y()
      add_Force   = add_Force + jumpForce
    end
    isGrounded  = false
    timer_checkGrounded = 0.5
  end
  
  -- UP
  if (axisUp > 0) then
    upForce     = -VectorForward * cur_MovementForce * axisUp
    add_Force   = add_Force + upForce
  end
  
  -- DOWN
  if (axisDown > 0) then
    downForce   = VectorForward * cur_MovementForce * axisDown
    add_Force   = add_Force + downForce
  end
  
  -- NOT MOVING UP AND DOWN
  if (axisUp <= 0 and axisDown <= 0) then
    walkForce   = cur_Force
    walkForce.y = 0
    
    dot         = walkForce:Dot(-VectorForward)
    if (dot > 0) then
      walkForce   = walkForce:Project(-VectorForward)
      add_Force   = add_Force - walkForce
    elseif (dot < 0) then
      walkForce   = walkForce:Project(VectorForward)
      add_Force   = add_Force - walkForce
    else
      add_Force = add_Force
    end
  end
  
  -- Left
  if (axisLeft > 0) then
    leftForce   = -VectorRight * cur_MovementForce * axisLeft
    add_Force   = add_Force + leftForce
  end
  
  -- RIGHT
  if (axisRight > 0) then
    rightForce  = VectorRight * cur_MovementForce * axisRight
    add_Force   = add_Force + rightForce
  end
  
  -- NOT MOVING LEFT AND RIGHT
  if (axisLeft <= 0 and axisRight <= 0) then
    walkForce   = cur_Force
    walkForce.y = 0
    
    dot         = walkForce:Dot(VectorRight)
    if (dot > 0) then
      walkForce   = walkForce:Project(VectorRight)
      add_Force   = add_Force - walkForce
    elseif (dot < 0) then
      walkForce   = walkForce:Project(-VectorRight)
      add_Force   = add_Force - walkForce
    else
      add_Force = add_Force
    end
  end
  
  -- ADD FORCE
  owner_rigidbody:AddForce(add_Force)
  
  -- Check if player is in build mode and moving
  if (not currentPlayerMode and add_Force:SquareLength() > 0) then
    lookForward = true
  end
  
  -- Clamp Speed
  currVel   = owner_rigidbody:GetVelocity()
  currVelY  = currVel:y()
  currVel.y = 0
  if (currVel:Length() >= max_MovementSpeed) then
    currVel = currVel:Normalized() * max_MovementSpeed
    currVel.y = currVelY
    owner_rigidbody:SetVelocity(currVel)
  end
end

function MakePlayerLookForward (dt)
  ---- Get directions
  --playerForward   = -owner_transform:ForwardVector()
  --playerRight     =  owner_transform:RightVector()
  --cameraForward   = camera_Camera:GetLookAt()
  --cameraForward.y = 0
  --cameraForward   = cameraForward:Normalized()
  --
  ---- Compare
  --distance   = cameraForward - playerForward
  --distance.y = 0
  --if (distance:Length() <= 0.01) then
  --  lookForward = false
  --  return
  --end
  --
  --direction = 0.0
  --if (playerRight:Dot(distance) < 0) then
  --  direction = 1.0
  --else
  --  direction = -1.0
  --end
  --
  --playerRotation   = owner_transform:GetWorldRotation()
  --playerRotation.y = playerRotation:y() + direction * 750 * dt
  --owner_transform:SetWorldRotation(playerRotation)
  
  destY = script_playerCamera:GetVariable("currentRotationY")
  --destY = Mod(destY, 360)
  curr  = owner_transform:GetWorldRotation()
  --currY = Mod(curr:y(), 360)
  currY = curr:y()
  direction = FindFastestTuriningDirection(destY, currY)
  angle = destY - currY
  
  if (angle > 180) then
    angle = angle - 360
  elseif (angle < -180) then
    angle = angle + 360
  end
  
  angle = angle * direction
  if (angle <= 0.1) then
    lookForward = false
    curr.y = destY
    owner_transform:SetWorldRotation(curr)
    return
  end
  
  curr.y = currY + direction * angle * dt
  owner_transform:SetWorldRotation(curr)
end

function FindFastestTuriningDirection (target, curr)
  diff = target - curr;
  
  if(diff < 0) then
      diff = diff + 360
  end
  
  if(diff > 180) then
    return -1
  else
    return 1
  end
  
  return 0
end

-- ANIMATION ===================================================================
function PlayAnimationIfNotPlaying(newAnimation)
  script = owner:GetLuaScript("PlayerScript.lua")
  if (script:GetVariable("IsDead") and newAnimation ~= "Death") then
    return
  end
  
  if (currentAnimation == nil or currentAnimation ~= newAnimation) then
    write(newAnimation)
    currentAnimation = newAnimation
    --newAnimationSet = GetResource_Animation(newAnimation)
    --playerModel_meshAnimator:SetAnimation(newAnimationSet)
    playerModel_meshAnimator:Play(newAnimation)
  end
end

-- CAMERA CHANGE MODE (BUILDING TRAPS / SHOOTING) ==============================
function ChangeCameraMode()
  currentPlayerMode = not currentPlayerMode
end

-- PAUSE/RESUME ================================================================
function Pause()
  disable = true
  keyPad = Vector3()
  
  PlayAnimationIfNotPlaying(animWeaponIdle)
end

function Resume()
  disable = false;
end

function PlayWalkAnim()
  keyPad.x = 1
end

function PlayIdleAnim()
  keyPad = Vector3()
end
