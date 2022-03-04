
plane = nil
trans = nil
player = nil
point = nil
run = false
firstUpdate = true
local timer = 0.2

function Constructor()
  plane = owner:GetLayer():GetObject("plane")
  trans = owner:GetLayer():GetObject("plane"):GetComponent("Transform")
  player = owner:GetLayer():GetObject("player"):GetComponent("Transform")
  point = owner:GetLayer():GetObject("P15"):GetComponent("Transform")
  
end

function OnUpdate(dt)
    p = trans:GetWorldPosition()
    p = p + trans:RightVector()*dt*80
    if(timer < 0) then
      timer = 0.2
      s = trans:GetWorldRotation()
      s.y = s:y()+7
      if(s:y() > 360) then s.y = s:y()-359 end
      trans:SetWorldRotation(s)
    end
    trans:SetWorldPosition(p)
    timer= timer - dt
end