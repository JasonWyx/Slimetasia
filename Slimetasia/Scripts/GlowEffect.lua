maxBreatheIntensity   = 3.0
minBreatheIntensity   = 1.0
breatheDuration       = 2.0
timer                 = 0.0
position              = true


local meshRenderer = nil

function Constructor()

  meshRenderer = owner:GetLayer():GetComponent("MeshRenderer")
  

end

function OnUpdate(dt)

  if(timer > breatheDuration) then
    timer = timer % breatheDuration;
  end

  colorValue = minBreatheIntensity(timer / breatheDuration) * 
  meshRenderer:SetColor(Color(colorValue, colorValue, colorValue, 1.0))
end