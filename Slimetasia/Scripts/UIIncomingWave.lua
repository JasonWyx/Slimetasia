--settings
local GO = nil
local TRANS = nil

local melee = nil
local range = nil
local kamikaze = nil
local tourguide = nil

local nextImage = nil
local nextImageTrans = nil
local timerObj = nil
local timerFrame = nil
local timerObjTrans = nil
local timerScript = nil
local timer = 0.0

local WaveInfo = nil
local haveWaveStarted = false
startPos = nil

local activate = true
local firstUpdate = true
local shootToStart = false
--trigger

local tekongEvent = nil

function Constructor()
  GO = owner
  melee = GO:GetLayer():GetObject("MeleeMugshot")
  range = GO:GetLayer():GetObject("RangeMugshot")
  kamikaze = GO:GetLayer():GetObject("KamikazeMugshot")
  tourguide = GO:GetLayer():GetObject("TourguideMugshot")
  nextImage = GO:GetLayer():GetObject("UI_IncomingWave")
  nextImageTrans = nextImage:GetComponent("Transform")
  timerObj = GO:GetLayer():GetObject("Timer")
  timerFrame = GO:GetLayer():GetObject("TimerFrame")
  timerScript = timerObj:GetComponent("TextRenderer")
  timerObjTrans = timerObj:GetComponent("Transform")
  TRANS = timerFrame:GetComponent("Transform")
  WaveInfo = CurrentLayer():GetObject("GameLogic"):GetLuaScript("GameLogic_VerticalSlice.lua")
  

  
  trigger = CurrentLayer():GetObject("GameLogic")
  tekongEvent = trigger:GetLuaScript("Event_Tekong.lua")
  if(tekongEvent ~= nil) then
      activate = false
  end
  
  
end

function OnUpdate(dt)
  if(firstUpdate == true) then
    firstUpdate = false
    shootToStart = WaveInfo:GetVariable("enableShootToStartWave")
    timer = 0.0
    nextImage:SetActive(false)
    timerObj:SetActive(false)
    timerFrame:SetActive(false)
    melee:SetActive(false)
    range:SetActive(false)
    kamikaze:SetActive(false)
    tourguide:SetActive(false)
    return
  end
  if(activate == false) then
    if(tekongEvent:GetVariable("currentEvent") == 7) then
      activate = true
    end
    return
  end
  if(WaveInfo:GetVariable("spawnStart") == true) then
    timer = 0.0
    nextImage:SetActive(false)
    timerObj:SetActive(false)
    timerFrame:SetActive(false)
    melee:SetActive(false)
    range:SetActive(false)
    kamikaze:SetActive(false)
    tourguide:SetActive(false)
    shootToStart = false
    return
  else
    haveWaveStarted = WaveInfo:GetVariable("waveRunning")
    currentLevel = PlayerPref_GetString      ("CurrentLevel")
    currentWave = WaveInfo:GetVariable("currentWave")+1
    waveName = ToString("Wave",ToString(currentWave))
    waveCreatures = PlayerPref_GetIntegerArray(waveName,currentLevel)
    timerObjTrans:SetWorldScale(Vector3(0.2,0.2,0.2))
    SetActiveIf(waveCreatures[1], melee)
    SetActiveIf(waveCreatures[2], range)
    SetActiveIf(waveCreatures[3], kamikaze)
    SetActiveIf(waveCreatures[4], tourguide)

    nextImage:SetActive(true)
    timerObj:SetActive(true)
    timerFrame:SetActive(true)
    timerScript:SetText("Press X\nTo\nStart")
  end
  

  UpdateTimerPosition()
  UpdateMugPosition()
end


function SetActiveIf(val, obj)
    if(val ~= 0) then
      obj:SetActive(true)
      isActive = true
    else 
      obj:SetActive(false)
    end
end

function UpdateTimerPosition()
  if(timerFrame:GetActive() == true) then
    startPos = TRANS:GetWorldPosition()
    startPos.z = -1
    nextPos = startPos
    if(shootToStart == true) then
      nextPos.x = startPos:x() - timerObjTrans:GetWorldScale():x()
      timerObjTrans:SetWorldPosition(nextPos)
    else
      nextPos.x = startPos:x() - timerObjTrans:GetWorldScale():x()/4
      timerObjTrans:SetWorldPosition(nextPos)
    end
    
    nextPos = startPos
    nextPos.x = nextPos:x() + TRANS:GetWorldScale():x()
    nextPos.x = nextPos:x() + nextImageTrans:GetWorldScale():x()/3
    nextPos.y = nextPos:y() - TRANS:GetWorldScale():z()/4
    nextPos.y = nextPos:y() - nextImageTrans:GetWorldScale():z()/2
    nextImageTrans:SetWorldPosition(nextPos)
    
  end
end

function UpdateMugPosition()
    startPos = TRANS:GetWorldPosition()
    startPos.x = startPos:x() + TRANS:GetWorldScale():x()/2
    startPos.z = -3
    if(melee:GetActive() == true) then
       GetNextPosition(melee:GetComponent("Transform"), startPos)
    end
    if(range:GetActive() == true) then
       GetNextPosition(range:GetComponent("Transform"), startPos)
    end
    if(kamikaze:GetActive() == true) then
       GetNextPosition(kamikaze:GetComponent("Transform"), startPos)
    end
    if(tourguide:GetActive() == true) then
       GetNextPosition(tourguide:GetComponent("Transform"), startPos)
    end
end

function GetNextPosition(imgTrans, startPos)
    startPos.x = startPos:x() +  imgTrans:GetWorldScale():x()/2
    imgTrans:SetWorldPosition(startPos)
    startPos.x = startPos:x() +  imgTrans:GetWorldScale():x()/2
end
