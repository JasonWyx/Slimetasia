-- VARIABLES ===================================================================
spawnTimer = 5

local ratSpawned = false
-- FUNCTIONS ===================================================================
function OnUpdate(dt)
  if (ratSpawned) then
    script = rat:GetLuaScript("RatMovement.lua")
    script:SetVariable("startPoint", owner:GetComponent("Transform"):GetWorldPosition())
    script:SetVariable("startConstruct", true)
    ratSpawned = false
  end
  
  if (spawnTimer > 0) then 
    spawnTimer = spawnTimer - dt
  else
    rat = CreatePrefab("Rat")
    ratSpawned = true
    spawnTimer = RandomRange(3, 6)
  end
  
  
end