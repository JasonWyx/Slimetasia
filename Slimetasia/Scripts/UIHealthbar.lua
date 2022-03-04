--settings
GO = nil
trans = nil
healthTrans = nil
healthBlackTrans = nil
healthText = nil
healthBlackText = nil

Player_PlayerScript = nil

playerHealthTrans = nil
isShaking = false
isShakingTimer = 0.0
isShakingCurrentCount = 0.0
isShakingPerCountTimer = 0.05
isShakingCount = 11.0

anchorPosition = Vector3(0,0,0)

glScript = nil
cam = nil
maxHealth = 0.0
function Constructor()
  GO = owner
  trans = GO:GetComponent("Transform") 
  
  obj = owner:GetLayer():GetObject("UI_PlayerHealth")
  healthTrans = obj:GetComponent("Transform")
  healthText = obj:GetComponent("TextRenderer") 
  
  obj = owner:GetLayer():GetObject("UI_PlayerHealthBlack")
  healthBlackTrans = obj:GetComponent("Transform")
  healthBlackText = obj:GetComponent("TextRenderer") 
  
  
  -- Find player
  GO_Player = CurrentLayer():GetObject("Player")
  if (GO_Player ~= nil) then
    Player_PlayerScript = GO_Player:GetLuaScript("PlayerScript.lua")
  end
  previousHealth = Player_PlayerScript:GetVariable("health")
  maxHealth = Player_PlayerScript:GetVariable("maxhealth")
  
  camObj = owner:GetLayer():GetObject("MainCamera")
  if (camObj ~= nil) then
    cam = camObj:GetComponent("Camera")
  end
end

function UpdateHealthPosition(dt)
    newPos    = trans:GetWorldPosition()
    newPos.x  = newPos:x()  - trans:GetWorldScale():x()/3.7
    newPos.y  = newPos:y()  - trans:GetWorldScale():z()/4
  if(isShakingCurrentCount <= 0) then
  elseif(isShakingTimer <= 0) then
    shakeOffset = trans:GetWorldScale():z()/10 * isShakingCurrentCount/isShakingCount
    newPos.x  = newPos:x()  + RandomRange(-shakeOffset,shakeOffset)
    newPos.y  = newPos:y()  + RandomRange(-shakeOffset,shakeOffset)
    isShakingTimer = isShakingPerCountTimer
    isShakingCurrentCount = isShakingCurrentCount - 1
  elseif(isShakingTimer > 0) then
    isShakingTimer = isShakingTimer - dt
  end
  newPos.z  = healthTrans:GetWorldPosition():z()
  healthTrans:SetWorldPosition(newPos)
  newPos.z  = healthBlackTrans:GetWorldPosition():z()
  healthBlackTrans:SetWorldPosition(newPos)
end

function UpdateFramePosition()
  CalculateAnchoredPosition( 0.05, 0.1425,1.0,0.0, trans)
  trans:SetWorldPosition(anchorPosition)
end

function UpdateText()
  if (CurrentLayer():GetObject("Player") ~= nil) then
    Player_PlayerScript = CurrentLayer():GetObject("Player"):GetLuaScript("PlayerScript.lua")
    if (Player_PlayerScript ~= nil) then
      dotPercent =  Player_PlayerScript:GetVariable("health")/maxHealth
      newText = ToInt( dotPercent * 100) .. "%"
      healthText:SetText(newText)
      healthBlackText:SetText(newText)
      healthText:SetColor(Color(1,dotPercent,dotPercent,1))
    end
  end
end
function OnUpdate(dt)
  if (CurrentLayer():GetObject("Player") ~= nil) then
    Player_PlayerScript = CurrentLayer():GetObject("Player"):GetLuaScript("PlayerScript.lua")
    if (Player_PlayerScript ~= nil) then
      if(previousHealth > Player_PlayerScript:GetVariable("health")) then
        isShakingCurrentCount = isShakingCount
      end
      previousHealth =  Player_PlayerScript:GetVariable("health")
      UpdateFramePosition()
      UpdateText()
      UpdateHealthPosition(dt)
    end
  end
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

