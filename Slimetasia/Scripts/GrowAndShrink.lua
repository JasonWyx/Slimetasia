
local transform             = nil
local direction             = 1
local maxScale              = 1
local minScale              = 0.8
local initialScale          = 0
local timer                 = 0
local timerToChangeScale    = 2
local newtimerToChangeScale = 0
local currScale             = 0
local fadeEffect            = false
local fadeEffectTimer       = 0

function Constructor()
  transform = owner:GetComponent("Transform")
  initialScale = transform:GetWorldScale()
  newtimerToChangeScale = timerToChangeScale
end

function OnUpdate(dt)
  timer = timer + dt
  
  if (direction == 1) then
    currScale = Lerp(minScale, maxScale, timer / newtimerToChangeScale)
    transform:SetWorldScale(initialScale * currScale)
  else
    currScale = Lerp(maxScale, minScale, timer / newtimerToChangeScale)
    transform:SetWorldScale(initialScale * currScale)
  end
  
  if (timer >= newtimerToChangeScale) then
    timer = 0
    direction = direction * -1
  end
  
  if (fadeEffect) then
    fadeEffectTimer = fadeEffectTimer + dt
    newtimerToChangeScale = Lerp(timerToChangeScale, timerToChangeScale * 2, fadeEffectTimer / 5)
  end
end

function FadeEffect ()
  fadeEffect = true
end