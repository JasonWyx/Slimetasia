--settings
local state = 0

local selectLevelObj = nil
local settingObj = nil
local exitGameObj = nil
 local bgObj = nil
 
 local logoObj = nil
 local mStart = nil
 local mSetting = nil
 local mQuit = nil

 firstUpdate = true
 
 local instructions = nil
 
 timer = 0.0

blackScreen = nil
transitionTimer = 0
function Constructor()
  owner:GetLayer():GetObject("Sound"):GetComponent("AudioEmitter"):SetAndPlayAudioClip("BGM_MainMenu")
  bgObj = owner:GetLayer():GetObject("Background")
  logoObj = owner:GetLayer():GetObject("MLogo")
  mStart = owner:GetLayer():GetObject("MStart")
  mSetting = owner:GetLayer():GetObject("MSetting")
  mQuit = owner:GetLayer():GetObject("MQuit")
  bg = bgObj:GetLuaScript("MainMenuBackgroundLogic.lua")
  state = 0
  mQuit:SetActive(true)
  mSetting:SetActive(true)
  mStart:SetActive(true)
  logoObj:SetActive(true)
  blackScreen = GetLayer("Transition"):GetObject("UI_Transition"):GetLuaScript("UISlide.lua")
  ShowMouseCursor(false)
end

function OnUpdate(dt)
  if( firstUpdate == true ) then
    UpdateColor()
    updateControllerInput()
    firstUpdate  = false
    blackScreen:CallFunction("DisappearInstant")
    return
  end 
  if(transitionTimer > 0)then
      transitionTimer = transitionTimer - dt
      if(transitionTimer <= 0)then
        SceneLoad("LevelSelect")
      end
      return
  end
  if(state ~= 3) then
    if(timer <= 0.0) then
      if(ControllerAxis( "Up" ) > 0.5) then
        owner:GetLayer():GetObject("SFX"):GetComponent("AudioEmitter"):SetAndPlayAudioClip("SFX_Shoot.ogg")
        state = state - 1
        if(state < 0) then
          state = 0
        end
        UpdateColor()
        updateControllerInput()
        timer = 0.1
      elseif(ControllerAxis( "Down" ) > 0.5) then
        owner:GetLayer():GetObject("SFX"):GetComponent("AudioEmitter"):SetAndPlayAudioClip("SFX_Shoot.ogg")
        state = state + 1
        if(state > 2) then
          state = 2
        end
        UpdateColor()
        updateControllerInput()
        timer = 0.1
      end
    else
      timer = timer - dt
    end
  end
  
  if(ControllerPress("Jump")) then
    if(state == 0) then
      blackScreen:CallFunction("Appear")
      transitionTimer = 1.0
    elseif(state == 2) then
      SceneQuit()
    end
  end
end

function updateControllerInput()
  if(state == 0) then
    mStart:GetComponent("MeshRenderer"):SetColor(Color(1.0,1.0,1.0,1.0))
    mSetting:GetComponent("MeshRenderer"):SetColor(Color(0.5,0.5,0.5,1.0))
    mQuit:GetComponent("MeshRenderer"):SetColor(Color(0.5,0.5,0.5,1.0))
  elseif(state == 1) then
    mStart:GetComponent("MeshRenderer"):SetColor(Color(0.5,0.5,0.5,1.0))
    mSetting:GetComponent("MeshRenderer"):SetColor(Color(1.0,1.0,1.0,1.0))
    mQuit:GetComponent("MeshRenderer"):SetColor(Color(0.5,0.5,0.5,1.0))
  elseif(state == 2) then
    mStart:GetComponent("MeshRenderer"):SetColor(Color(0.5,0.5,0.5,1.0))
    mSetting:GetComponent("MeshRenderer"):SetColor(Color(0.5,0.5,0.5,1.0))
    mQuit:GetComponent("MeshRenderer"):SetColor(Color(1.0,1.0,1.0,1.0))
  end
end

function UpdateColor()
  if(state < 1) then
    bg:SetVariable("nextColor",Color(0.8,0.4,0,1))
  elseif(state < 2) then
    bg:SetVariable("nextColor",Color(0.8,0.5,0,1))
  elseif(state < 3) then
    bg:SetVariable("nextColor",Color(0.8,0.6,0,1))
  elseif(state < 4) then
    bg:SetVariable("nextColor",Color(1,1,1,1))
  end 
end
