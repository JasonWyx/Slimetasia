--settings

function Constructor()
end

function OnUpdate(dt)
  if(ControllerPress("SwitchMode")) then
      SceneLoad("Level_MainMenu")
  end
end