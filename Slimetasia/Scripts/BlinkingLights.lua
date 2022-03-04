-- VARIABLES ===================================================================
timer = 0
blinkInterval = 0.2
originalIntensity = nil
lightComponent = nil
meshRenderer = nil
toggleEmissive = false

erraticTimer = 0
erraticInterval = 1
erraticRuntime = 0
startErraticBlink = false
-- FUNCTIONS ===================================================================
function Constructor()
  lightComponent = owner:GetComponent("DirectionalLight")
  if (lightComponent == nil) then
    lightComponent = owner:GetComponent("PointLight")
    if (lightComponent == nil) then
      lightComponent = owner:GetComponent("SpotLight")
    end
  end
  originalIntensity = lightComponent:GetIntensity()
  meshRenderer = owner:GetComponent("MeshRenderer")
  if (meshRenderer ~= nil and meshRenderer:GetEnableEmissive()) then
    toggleEmissive = true
    write("ASD" , toggleEmissive)
  end
  
end

function OnUpdate(dt)
  if (not startErraticBlink and erraticTimer > erraticInterval) then
    startErraticBlink = true
    erraticRuntime = RandomRange(1,2)    -- Runtime for each blink
  else
    erraticTimer = erraticTimer + dt
  end
  
  if (startErraticBlink) then
    if (erraticRuntime > 0) then
      ErraticBlinking(dt)
      erraticRuntime = erraticRuntime - dt
    else
      startErraticBlink = false
      erraticInterval = RandomRange(2,4) -- Cooldown between each blink
      erraticTimer = 0
      lightComponent:SetIntensity(0)
      if (meshRenderer ~= nil and toggleEmissive) then
        meshRenderer:SetEnableEmissive(false)
      end
    end
  end
end

function ErraticBlinking(dt)
  if (timer > blinkInterval) then
    if (lightComponent:GetIntensity() == 0)then
      lightComponent:SetIntensity(originalIntensity)
        if (meshRenderer ~= nil and toggleEmissive) then
          meshRenderer:SetEnableEmissive(true)
        end
    else
      lightComponent:SetIntensity(0)
      blinkInterval = RandomRange(0,0.2)
      if (meshRenderer ~= nil and toggleEmissive) then
        meshRenderer:SetEnableEmissive(false)
      end
    end
    timer = 0
  else
    timer = timer + dt
  end
end