--settings
local up = {}
local down = {}
local left = {}
local right = {}

local upSize = 0
local downSize = 0
local leftSize = 0
local rightSize = 0

local upTrans = nil
local downTrans = nil
local leftTrans = nil
local rightTrans = nil

local GO = nil
local TRANS = nil

local speed = 70.0
local current = nil
local currentTrans = nil

local LSSSelect =  nil
local LSSelectTrans  = nil
local LSDown =  nil
local LSDownTrans  = nil
local LSUp =  nil
local LSUpTrans  = nil
local currentController = false
    
showDirection = false

firstUpdate = true


--local playerModel_initialRotation = Vector3()
--local animation_jump   = "WeaponJump_MeshAnimationSet"
--local animation_walk   = "Weapon_Run_Mesh_01AnimationSet"
--local animation_idle   = "Idle_Mesh_03AnimationSet"

--local animBookIdle        = "BookIdle"
--local animBookStrafe      = "BookStrafe"
--local animWeaponIdle      = "WeaponIdle"
--local animWeaponJump      = "WeaponJump"
--local animWeaponRun01     = "WeaponRun01"
--local animWeaponRun02     = "WeaponRun02"
--local animWeaponStrafe01  = "WeaponStrafe01"
--local animWeaponStrafe02  = "WeaponStrafe02"
--local animWeaponSwitch    = "WeaponSwitch"
--local currentAnimation = nil


playerModel_transform       = nil
--playerModel_initialRotation = nil
--playerModel_meshAnimator    = nil

target = ""
targetTrans = nil
locations = {}
locationsSize = 0

currentIndex = 1
sm = nil;

function Constructor()
  GO = owner
  TRANS = GO:GetComponent("Transform")
  --playerModel_meshAnimator    = GO:GetComponent("MeshAnimator")
  current = GO:GetLayer():GetObject("P11")
  LevelInfo = PlayerPref_GetString("CurrentLevel")
  if(LevelInfo == "Level_Tekong") then
    current = GO:GetLayer():GetObject("P11")
  elseif(LevelInfo == "Level_ChinaTown") then
    current = GO:GetLayer():GetObject("P21")
  elseif(LevelInfo == "Level_Changi") then
    current = GO:GetLayer():GetObject("P25")
  end
  TRANS:SetWorldPosition(current:GetComponent("Transform"):GetWorldPosition())
  currentScript = current:GetLuaScript("LSPoint.lua")
  currentController = GetControllerInput()
  sm = GO:GetLayer():GetObject("SM"):GetLuaScript("LSSceneManager.lua")
  LSUp = GetLayer("UI"):GetObject("LSUpArrow")
  LSDown = GetLayer("UI"):GetObject("LSDownArrow")
  LSSSelect = GetLayer("UI"):GetObject("LSSelect")
  --hide the cursor
  ShowMouseCursor(false)
end

function OnUpdate(dt)
  if(firstUpdate == true) then
    firstUpdate = false
    LSUp:SetActive(false) 
    LSDown:SetActive(false) 
    updateControls()
    return
  end
  
  --Automove mode
  if(locationsSize > 0) then
      LSSSelect:SetActive(false)
      LSDown:SetActive(false)
      LSUp:SetActive(false)
      autoMove(dt)
  else
    
    --if (currentAnimation == nil or currentAnimation ~= animBookIdle) then
    --  currentAnimation = animBookIdle
    --  playerModel_meshAnimator:Play(animBookIdle)
    --end
    if(ControllerAxis("Up")> 0.5) then
      locations = up
      locationsSize = upSize
    elseif(ControllerAxis("Down") > 0.5) then
      locations = down
      locationsSize = downSize
    elseif(ControllerAxis("Left") > 0.5) then 
      locations = left
      locationsSize = leftSize
    elseif(ControllerAxis("Right") > 0.5) then 
      locations = right
      locationsSize = rightSize
    elseif(currentScript:GetVariable("allowInteraction") == true) then
      LSSSelect:SetActive(true)
      if(upSize > 0) then LSUp:SetActive(true) end
      if(downSize > 0) then LSDown:SetActive(true) end
      if(ControllerPress("Jump")) then
        upSize = 0
        downSize = 0
        leftSize = 0
        rightSize = 0
        sm:SetVariable("levelName", currentScript:GetVariable("levelName"))
        sm:SetVariable("levelPref", currentScript:GetVariable("levelPref"))
        sm:CallFunction("TransitToNextLevel")
      end
    end

    if(locationsSize > 0) then
      target = GO:GetLayer():GetObject(locations[currentIndex])
      targetTrans = target:GetComponent("Transform")
    end
  end
