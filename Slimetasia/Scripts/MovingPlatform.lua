-- VARIABLES ===================================================================
-- [positions]
local positions = {Vector3(-39.5, -0.25, -4.6), Vector3(-39.5, 2, -24)}
local index     = 1
local minDist   = 0.05
local speed     = 1000

local owner_transform = nil
local owner_rigidBody = nil

-- FUNCTIONS ===================================================================
function Constructor()
	owner_transform = owner:GetComponent("Transform")
	owner_rigidBody = owner:GetComponent("RigidBody")
	
	minDist = minDist * minDist
end

function OnUpdate(dt)
	if (CheckIfReached()) then
		MoveToNextPosition()
	end
end

-- CHECK IF REACH =============================================================
function CheckIfReached()
	curentPositon = owner_transform:GetWorldPosition()
	destination   = positions[index]
	distance      = destination - curentPositon
	
	
	if (distance:SquareLength() <= minDist) then
		index = index + 1
		
		if (index > #positions) then
			index = 1
		end
		return true
	end
	
	return false
end

function MoveToNextPosition()
	curentPositon = owner_transform:GetWorldPosition()
	destination   = positions[index]
	direction     = destination - curentPositon
	
	owner_rigidBody:SetVelocity(Vector3())
	owner_rigidBody:AddForce(direction:Normalize() * speed)
end