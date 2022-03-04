-- VARIABLES ===================================================================

-- FUNCTIONS ===================================================================
function Constructor()
  circleParticle = owner:GetComponent("ParticleEmitter_Circle")  
  if (circleParticle ~= nil) then
    circleParticle:AddAttractor(owner:GetID())
  end
  
  boxParticle = owner:GetComponent("ParticleEmitter_Box")  
  if (boxParticle ~= nil) then
    boxParticle:AddAttractor(owner:GetID())
  end  
end

function MyConstructor()
end

function OnUpdate(dt)
end