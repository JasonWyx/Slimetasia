--settings

vertSize = 10
local camObj = nil
local cam = nil
GO = nil
rb = nil
trans = nil
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
  size = Vector3(cam:GetViewporSize():x()/10,0,cam:GetViewporSize():y()/10)
  trans:SetWorldScale(size)
end