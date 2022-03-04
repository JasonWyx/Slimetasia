-- components
local meshRenderer = nil

-- MISC
local timer           = 0
local flickerDuration = 2
local dir             = -1
local originalR       = 1
local originalG       = 1
local originalB       = 1
local multiplier      = 10

-- Constructor / Update
function Constructor()
  meshRenderer  = owner:GetComponent("MeshRenderer")
  
  if (meshRenderer ~= nil) then
    originalColor = meshRenderer:GetColor()
    originalR = originalColor:r()
    originalG = originalColor:g()
    originalB = originalColor:b()
  end
end

function OnUpdate(dt)
  if (meshRenderer ~= nil) then
    -- Handle timer
    timer = timer + dt
    if (timer >= flickerDuration) then
      timer = 0
      dir   = dir * -1
    end
    
    -- Flicker
    currR = 0
    currG = 0
    currB = 0
    if (dir == -1) then
      currR = Lerp(originalR * multiplier, 0, timer / flickerDuration)
      currG = Lerp(originalG * multiplier, 0, timer / flickerDuration)
      currB = Lerp(originalB * multiplier, 0, timer / flickerDuration)
    else
      currR = Lerp(0, originalR * multiplier, timer / flickerDuration)
      currG = Lerp(0, originalG * multiplier, timer / flickerDuration)
      currB = Lerp(0, originalB * multiplier, timer / flickerDuration)
    end
    
    -- Set alpha
    currColor = meshRenderer:GetColor()
    currColor.r = currR
    currColor.g = currG
    currColor.b = currB
    meshRenderer:SetColor(currColor)
  end
end