-- VARIABLES ===================================================================
local owner_transform = nil
local owner_CircleParticle = nil

local timer = 0
RunTime = 0.1
--emitRate = 150
-- FUNCTIONS ===================================================================
function Constructor()
  owner_CircleParticle = owner:GetComponent("ParticleEmitter_Circle")
  owner_transform = owner:GetComponent("Transform")
end

function OnUpdate(dt)
  if (timer < RunTime) then
    timer = timer + dt
  else
    owner_CircleParticle:SetEmitRate(0)
    owner:Destroy()
  end
  
  --Debug purposes
  --if (IsKeyPressed(KEY_1)) then
  --  timer = 0
  --  owner_CircleParticle:SetEmitRate(emitRate)
  --end
end