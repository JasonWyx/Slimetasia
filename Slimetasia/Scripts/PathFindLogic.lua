-- VARIABLES ===================================================================
reachedCore = false
isFollowPlayer = false
-- [components]
local owner_pathFinding   = nil
local owner_rigidBody     = nil
local owner_transform     = nil
-- [A* Search]
GO_Core        = nil
Core_transform = nil
PathSize = 0
local a_index = 1
AStarDir = Vector3()
owner_pathFinding = nil
attackBlock = false
isConstruct = false
--Enemy INFO
EnemySpeed    = 4.0
maxVelocity   = 4.0
owner_startpos = Vector3()
spawnIndex = 0
--whether using indv path or not
local isIndv = false
local finalPos = Vector3()
local enemyFromCoreDist = 6.0--Default

-- FUNCTIONS ===================================================================
function Constructor()
end
function MyConstructor()
  -- Find components
  owner_transform    = owner:GetComponent("Transform")
  owner_pathFinding  = owner:GetComponent("PathFinding")  
  owner_rigidBody    = owner:GetComponent("RigidBody")
  GO_Core      = CurrentLayer():GetObject("Core")
  -- Find other components
  Core_transform   = GO_Core:GetComponent("Transform")
  levelName        = PlayerPref_GetString      ("CurrentLevel")
  spawnPoints      = PlayerPref_GetVector3Array("SpawnPositions", levelName)
  owner_startpos = owner_transform:GetWorldPosition()
  spawnIndex  = 1
   -- Set up path finding
  --if(AISystem_GetPathChanged() == false)then
  --  --write("I am running on shared path", AISystem_GetPathChanged())
  --  pathFound = PlayerPref_GetBoolArray("isPathFound")
  --  if(pathFound[spawnIndex])then
  --   --write("Path Found test for wave-------------------!")
  --    --minus 1 as c++ script use index start from 0
  --    a_index    = 1
  --    PathSize   = AISystem_GetStartPathSize(spawnIndex)
  --    if(PathSize == 0)then
  --      ComputePath_Indv()
  --    else
  --      posAtIndex = AISystem_GetStartPathByIndex(spawnIndex, a_index)
  --    end
  --    tmpPos     = posAtIndex - owner_transform:GetWorldPosition()
  --    tmpPos.y   = 0.0
  --    AStarDir   = tmpPos:Normalize()
  --    AStarDir.y = 0.0
  --    finalPos = AISystem_GetStartPathByIndex(spawnIndex, PathSize)
  --    finalPos.y = 0
  --    --AISystem_SetPathChanged(true)
  --  else
  --    --path no found!
  --    write("RHEHRHERHEHREHRHER")
  --    FindPathWithBaseGrid()
  --  end
  --else
   -- write("I am running on indv path",AISystem_GetPathChanged())
    ComputePath_Indv()
 -- end
  kamiscript = owner:GetLuaScript("Enemy_Kamikaze.lua")
  lscript = owner:GetLuaScript("EnemyBehavior.lua")
  myRange = false
  if(lscript ~= nil)then
    myRange = lscript:GetVariable("isRanged")
  end
   enemyFromCoreDist = 6.0
  if(myRange)then
    enemyFromCoreDist = 9.0
    EnemySpeed    = 3.5
    maxVelocity   = 3.5
  end
  if(kamiscript ~= nil)then
    EnemySpeed    = 6.5
    maxVelocity   = 6.5
  end
end

function OnUpdate(dt)
  if(isConstruct == false)then
    MyConstructor()
    isConstruct = true
  end
end

-- FUNCTIONS RELATED TO MOVEMENT ===============================================
function AStarMove()
  AwayFromPlayer()
  -- Distance check to core, -100 is reached core
  if(DistanceCheck() == true or reachedCore)then
    return
  end
  myPosAtindex = GetPathAtIndex(spawnIndex,a_index )
  -- Walk to the end
  if(a_index < PathSize)then 
    distGrid = IgnoreY(myPosAtindex,owner_transform:GetWorldPosition())
    if (owner_rigidBody:GetVelocity():Length() > maxVelocity)then
      myVelY = owner_rigidBody:GetVelocity():y()
      idealVelocity = owner_rigidBody:GetVelocity():Normalize() * maxVelocity
      idealVelocity.y = myVelY
      owner_rigidBody:SetVelocity(idealVelocity)
    else
      --Set distance for overcrowding of enemies
      if(distGrid <= 3.0)then
        a_index = a_index + 1
        myPosAtindex = GetPathAtIndex(spawnIndex,a_index )
        tmpPos = myPosAtindex - owner_transform:GetWorldPosition() 
        tmpPos.y = 0.0
        AStarDir = tmpPos:Normalize()
        AStarDir.y = 0.0
        owner_rigidBody:SetVelocity(Vector3())
      else
        tmpPos = myPosAtindex - owner_transform:GetWorldPosition() 
        tmpPos.y = 0.0
        AStarDir = tmpPos:Normalize()
        AStarDir.y = 0.0
        owner_rigidBody:AddVelocity(AStarDir * EnemySpeed)
      end
    end
  else
    slimeVel = owner_rigidBody:GetVelocity()
    slimeVel.x = 0
    slimeVel.z = 0
    owner_rigidBody:SetVelocity(slimeVel)
    reachedCore = true
  end
