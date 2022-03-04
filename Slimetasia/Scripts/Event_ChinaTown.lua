-- VARIABLES ===================================================================

currentEvent             = 0     --Used with UIIncomingWave
local currentInstruction = 0
local timer_EventStart   = 0
local inEvent            = false
inCutScene               = false --Used with UICutsceneHandler
local camera             = nil
local cameraScript       = nil
local cameraTransform    = nil
local playerScript       = nil
local playerResource     = nil
local playerTransform    = nil
local instructionScript  = nil
local instructionState   = 0
local GameLogicScript    = nil
local isConstruct        = false
local cutSceneState      = 1
local timer              = 0
local timer2             = 0
local vector             = Vector3()
local vector2            = Vector3()
local vector3            = Vector3()
local number             = 0

-- Event 1 : boat moving in + pan around the world + look around merlion + zoom into player
local skipped = false
local event1_startdelay = 0.01
local boat              = nil
local boatTransform     = nil

local core_Transform    = nil
local current_Angle     = 0

local originalPos    = Vector3()
local originalLookAt = Vector3()

-- Event 2 : Spawn spawner
local event2_startdelay        = 0.01
local spawnerParticle          = nil
local spawnerParticleTransform = nil

-- Event 3 : Spawner flies away
local event3_startdelay        = 0.01

-- FUNCTIONS ===================================================================
-- Note: Constructor might call multiple copies, hence do not create stuff in
-- Constructor
function Constructor()
end

--ONLY CALLS ONCE
function MyConstructor()
  GameLogicScript = owner:GetLuaScript("GameLogic_VerticalSlice.lua")
  GameLogicScript:SetVariable("enableShootToStartWave", false)
  GameLogicScript:SetVariable("quitLevelWhenGameEnd", false)
  
  -- MISC
  camerGO           = CurrentLayer():GetObject("Camera")
  camera            = camerGO:GetComponent("Camera")
  cameraScript      = camerGO:GetLuaScript("PlayerCamera.lua")
  cameraTransform   = camerGO:GetComponent("Transform")
  instructionScript = camerGO:GetLuaScript("Instructions.lua")
  
  playerGO        = CurrentLayer():GetObject("Player")
  playerScript    = playerGO:GetLuaScript("PlayerScript.lua")
  playerResource  = playerGO:GetLuaScript("PlayerResourceManagement.lua")
  playerTransform = playerGO:GetComponent("Transform")
  
  core_GO = CurrentLayer():GetObject("Core")
  core_Transform = core_GO:GetComponent("Transform")
  
  originalPos    = cameraTransform:GetWorldPosition()
  originalLookAt = camera:GetLookAt()
  
  boat = CurrentLayer():GetObject("Boat")
  boatTransform = boat:GetComponent("Transform")
  
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
  
  -- Skip cut scene
  if(currentEvent == 1 and inCutScene and not skipped) then
    if (ControllerPress("Shoot"))then
      skipped = true
      cutSceneState = 7
    end
  end

  -- Update events
  if (inEvent) then
    if (inCutScene) then
          if (currentEvent == 1) then UpdateCutScene1(dt)
      elseif (currentEvent == 2) then UpdateCutScene2(dt)
      elseif (currentEvent == 3) then UpdateCutScene3(dt)
      else write("Unexpected Event occured in Tekong") end
    else
          if (currentEvent == 1) then UpdateEvent1(dt)
      elseif (currentEvent == 2) then UpdateEvent2(dt)
      elseif (currentEvent == 3) then UpdateEvent3(dt)
      else write("Unexpected Event occured in Tekong") end
    end
  end
end

-- EVENTS ======================================================================
function StartNextEvent()
  write("Starting next Event", currentEvent + 1)
  
  currentEvent = currentEvent + 1
  inEvent      = false
  
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
  else write("Unexpected Event occured in Tekong") end
end

-- EVENTS Start ================================================================ (Use to set up an event)
function StartEvent1()
  StartCutScene()
end

function StartEvent2()
  boat:Destroy()
  spawnerParticle = CreatePrefab("SummonerMovement")
  spawnerParticleTransform = spawnerParticle:GetComponent("Transform")
  
  StartCutScene()
end

function StartEvent3()
  StartCutScene()
end

