-- VARIABLES ===================================================================
local owner_transform = nil
local owner_rigidbody = nil

local startPoint = Vector3()
local jumpForce = Vector3(0, 1, 0)

speed = 2
force = 7.5
jumpTime = 1
local jumpTimer = 0
-- FUNCTIONS ===================================================================
function Constructor()
  owner_transform = owner:GetComponent("Transform")
  owner_rigidbody = owner:GetComponent("RigidBody")
  startPoint = owner_transform:GetWorldPosition()
end

function OnUpdate(dt)
  if (jumpTimer > 0) then
    jumpTimer = jumpTimer - dt
  else
    owner_rigidbody:AddVelocity(jumpForce * force)
    jumpTimer = jumpTime
  end
  
  currPos = Vector3Lerp(owner_transform:GetWorldPosition(), startPoint, speed * dt)
  owner_transform:SetWorldPosition(currPos)
end