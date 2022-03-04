-- VARIABLES ===================================================================
-- Amount used in place of passing arguments to a function
gold = 2.0

-- For rotation
local rotationSpeed = 1.0
local COIN_TRANSFORM = nil
local COIN_RIGIDBODY = nil

-- Magnet
local timer              = 0
local timeToReachPlayer  = 3
local magnetDistance     = 5
local player_Transform   = nil
local flyingToPlayer = false
local coinPos        = Vector3()

-- FUNCTIONS ===================================================================
function Constructor()
  player_GO        = CurrentLayer():GetObject("Player")
  player_Transform = player_GO:GetComponent("Transform")
  
  COIN_TRANSFORM = owner:GetComponent("Transform")
  COIN_RIGIDBODY = owner:GetComponent("Rigidbody")
  COIN_RIGIDBODY:SetGhost(true)
end

function OnUpdate(dt)
  -- Check if player is near to the coin
  playerPos = player_Transform:GetWorldPosition()
  coinPos = COIN_TRANSFORM:GetWorldPosition()
  
  -- If near enough to coin
  if (not flyingToPlayer) then
    distVec = playerPos - coinPos
    if (distVec:Length() <= magnetDistance) then
      flyingToPlayer = true
    end
  -- Flying to player
  else
    timer  = timer + dt
    newPos = Vector3Lerp(coinPos, playerPos, timer / timeToReachPlayer)
    COIN_TRANSFORM:SetWorldPosition(newPos)
  end
end

function OnCollisionEnter(other)
  if (other:Name() == "Player") then
    script = other:GetLuaScript("PlayerResourceManagement.lua")
    if (script ~= nil) then
      script:SetVariable("amount", gold)
      script:CallFunction("AddGold")
      owner:Destroy()
    else
      write("PlayerResourceManagement luascript not found in Player!")
    end
  end
end
