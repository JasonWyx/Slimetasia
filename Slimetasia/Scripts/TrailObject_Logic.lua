-- VARIABLES ===================================================================
-- [components]
local owner_rigidBody     = nil
local owner_transform     = nil
-- [A* path]
local MyPathSize = 0
local a_index = 1
AStarDir = Vector3()
--Enemy INFO
EnemySpeed    = 5.5
maxVelocity   = 5.5
GO_Core      = nil
Core_transform = nil
isConstruct = false
local finalPos = Vector3()

function Constructor()
  -- Find components
  owner_transform    = owner:GetComponent("Transform")
  owner_rigidBody    = owner:GetComponent("RigidBody")
end

function MyConstructor()
  levelName        = PlayerPref_GetString      ("CurrentLevel")
  spawnPoints      = PlayerPref_GetVector3Array("SpawnPositions", levelName)
  owner_startpos = owner_transform:GetWorldPosition()
  spawnpt = owner:GetParent()
  spawnIndex = 1

  spawn_pathfind = nil
  
  if(spawnpt ~= nil)then
    spawn_pathfind = spawnpt:GetComponent("PathFinding")
  end
  if(spawn_pathfind ~= nil)then
    MyPathSize = AISystem_GetStartPathSize(spawnIndex)
    finalPos = AISystem_GetStartPathByIndex(spawnIndex, MyPathSize)
    finalPos.y = 0
   -- write("I AM IN TRAILOBJECT~~~~~~", spawnIndex)
  end
  GO_Core = CurrentLayer():GetObject("Core")
  Core_transform   = GO_Core:GetComponent("Transform")
  --write("I AM IN TRAILOBJECT")
end

function OnUpdate(dt)
  if(not isConstruct)then
    MyConstructor()
    isConstruct = true
  end
  
  if(MyPathSize ~= 0)then
    AStarMove()
  end
  if(Core_transform:GetWorldPosition():DistanceTo(owner_transform:GetWorldPosition()) < 3.0)then
    owner:Destroy()
  end
end
-- FUNCTIONS RELATED TO MOVEMENT ===============================================
function AStarMove()
  -- Distance check to core
  currePos = owner_transform:GetWorldPosition()
  currePos.y = 0
  distance = finalPos - currePos
  distanceLength = distance:Length()
  if (distanceLength < 10.0) then
    --write("REACHED")
    owner:Destroy()
    return
  end
  posAtIndex = AISystem_GetStartPathByIndex(spawnIndex, a_index)
  if(a_index < MyPathSize)then 
  distGrid = (posAtIndex - owner_transform:GetWorldPosition()):Length()
  if (owner_rigidBody:GetVelocity():Length() > maxVelocity)then
   idealVelocity = owner_rigidBody:GetVelocity():Normalize() * maxVelocity
   idealVelocity.y = owner_rigidBody:GetVelocity():y()
   owner_rigidBody:SetVelocity(idealVelocity)
  else
   --Set distance for overcrowding of enemies
   if(distGrid <= 3.0)then
     a_index = a_index + 1
     owner_rigidBody:SetVelocity(Vector3())
   else
     tmpPos = posAtIndex - owner_transform:GetWorldPosition() 
     AStarDir = tmpPos:Normalize()
     owner_rigidBody:AddVelocity(AStarDir * EnemySpeed)
   end
  end    
  else
  owner:Destroy()
  end
end