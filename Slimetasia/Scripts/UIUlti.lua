--settings
GO = nil

PlayerGO = nil
playerTrans = nil

merlionTrans = nil
merlionUlti = nil

trans = nil
text = nil
blackText = nil

chargeTrans = nil
chargeBlackTrans = nil
coreHealthTrans = nil

anchorPosition = Vector3(0,0,0)

currentX = 0
cam = nil
function Constructor()
  GO = owner
  PlayerGO = owner:GetLayer():GetObject("Player")
  if(PlayerGO ~= nil) then
      playerTrans = PlayerGO:GetComponent("Transform")
  end
    
  trans = GO:GetComponent("Transform") 
  coreHealthTrans = owner:GetLayer():GetObject("UI_CoreHealthFrame"):GetComponent("Transform")
  text = owner:GetLayer():GetObject("UI_Charger"):GetComponent("TextRenderer")
  chargeTrans = owner:GetLayer():GetObject("UI_Charger"):GetComponent("Transform")
  blackText =     owner:GetLayer():GetObject("UI_ChargerBlack"):GetComponent("TextRenderer")
  chargeBlackTrans = owner:GetLayer():GetObject("UI_ChargerBlack"):GetComponent("Transform")
  m = CurrentLayer():GetObject("Core")
  if(m ~= nil) then
    merlionTrans = m:GetComponent("Transform")
    merlionUlti = m:GetLuaScript("MerlionUlti.lua")
  end
  
  camObj = owner:GetLayer():GetObject("MainCamera")
  if (camObj ~= nil) then
    cam = camObj:GetComponent("Camera")
  end
end

function OnUpdate(dt)
  if(PlayerGO == nil) then
    PlayerGO = CurrentLayer():GetObject("Player")
  end
  if(PlayerGO ~= nil) then
      playerTrans = PlayerGO:GetComponent("Transform")
  end
  newPos = coreHealthTrans:GetWorldPosition()
  newPos.y = coreHealthTrans:GetWorldPosition():y() + coreHealthTrans:GetWorldScale():z()/1.3
  newPos.x = coreHealthTrans:GetWorldPosition():x() - coreHealthTrans:GetWorldScale():x()/3.6
  newPos.z = trans:GetWorldPosition():z()
  trans:SetWorldPosition(newPos)
  percentage = merlionUlti:GetVariable("percentage")
  if(percentage >= 100.0) then
    text:SetText("Merlion Blast Ready!")
    blackText:SetText("Merlion Blast Ready!")
  else
    text:SetText("Charging ulti at " .. ToInt(percentage) .. "%")
    blackText:SetText("Charging ulti at " .. ToInt(percentage) .. "%")
  end
  posUpdate(dt, percentage)
end


function posUpdate(dt, percentage)
  playerPos = playerTrans:GetWorldPosition()
  merlionPos = merlionTrans:GetWorldPosition()
  x = merlionPos:x() - playerPos:x()
  x = x * x
  y = merlionPos:y() - playerPos:y()
  y = y * y
  z = merlionPos:z() - playerPos:z()
  z = z * z
  dist = x + y + z
  if(dist <= 49 and percentage>= 100.0) then
    currentX = currentX + dt*3
  else
    currentX = currentX - dt*3
  end
  
  if(currentX < 0) then
    currentX = 0
  elseif(currentX > 1.75) then
    currentX = 1.75
  end
  uiDistX = (trans:GetWorldPosition():x() + trans:GetWorldScale():x()/2)
  uiDistX = uiDistX - (coreHealthTrans:GetWorldPosition():x() - coreHealthTrans:GetWorldScale():x()/2) 
  CalculateAnchoredPosition(0,0,currentX,1, trans)
  anchorPosition.y = coreHealthTrans:GetWorldPosition():y() + coreHealthTrans:GetWorldScale():z()/2 + trans:GetWorldScale():z()
  trans:SetWorldPosition(anchorPosition)
  
  anchorPosition.x =  anchorPosition:x() + uiDistX
  anchorPosition.z = chargeTrans:GetWorldPosition():z()
  chargeTrans:SetWorldPosition(anchorPosition)
  anchorPosition.z = chargeBlackTrans:GetWorldPosition():z()
  chargeBlackTrans:SetWorldPosition(anchorPosition)
end

function CalculateAnchoredPosition(screenPosX,screenPosY,pivotX,pivotY, t)
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
  y =  pivotOffset:y() * t:GetWorldScale():y()
  z =  0-- pivotOffset:z() * t:GetWorldScale():z()
  pivotOffset = Vector3( x, y, z)
  v = v + pivotOffset
  anchorPosition =  v
  anchorPosition.z =  t:GetWorldPosition():z()
end

