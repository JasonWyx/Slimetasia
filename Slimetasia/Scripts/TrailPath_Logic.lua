-- VARIABLES ===================================================================
-- [components]
local owner_pathFinding   = nil
local owner_transform     = nil
GO_Core        = nil
Core_transform = nil
local isConstruct = false
--GlowObjects
local glowObj = nil
local currentTime = 0.0
local pathYpos = Vector3()
local interval = 15.0
pathFound = false
spawnIndex = 0
-- FUNCTIONS ===================================================================
function Constructor()
end
function MyConstructor()
  -- Find components
  owner_transform    = owner:GetComponent("Transform")
  owner_pathFinding  = owner:GetComponent("PathFinding")
  GO_Core      = CurrentLayer():GetObject("Core")
  -- Find other components
  Core_transform   = GO_Core:GetComponent("Transform")
  
  levelName        = PlayerPref_GetString      ("CurrentLevel")
  spawnPoints      = PlayerPref_GetVector3Array("SpawnPositions", levelName)
  owner_startpos = owner_transform:GetWorldPosition()
  spawnIndex = 1
  
   -- Set up path finding
  boolArr = PlayerPref_GetBoolArray("isPathFound")
  if(boolArr[spawnIndex] == false)then
    --Switch to base map(No obstacles)
    owner_pathFinding:ChangeLocalToBaseMap()
    pathFound = owner_pathFinding:FindPath(Core_transform:GetWorldPosition(),false)
    boolArr[spawnIndex] = pathFound
    PlayerPref_SetBoolArray("isPathFound", boolArr)
    if(pathFound)then
      write("FOUND VALID ORIGINAL PATH", spawnIndex)
      pathYpos = owner_transform:GetWorldPosition()
      myFirstPos = AISystem_GetStartPathByIndex(spawnIndex, 1)
      pathYpos.y = myFirstPos:y()
      ConstructGlowObj()
      --write("obtain starpath")
      owner_pathFinding:ChangeLocalToBaseMap()
    else
      --path no found!
      write("you wrong, impossible to go in unless base map has no valid path")
    end
    
  end
end

function OnUpdate(dt)
  if(not isConstruct)then
    MyConstructor()
    isConstruct = true
  end
  --Construct new Glow Object
  if(currentTime > interval)then
    currentTime = currentTime - interval
    ConstructGlowObj()
  end
  currentTime = currentTime + dt
  
end

function ConstructGlowObj()
  glowObj = CreatePrefab("Trail_GlowObject")
  glowObj:GetComponent("Transform"):SetWorldPosition(pathYpos)
  script = glowObj:GetLuaScript("TrailObject_Logic.lua")
  glowObj:SetParent(owner:GetID())
end


