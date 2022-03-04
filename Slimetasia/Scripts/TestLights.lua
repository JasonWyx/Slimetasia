-- VARIABLES ===================================================================
dLight = false
pLight = false
sLight = false
currentLight = nil
-- FUNCTIONS ===================================================================
function Constructor()
end

function OnUpdate(dt)
  if (IsKeyPressed(KEY_1)) then
    owner:AddComponent("DirectionalLight")
    owner:RemoveComponent("PointLight")
    owner:RemoveComponent("SpotLight")
    currentLight = owner:GetComponent("DirectionalLight")
    dLight = true
    pLight = false
    sLight = false
  end
  if (IsKeyPressed(KEY_2)) then
    owner:AddComponent("PointLight")
    owner:RemoveComponent("DirectionalLight")
    owner:RemoveComponent("SpotLight")
    currentLight = owner:GetComponent("PointLight")
    dLight = false
    pLight = true
    sLight = false
  end
  if (IsKeyPressed(KEY_3)) then
    owner:AddComponent("SpotLight")
    owner:RemoveComponent("DirectionalLight")
    owner:RemoveComponent("PointLight")
    currentLight = owner:GetComponent("SpotLight")
    dLight = false
    pLight = false
    sLight = true
  end
  
  
  if (IsKeyPressed(KEY_Q)) then
    write("Inten: ", currentLight:GetIntensity())
  end
  
  
  if (IsKeyPressed(KEY_W)) then
    currentLight:SetIntensity(currentLight:GetIntensity() + 1)
    write("Inten: ", currentLight:GetIntensity())
  end
  
  
  if (IsKeyPressed(KEY_E)) then
    write("Color: ", currentLight:GetColor())
  end
  
  
  if (IsKeyPressed(KEY_R)) then --Prob
    if (currentLight:GetColor() == Color(1,1,1,1)) then
      currentLight:SetColor(Color(1,0,0,1))
      write("Color: ", currentLight:GetColor())
    
    elseif (currentLight:GetColor() == Color(1,0,0,1)) then
      currentLight:SetColor(Color(0,1,0,1))
      write("Color: ", currentLight:GetColor())
    
    elseif (currentLight:GetColor() == Color(0,1,0,1)) then
      currentLight:SetColor(Color(0,0,1,1))
      write("Color: ", currentLight:GetColor())
    
    elseif (currentLight:GetColor() == Color(0,0,1,1)) then
      currentLight:SetColor(Color(1,0,0,1))
      write("Color: ", currentLight:GetColor())
    end
  end

  
  if (IsKeyPressed(KEY_T)) then
    write("CastShad: ",currentLight:GetCastShadows())
  end

  
  if (IsKeyPressed(KEY_Y)) then
    currentLight:SetCastShadows(not currentLight:GetCastShadows())--prob
    write("CastShad: ",currentLight:GetCastShadows())
  end

  
  if (IsKeyPressed(KEY_U)) then
    write("ShadowDistance: ",currentLight:GetShadowDistance())
  end

  
  if (IsKeyPressed(KEY_I)) then
    write("GetShadowBias: ",currentLight:GetShadowBias())
  end

  
  if (IsKeyPressed(KEY_O) and (dLight or sLight)) then
    write("Dir: ",currentLight:GetDirection())
  end

  if (IsKeyPressed(KEY_A) and (dLight or sLight)) then
    currentLight:SetDirection(currentLight:GetDirection() + Vector3(1,1,1))
    write("Dir: ",currentLight:GetDirection())
  end
end
