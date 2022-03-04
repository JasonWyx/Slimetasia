-- VARIABLES ===================================================================
local disable = false

-- [Bullet prefab]
local bulletPrefabName = "Bullet"

-- [Player script]
local Player_PlayerScript = nil
local Player_Transform    = nil
local GO_Camera           = nil
local Camera_Transform    = nil
local Camera_Camera       = nil

-- [Shooting timer]
local shootInterval = 0.2
local shootTimer = 0

-- [Auto aim]
local useAutoAim = true
local autoAim_target = nil
local autoAim_mode = 1 -- 1 (Ratchet and Clank) 2 (tab to change target)
local autoAim_Radius = 5.0
local slime_Height = Vector3(0.0, 1, 0)

-- [FOR UI]
lookingAtEnemy = false

-- FOR POPup
popupScript = nil

-- FUNCTIONS ===================================================================
function Constructor()
  -- Get player varaiables
  Player_PlayerScript = owner:GetLuaScript("PlayerScript.lua")
  Player_Transform    = owner:GetComponent("Transform")
  GO_Camera           = CurrentLayer():GetObject("Camera")
  Camera_Transform    = GO_Camera:GetComponent("Transform")
  Camera_Camera       = GO_Camera:GetComponent("Camera")
  popupScript         = owner:GetLuaScript("shoot_UI.lua")
end

function OnUpdate(dt)
  -- If script is disabled do nothing
  if(disable) then
    return
  end
  
  -- Auto aim target
  if (useAutoAim) then
    -- Check if target is still alive in the scene
    if (CheckTargetStatus()) then
      autoAim_target = nil
    end
    
    -- Mode 1
    if (autoAim_mode == 1) then
      if (ControllerPress("SelectPrev") or ControllerPress("SelectNext")) then 
        FindNewAutoAimTarget()
      end
      
      if (autoAim_target == nil) then 
        FindNewAutoAimTarget()
      elseif (not CheckInRadius(autoAim_target)) then
        FindNewAutoAimTarget()
      end
      
      lookingAtEnemy = autoAim_target ~= nil
      
    -- Mode 2
    elseif (autoAim_mode == 2) then
      write("AUTO AIM MODE : 2 - WIP")
    end
  end
  
  -- Check if player can shoot and is in shoot mode
  if (shootTimer <= 0) then
    if (Player_PlayerScript:GetVariable("currentMode")) then
      if(ControllerDown("Shoot")) then
        if(popupScript ~= nil) then popupScript:CallFunction("Start") end
        shootTimer = shootInterval
        ShootWithAssist()
      end
    end
  else
    shootTimer = shootTimer - dt
  end
end

-- SHOOT =======================================================================
function Shoot()
  newBullet       = CreatePrefab(bulletPrefabName)    
  newBulletScript = newBullet:GetLuaScript("PlayerBullet.lua")
  newBulletScript:CallFunction("MoveForward")
end

function ShootWithAssist()
  if (not useAutoAim or autoAim_target == nil) then
    Shoot()
    return
  end
  
  autoAim_targetTransform = autoAim_target:GetComponent("Transform")
  autoAim_targetPosititon = autoAim_targetTransform:GetWorldPosition() + slime_Height
  
  newBullet       = CreatePrefab(bulletPrefabName)
  newBulletScript = newBullet:GetLuaScript("PlayerBullet.lua")
  newBulletScript:SetVariable("autoAim_enabled", true)
  newBulletScript:SetVariable("autoAim_targetPosition", autoAim_targetPosititon)
  newBulletScript:CallFunction("MoveForward")
end

function CheckInRadius(target)
  -- Check distance from target
  shootPosition    = Player_Transform:GetWorldPosition()
  shootForward     = Player_Transform:ForwardVector()
  target_Transform = target:GetComponent("Transform")
  targetPosition   = target_Transform:GetWorldPosition() + slime_Height
  vectorToTarget   = targetPosition - shootPosition
  vectorToTarget2D = vectorToTarget - vectorToTarget:Project(shootForward)
  distance         = vectorToTarget2D:Length()
  
  -- Check direction
  dot = vectorToTarget:Dot(shootForward)
  if (dot > 0.0) then
    return false
  end
  
  -- Color
  red    = Color(1.0, 0.0, 0.0, 1.0)
  green  = Color(0.0, 1.0, 0.0, 1.0)
  blue   = Color(0.0, 0.0, 1.0, 1.0)
  yellow = Color(1.0, 1.0, 0.0, 1.0)
  
  -- Target if within radius
  if (distance <= autoAim_Radius) then
    -- check if target is blocked by anything
    hitInfo = RayCast(shootPosition, vectorToTarget, distance, "Player", "Bullet", "Coin", "Trap")
    
    -- If there is nothing inbetween the target and player
    if (hitInfo:GameObject() == nil or hitInfo:GameObject():GetID() == target:GetID()) then
      
      -- There is a current target
      if (autoAim_target ~= nil) then
        -- Target is the current autoAim_target
        if (autoAim_target:GetID() == target:GetID()) then
          DebugDrawLine(shootPosition, targetPosition, green)
        -- Target is another target
        else
          DebugDrawLine(shootPosition, targetPosition, blue)
        end
      -- No current autoAim_target
      else
        DebugDrawLine(shootPosition, targetPosition, green)
      end
      
      return true
    -- There is something inbetween the target and player
    else
      DebugDrawLine(shootPosition, targetPosition, yellow)
    end
  -- Target is out of range
  else
    DebugDrawLine(shootPosition, targetPosition, red)
  end
  
  return false
end

function FindNewAutoAimTarget()
  -- Get all enemies in screen
  enemies             = CurrentLayer():GetObjectsListByTag("Slime")
  enemiesInRange      = {}
  enemiesInRangeIndex = 0
  
  if (#enemies > 0) then
    for i = 1, #enemies do
      -- Check if enemy is in radius
      if (CheckInRadius(enemies[i])) then
        if (autoAim_target == nil or enemies[i]:GetID() ~= autoAim_target:GetID()) then
          enemiesInRangeIndex = enemiesInRangeIndex + 1
          enemiesInRange[enemiesInRangeIndex] = enemies[i]
        end
      end
    end
  end
  
  -- Get a random index from the enemies in range
  if (enemiesInRangeIndex == 0) then
    autoAim_target = nil
  elseif (enemiesInRangeIndex == 1) then
    autoAim_target = enemiesInRange[1]
  else
    enemiesInRangeIndex = RandomRange(1, enemiesInRangeIndex)
    enemiesInRangeIndex = Round(enemiesInRangeIndex)
    --write(enemiesInRangeIndex, #enemiesInRange)
    autoAim_target      = enemiesInRange[enemiesInRangeIndex]
  end
end

function CheckTargetStatus()
  if (autoAim_target == nil) then
    return false
  end
  
  enemies = CurrentLayer():GetObjectsListBySubName("Slime")
  for i = 1, #enemies do
    if (autoAim_target:GetID() == enemies[i]:GetID()) then
      return false
    end
  end
  
  return true
end

-- WIP
function FindNextTargetOnScreen()
  -- Get all enemies in screen
  enemies             = CurrentLayer():GetObjectsListBySubName("Slime")
  enemiesInRange      = {}
  enemiesInRangeIndex = 1
end

-- PAUSE/RESUME ================================================================
function Pause()
  disable = true
end

function Resume()
  disable = false;
end
