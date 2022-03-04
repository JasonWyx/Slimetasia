
local transform        = nil
local particle         = nil
local direction        = 1
local maxHeight        = 0.1
local minHeight        = -0.1
local initialHeight    = 0
local timer            = 0
local timerToChangeDir = 2

function Constructor()
  transform = owner:GetComponent("Transform")
  particle = owner:GetComponent("ParticleEmitter_Circle")
  initialPos = transform:GetWorldPosition()
  initialHeight = initialPos:y()
  maxHeight = initialHeight + maxHeight
  minHeight = initialHeight + minHeight
end

function OnUpdate(dt)
  timer = timer + dt
  
  if (direction == 1) then
    currHeight = Lerp(minHeight, maxHeight, timer / timerToChangeDir)
  else
    currHeight = Lerp(maxHeight, minHeight, timer / timerToChangeDir)
  end
  
  currPos = transform:GetWorldPosition()
  currPos.y = currHeight
  transform:SetWorldPosition(currPos)
  
  if (timer >= timerToChangeDir) then
    timer = 0
    direction = direction * -1
  end
  
  particle:SetFloorHeight(currHeight)
end