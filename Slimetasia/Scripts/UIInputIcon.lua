--settings
local keyIcon = nil
local controllerIcon = nil
isController = 0
swtichControl = true

controllerName = "ButtonSpace"
buttonName = "ButtonA"
    
mr = nil
function Constructor()
  controllerIcon = owner
  mr = owner:GetComponent("MeshRenderer")
  if(owner:Name() == "UI_Jump") then
    controllerName = "ButtonA"
    buttonName = "ButtonSpace"
  end
  if(owner:Name() == "UI_SwitchMode") then
    controllerName = "ButtonB"
    buttonName = "ButtonF"
  end
  if(owner:Name() == "UI_LTIcon") then
    controllerName = "ButtonLT"
    buttonName = "ButtonR"
  end
  if(owner:Name() == "UI_RTIcon") then
    controllerName = "ButtonRT"
    buttonName = "ButtonLClick"
  end
  if(owner:Name() == "LSSelect") then
    controllerName = "ButtonA"
    buttonName = "ButtonSpace"
  end
  if(owner:Name() == "LSSSelect") then
    controllerName = "ButtonA"
    buttonName = "ButtonSpace"
  end
  if(owner:Name() == "LSReturn") then
    controllerName = "ButtonB"
    buttonName = "ButtonF"
  end
  if(owner:Name() == "LSMovement") then
    controllerName = "AnalogStick"
    buttonName = "ButtonDirectional"
    keyIcon = owner:GetLayer():GetObject("LSMovement2")
  end
  if(owner:Name() == "UI_CycleLeft") then
    controllerName = "ButtonLB"
    buttonName = "ButtonQ"
  end
  if(owner:Name() == "UI_CycleRight") then
    controllerName = "ButtonRB"
    buttonName = "ButtonE"
  end
  if(owner:Name() == "UI_UltiIcon") then
    controllerName = "ButtonY"
    buttonName = "ButtonRClick"
  end
  keyIcon:GetComponent("Transform"):SetWorldPosition(owner:GetComponent("Transform"):GetWorldPosition())
end

function OnUpdate(dt)
  if(GetControllerInput() ~= isController) then
    swtichControl = true
  end
  if(swtichControl == true) then
    swtichControl = false
    isController = GetControllerInput()
    if(isController == 1) then
      mr:SetDiffuseTexture(controllerName)
    else
      mr:SetDiffuseTexture(buttonName)
    end
  end
end
