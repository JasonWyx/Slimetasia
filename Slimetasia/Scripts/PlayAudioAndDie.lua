audioClipName = ""

local AUDIOEMITTER = nil
local isPlaying    = false

function Constructor()
  AUDIOEMITTER = owner:GetComponent("AudioEmitter")
end

function OnUpdate(dt)
  if (isPlaying) then
    if (not AUDIOEMITTER:IsPlaying()) then
      owner:Destroy()
    end
  end
end

function PlayAudio()
  isPlaying = true;
  AUDIOEMITTER:SetAndPlayAudioClip(audioClipName)
end