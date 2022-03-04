-- VARIABLES ===================================================================

-- [components]
local owner_pathFinding   = nil
local owner_transform     = nil
local owner_rigidBody     = nil

local isConstruct = false

--Enemy INFO
ParticleSpeed     = 20
myMoveIndex       = 2
local telePoints  = nil
local finalPos = Vector3()
local currIndex = 1
isReverse = false
local direction = Vector3()
local sizeTele  = 0
local Spawner   = nil
local spawnerScript = nil
moveIndex = 1

local moveDelayTimer = 20.0
--Amount of time it delays for start and then end
DelayAmount          = 3.0
--To Delay when it reaches each checkpoint
DelayYet = false
local moveVecNow = Vector3(0.0,1.0,0.0)
-- FUNCTIONS ===================================================================
function Constructor()
end

function MyConstructor()
  -- Find components
  write("MyConstructor is called")
  owner_transform    = owner:GetComponent("Transform")
  owner_rigidBody    = owner:GetComponent("RigidBody")
  Spawner = CurrentLayer():GetObject("Slime_Spawner")
  if(Spawner ~= nil) then
    spawnerScript = Spawner:GetLuaScript("Enemy_Spawner.lua")
  end
  
  levelName        = PlayerPref_GetString      ("CurrentLevel")
  --Take note in case of future levels?
  if(levelName == "Level_Changi")then
    telePathName = PlayerPref_GetStringArray("NamesWaypoints", levelName)
    nextPos = telePathName[ToInt(moveIndex)]
    telePoints       = PlayerPref_GetVector3Array(nextPos, levelName)
  else
    telePoints       = PlayerPref_GetVector3Array("TeleportTrail", levelName)
  end
  owner_startpos   = owner_transform:GetWorldPosition()
  sizeTele  = #telePoints
  --Uses waypoints
  if(isReverse)then
    currIndex = sizeTele - 1    
    finalPos  = telePoints[1]
  else
    currIndex = 2    
    finalPos  = telePoints[sizeTele]
  end
  direction = telePoints[currIndex] - owner_startpos
  direction = direction:Normalized()
  isConstruct = true  
  
  particle = owner:GetComponent("ParticleEmitter_Circle")
  particle:AddAttractor(owner:GetID())
  --particle:SetEmitRate(2.5)
  owner:GetComponent("Attractor"):SetForce(1)
  
  --script = owner:GetLuaScript("ParticleEmission.lua")
  --if (script ~= nil) then
  --  script:SetVariable("interval", 0.2)
  --  script:SetVariable("emitRate", 2500)
  --  script:SetVariable("speed", 10)
  --else
  --  write("ParticleEmission.lua is missing")
  --end
end

function OnUpdate(dt)
  if(isConstruct)then
    if (moveDelayTimer < DelayAmount) then
      moveDelayTimer = moveDelayTimer + dt
      MoveIdle(dt)
    else
      --:GetComponent("Attractor"):SetForce(0.15)
      MoveToEnd()
    end
  end
end

function MoveIdle(dt)
  currVel = owner_rigidBody:GetVelocity()
  owner_rigidBody:AddForce( moveVecNow* 20.0)
  if(currVel:y() <=  -7.0)then
  moveVecNow = Vector3(0.0,1.0,0.0)
  elseif(currVel:y() >= 7.0)then
  moveVecNow = Vector3(0.0,-1.0,0.0)
  end
end

-- FUNCTIONS RELATED TO MOVEMENT ===============================================
function MoveToEnd()
  -- Distance check to end pt
  if(DistanceCheck() == true)then
    if(not DelayYet)then
      moveDelayTimer = 0.0
      moveVecNow = Vector3(0.0,1.0,0.0)
      owner_rigidBody:AddForce( moveVecNow* 20.0)
      DelayAmount = 15.0
      DelayYet = true
      return
    end
    if(spawnerScript ~=nil)then
      spawnerScript:CallFunction("IdleEnd")
    end
    owner:Destroy()
    return
  end
  ChangeWayPoint()
  owner_rigidBody:SetVelocity(direction * ParticleSpeed)
end

function DistanceCheck()
  currePos = owner_transform:GetWorldPosition()
  distance = finalPos - currePos
  distanceLength = distance:Length()
  if (distanceLength < 2.0) then
    myVel = owner_rigidBody:GetVelocity()
    myVel.x = 0
    myVel.z = 0
    owner_rigidBody:SetVelocity(myVel)
    return true
  end
  return false
end

function ChangeWayPoint()
  currePos = owner_transform:GetWorldPosition()
  distance = telePoints[currIndex] - currePos
  distanceLength = distance:Length()
  direction = telePoints[currIndex] - currePos
  direction = direction:Normalized()
  --!NOTE! this distance cant be more than the distancecheck 
  --distancelength
  if (distanceLength < 1.0) then
    if(isReverse)then
      currIndex = currIndex - 1
    else
      currIndex = currIndex + 1
    end
    owner_rigidBody:SetVelocity(Vector3())
    direction = telePoints[currIndex] - currePos
    direction = direction:Normalized()
  end
end