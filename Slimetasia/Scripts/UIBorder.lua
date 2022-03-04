--settings
local blackTop = nil
local blackBottom = nil
local blackTTrans = nil
local blackBTrans = nil

function Constructor()
  GO = owner
  trans = GO:GetComponent("Transform")  
  blackTop = owner:GetLayer():GetObject("blackTop")
  blackBottom = owner:GetLayer():GetObject("blackBottom")
  blackTTrans = blackBottom:GetComponent("Transform")  
  blackBTrans = blackBottom:GetComponent("Transform")  
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