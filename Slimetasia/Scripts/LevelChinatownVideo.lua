-- VARIABLES ===================================================================
-- Amount used in place of passing arguments to a function

timer = 0.0
audio = nil
-- FUNCTIONS ===================================================================
function Constructor()
  audio = owner:GetComponent("AudioEmitter")
  audio:SetAndPlayAudioClip("OnwardsToNext")
end

function OnUpdate(dt)

  timer = timer + dt
  
  if(ControllerDown("Shoot")) then
    SceneLoad("Level_Chinatown")
  end

  if (timer > 15.8) then
    SceneLoad("Level_Chinatown")
  end
  
end
