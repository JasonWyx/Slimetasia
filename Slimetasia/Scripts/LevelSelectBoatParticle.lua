-- VARIABLES ===================================================================
local owner_particle = nil
local owner_transform = nil
local boat = nil
height = 0.5
rate = 1
emitRate = 7500
constructed = false
local offset = Vector3(0, 0, 4)
-- FUNCTIONS ===================================================================
function MyConstructor()
  owner_particle = owner:GetComponent("ParticleEmitter_Circle")
  owner_transform = owner:GetComponent("Transform")
  constructed = true
  boat = CurrentLayer():GetObject("Player")
  boatPos = boat:GetComponent("Transform"):GetWorldPosition()
  owner_transform:SetWorldPosition(boatPos + offset)
  owner:SetParent(boat:GetID())
end

function OnUpdate(dt)
  if (constructed == false) then
    MyConstructor()
  end
  
  if (owner_transform:GetWorldPosition():y() > height) then
    owner_particle:SetEmitRate(0)
    rate = 1
  else
    if (rate < emitRate) then
      rate = rate * 2
    end
    owner_particle:SetEmitRate(rate)
  end
end