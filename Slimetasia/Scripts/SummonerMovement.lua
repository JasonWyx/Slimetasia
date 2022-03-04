-- VARIABLES ===================================================================
targetPos = Vector3()
spawnerOwner = nil

pathFound = false

-- [components]
local owner_pathFinding   = nil
local owner_transform     = nil
local owner_rigidBody     = nil

local isConstruct = false
AStarDir = Vector3()

-- [A* path]
local a_index = 1
AStarDir = Vector3()
--Enemy INFO
ParticleSpeed     = 5.0
maxVelocity       = 20.0
myMoveIndex       = 2
local spawnPoints = nil
local PathSize    = 0
reachedEnd        = false
local finalPos = Vector3()
-- FUNCTIONS ===================================================================
function Constructor()
end

function MyConstructor()
  write("MyCtor called")
  -- Find components
  owner_transform    = owner:GetComponent("Transform")
  owner_rigidBody    = owner:GetComponent("RigidBody")
  owner_pathFinding  = owner:GetComponent("PathFinding")
  
  --Switch to base map(No obstacles)
  owner_pathFinding:ChangeLocalToBaseMap()
  levelName        = PlayerPref_GetString      ("CurrentLevel")
  spawnPoints      = PlayerPref_GetVector3Array("SpawnPositions", levelName)
  owner_startpos = owner_transform:GetWorldPosition()
  --Uses indv pathfind
  pathFound = owner_pathFinding:FindPath(spawnPoints[myMoveIndex], true)
	if(pathFound)then
    write("Path Found for summ move!")
    PathSize = owner_pathFinding:GetPathSize()
    write("size: ", PathSize)
    myPosAtindex = owner_pathFinding:GetPathByIndex(a_index)
    tmpPos     = myPosAtindex - owner_startpos
    tmpPos.y   = 0.0
    AStarDir   = tmpPos:Normalize()
    AStarDir.y = 0.0
    owner_pathFinding:ChangeLocalToBaseMap()
    finalPos = owner_pathFinding:GetPathByIndex(PathSize)
    finalPos.y = 0
  else
    write("Base map path got problem! either start or end is wrong!")
  end
  isConstruct = true
end

function OnUpdate(dt)
  if(isConstruct)then
    if(PathSize ~= 0)then
      AStarMove()
    end
  end
  
end

-- FUNCTIONS RELATED TO MOVEMENT ===============================================
function AStarMove()
  -- Distance check to end pt
  if(DistanceCheck() == true)then
    owner:Destroy()
  end
  myPosAtindex = owner_pathFinding:GetPathByIndex(a_index)
  -- Walk to the end
  if(a_index < PathSize)then 
   distGrid = (myPosAtindex - owner_transform:GetWorldPosition()):Length()
   if (owner_rigidBody:GetVelocity():Length() > maxVelocity)then
     idealVelocity = owner_rigidBody:GetVelocity():Normalize() * maxVelocity
     idealVelocity.y = owner_rigidBody:GetVelocity():y()
     owner_rigidBody:SetVelocity(idealVelocity)
   else
     --Set distance for overcrowding of enemies
     if(distGrid <= 2.0)then
       a_index = a_index + 1
       owner_rigidBody:SetVelocity(Vector3())
     else
       tmpPos = myPosAtindex - owner_transform:GetWorldPosition() 
       AStarDir = tmpPos:Normalize()
       owner_rigidBody:AddVelocity(AStarDir * ParticleSpeed)
     end
   end
  else
    owner:Destroy()
  end
end

function DistanceCheck()
  currePos = owner_transform:GetWorldPosition()
  currePos.y = 0
  distance = finalPos - currePos
  distanceLength = distance:Length()
  if (distanceLength < 3.0) then
    --write("REACHED")
    reachedEnd = true
    myVel = owner_rigidBody:GetVelocity()
    myVel.x = 0
    myVel.z = 0
    owner_rigidBody:SetVelocity(myVel)
    return true
  end
  return false
end

