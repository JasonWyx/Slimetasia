-- VARIABLES ===================================================================
local callOnce    = true
local deathArray  = {}
local index       = 1
local clearIndex  = 1

-- FUNCTIONS ===================================================================
function Constructor()
end

function OnUpdate(dt)
  if (callOnce) then
    callOnce = false
    
    trans = owner:GetComponent("Transform")
    pos = trans:GetWorldPosition()
    pos.y = 0.6
    trans:SetWorldPosition(pos)
  end
  
  while(clearIndex ~= index) do
    --deathArray[clearIndex]:Destroy()
    healthScript = deathArray[clearIndex]:GetLuaScript("Health.lua")
    health = healthScript:GetVariable("health")
    healthScript:SetVariable("damage", health)
    healthScript:CallFunction("DealDamge")
    clearIndex = clearIndex + 1
  end
end

function OnCollisionEnter(go)
  if (go:Tag() == "Slime") then
    deathArray[index] = go
    index = index + 1
  end
end

function OnCollisionPersist(go)
  if (go:Tag() == "Slime") then
    deathArray[index] = go
    index = index + 1
  end
end