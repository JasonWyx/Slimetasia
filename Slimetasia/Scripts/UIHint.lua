
GO = nil
playerTrans = nil
defaultPos = nil


startFadeTime = 4.0
appearTime = 3.0
currentTime = 0.0

appeared = true
isFading = false

colorVal = 1.0

prevPos = Vector3()
lateStart = true


cam = nil
currentPivotX = 0
local newPosition = Vector3()

icon = nil
iconTrans = nil
text = nil
textTrans = nil
trans = nil

function Constructor()
  GO = owner
  trans = owner:GetComponent("Transform")
  camobj = GO:GetLayer():GetObject("MainCamera")
  cam = camobj:GetComponent("Camera")
  

  
  if(GO:Name() == "UI_InfoBg1") then
    icon = GO:GetLayer():GetObject("UI_Jump") 
    text = GO:GetLayer():GetObject("UI_HintTextJump") 
  end
  if(GO:Name() == "UI_InfoBg2") then
    icon = GO:GetLayer():GetObject("UI_SwitchMode")
    text = GO:GetLayer():GetObject("UI_HintTextSwitch") 
  end
  if(GO:Name() == "UI_InfoBg3") then
    icon = GO:GetLayer():GetObject("UI_LTIcon")  
    text = GO:GetLayer():GetObject("UI_HintTextRotate") 
  end
  if(GO:Name() == "UI_InfoBg4" ) then
    icon = GO:GetLayer():GetObject("UI_RTIcon")
    text = GO:GetLayer():GetObject("UI_HintTextFire") 
  end
  iconTrans = icon:GetComponent("Transform") 
  textTrans = text:GetComponent("Transform") 

end

function SetPosition(screenPosX,screenPosY,pivotX,pivotY, t)
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
  z =  pivotOffset:z()
  pivotOffset = Vector3( x, y, z)
  v = v + pivotOffset
  newPosition =  v
end

function OnUpdate(dt)
  if(lateStart) then
    lateStart = false
    currentTime = 0.0 
    appeared = true
    icon:SetActive(true)
    text:SetActive(true)
    return
  end

  if(appeared == false) then
      currentPivotX = currentPivotX + dt*1.3
      if(currentPivotX > 1) then
        currentPivotX = 1
      end
        UpdatePosition()
      if(ControllerPress("ShowHint")) then
        currentTime = 0.0 
        appeared = true
        icon:SetActive(true)
        text:SetActive(true)
      end
  else
      currentPivotX = currentPivotX - dt*1.3
      if(currentPivotX < 0) then
        currentPivotX = 0
      end
      UpdatePosition()
      currentTime = currentTime + dt
      if(currentTime >= startFadeTime) then
        currentTime = 0.0 
        appeared = false
        icon:SetActive(false)
        text:SetActive(false)
      end
  end
end

function UpdatePosition()
  oldZ = textTrans:GetWorldPosition():z()
  if(GO:Name() == "UI_InfoBg1") then
    SetPosition(1,0.5, currentPivotX,0.5,trans)
  elseif(GO:Name() == "UI_InfoBg2") then
    SetPosition(1,0.58,currentPivotX,0.5,trans)
  elseif(GO:Name() == "UI_InfoBg3") then
    SetPosition(1,0.66,currentPivotX,0.5,trans)
  elseif(GO:Name() == "UI_InfoBg4" ) then
    SetPosition(1,0.74,currentPivotX,0.5,trans)
  end
  
  trans:SetWorldPosition(newPosition)
  txPos = newPosition
  txPos.x = txPos:x() + trans:GetWorldScale():x()/6
  txPos.z = txPos:z() + 1
  textTrans:SetWorldPosition(txPos)
  
  n = newPosition
  n.x = n:x() - trans:GetWorldScale():x()/2 
  n.y = n:y() + trans:GetWorldScale():z()/2 
  n.z =  n:z() + 1
  iconTrans:SetWorldPosition(n)
end