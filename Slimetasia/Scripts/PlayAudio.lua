
local AUDIOEMITTER = nil
function Constructor()
  AUDIOEMITTER = owner:GetComponent("AudioEmitter")
end

function OnUpdate(dt)
    if (AUDIOEMITTER:IsPlaying() == false) then
      AUDIOEMITTER:Play()
    end
end
