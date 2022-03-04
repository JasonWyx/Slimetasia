--settings
local GO = nil
local UI_WaveBg = nil
local UI_WaveBegin = nil
local UI_WaveEnd = nil
local UI_Win = nil
local UI_Lose = nil
local WaveInfo = nil

local currentWave = -1
local totalWave = 0
local prevWave = false

local stayTimer = 0.0
local stayMaxTime = 2.0
local coreLogic = nil
local winSound = nil

local waveComplete = false
local activate = true
local tekongEvent = nil


local prevWaveRunning = false
local firstUpdate = true

function Constructor()
  GO = owner
  s = CurrentLayer():GetObject("Sound")
  if(s ~= nil) then
    winSound =  s:GetComponent("AudioEmitter")
  end
  UI_WaveBg     = GO:GetLayer():GetObject("UI_WaveAnnouncement"):GetLuaScript("UIActivator.lua")
  UI_WaveBegin  = GO:GetLayer():GetObject("UI_WaveBegin"):GetLuaScript("UIActivator.lua")
  UI_WaveEnd    = GO:GetLayer():GetObject("UI_WaveEnd"):GetLuaScript("UIActivator.lua")
  UI_Win    = GO:GetLayer():GetObject("UI_WinAnnouncement"):GetLuaScript("UIActivator.lua")
  UI_Lose    = GO:GetLayer():GetObject("UI_LoseAnnouncement"):GetLuaScript("UIActivator.lua")
  coreLogic = CurrentLayer():GetObject("Core"):GetLuaScript("CoreLogic.lua")
  WaveInfo     = CurrentLayer():GetObject("GameLogic"):GetLuaScript("GameLogic_VerticalSlice.lua")
  
  trigger = CurrentLayer():GetObject("GameLogic")
end

function OnUpdate(dt)
  if(firstUpdate == true) then
    firstUpdate = false 
    
    tekongEvent = trigger:GetLuaScript("Event_Tekong.lua")
    if(tekongEvent ~= nil) then
        activate = false
    end
    
    levelName        = PlayerPref_GetString      ("CurrentLevel")
    totalWave = PlayerPref_GetInteger     ("WaveCount"     , levelName)   
    return
  end
  if(WaveInfo:GetVariable("winLiao") == true) then
    UI_Win:CallFunction("Activate")
  end
  
  if(activate == false) then
    if(tekongEvent:GetVariable("currentEvent") == 5) then
      activate = true
    end
    return
  end  
  
  if(lateStart == true) then
    lateStart = false  
  end
  
  if(coreLogic:GetVariable("CoreHealth") <= 0) then
    Finish()
    UI_Lose:CallFunction("Activate")
    return
   end
   
  if( WaveInfo:GetVariable("currentWave") > totalWave) then
    if(tekongEvent == nil) then
      UI_Win:CallFunction("Activate")
    end
    if(waveComplete == false and winSound ~= nil) then
      winSound:SetAndPlayAudioClip("SFX_WinEffect")
    end
    waveComplete = true
    return
  end
  
  if(stayTimer > 0) then
      stayTimer = stayTimer - dt
      if(stayTimer <= 0.0) then
        stayTimer = 0.0
        Finish()
      end
  end
  if( WaveInfo:GetVariable("waveRunning") == true) then
    if(prevWaveRunning == false) then
      Begin()
      stayTimer = stayMaxTime
      currentWave = WaveInfo:GetVariable("currentWave")
      winSound:SetAndPlayAudioClip("SFX_WinEffect")
      prevWaveRunning = true
    end
  end
  
  if( WaveInfo:GetVariable("waveRunning") == false) then
    if(prevWaveRunning == true) then
      End()
      stayTimer = stayMaxTime
      currentWave = WaveInfo:GetVariable("currentWave")
      winSound:SetAndPlayAudioClip("SFX_WinEffect")
      prevWaveRunning = false
    end
  end
end

function Begin()
    UI_WaveBg:CallFunction("Activate")
    UI_WaveBegin:CallFunction("Activate")
    UI_WaveEnd:CallFunction("Deactivate")
end

function End()
    UI_WaveBg:CallFunction("Activate")
    UI_WaveBegin:CallFunction("Deactivate")
    UI_WaveEnd:CallFunction("Activate")
end

function Finish()
    UI_WaveBg:CallFunction("Deactivate")
    UI_WaveBegin:CallFunction("Deactivate")
    UI_WaveEnd:CallFunction("Deactivate")
end