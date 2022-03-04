local currentTime = 0.0
local interval = 0.5
function Constructor()
end

function OnUpdate(dt)
  if(currentTime > interval)then
    currentTime = currentTime - interval
    owner:Destroy()
  end
  currentTime = currentTime + dt
end