-- CUTSCENE Update =============================================================
-- Boat enters the jetty
function UpdateCutScene1(dt)
  if     (cutSceneState == 1) then  timer   = 0
                                    
                                    vector = Vector3(-12, 22.5, -112)
                                    
                                    cameraTransform:SetWorldPosition(vector)
                                    camera:SetLookAt(Vector3(0, -1, 0.35))
                                    cutSceneState = cutSceneState + 1
                                    
  elseif (cutSceneState == 2) then  timer   = timer + dt
                                    boatPos = Vector3Lerp(Vector3(-12, 0, -130), Vector3(-12, 0, -85), timer / 6)
                                    boatTransform:SetWorldPosition(boatPos)
                                    
                                    if (timer >= 6) then
                                      vector  = cameraTransform:GetWorldPosition()
                                      vector2 = camera:GetLookAt()
                                      timer = 0
                                      cutSceneState = cutSceneState + 1
                                    end
                                    
  elseif (cutSceneState == 3) then  timer        = timer + dt
                                    cameraPos    = Vector3Lerp(vector , Vector3(14, 20, 3), timer / 10)
                                    cameraLookAt = Vector3Lerp(vector2, Vector3(-0.6, -0.45, 0.25), timer / 10)
                                    cameraTransform:SetWorldPosition(cameraPos)
                                    camera:SetLookAt(cameraLookAt)
                                    
                                    if (timer >= 10) then
                                      vector  = cameraTransform:GetWorldPosition()
                                      vector2 = camera:GetLookAt()
                                      timer = 0
                                      cutSceneState = cutSceneState + 1
                                    end
  -- Look around merlion            
  elseif (cutSceneState == 4) then  timer        = timer + dt
                                    cameraPos    = Vector3Lerp(vector , Vector3(14, 20, 35), timer / 4)
                                    cameraLookAt = Vector3Lerp(vector2, Vector3(-0.648, -0.762, 0), timer / 4)
                                    cameraTransform:SetWorldPosition(cameraPos)
                                    camera:SetLookAt(cameraLookAt)
                                    
                                    if (timer >= 4) then
                                      vector  = cameraTransform:GetWorldPosition()
                                      vector2 = cameraTransform:GetWorldPosition()
                                      timer = 1
                                      cutSceneState = cutSceneState + 1
                                      
                                      vector3 = cameraTransform:GetWorldPosition() - core_Transform:GetWorldPosition()
                                    end
                                    
  elseif (cutSceneState == 5) then  timer            = timer + dt
                                    currCameraPos  = Vector3Lerp(vector , vector2, timer / 0.01)
                                    cameraTransform:SetWorldPosition(currCameraPos)
                                    
                                    cameraLookAt = core_Transform:GetWorldPosition() - cameraTransform:GetWorldPosition()
                                    camera:SetLookAt(cameraLookAt:Normalize())
                                    
                                    if (timer > 0.01) then
                                      current_Angle = current_Angle + 1
                                      
                                      if (current_Angle > 180) then
                                        vector  = cameraTransform:GetWorldPosition()
                                        vector2 = camera:GetLookAt()
                                        timer = 0
                                        
                                        cutSceneState = cutSceneState + 1
                                        
                                        originalPos  = playerTransform:GetWorldPosition()
                                        offsetVector = Vector3(0.0, 4.0,  5.0)
                                        originalPos  = originalPos + offsetVector
                                      else
                                        timer = 0
                                        vector  = cameraTransform:GetWorldPosition()
                                        vector2 = core_Transform:GetWorldPosition() + vector3:Rotate("y", -current_Angle)
                                      end
                                    end
  -- Pan to player
                                    
  elseif (cutSceneState == 6) then  timer        = timer + dt
                                    currCameraPos    = Vector3Lerp(vector, originalPos, timer / 5)
                                    currCameraLookAt = Vector3Lerp(vector2, originalLookAt, timer / 5)
                                    cameraTransform:SetWorldPosition(currCameraPos)
                                    camera:SetLookAt(currCameraLookAt)
                                    
                                    if (timer >= 5) then
                                      vector  = cameraTransform:GetWorldPosition()
                                      vector2 = camera:GetLookAt()
                                      timer = 0
                                      
                                      cutSceneState = cutSceneState + 1
                                    end
  
  elseif (cutSceneState == 7) then  timer = timer + dt
                                    if (timer >= 0.5) then
                                      camerGO:GetLuaScript("Chinatown_intro.lua"):CallFunction("start")
                                      cutSceneState = cutSceneState + 1
                                      timer = 0
                                    end
                                    
  elseif (cutSceneState == 8) then  timer = timer + dt
                                    if (timer >= 5) then
                                      EndCutScene()
                                    end
  else end
end

