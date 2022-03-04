-- VARIABLES ===================================================================
local owner_particle = nil
local owner_transform = nil
local boat = nil
local boat_transform = nil
rate = 1
emitRate = 7500
local constructed = false
local offset = Vector3(-10, -0.5, 0)
activateParticle = false
-- FUNCTIONS ===================================================================
function MyConstructor()
  owner_particle = owner:GetComponent("ParticleEmitter_Circle")
  owner_transform = owner:GetComponent("Transform")
  constructed = true
  boat = CurrentLayer():GetObject("SingaporeBoat")
  boat_transform = boat:GetComponent("Transform")
  owner:SetParent(boat:GetID())
end

function OnUpdate(dt)
  if (constructed == false) then
    MyConstructor()
  end
  owner_transform:SetWorldPosition(boat_transform:GetWorldPosition() + offset)
  if (activateParticle == false) then
    owner_particle:SetEmitRate(0)
    rate = 1
  else
    if (rate < emitRate) then
      rate = rate * 2
    end
    owner_particle:SetEmitRate(rate)
  end
end