--settings
GO = nil
trans = nil
healthTrans = nil
healthBlackTrans = nil
healthText = nil
healthBlackText = nil

CoreScript = nil

playerHealthTrans = nil

isShaking = false
isShakingTimer = 0.0
isShakingCurrentCount = 0.0
isShakingPerCountTimer = 0.05
isShakingCount = 11.0
function Constructor()
  GO = owner
  trans = GO:GetComponent("Transform") 
  
  obj = owner:GetLayer():GetObject("UI_CoreHealth")
  healthTrans = obj:GetComponent("Transform")
  healthText = obj:GetComponent("TextRenderer") 
  
  obj = owner:GetLayer():GetObject("UI_CoreHealthBlack")
  healthBlackTrans = obj:GetComponent("Transform")
  healthBlackText = obj:GetComponent("TextRenderer") 
  
  CoreScript = CurrentLayer():GetObject("Core"):GetLuaScript("CoreLogic.lua")
  previousHealth = CoreScript:GetVariable("CoreHealth")
  
  playerHealthTrans = owner:GetLayer():GetObject("UI_PlayerHealthFrame"):GetComponent("Transform") 
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
  newPos = playerHealthTrans:GetWorldPosition()
  newPos.y = newPos:y()+playerHealthTrans:GetWorldScale():z()/2 + trans:GetWorldScale():z()/2
  trans:SetWorldPosition(newPos)
end

function UpdateText()
  dotPercent = CoreScript:GetVariable("CoreHealth")/20
  newText = ToInt(CoreScript:GetVariable("CoreHealth"))
  healthText:SetText(newText)
  healthBlackText:SetText(newText)
  healthText:SetColor(Color(1,dotPercent,dotPercent,1))
end
function OnUpdate(dt)
  if(previousHealth >  CoreScript:GetVariable("CoreHealth")) then
    isShakingCurrentCount = isShakingCount
  end
    previousHealth = CoreScript:GetVariable("CoreHealth")
  UpdateFramePosition()
  UpdateText()
  UpdateHealthPosition(dt)
end
