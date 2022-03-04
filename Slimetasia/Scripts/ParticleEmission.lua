-- VARIABLES ===================================================================
local owner_particle = nil
timer                = 0.0
emitRate             = 1024.0
interval             = 0.5
speed                = 2.0

constructed = false
-- FUNCTIONS ===================================================================
function Constructor()
end

function MyConstructor()
  owner_particle = owner:GetComponent("ParticleEmitter_Circle")
  write("EMIT RATE a ", emitRate)
  write("EMIT RATE b ", owner_particle:GetEmitRate())
  --owner_particle:SetEmitRate(1)
  constructed = true

end

function OnUpdate(dt)
  if (constructed == false) then
    MyConstructor()
  end
  rate = owner_particle:GetEmitRate()
  --write("Owner name :" , owner:Name())  
  if (rate < emitRate) then
    --write("rate/emitRate ", rate, "/", emitRate)
    if (timer < interval) then
      --write("timer/interval ", timer ,"/", interval)
      timer = timer + dt
    else
      owner_particle:SetEmitRate(rate * speed)
      timer = 0
    end
  end
  
end

function OnCollisionEnter(go)
end