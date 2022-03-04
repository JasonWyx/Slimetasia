-- VARIABLES ===================================================================
-- Amount used in place of passing arguments to a function

timer = 0.0
audio = nil
-- FUNCTIONS ===================================================================
function Constructor()
  audio = owner:GetComponent("AudioEmitter")
  audio:SetAndPlayAudioClip("IntroAudio")
end

function OnUpdate(dt)

  timer = timer + dt
  
  if(ControllerDown("Shoot")) then
    SceneLoad("Level_Tekong")
  end

  if timer > 39 then
    SceneLoad("Level_Tekong")
  end
  
end
