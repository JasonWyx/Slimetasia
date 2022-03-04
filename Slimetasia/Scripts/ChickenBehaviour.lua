-- VARIABLES ===================================================================
local owner_transform = nil
local owner_rigidbody = nil

speed = 2
local timer = 0
TurnTime = 3

local startPoint = nil
distanceAway = 9
-- FUNCTIONS ===================================================================
function Constructor()
  owner_rigidbody = owner:GetComponent("RigidBody")
  owner_transform = owner:GetComponent("Transform")
  startPoint = owner_transform:GetWorldPosition()
end

function OnUpdate(dt)
  currVel   = owner_rigidbody:GetVelocity()
  angle = VectorAngleWorldAxis(currVel)  
  rot = owner_transform:GetWorldRotation()
  rot.y = angle
  owner_transform:SetWorldRotation(rot)
  
  distVec = owner_transform:GetWorldPosition() - startPoint
  if (VectorLength(distVec) > distanceAway) then
    owner_rigidbody:SetVelocity(VectorNormalized(distVec) * -1 * speed)
  else
    if (timer > 0) then
      timer = timer - dt
    else
      timer = TurnTime
      vector = owner_transform:ForwardVector():Rotate("y", RandomRange(0,360))
      owner_rigidbody:SetVelocity(vector * speed)
    end
  end
  
  --Debug draw
  --DebugDrawLine(startPoint, startPoint + Vector3(distanceAway, 0 , distanceAway), Color(1.0, 0.0, 0.0, 1.0)) --Red
  --DebugDrawLine(startPoint, startPoint + Vector3(distanceAway, 0 , -distanceAway), Color(1.0, 0.0, 0.0, 1.0)) --Red
  --DebugDrawLine(startPoint, startPoint + Vector3(-distanceAway, 0 , -distanceAway), Color(1.0, 0.0, 0.0, 1.0)) --Red
  --DebugDrawLine(startPoint, startPoint + Vector3(-distanceAway, 0 , distanceAway), Color(1.0, 0.0, 0.0, 1.0)) --Red
  --DebugDrawLine(startPoint, startPoint + Vector3(-distanceAway, 0 , 0), Color(1.0, 0.0, 0.0, 1.0)) --Red
  --DebugDrawLine(startPoint, startPoint + Vector3(distanceAway, 0 , 0), Color(1.0, 0.0, 0.0, 1.0)) --Red
  --DebugDrawLine(startPoint, startPoint + Vector3(0 , 0 , -distanceAway), Color(1.0, 0.0, 0.0, 1.0)) --Red
  --DebugDrawLine(startPoint, startPoint + Vector3(0 , 0 , distanceAway), Color(1.0, 0.0, 0.0, 1.0)) --Red
end