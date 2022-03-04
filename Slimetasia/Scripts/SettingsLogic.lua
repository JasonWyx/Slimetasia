--settings
local state = 0
local maxState = 5

 local logoObj = nil
 
 local objects  = {"Text_BGM", "Text_SFX", "Text_Sensitivity", "Text_ToggleFullscreen", "Text_Return"}
 local names  = {}
 local options = {}
 local trans  = {}
 local oTrans  = {}
 
 
 firstUpdate = true
 
 local instructions = nil
 
 timer = 0.0
 newPos = Vector3()
 camObj = nil
 cam = nil
 
 prevSpeed = 0
 
 fullscreen = true
 
 delay = 1
 local selectSfx = "Menu_Select"
 local confirmSfx = "Game_Start_play"
 
 
 bgmBar = nil
 sfxBar = nil
 sensitivityBar = nil
 
 firstUpdate = true
 
 prev = false
function Constructor()
     for i = 1, maxState do
      createObj(i, objects[i])
      if(options[i] == nil) then
        maxState = maxState - 1
        i = i - 1
      end
     end
  sfxBar = owner:GetLayer():GetObject("SFXBar"):GetLuaScript("UIOptionBar.lua")
  bgmBar = owner:GetLayer():GetObject("BGMBar"):GetLuaScript("UIOptionBar.lua")
  sensitivityBar = owner:GetLayer():GetObject("SensitivityBar"):GetLuaScript("UIOptionBar.lua")
  
  camObj = owner:GetLayer():GetObject("MainCamera")
  if (camObj ~= nil) then
    cam = camObj:GetComponent("Camera")
  end
end

function createObj(index, name)
  names[index]   =  name
  options[index] = owner:GetLayer():GetObject(name)
  trans[index]  = options[index]:GetComponent("Transform")
  temporaryObj = owner:GetLayer():GetObject(name .. "O")
  oTrans[index]  = temporaryObj:GetComponent("Transform")
end

function OnUpdate(dt)
  if(firstUpdate == true) then
    camObj:SetActive(false)
    firstUpdate = false
  
  name = options[4]:Name()
  owner:GetLayer():GetObject(name .. "O"):GetComponent("TextRenderer"):SetText("W")
  options[4]:GetComponent("TextRenderer"):SetText("  indow")
  end
  if(camObj:GetActive() == false) then
    state = 0
    prev = camObj:GetActive()
    return
  end
   if(prev == false) then
     updateSelectedInput()
  end
  prev = camObj:GetActive()
  posY = 0.8
  for i = 1, maxState do
    calculatePosition(0.15,posY,1.0,0.5,trans[i])
    trans[i]:SetWorldPosition(newPos)
    oTrans[i]:SetWorldPosition(newPos)
    posY = posY - 0.1
  end
  if( delay > 0) then
    delay = delay - UnscaledDT()
    return
  end
  if(ControllerAxis( "Up" ) > 0.0) then
    prev = state
    if(prevSpeed == 0) then 
      state = state - 1      
      delay = 0.1
    else
      state = state - ControllerAxis( "Up" ) * UnscaledDT() * 4
    end
    if(state < 0) then state = 0 end
    if(ToInt(state) ~= ToInt(prev) ) then 
      AudioSystem_PlayAudioAtLocation(selectSfx, Vector3(0, 0, 0), 1, 50, 500)
    end
    updateSelectedInput()
  elseif(ControllerAxis( "Down" ) > 0.0)  then
    prev = state
    if(prevSpeed == 0) then 
      state = state + 1
      delay = 0.1
    else
      state = state + ControllerAxis( "Down" ) * UnscaledDT() * 4
    end
    if(state > maxState-1) then state = maxState-1 end
    if(ToInt(state) ~= ToInt(prev) ) then 
      AudioSystem_PlayAudioAtLocation(selectSfx, Vector3(0, 0, 0), 1, 50, 500)
    end
    updateSelectedInput()
  end
  prevSpeed = ControllerAxis( "Up" ) + ControllerAxis( "Down" )
  if(ControllerUp("Jump")) then
    selectInput()
    AudioSystem_PlayAudioAtLocation(confirmSfx, Vector3(0, 0, 0), 1, 50, 500)
  end
  
end

function updateSelectedInput()
   for i = 1, maxState do
    options[i]:GetComponent("TextRenderer"):SetColor(Color(1.0,1.0,1.0,1.0))
   end
    
   optionIndex = ToInt(state) + 1
   
  if(names[optionIndex] == "Text_Sensitivity") then
    if(sensitivityBar ~=nil)then
      sensitivityBar:SetVariable("isActive", true)
    end
  else
    if(sensitivityBar ~= nil)then
      sensitivityBar:SetVariable("isActive", false)
    end
  end
  
  if(names[optionIndex] == "Text_SFX") then
    if(sensitivityBar ~=nil)then
      sfxBar:SetVariable("isActive", true)
    end
  else
    if(sensitivityBar ~= nil)then
      sfxBar:SetVariable("isActive", false)
    end
  end
  
  if(names[optionIndex] == "Text_BGM") then
    if(sensitivityBar ~=nil)then
      bgmBar:SetVariable("isActive", true)
    end
  else
    if(sensitivityBar ~= nil)then
      bgmBar:SetVariable("isActive", false)
    end
  end
  
  options[optionIndex]:GetComponent("TextRenderer"):SetColor(Color(0.815,0.552,0.058,1.0))
end

function selectInput()
   optionIndex = ToInt(state) + 1
   
  if(names[optionIndex] == "Text_ToggleFullscreen") then
    fullscreen = not fullscreen
    ToggleFullscreen(fullscreen)
    if(fullscreen == false) then
      name = options[optionIndex]:Name()
      options[optionIndex]:GetComponent("TextRenderer"):SetText("  ullscreen")
      owner:GetLayer():GetObject(name .. "O"):GetComponent("TextRenderer"):SetText("F")
    else
      name = options[optionIndex]:Name()
      owner:GetLayer():GetObject(name .. "O"):GetComponent("TextRenderer"):SetText("W")
      options[optionIndex]:GetComponent("TextRenderer"):SetText("  indow")
    end
  elseif(names[optionIndex] == "Text_Return") then
      camObj:SetActive(false)
  elseif(names[optionIndex] == "Text_ReturnToMap") then
      SetTimeScale(1)
      SceneLoad("LevelSelect")
  elseif(names[optionIndex] == "Text_ExitGame") then
      SetTimeScale(1)
      SceneQuit()
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