end

-- Recomputes the A star when a blockade is placed this uses shared path
function ComputePath()
 -- write("ComputePatH called")
  pathFound = owner_pathFinding:FindPath(Core_transform:GetWorldPosition(),false)
  if(pathFound)then
    a_index = AISystem_NearestPathIndex(owner_transform:GetWorldPosition()) + 2
    attackBlock = false
    isFollowPlayer = false
    PathSize   = AISystem_GetStartPathSize(spawnIndex)
    myPosAtindex = AISystem_GetStartPathByIndex(spawnIndex, a_index)
    tmpPos     = myPosAtindex - owner_transform:GetWorldPosition()
    tmpPos.y   = 0.0
    AStarDir   = tmpPos:Normalize()
    AStarDir.y = 0.0
    AISystem_SetPathChanged(true)
  else
    write("Path Not Found3!")
    attackBlock = true
  end
end

--this uses non shared path
function ComputePath_Indv()
  --write("ComputePath_Indv called")
  isIndv = true
  pathFound = owner_pathFinding:FindPath(Core_transform:GetWorldPosition(),true)
  if(pathFound)then
    write("Path Found3!")
    attackBlock = false
    isFollowPlayer = false
    a_index = 1
    PathSize = owner_pathFinding:GetPathSize()
    myPosAtindex = GetPathAtIndex(spawnIndex,a_index )
    tmpPos     = myPosAtindex - owner_transform:GetWorldPosition()
    tmpPos.y   = 0.0
    AStarDir   = tmpPos:Normalize()
    AStarDir.y = 0.0
    finalPos = owner_pathFinding:GetPathByIndex(PathSize)
    finalPos.y = 0
  else
    write("Path Not Found3!")
    attackBlock = true
    FindPathWithBaseGrid()
  end
end

function GetPathAtIndex(myspwnIndex, pathIndex)
  if(isIndv)then
    PathSize = owner_pathFinding:GetPathSize()
    if(PathSize == 0 )then
      ComputePath_Indv()
    end
    return owner_pathFinding:GetPathByIndex(pathIndex)
  else
    PathSize = AISystem_GetStartPathSize(myspwnIndex)
    return AISystem_GetStartPathByIndex(myspwnIndex, pathIndex)
  end
end

function DistanceCheck()
  currePos = owner_transform:GetWorldPosition()
  currePos.y = 0
  distance = finalPos - currePos
  distLeft = distance:Length()
  distanceLength = distance:Length() 
  if (distanceLength < enemyFromCoreDist) then
    reachedCore = true
    slimeVel = owner_rigidBody:GetVelocity()
    slimeVel.x = 0
    slimeVel.z = 0
    owner_rigidBody:SetVelocity(slimeVel)
    return true
  end
  return false
end

function AwayFromPlayer()
  --activates if enemy is away from player after chasing player
  if( isFollowPlayer)then
    isPathFound = owner_pathFinding:FindPath(Core_transform:GetWorldPosition(),true)
    if(isPathFound)then
      a_index = 1
      PathSize = owner_pathFinding:GetPathSize()
      myPosAtindex = owner_pathFinding:GetPathByIndex(a_index)
      tmpPos = myPosAtindex - owner_transform:GetWorldPosition()
      tmpPos.y = 0.0
      AStarDir = tmpPos:Normalize()
      AStarDir.y = 0.0      
      write("Path Found")
      isIndv = true
    else
      write("PATH NO FOUND2!")
      attackBlock = true
      FindPathWithBaseGrid()
    end
    isFollowPlayer =  false
  end
end

function FindPathWithBaseGrid()
  isIndv = true
  owner_pathFinding:ChangeLocalToBaseMap()
  pfound = owner_pathFinding:FindPath(Core_transform:GetWorldPosition(),true)
  if(pfound)then
    a_index    = 1
    PathSize = owner_pathFinding:GetPathSize()
    myPosAtindex = owner_pathFinding:GetPathByIndex(a_index)
    tmpPos     = myPosAtindex - owner_transform:GetWorldPosition()
    tmpPos.y   = 0.0
    AStarDir   = tmpPos:Normalize()
    AStarDir.y = 0.0
    attackBlock = true
    owner_pathFinding:ChangeLocalToBaseMap()
    finalPos = owner_pathFinding:GetPathByIndex(PathSize)
    finalPos.y = 0
  else
    write("STILL  NO PATH, means base path doesnt exist valid path")
  end
end

function IgnoreY(posA, posB)
  posA.y = 0.0
  posB.y = 0.0
  return (posA - posB):Length()
end