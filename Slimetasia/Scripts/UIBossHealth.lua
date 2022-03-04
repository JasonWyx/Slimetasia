--settings
GO = nil
trans = nil
healthTrans = nil
healthBlackTrans = nil
healthText = nil
healthBlackText = nil

SpawnerScript = nil

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

pivotY = 0.0
function Constructor()
  GO = owner
  trans = GO:GetComponent("Transform") 
  
  obj = owner:GetLayer():GetObject("UI_BossHealth")
  healthTrans = obj:GetComponent("Transform")
  healthText = obj:GetComponent("TextRenderer") 
  
  obj = owner:GetLayer():GetObject("UI_BossHealthBlack")
  healthBlackTrans = obj:GetComponent("Transform")
  healthBlackText = obj:GetComponent("TextRenderer") 
  
  
  playerHealthTrans = owner:GetLayer():GetObject("UI_PlayerHealthFrame"):GetComponent("Transform") 
  
  gameLogic = CurrentLayer():GetObject("GameLogic")
  glScript = gameLogic:GetLuaScript("GameLogic_VerticalSlice.lua")
  
  camObj = owner:GetLayer():GetObject("MainCamera")
  if (camObj ~= nil) then
    cam = camObj:GetComponent("Camera")
  end
end

function UpdateHealthPosition(dt)
    newPos    = trans:GetWorldPosition()
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
  CalculateAnchoredPosition( 0.5, 1.0,0.5,pivotY, trans)
  trans:SetWorldPosition(anchorPosition)
end

function UpdateText()
    spawner = CurrentLayer():GetObject("Slime_Spawner")
    curhealth = 0
    if(spawner ~= nil) then
      curhealth = SpawnerScript:GetVariable("health")
    end
    
  dotPercent =  curhealth/maxHealth
  newText = ToInt(curhealth/maxHealth * 100) .. "%"
  healthText:SetText(newText)
  healthBlackText:SetText(newText)
  healthText:SetColor(Color(1,dotPercent,dotPercent,1))
end
function OnUpdate(dt)
  if( glScript:GetVariable("spawnStart") == true) then
    if(SpawnerScript == nil) then
      spawner = CurrentLayer():GetObject("Slime_Spawner")
      if(spawner == nil) then 
        healthText:SetText("0%")
        healthBlackText:SetText("0%")
      else
        SpawnerScript = CurrentLayer():GetObject("Slime_Spawner"):GetLuaScript("Health.lua")
        if(SpawnerScript ~= nil) then
          previousHealth = SpawnerScript:GetVariable("health")
          maxHealth = previousHealth
        else
          previousHealth = 0
          maxHealth = 0
        end
      end
    end
  else  
    pivotY = 2
    UpdateFramePosition()
    healthText:SetText("")
    healthBlackText:SetText("")
    UpdateHealthPosition(dt)
    return
  end
  pivotY =  pivotY - dt
  if(pivotY < -0.2) then
    pivotY = -0.2
  end
  
  spawner = CurrentLayer():GetObject("Slime_Spawner")
  if(spawner~= nil) then
    SpawnerScript = spawner:GetLuaScript("Health.lua")
    if (SpawnerScript ~= nil) then
      if(previousHealth >  SpawnerScript:GetVariable("health")) then
        isShakingCurrentCount = isShakingCount
      end
      previousHealth =  SpawnerScript:GetVariable("health")
    end
  else
    previousHealth = 0
  end
  
  UpdateFramePosition()
  UpdateText()
  UpdateHealthPosition(dt)
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