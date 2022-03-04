-- VARIABLES ===================================================================
local isConstruct = false
local cutSceneState = 0
local currentEvent = 0
local timer_EventStart = 0
local camerGO = nil 
local camera = nil
local cameraTransform = nil
local timer = 0
local timer2 = 0
local vector = Vector3()
local vector2 = Vector3()
local vector3 = Vector3()
local vector4 = Vector3()
local vector5 = Vector3()
local vector6 = Vector3()
local vector7 = Vector3()


local winState = 0

-- Event 1 : Boat comes in
local event1_startdelay  = 0.01
local boatGO             = nil
local boatTransform      = nil
local Player             = nil
local PlayerTransform     = nil
local TombStone          = nil
local TombStoneTransform = nil
local Encik              = nil
local EncikTransform     = nil

-- Event 2 : show credits
local event2_startdelay = 0.01
local creditGO          = nil
local creditTransform   = Vector3()

-- Event 3 : boat moves into the sunset
local event3_startdelay = 0.01
local sunGO = nil
local sunTransform = nil

local winState = 0

-- FUNCTIONS ===================================================================
-- Note: Constructor might call multiple copies, hence do not create stuff in
-- Constructor
function Constructor()
end

--ONLY CALLS ONCE
function MyConstructor()
  camerGO = CurrentLayer():GetObject("MainCamera")
  camera = camerGO:GetComponent("Camera")
  cameraTransform = camerGO:GetComponent("Transform")
  cameraEmitter = camerGO:GetComponent("AudioEmitter")
  
  boatGO = CurrentLayer():GetObject("Boat")
  boatTransform = boatGO:GetComponent("Transform")
  
  sunGO = CurrentLayer():GetObject("Sun")
  sunTransform = sunGO:GetComponent("Transform")
  
  Player    = CurrentLayer():GetObject("Player")
  TombStone = CurrentLayer():GetObject("Tomb")
  Encik     = CurrentLayer():GetObject("Encik")
  
  creditGO = CurrentLayer():GetObject("Credits")
  creditTransform = creditGO:GetComponent("Transform")
  
  -- Win state (caters for when the player does not win the game)
  winState = PlayerPref_GetInteger("WinState")
  if (winState == 0) then
    currentEvent = 1
    boatTransform:SetWorldPosition(Vector3(45, 0, 0))
    
    Player:Destroy()
    TombStone:Destroy()
    Encik:Destroy()
    
    cameraEmitter:SetAndPlayAudioClip("CreditsWin")
  elseif (winState == 1) then
    TombStone:Destroy()
    
    PlayerTransform    = Player:GetComponent("Transform")
    EncikTransform     =  Encik:GetComponent("Transform")
    
    cameraEmitter:SetAndPlayAudioClip("CreditsWin")
  elseif (winState == 2) then
    Encik:Destroy()
    
    PlayerTransform    = Player:GetComponent("Transform")
    TombStoneTransform = TombStone:GetComponent("Transform")
    
    cameraEmitter:SetAndPlayAudioClip("CreditsLose")
  end
  
  -- Animation
  Encik = CurrentLayer():GetObject("Encik")
  EncikAnimator = Encik:GetComponent("MeshAnimator")
  EncikAnimator:Play("Wave")
  
  StartNextEvent()
end

function OnUpdate(dt)

  -- Call once
  if(isConstruct == false)then
    MyConstructor()
    isConstruct = true
  end
  
  -- Timer delay
  if (timer_EventStart > 0) then
    timer_EventStart = timer_EventStart - dt
    if (timer_EventStart <= 0) then
      StartEvent()
    end
  end
  
  -- Update events
  if (inEvent) then
    if (inCutScene) then
          if (currentEvent == 1) then UpdateCutScene1(dt)
      elseif (currentEvent == 2) then UpdateCutScene2(dt)
      elseif (currentEvent == 3) then UpdateCutScene3(dt)
      else write("Unexpected Event occured in Credits") end
    else
          if (currentEvent == 1) then UpdateEvent1(dt)
      elseif (currentEvent == 2) then UpdateEvent2(dt)
      elseif (currentEvent == 3) then UpdateEvent3(dt)
      else write("Unexpected Event occured in Credits") end
    end
  end