-- Spawn spawnner
function UpdateCutScene2(dt)
  -- Set camera position
  if (cutSceneState == 1) then
    cameraTransform:SetWorldPosition(Vector3(2, 3, -10))
    vector = Vector3(0, 0.5, -1)
    camera:SetLookAt(vector)
    
    vector2 = Vector3(1, 100, -37)
    spawnerParticleTransform:SetWorldPosition(vector2)
    
    timer  = 0
    cutSceneState = cutSceneState + 1
  
  -- Particle comes down
  elseif (cutSceneState == 2) then
    timer = timer + dt
    
    lookAt = Vector3Lerp(vector, Vector3(0, 0, -1), timer / 2)
    camera:SetLookAt(lookAt)
    
    pos = Vector3Lerp(vector2, Vector3(1, 0, -37), timer / 2)
    spawnerParticleTransform:SetWorldPosition(pos)
    
    if (timer >= 2.5) then
      timer  = 0
      cutSceneState = cutSceneState + 1
      
      GameLogicScript:CallFunction("StartSpawn")
      spawnerParticleTransform:SetWorldPosition(Vector3(0, -100, 0))
    end
  
  -- Spawn wave
  elseif (cutSceneState == 3) then
    timer = timer + dt
    
    if (timer > 2) then
      EndCutScene()
    end
  end
end

-- Spawner flies away
function UpdateCutScene3(dt)
  -- Move to position
  if (cutSceneState == 1) then
    vector = GameLogicScript:GetVariable("spawnerLastPosition")
    spawnerParticleTransform:SetWorldPosition(vector)
    
    cameraTransform:SetWorldPosition(Vector3(11.25, 18, -6.35))
    vector2 = Vector3(-0.5, -0.5, -0.8)
    camera:SetLookAt(vector2)
    
    timer = 0
    cutSceneState = cutSceneState + 1
  
  -- particle flies up
  elseif (cutSceneState == 2) then
    timer = timer + dt
    currPos = Vector3Lerp(vector, vector + Vector3(0, 30, 0), timer / 2)
    spawnerParticleTransform:SetWorldPosition(currPos)
    
    lookAt = Vector3Lerp(vector2,  Vector3(-0.5, 0, -0.8), timer / 3)
    camera:SetLookAt(lookAt)
    
    if (timer >= 2) then
      cutSceneState = cutSceneState + 1
      vector = currPos
    end
  
  -- End scene
  elseif (cutSceneState == 3) then
    timer = timer + dt
    
    currPos = Vector3Lerp(vector, vector + Vector3(0, 30, 0), (timer - 2) / 1)
    spawnerParticleTransform:SetWorldPosition(currPos)
    
    lookAt = Vector3Lerp(vector2,  Vector3(-0.5, 0, -0.8), timer / 3)
    camera:SetLookAt(lookAt)
    
    if (timer >= 2) then
      script = owner:GetLuaScript("fadeOut.lua")
      script:CallFunction("StartFadeOut")
      cutSceneState = cutSceneState + 1
    end
    
  elseif (cutSceneState == 4) then
    timer = timer + dt
    
    currPos = Vector3Lerp(vector, vector + Vector3(0, 30, 0), (timer - 2) / 1)
    spawnerParticleTransform:SetWorldPosition(currPos)
    
    lookAt = Vector3Lerp(vector2,  Vector3(-0.5, 0, -0.8), timer / 3)
    camera:SetLookAt(lookAt)
    
    if (timer >= 3) then
      SceneLoad("LevelSelect")
    end
  end
end

function StartCutScene()
  inCutScene    = true
  cutSceneState = 1
  
  -- Disable scripts here
  cameraScript:CallFunction("Pause")
  playerScript:CallFunction("Pause")
end

function EndCutScene()
  inCutScene = false
  
  -- Renable scripts here
  cameraScript:CallFunction("Resume")
  playerScript:CallFunction("Resume")
end

-- EVENTS Update =============================================================== (update event)
-- Check if player started spawning waves
function UpdateEvent1(dt)
  if (ControllerDown("StartWave")) then
    StartNextEvent()
  end
end

-- Check if wave ended
function UpdateEvent2(dt)
  gameEnded = GameLogicScript:GetVariable("gameEnded")
  gameLoose = GameLogicScript:GetVariable("gameLose")
  if (gameEnded)then
    if (gameLoose) then
      SceneLoad("LevelSelect")
    else
      PlayerPref_SetBool("ClearedChinaTown", true)
      StartNextEvent()
    end
  end
end

function UpdateEvent3(dt)
end