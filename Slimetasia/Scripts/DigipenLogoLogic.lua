--settings

local time_before_change = 4.0
local color_value = 1.0

local mesh = nil

function Constructor()
  mesh = owner:GetComponent("MeshRenderer")
  --hide the cursor
  ShowMouseCursor(false)
end

function OnUpdate(dt)
  if(dt > 1.0) then
    return
  end
  
  if(time_before_change > 0) then
    time_before_change = time_before_change - dt
  else
      SceneLoad("Level_SpenguinLogo")
  end
end