end

-- EVENTS ======================================================================
function StartNextEvent()
  write("Starting next Event", currentEvent + 1)
  
  currentEvent  = currentEvent + 1
  cutSceneState = 1
  inEvent       = false
  
      if (currentEvent == 1) then timer_EventStart = event1_startdelay
  elseif (currentEvent == 2) then timer_EventStart = event2_startdelay
  elseif (currentEvent == 3) then timer_EventStart = event3_startdelay
  else write("Unexpected Event occured in Tekong") end
end

function StartEvent()
  write("Starting Event", currentEvent)
  
  inEvent = true

      if (currentEvent == 1) then StartEvent1()
  elseif (currentEvent == 2) then StartEvent2()
  elseif (currentEvent == 3) then StartEvent3()
  elseif (currentEvent == 4) then StartEvent4()
  elseif (currentEvent == 5) then StartEvent5()
  else write("Unexpected Event occured in Changi") end
end

-- EVENTS Start ================================================================ (Use to set up an event)
function StartEvent1()
  vector = Vector3(0.5, 0.3, 0.7)
  camera:SetLookAt(Vector3(-0.4, 2, -0.9))
  cameraTransform:SetWorldPosition(Vector3(33.5, 1.5, 4))
  
  StartCutScene()
end

function StartEvent2()
  timer2 = 0
  camera:SetLookAt(Vector3(0.6, 0.1, -0.8))
  cameraTransform:SetWorldPosition(Vector3(33.5, 1.5, 4))
  vector = boatTransform:GetWorldPosition()
  
  StartCutScene()
end

function StartEvent3()
  timer2 = 0
  timer = 0
  StartCutScene()
end

-- CUTSCENE Update =============================================================
function UpdateCutScene1(dt)
  -- Look at sky
  if (cutSceneState == 1) then
    timer = timer + dt
    currPos = Vector3Lerp(vector, Vector3(0.7, 0.3, 0.7), timer / 2)
    camera:SetColor(Color(currPos:x(), currPos:y(), currPos:z(), 1))
    
    if (timer >= 2) then
      timer = 0
      cutSceneState = cutSceneState + 1
    end
  
  -- Setup boat
  elseif (cutSceneState == 2) then
    timer = 0
    cutSceneState = cutSceneState + 1
    
    vector  = Vector3(50, 0, 0)
    vector2 = boatTransform:GetWorldPosition()
    boatTransform:SetWorldPosition(vector2 + vector)
    if (PlayerTransform    ~= nil) then vector3 = PlayerTransform:GetWorldPosition()    PlayerTransform:SetWorldPosition(vector3 + vector)    end
    if (EncikTransform     ~= nil) then vector4 = EncikTransform:GetWorldPosition()     EncikTransform:SetWorldPosition(vector4 + vector)     end
    if (TombStoneTransform ~= nil) then vector5 = TombStoneTransform:GetWorldPosition() TombStoneTransform:SetWorldPosition(vector5 + vector) end
  
  -- Pan to boat and boat comes in
  elseif (cutSceneState == 3) then
    timer = timer + dt
    
    currPos = Vector3Lerp(Vector3(-0.4, 2, -0.9), Vector3(0.6, 0.1, -0.8), timer / 6)
    camera:SetLookAt(currPos)
    
    actualPos = boatTransform:GetWorldPosition()
    currPos = Vector3Lerp(vector2 + vector, vector2, timer / 6)
    currPos.y = actualPos:y()
    boatTransform:SetWorldPosition(currPos)
    
    if (PlayerTransform ~= nil) then
      actualPos = PlayerTransform:GetWorldPosition()
      currPos = Vector3Lerp(vector3 + vector, vector3, timer / 6)
      currPos.y = actualPos:y()
      PlayerTransform:SetWorldPosition(currPos)
    end
    
    if (EncikTransform ~= nil) then
      actualPos = EncikTransform:GetWorldPosition()
      currPos = Vector3Lerp(vector4 + vector, vector4, timer / 6)
      currPos.y = actualPos:y()
      EncikTransform:SetWorldPosition(currPos)
    end
    
    if (TombStoneTransform ~= nil) then
      actualPos = TombStoneTransform:GetWorldPosition()
      currPos = Vector3Lerp(vector5 + vector, vector5, timer / 6)
      currPos.y = actualPos:y()
      TombStoneTransform:SetWorldPosition(currPos)
    end
    
    if (timer >= 6) then
      timer = 0
      EndCutScene()
    end
  end
