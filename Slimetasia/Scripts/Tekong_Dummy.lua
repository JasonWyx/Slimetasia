-- VARIABLES ===================================================================

local eventsScript     = nil
local healthScript     = nil
local timer            = 0
local shakeDuration    = 2
local TRANSFORM        = nil
local originalRotation = Vector3()
local direction        = 1
local maxAngle         = 5
local currentAngle     = 0
local speed            = 20


local isConstruct  = false
local dead         = false

-- FUNCTIONS ===================================================================
--Note: Constructor might call multiple copies, hence do not create stuff in
--Constructor
function Constructor()

end

--ONLY CALLS ONCE
function MyConstructor()
  eventsGO     = CurrentLayer():GetObject("GameLogic")
  eventsScript = eventsGO:GetLuaScript("Event_Tekong.lua")
  healthScript = owner:GetLuaScript("Health.lua")
  healthScript:SetVariable("destroyOndeath", false)
  
  TRANSFORM        = owner:GetComponent("Transform")
  originalRotation = TRANSFORM:GetWorldRotation()
end

function OnUpdate(dt)
  if(isConstruct == false)then
    MyConstructor()
    isConstruct = true
  end
  
  currentHealth = healthScript:GetVariable("health")
  if(currentHealth <= 0 and not dead) then
     Die()
  end
  
  if (timer > 0) then
    timer = timer - dt
    
    -- Move shake angle
    change       = direction * speed * dt
    currentAngle = currentAngle + change
    
    -- Check if reach
    if (direction > 0 and currentAngle >= maxAngle) then
      direction    = -1
      currentAngle = maxAngle
    elseif (direction < 0 and currentAngle <= -maxAngle) then
      direction = 1
      currentAngle    = -maxAngle
    end
    
    vector = Vector3(0, 0, currentAngle)
    TRANSFORM:SetWorldRotation(originalRotation + vector)
  end
end

function OnCollisionEnter(other)
  if(other:Tag() == "Bullet") then
    healthScript:SetVariable("damage", 1)
    healthScript:CallFunction("DealDamge")
    timer = shakeDuration
  end
end

function Die()
  dead = true
  eventsScript:CallFunction("ReduceDummyCount")
  owner:Destroy()
end