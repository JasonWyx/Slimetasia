--settings
GO = nil
trans = nil
backTrans = nil
textTrans = nil
camObj = nil
cam = nil

local maxLength = 0.0

currentAmount = 0.5
local minAmount = 0
local maxAmount = 1
firstUpdate = 1

isActive = true

mr = nil
prev = false
function Constructor()
  GO = owner
  trans = GO:GetComponent("Transform")  
  mr = GO:GetComponent("MeshRenderer")  
  
  maxLength = trans:GetWorldScale():x()
  camObj = owner:GetLayer():GetObject("MainCamera")
  if (camObj ~= nil) then
    cam = camObj:GetComponent("Camera")
  end
  
  if(GO:Name() == "SensitivityBar") then
    currentAmount= PlayerPref_GetFloat("SensitivityController", "Settings_Player")
    minAmount = 0.1
    maxAmount = 2
    textTrans =  owner:GetLayer():GetObject("Text_Sensitivity"):GetComponent("Transform")
    backTrans =  owner:GetLayer():GetObject("SensitivityBarBack"):GetComponent("Transform")
  end
  if(GO:Name() == "BGMBar") then
    currentAmount= AudioSystem_GetChannelGrpVolume("BGM")
    textTrans =  owner:GetLayer():GetObject("Text_BGM"):GetComponent("Transform")
    backTrans =  owner:GetLayer():GetObject("BGMBarBack"):GetComponent("Transform")
  end
  if(GO:Name() == "SFXBar") then
    currentAmount= AudioSystem_GetChannelGrpVolume("SFX")
    textTrans =  owner:GetLayer():GetObject("Text_SFX"):GetComponent("Transform")
    backTrans =  owner:GetLayer():GetObject("SFXBarBack"):GetComponent("Transform")
  end
end

function OnUpdate(dt)
  if(firstUpdate > 0) then
    UpdateLength(0)
    firstUpdate = firstUpdate - 1
    return
  end

  if(camObj:GetActive() == false) then return end
  if(isActive == false) then 
    mr:SetColor(Color(1.0,1.0,1.0,1.0))
    UpdatePos()
    UpdateMaxLength()
    return
  end
    mr:SetColor(Color(0.9,0.8,0.4,1.0))
  if(ControllerAxis( "Left" ) > 0.0) then
    UpdateLength(-ControllerAxis( "Left" ) * UnscaledDT() * maxAmount)
  elseif(ControllerAxis( "Right" ) > 0.0) then
    UpdateLength(ControllerAxis( "Right" ) * UnscaledDT() * maxAmount)
  end

  UpdatePos()
  UpdateMaxLength()
end

function UpdatePos()
  pos = textTrans:GetWorldPosition()
  pos.y = pos:y() - textTrans:GetWorldScale():y()/10
  pos.x = pos:x() + textTrans:GetWorldScale():x() * 10 + trans:GetWorldScale():x()/2
  trans:SetWorldPosition(pos) 
  
  
  --Reposition the background of the health
  pos = textTrans:GetWorldPosition()
  pos.y = pos:y() - textTrans:GetWorldScale():y()/10
  pos.x = pos:x() + textTrans:GetWorldScale():x() * 10 + backTrans:GetWorldScale():x()/2
  pos.z = trans:GetWorldPosition():z()-1;
  backTrans:SetWorldPosition(pos)
end

function UpdateMaxLength()
  scale =  trans:GetWorldScale()
  vertSize = 10
  ratio = cam:GetViewporSize():x()/cam:GetViewporSize():y()
  horiSize = vertSize*ratio
  maxLength = (horiSize/4)
  UpdateLength(0)
end

function UpdateLength(amount)
 currentAmount =  currentAmount + amount
  s = trans:GetWorldScale()
  if( currentAmount  < minAmount) then currentAmount = minAmount end
  if( currentAmount  > maxAmount) then currentAmount = maxAmount end
  s.x = maxLength * ( currentAmount /maxAmount)   
  trans:SetWorldScale(s)
  s.x = maxLength
  backTrans:SetWorldScale(s)
  
  if(amount ~= 0) then
    if(GO:Name() == "SensitivityBar") then
      PlayerPref_SetFloat("SensitivityController", currentAmount, "Settings_Player")
      PlayerPref_SetFloat("SensitivityKeyBoard", currentAmount, "Settings_Player")
      obj = CurrentLayer():GetObject("Camera")
      if(obj) then
        script = obj:GetLuaScript("PlayerCamera.lua")
        if(script) then
          script:CallFunction("ChangeSensitivity")
        end
      end
    elseif(GO:Name() == "BGMBar") then
      AudioSystem_SetChannelGrpVolume("BGM", currentAmount)
    elseif(GO:Name() == "SFXBar") then
      AudioSystem_SetChannelGrpVolume("SFX", currentAmount)
    end
  end
end

function UpdateOriginalPosition(screenPosX,screenPosY,pivotX,pivotY, t)
  vertSize = 10
  --Get the new position as thought it's 0.5 at pivot
  y = (screenPosY*vertSize)-(vertSize/2)
  ratio = cam:GetViewporSize():x()/cam:GetViewporSize():y()
  horiSize = vertSize*ratio
  x = (screenPosX*horiSize)-(horiSize/2)
  v = Vector3(x, y, t:GetWorldPosition():z())

  --offset the new position base on the pivot
  pivotOffset = Vector3( pivotX - 0.5,  
                         pivotY - 0.5, 
                         0.0)
  x =  pivotOffset:x() * t:GetWorldScale():x()
  y =  pivotOffset:y() * t:GetWorldScale():z()
  z =  pivotOffset:z() * t:GetWorldScale():y()
  pivotOffset = Vector3( x, y, z)
  v = v + pivotOffset
  originalPosition =  v
end