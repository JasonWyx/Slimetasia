trans = nil
local timer = 0.2

function Constructor()
  trans = owner:GetLayer():GetObject("pig"):GetComponent("Transform")
  
end

function OnUpdate(dt)
    p = trans:GetWorldPosition()
    p = p + trans:RightVector()*dt*40
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