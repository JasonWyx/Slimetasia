-- VARIABLES ===================================================================
local owner_transform = nil

local timer = 0
local interval = 0.1

local originalPos = Vector3()

local constructed = false
-- FUNCTIONS ===================================================================
function Constructor()
  
end

function MyConstructor()
  owner_transform = owner:GetComponent("Transform")
  originalPos = owner:GetComponent("Transform"):GetWorldPosition()
end

function OnUpdate(dt)
  if (not constructed) then
    MyConstructor()
    constructed = true
  end
  
  if (timer < interval) then
    timer = timer + dt
  else
    owner_transform:SetWorldPosition(originalPos + Vector3(RandomRange(-3, 3), RandomRange(-3, 3), RandomRange(-3, 3)))
    timer = 0
  end
end

function OnCollisionEnter(go)
end