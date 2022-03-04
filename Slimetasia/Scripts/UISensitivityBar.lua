--settings
GO = nil
rb = nil
mr = nil
trans = nil
cam = nil

local maxLength = 0.0
local posX = 0.0

local healthUnit = 0
Player_PlayerScript = nil
SpawnerScript = nil
CoreScript = nil
glScript = nil

IsShaking = false

textObj = nil
textObjTrans = nil

firstUpdate = 1

isActive = false
function Constructor()
  GO = owner
  trans = GO:GetComponent("Transform")  

  maxLength = trans:GetWorldScale():x()
  posX = trans:GetWorldPosition():x()
  
  camObj = owner:GetLayer():GetObject("MainCamera")
  if (camObj ~= nil) then
    cam = camObj:GetComponent("Camera")
  end
  
  textObj = owner:GetLayer():GetObject("Text_Sensitivity")
  textObjTrans = textObj:GetComponent("Transform")
end

function OnUpdate(dt)
  if(firstUpdate > 0) then
    UpdateLength(0)
    firstUpdate = firstUpdate - 1
    return
  end
  if(camObj:GetActive() == false) then
    return
  end
  if(isActive == false) then return end
    if(ControllerAxis( "Left" ) > 0.0) then
      UpdateLength(-ControllerAxis( "Left" ) * UnscaledDT())
    elseif(ControllerAxis( "Right" ) > 0.0) then
      UpdateLength(ControllerAxis( "Right" ) * UnscaledDT())
    end
    
  
  pos =  textObjTrans:GetWorldPosition()
  pos.x = pos:x() + textObjTrans:GetWorldScale():x() * 12
  pos.x = pos:x() + trans:GetWorldScale():x()/2.0
  pos.y = pos:y() + trans:GetWorldScale():z()/2
  trans:SetWorldPosition(pos)
end


function UpdateBarPositions()
  originalPosition =  textObjTrans:GetWorldPosition()
  originalPosition.x = originalPosition:x() + textObjTrans:GetWorldScale():x() * 3
end
function UpdateLength(amount)
  vectest = PlayerPref_GetFloat("SensitivityController", "Settings_Player")
  f =  vectest + amount
  if(f < 0.1) then f = 0.1
  elseif(f>2) then f = 2 end
  PlayerPref_SetFloat("SensitivityController", f, "Settings_Player")
  PlayerPref_SetFloat("SensitivityKeyBoard", f, "Settings_Player")
  s = trans:GetWorldScale()
  s.x = maxLength * (f/2)   
  trans:SetWorldScale(s)
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