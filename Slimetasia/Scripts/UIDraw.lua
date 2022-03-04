--settings
moveSpeed = 8.0

GO = nil
rb = nil
trans = nil

lateUpdate = true
local pos = Vector3();

isActive = true

lateUpdate = true
function Constructor()
  GO = owner
  trans = GO:GetComponent("Transform")  
  rb = GO:GetComponent("RigidBody")
  camObj = CurrentLayer():GetObject("Camera")
  pos = trans:GetWorldPosition()
  moveSpeed = RandomRange(5.0, 8.0)
end

function OnUpdate(dt)
if(lateUpdate == true) then
  lateUpdate = false
  trans:SetWorldPosition(Vector3(0,0,0))
end
  targetPos = Vector3()
  if(isActive) then
    targetPos = pos
  else
    targetPos = Vector3(0,0,0)
  end
  
    currentPos = trans:GetWorldPosition()
     nextPos = Vector3(0,0,0)
    if((currentPos-targetPos):Length() <= 0.01) then
      nextPos = targetPos
    else
      nextPos = currentPos + (targetPos - currentPos):Normalized() * moveSpeed * dt
    end
    trans:SetWorldPosition(nextPos);
end

function OffActive()

  isActive = false
end

function OnActive()

  isActive = true
end