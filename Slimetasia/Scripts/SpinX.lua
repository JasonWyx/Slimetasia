-- VARIABLES ===================================================================

-- [Settings]
speed = 100

-- [Components]
local owner_Transform = nil

-- FUNCTIONS ===================================================================
function Constructor()
  owner_Transform = owner:GetComponent("Transform")
end

function OnUpdate(dt)
  currentRot = owner_Transform:GetWorldRotation()
  currentRot.x = currentRot:x() + speed * dt
  owner_Transform:SetWorldRotation(currentRot)
end