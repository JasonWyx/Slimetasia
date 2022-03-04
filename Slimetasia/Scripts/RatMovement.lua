-- VARIABLES ===================================================================
local endPoint = Vector3()
startPoint = Vector3()

constructed = false
startConstruct = false

local owner_rigidbody = nil
local owner_transform = nil
local timer = 0

local ending = false

local speedTimer = 1
speed = 1

local player_transform = nil
distFromPlayerThreshold = 3

endPointChanged = false

local lifeTime = 5

-- FUNCTIONS ===================================================================
function Constructor()
  owner_rigidbody = owner:GetComponent("RigidBody")
  owner_transform = owner:GetComponent("Transform")
end

function MyConstructor()
  --startPoint = owner_transform:GetWorldPosition()
  r = RandomRange(5,10)
  angle = ToRad(RandomRange(0, 360))
  
  newx = startPoint:x() + r * Cos(angle)
  newz = startPoint:z() + r * Sin(angle)
  endPoint = Vector3(newx, startPoint:y(), newz)
  
  if (RandomRangeInt(0,10) >= 5) then
    --temp = startPoint
    --startPoint = endPoint
    --endPoint = temp
  end
  
  owner_transform:SetWorldPosition(startPoint)
  --write(startPoint)
  --write(endPoint)
  
  --Debug
  --startR = CreatePrefab("Coin")
  --endR   = CreatePrefab("Cash")
  --startR:GetComponent("Transform"):SetWorldPosition(startPoint)
  --endR:GetComponent("Transform"):SetWorldPosition(endPoint)
  
  speed = RandomRange(0,5)
  owner_rigidbody:SetYVelocity(3)
  
  player_transform = CurrentLayer():GetObject("Player"):GetComponent("Transform")
end

function OnUpdate(dt)
  if (not constructed and startConstruct) then
    MyConstructor()
    constructed = true
  end
  
  if (constructed) then
    playerPos = player_transform:GetWorldPosition()
    distFromPlayerVec = owner_transform:GetWorldPosition() - playerPos
    distFromPlayer = VectorLength(distFromPlayerVec)
    if (not endPointChanged and distFromPlayer < distFromPlayerThreshold) then
      direction = VectorNormalized(distFromPlayerVec)
      endPoint = Vector3(playerPos:x(), endPoint:y(), playerPos:z()) + Vector3(direction:x()* 10, 0, direction:z() * 10)
      endPointChanged = true
    end
    speedModifier = 10 / (distFromPlayer)
    
    owner_rigidbody:SetVelocity(VectorNormalized(endPoint - startPoint) * speed * speedModifier)
    
    if (not ending and timer > 2) then
      owner_rigidbody:SetYVelocity(0)
    else
      timer = timer + dt
    end
    
    distToEnd = endPoint - owner_transform:GetWorldPosition()
    distToEnd = Vector3(distToEnd:x(), endPoint:y(), distToEnd:z())
    if (VectorLength(distToEnd) < 1) then
      owner_rigidbody:SetYVelocity(-3 * speedModifier)
      ending = true
      timer = 0
    end
    
    if (ending) then
      if (timer > 2) then
        owner:Destroy()
      else
        timer = timer + dt
      end
    end
    
    if (speedTimer > 0) then
      speedTimer = speedTimer - dt
    else
      speedTimer = RandomRange(1,3)
      speed = RandomRange(1,5)
    end
    
    
    
    
    currVel   = owner_rigidbody:GetVelocity()
    --s         = owner_transform:GetWorldPosition()
    
    angle = VectorAngleWorldAxis(currVel)
    rot = owner_transform:GetWorldRotation()
    rot.y = angle
    owner_transform:SetWorldRotation(rot)
    
    --Debug draw
    --DebugDrawLine(s, s + currVel, Color(0.0, 0.0, 1.0, 1.0))
    --DebugDrawLine(s, s + owner_transform:UpwardVector(), Color(1.0, 0.0, 0.0, 1.0)) --Red
    --DebugDrawLine(s, s + owner_transform:ForwardVector(), Color(1.0, 1.0, 0.0, 1.0)) --Yellow
    if (lifeTime < 0 ) then
      owner:Destroy()
    else
      lifeTime = lifeTime - dt
    end
  end
end