end

function UpdateCutScene2(dt)
  if (cutSceneState == 1) then
    timer = timer + dt
    if (timer >= 0.5) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      vector = creditTransform:GetWorldPosition()
    end
  
  elseif (cutSceneState == 2) then
      timer = timer + dt
      if (ControllerDown("Shoot")) then
        timer = timer + dt * 5
      end
      
      currPos = Vector3Lerp(vector, vector + Vector3(0, 1180 - vector:y(), 0), timer / 60)
      creditTransform:SetWorldPosition(currPos)
      
      if (timer >= 60) then
        EndCutScene()
      end
  end
end

function UpdateCutScene3(dt)
  timer2 = timer2 + dt
  
  actualPos = boatTransform:GetWorldPosition()
  currPos = Vector3Lerp(vector2, vector2 + Vector3(-100, 0, 0), timer2 / 50)
  currPos.y = actualPos:y()
  boatTransform:SetWorldPosition(currPos)
  
  if (PlayerTransform ~= nil) then
    actualPos = PlayerTransform:GetWorldPosition()
    currPos = Vector3Lerp(vector3, vector3 + Vector3(-100, 0, 0), timer2 / 50)
    currPos.y = actualPos:y()
    PlayerTransform:SetWorldPosition(currPos)
  end
  
  if (EncikTransform ~= nil) then
    actualPos = EncikTransform:GetWorldPosition()
    currPos = Vector3Lerp(vector4, vector4 + Vector3(-100, 0, 0), timer2 / 50)
    currPos.y = actualPos:y()
    EncikTransform:SetWorldPosition(currPos)
  end
  
  if (TombStoneTransform ~= nil) then
    actualPos = TombStoneTransform:GetWorldPosition()
    currPos = Vector3Lerp(vector5, vector5 + Vector3(-100, 0, 0), timer2 / 50)
    currPos.y = actualPos:y()
    TombStoneTransform:SetWorldPosition(currPos)
  end
  
  -- Sun
  currPos = sunTransform:GetWorldPosition()
  currPos.y = Lerp(-25, -10, timer2 / 40)
  sunTransform:SetWorldPosition(currPos)
  
  if (cutSceneState == 1) then
    timer = 0
    cutSceneState = cutSceneState + 1
    
    vector6 = camera:GetLookAt()
    vector7 = cameraTransform:GetWorldPosition()
    
  elseif (cutSceneState == 2) then
    timer = timer + dt
    
    currPos = Vector3Lerp(vector6, Vector3(-1, -0.3, 0), timer / 3)
    camera:SetLookAt(currPos)
    
    currPos = Vector3Lerp(vector7, Vector3(52, 8, 0), timer / 3)
    cameraTransform:SetWorldPosition(currPos)
    
    if (timer >= 3.5) then
      timer = 0
      cutSceneState = cutSceneState + 1
    end
  
  elseif (cutSceneState == 3) then
    timer = timer + dt
    
    if (ControllerDown("Shoot") or timer >= 10)then
      EndCutScene()
    end
  end
end

function StartCutScene()
  inCutScene    = true
  cutSceneState = 1
end

function EndCutScene()
  inCutScene = false
end

-- EVENTS Update ===============================================================
function UpdateEvent1(dt)
  StartNextEvent()
end

function UpdateEvent2(dt)
  if (winState == 0) then
    SceneLoad("Level_MainMenu")
  else
    if (ControllerPress("Shoot")) then
      StartNextEvent()
    end
  end
end

function UpdateEvent3(dt)
  SceneLoad("Level_MainMenu")
end