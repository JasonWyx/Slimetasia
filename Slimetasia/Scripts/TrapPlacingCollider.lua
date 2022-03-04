-- VARIABLES ===================================================================
currentCollidedID    = 0
collidedCount = 0
local GO           = nil
local MESHRENDERER = nil
local GO_Player = nil
local myBOOLEAN = false
validPlacement = true

local PlayerTrapPlacingScript = nil


                                                                                local scriptOk = true

-- FUNCTIONS ===================================================================
function Constructor()
  GO = owner
  MESHRENDERER = GO:GetComponent("MeshRenderer")
                                                                                if (MESHRENDERER == nil) then scriptOk = false write("Trap Placing : Missing Mesh renderer") end
                                                                                
                                                                                if (scriptOk) then
                                                                                  write("Trap Placing : pass construction")
                                                                                else
                                                                                  write("Trap Placing : fail construction")
                                                                                end
  GO_Player = CurrentLayer():GetObject("Player")
  
  PlayerTrapPlacingScript = GO_Player:GetLuaScript("PlayerTrapPlacing.lua")
end

function OnUpdate(dt)
                                                                                if (scriptOk) then
  -- Get current trap
  currTrap = PlayerTrapPlacingScript:GetVariable("trapSelected")
  
  -- Location is empty
  if (collidedCount <= 0) then
    collidedCount = 0
    
    -- Check if placing in grid
    placeInSpecificGrids = PlayerTrapPlacingScript:GetVariable("onlyPlaceInGrid")
    foundSpawnPoint      = PlayerTrapPlacingScript:GetVariable("Placement_Allowed")
    
    if ((placeInSpecificGrids and foundSpawnPoint) or (not placeInSpecificGrids)) then
      if (currTrap ~= 4) then
        MESHRENDERER:SetColor(Color(0, 1, 0, 1))
      else
        MESHRENDERER:SetColor(Color(1, 0, 0, 1))
      end
      validPlacement = true
      myBOOLEAN = true
    else
      MESHRENDERER:SetColor(Color(1, 0, 0, 1))
      validPlacement = false
      myBOOLEAN = false
    end
  -- Location is full
  else
    if (currTrap == 4) then
      MESHRENDERER:SetColor(Color(0, 1, 0, 1))
    else
      MESHRENDERER:SetColor(Color(1, 0, 0, 1))
    end
    
    validPlacement = false
  end
  
  if(myBOOLEAN == true and currTrap ~= 4) then
    resourceScript = GO_Player:GetLuaScript("PlayerResourceManagement.lua")
    resourceAmt = resourceScript:GetVariable("resources")
    trapPlacing = GO_Player:GetLuaScript("PlayerTrapPlacing.lua")
    trapCost = trapPlacing:GetVariable("currentTrapCost")
    if (resourceAmt >= trapCost) then
      MESHRENDERER:SetColor(Color(0, 1, 0, 1))
    else
      MESHRENDERER:SetColor(Color(1, 0, 0, 1))
    end
  end

  collidedCount = 0
  currentCollided = 0
  myBOOLEAN = false
                                                                                end
end

--function OnCollisionEnter(go)
--  if (go:Tag() == "Player" or go:Name() == "Slime" or go:Tag() == "Ground" or go:Name() == "waypoint" ) then
--    collidedCount = collidedCount
--  else
--  write("ENTER : ", go:Name(), collidedCount)
--    collidedCount = collidedCount + 1
--  end
--end
--
--function OnCollisionExit(go)
--  if (go:Tag() == "Player" or go:Name() == "Slime" or go:Tag() == "Ground" or go:Name() == "waypoint" ) then
--    collidedCount = collidedCount
--  else
--    write("EXIT : ", go:Name(), collidedCount)
--    collidedCount = collidedCount - 1
--  end
--end

function OnCollisionPersist(go)
  if (go:Tag()  == "Player"      or 
      go:Tag()  == "Ground"      or 
      go:Tag()  == "Bullet"      or 
      go:Name() == "Slime"       or 
      go:Name() == "TrapPlacing" or 
      go:Name() == "waypoint" ) then
    collidedCount = collidedCount
  else
-----  write("ENTER : ", go:Name(), collidedCount)
    collidedCount = collidedCount + 1
    
    if (go:Tag() == "Trap") then
      currentCollidedID = go:GetID()
      write(currentCollidedID)
    end
  end
end