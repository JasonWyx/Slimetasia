timeToLive = 0.1
local timer = 0.0

function OnUpdate(dt)
  if (timer < timeToLive) then
    timer = timer + dt
  else
    owner:Destroy()
  end
end