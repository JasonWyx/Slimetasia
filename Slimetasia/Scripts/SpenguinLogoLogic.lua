--settings

local time_before_change = 3.0
local color_value = 1.0

local mesh = nil

function Constructor()
  mesh = owner:GetComponent("MeshRenderer")
  --hide the cursor
  ShowMouseCursor(false)
end

function OnUpdate(dt)
  if(time_before_change > 0) then
    time_before_change = time_before_change - dt
  else
    color_value = mesh:GetColor()
    if(color_value:a() > 0) then
      color_value.a = color_value:a() - (dt / 2)
      mesh:SetColor(color_value)
    else
      SceneLoad("Level_MainMenu")
    end
  end
end