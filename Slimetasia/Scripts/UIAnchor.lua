--settings
local screenPosX = 0.0
local screenPosY = 0.0
local offsetPosX = 0.0
local offsetPosY = 0.0
local pivotX = 0.5
local pivotY = 0.5
local vertSize = 10

local GO = nil
local trans = nil
local camObj = nil
local cam = nil

function Constructor()
  GO = owner
  trans = GO:GetComponent("Transform")  
  camObj = owner:GetLayer():GetObject("MainCamera")
  if (camObj ~= nil) then
    cam = camObj:GetComponent("Camera")
  end
  
  if(GO:Name() == "UI_PlayerHealthBlack") then
    screenPosX = 0.05
    screenPosY = 0.1
    offsetPosX = 0.0
    offsetPosY = 0.0
    pivotX = 1
  end
  if(GO:Name() == "UI_PlayerIcon") then
    screenPosX = 0.05
    screenPosY = 0.1
    offsetPosX = 0.0
    offsetPosY = 0.0
  end
  if(GO:Name() == "UI_CoreHealthFrame") then
    screenPosX = 0.05
    screenPosY = 0.2
    offsetPosX = 0.0
    offsetPosY = 0.0
    pivotX = 1
  end
  if(GO:Name() == "UI_CoreHealthBlack") then
    screenPosX = 0.05
    screenPosY = 0.2
    offsetPosX = 0.0
    offsetPosY = 0.0
    pivotX = 1
  end
  if(GO:Name() == "UI_CoreIcon") then
    screenPosX = 0.05
    screenPosY = 0.2
    offsetPosX = 0.0
    offsetPosY = 0.0
  end  if(GO:Name() == "UI_Resource") then
    screenPosX = 0.96
    screenPosY = 0.15
    offsetPosX = 0.0
    offsetPosY = 0.0
    pivotX = 0
  end
  if(GO:Name() == "UI_InfoBg1") then
    screenPosX = 1
    screenPosY = 0.5
    offsetPosX = 0.0
    offsetPosY = 0.0
    pivotX = 0
  end
  if(GO:Name() == "UI_InfoBg2") then
    screenPosX = 1
    screenPosY = 0.58
    offsetPosX = 0.0
    offsetPosY = 0.0
    pivotX = 0
  end
  if(GO:Name() == "UI_InfoBg3") then
    screenPosX = 1
    screenPosY = 0.66
    offsetPosX = 0.0
    offsetPosY = 0.0
    pivotX = 0
  end
  if(GO:Name() == "UI_InfoBg4") then
    screenPosX = 1
    screenPosY = 0.74
    offsetPosX = 0.0
    offsetPosY = 0.0
    pivotX = 0
  end
  if(GO:Name() == "LSTextSelect") then
    screenPosX = 0.29
    screenPosY = 0.045
    offsetPosX = 0.0
    offsetPosY = 0.0
  end
  if(GO:Name() == "LSReturn"or GO:Name() == "LSReturn2") then
    screenPosX = 0.05
    screenPosY = 0.045
    offsetPosX = 0.0
    offsetPosY = 0.0
    pivotX = 1.0
  end
  if(GO:Name() == "LSUpgrade") then
    screenPosX = 0.585
    screenPosY = 0.045
    offsetPosX = 0.0
    offsetPosY = 0.0
  end
  if(GO:Name() == "LSMovement"or GO:Name() == "LSMovement2") then
    screenPosX = 0.045
    screenPosY = 0.045
    offsetPosX = 0.0
    offsetPosY = 0.0
  end
  if(GO:Name() == "LSTextNavigate") then
    screenPosX = 0.15
    screenPosY = 0.045
    offsetPosX = 0.0
    offsetPosY = 0.0
  end
  if(GO:Name() == "LSTextReturn") then
    screenPosX = 0.065
    screenPosY = 0.045
    offsetPosX = 0.0
    offsetPosY = 0.0
    pivotX = 1.0
  end
  if(GO:Name() == "HelpText") then
    screenPosX = 0.92
    screenPosY = 0.08
    offsetPosX = 0.0
    offsetPosY = 0.0
    pivotY = 0
    pivotX = 1
  end
  if(GO:Name() == "TimerFrame") then
    screenPosX = 0
    screenPosY = 0.9
    offsetPosX = 0.0
    offsetPosY = 0.0
    pivotY = 0
    pivotX = 1
  end
  if(GO:Name() == "MStart") then
    screenPosX = 0.5
    screenPosY = 0.27
    offsetPosX = 0.0
    offsetPosY = 0.0
    pivotY = 1
  end
  if(GO:Name() == "MSetting") then
    screenPosX = 0.5
    screenPosY = 0.2025
    offsetPosX = 0.0
    offsetPosY = 0.0
    pivotY = 1
  end
  if(GO:Name() == "MQuit") then
    screenPosX = 0.5
    screenPosY = 0.1
    offsetPosX = 0.0
    offsetPosY = 0.0
    pivotY = 1
  end
  if(GO:Name() == "MLogo") then
    screenPosX = 0.5
    screenPosY = 0.18
    offsetPosX = 0.0
    offsetPosY = 0.0
    pivotY = 1
  end
  if(GO:Name() == "UI_Transition") then
    screenPosX = 0.5
    screenPosY = 0.5
    offsetPosX = 0.0
    offsetPosY = 0.0
  end
  
end

function OnUpdate(dt)
  if(trans == nil) then
    write("UI Anchor: No Transform found")
    return
  end
  if (camObj ~= nil) then
    if (cam ~= nil) then
      SetAspectPosition()
    else
      cam = camObj:GetComponent("Camera")
    end
  else
    camObj = owner:GetLayer():GetObject("MainCamera")
  end
end

function SetAspectPosition()
--Get the new position as thought it's 0.5 at pivot
y = (screenPosY*vertSize)-(vertSize/2) + offsetPosY
ratio = cam:GetViewporSize():x()/cam:GetViewporSize():y()
horiSize = vertSize*ratio
x = (screenPosX*horiSize)-(horiSize/2) + offsetPosX
v = Vector3(x, y, trans:GetWorldPosition():z())

--offset the new position base on the pivot
pivotOffset = Vector3( pivotX - 0.5,  
                       pivotY - 0.5, 
                       0.0)
x =  pivotOffset:x() * trans:GetWorldScale():x()
y =  pivotOffset:y() * trans:GetWorldScale():z()
z =  pivotOffset:z() * trans:GetWorldScale():y()
pivotOffset = Vector3( x, y, z)
v = v + pivotOffset
trans:SetWorldPosition(v)
end

function SetPosition(v)
    screenPosX = v:x()
    screenPosY = v:y()
    SetAspectPosition()
end

