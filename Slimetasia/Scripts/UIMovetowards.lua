--settings

vertSize = 10
local camObj = nil
local cam = nil
GO = nil
rb = nil
trans = nil

timer = 0.0
pos = Vector3(0,0,0)
function Constructor()
  GO = owner
  trans = GO:GetComponent("Transform")  
  rb = GO:GetComponent("RigidBody")
  camObj = owner:GetLayer():GetObject("MainCamera")
  if (camObj ~= nil) then
    cam = camObj:GetComponent("Camera")
  end
end

function OnUpdate(dt)
  offset = targetPos - go:trans:GetWorldPosition();
  magnitude = offset:Length();
  direction = offset:Normalized();
if(timer < dt) then
else

end
  size = Vector3(cam:GetViewporSize():x()/10,0,cam:GetViewporSize():y()/10)
  trans:SetWorldScale(size)
end

function MoveTime(targetPos,reachTime)
  pos = targetPos
  timer = reachTime
end