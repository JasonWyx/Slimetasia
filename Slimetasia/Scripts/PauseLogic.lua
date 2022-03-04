--settings
local state = 0
local maxState = 0

local deltaFocus = false

 local logoObj = nil
 
 local objects  = {"Text_Resume" ,"Text_Settings",  "Text_HowToPlay",  "Text_ReturnToMap", "Text_ExitGame"}
 local names  = { "  esume", "  ettings","  ow To Play", "  eturn To Map", "  uit Game"}
 local namesO  = {"R","S",  "H", "R", "Q"}
 
 local Xpos      = {0,0,0,0,0}
 local dtMultiplier  = {2.4, 2.0, 1.6, 1.2, 0.8}
 
 local options = {}
 local optionsO = {}
 
 local trans  = {}
 local oTrans  = {}
 
 
 firstUpdate = true
 
 local instructions = nil
 
 timer = 0.0
 newPos = Vector3()
 
 cam = nil
 camObj = nil
 settingsCamObj = nil
 confirmCamObj = nil
 howToPlayCamObj = nil
 
 prevSpeed = 0
 
 fullscreen = true
 
 delay = 1
 local selectSfx = "Menu_Select"
 local confirmSfx = "Game_Start_play"
 
 
 
 firstUpdate = true
 
 prevState = 0
 prevOffset = 0
 
 inControl = true
 isPaused = false
 
 leastXpos = -0.8
 maxXpos = 0.15
 
 bgObj = nil
function Constructor()

  
  GO = owner
  camObj = owner:GetLayer():GetObject("MainCamera")
  settingsCamObj =GetLayer("SettingsLayer"):GetObject("MainCamera")
  confirmCamObj = GetLayer("ConfirmationLayer"):GetObject("MainCamera")
  howToPlayCamObj = GetLayer("HowToPlay"):GetObject("MainCamera")
  bgObj  = GetLayer("PauseLayer"):GetObject("BlackBackground")
  if (camObj ~= nil) then
    cam = camObj:GetComponent("Camera")
  end
      SetTimeScale(1.0)
end

function createObj(index, name)
  options[index] = GetLayer("PauseLayer"):Create(objects[index])
  trans[index]  = options[index]:AddComponent("Transform")
  t = options[index]:AddComponent("TextRenderer")
  t:SetText(names[index])
  t:SetFont("BD_CARTOON_SHOUT")
  
  optionsO[index] = GetLayer("PauseLayer"):Create(objects[index] .. "O")
  oTrans[index]  = optionsO[index]:AddComponent("Transform")
  t = optionsO[index]:AddComponent("TextRenderer")
  t:SetText(namesO[index])
  t:SetFont("BD_CARTOON_SHOUT")
  t:SetColor(Color(1.0,0.5,0.25,1.0))
 
end

function OnUpdate(dt)
  if(firstUpdate == true) then
     firstUpdate = false
     maxState = #objects     
     for i = 1, maxState do
      createObj(i, objects[i])
     end
     UpdatePosition(2000)
  end
  
  UpdatePosition(UnscaledDT())
  
  inControl = (settingsCamObj:GetActive() == false) and (confirmCamObj:GetActive() == false) and (howToPlayCamObj:GetActive() == false)
  if(inControl == false) then return end
  
  if(not deltaFocus and GotFocus()) then
  isPaused = true
        bgObj:GetLuaScript("UICamBgFade.lua"):CallFunction("activate")
        SetTimeScale(0.001)
  end
  deltaFocus = GotFocus()
  
  if (ControllerPress("ReturnToLevelSelect")) then
      isPaused = not isPaused
      if( isPaused == false) then
        bgObj:GetLuaScript("UICamBgFade.lua"):CallFunction("deactivate")
        SetTimeScale(1.0)
      else
        bgObj:GetLuaScript("UICamBgFade.lua"):CallFunction("activate")
        SetTimeScale(0.001)
      end
      return
  end
  
  if(isPaused == false) then return end
    currentOffset =  -ControllerAxis( "Up" ) * UnscaledDT() * 4 
    currentOffset = currentOffset + ControllerAxis( "Down" ) * UnscaledDT() * 4
    if(prevOffset == 0) then
      if(currentOffset < 0)     then state = state - 1
      elseif(currentOffset > 0) then state = state + 1 end
    else
      state = state + currentOffset
    end
    if(state > maxState) then state = 0 end
    if(state < 0) then state = maxState - 1 end
    prevOffset = currentOffset
    updateSelectedInput()
    if(prevState ~= state) then
      if(ToInt(state) ~= ToInt(prevState) ) then 
        AudioSystem_PlayAudioAtLocation(selectSfx, Vector3(0, 0, 0), 1, 50, 500)
      end
    end
    prevState = state
    
  if(ControllerUp("Jump")) then
    AudioSystem_PlayAudioAtLocation(confirmSfx, Vector3(0, 0, 0), 1, 50, 500)
    selectInput()
  end
  
end

function updateSelectedInput()
  for i = 1, maxState do
    options[i]:GetComponent("TextRenderer"):SetColor(Color(1.0,1.0,1.0,1.0))
  end
  optionIndex = ToInt(state) + 1
  options[optionIndex]:GetComponent("TextRenderer"):SetColor(Color(0.815,0.552,0.058,1.0))
end

function selectInput()
   optionIndex = ToInt(state) + 1
  if(objects[optionIndex] == "Text_Resume") then
    bgObj:GetLuaScript("UICamBgFade.lua"):CallFunction("deactivate")
    isPaused = false
    SetTimeScale(1.0)
  elseif(objects[optionIndex] == "Text_Settings") then
    settingsCamObj:SetActive(true)
  elseif(objects[optionIndex] == "Text_HowToPlay") then
    howToPlayCamObj:SetActive(true)
  elseif(objects[optionIndex] == "Text_ReturnToMap") then
    SetTimeScale(1)
    SceneLoad("LevelSelect")
  elseif(objects[optionIndex] == "Text_ExitGame") then
    confirmCamObj:SetActive(true)
  end
end

function UpdatePosition(dt)
  posY = 0.8
  for i = 1, maxState do
    skip = false
    if(isPaused) then
      if(Xpos[i] >= maxXpos) then
         skip = true
      end
      Xpos[i] = Xpos[i] + (dt*dtMultiplier[i])
    else
      if(Xpos[i] <= leastXpos) then
         skip = true
      end
      Xpos[i] = Xpos[i] - (dt*dtMultiplier[i])
    end
    if(skip == false) then
      if(Xpos[i] < leastXpos) then Xpos[i] = leastXpos end
      if(Xpos[i] > maxXpos) then Xpos[i] = maxXpos end
      
      calculatePosition(Xpos[i],posY,1.0,0.5,trans[i])
      trans[i]:SetWorldPosition(newPos)
      oTrans[i]:SetWorldPosition(newPos)
    end
    posY = posY - 0.1
  end
end

function calculatePosition(screenPosX,screenPosY,pivotX,pivotY, t)
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
  newPos =  v
end




