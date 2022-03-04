--settings

local GO = nil
local trans = nil
delay = 0
nextScale = Vector3(0,0,0)

function Constructor()
  GO = owner
  trans = GO:GetComponent("Transform")  
  nextScale = trans:GetWorldScale()
  delay = 10000
end

function OnUpdate(dt)
  if(delay > 0) then
    delay = delay - dt
  else
    trans:SetWorldScale(nextScale)
  end
end