end


function autoMove(dt)
  --Set a direction to move towards
  direction = targetTrans:GetWorldPosition() - TRANS:GetWorldPosition()
  step = dt * speed;
  stepVec = direction:Normalized()*dt*speed;
  
  --Set a new rotation
  directiontwo = direction;
  rotation = TRANS:GetWorldRotation()
  rotation.y = directiontwo:PolarAngle():y() + 90
  rotation.z = directiontwo:PolarAngle():z()
  rotation.x = directiontwo:PolarAngle():x() + 270
  TRANS:SetWorldRotation(rotation)
  
  --if (currentAnimation == nil or currentAnimation ~= animWeaponRun01) then
  --    currentAnimation = animWeaponRun01
  --    playerModel_meshAnimator:Play(animWeaponRun01)
  --end
  if(direction:Length() > step + 1.5) then
    pos = TRANS:GetWorldPosition() +  stepVec
    TRANS:SetWorldPosition(pos)
  else
    pos = targetTrans:GetWorldPosition()
    TRANS:SetWorldPosition(pos)
    currentIndex = currentIndex + 1;
    if(currentIndex <= locationsSize) then
      target = GO:GetLayer():GetObject(locations[currentIndex])
      targetTrans = target:GetComponent("Transform")
    else
      current = target
      target = nil
      locationsSize = -1
      currentIndex = 1
      updateControls()
    end
  end
end


function updateControls()
  if(current~=nil) then
    currentScript = current:GetLuaScript("LSPoint.lua")
    currentScript:SetVariable("toGet", "up")
    currentScript:CallFunction("GetObject")
    if(currentScript:GetVariable("receiveVariable") == true) then
      upSize =  currentScript:GetVariable("receiveVariableSize")
      for i = 1, upSize do
        currentScript:CallFunction("NextName")
        name = currentScript:GetVariable("receiveVariableName")
        up[i] = name
      end
    else
      up = {}
      upSize = 0
    end
    
    currentScript:SetVariable("toGet", "down")
    currentScript:CallFunction("GetObject")
    if(currentScript:GetVariable("receiveVariable") == true) then
      downSize =  currentScript:GetVariable("receiveVariableSize")
      for i = 1, downSize do
        currentScript:CallFunction("NextName")
        down[i] = currentScript:GetVariable("receiveVariableName")
      end
    else
      down = {}
      downSize = 0
    end
    
    currentScript:SetVariable("toGet", "left")
    currentScript:CallFunction("GetObject")
    if(currentScript:GetVariable("receiveVariable") == true) then
      leftSize =  currentScript:GetVariable("receiveVariableSize")
      for i = 1, leftSize do
        currentScript:CallFunction("NextName")
        left[i] = currentScript:GetVariable("receiveVariableName")
      end
    else
      left = {}
      leftSize = 0
    end
    
    currentScript:SetVariable("toGet", "right")
    currentScript:CallFunction("GetObject")
    if(currentScript:GetVariable("receiveVariable") == true) then
      right =  currentScript:GetVariable("receiveVariableSize")
      for i = 1, rightSize do
        currentScript:CallFunction("NextName")
        right[i] = currentScript:GetVariable("receiveVariableName")
      end
    else
      right = {}
      rightSize = 0
    end
  end
end

