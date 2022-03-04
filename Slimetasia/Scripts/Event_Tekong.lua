-- VARIABLES ===================================================================

currentEvent                = -1    --Used with UIIncomingWave
local currentInstruction    = 0
local timer_EventStart      = 0
local inEvent               = false
inCutScene                  = false --Used with UICutsceneHandler
local camera                = nil
local originalPos           = Vector3()
local originalLookAt        = Vector3()
local cameraScript          = nil
local cameraTransform       = nil
local playerGO              = nil
local playerScript          = nil
local playerResource        = nil
local playerTransform       = nil
local instructionScript     = nil
local instructionState      = 0
local DialogBoxScript       = nil
local dialogDelay           = 0
local GameLogicScript       = nil
local isConstruct           = false
local cutSceneState         = 1
local timer                 = 0
local timer2                = 0
local vector                = Vector3()
local vector2               = Vector3()
local vector3               = Vector3()
local number                = 0
local InitDialogBox         = false
local dialogState           = 0
local encik                 = nil
local encikTransform        = nil
local encikScript           = nil
local encikAnimator         = nil
local encikCurrentAnimation = ""
local spawnPoints           = nil
local bossPoints           = nil
local dialogLeftTexture     = 0
local dialogRightTexture    = 0

-- Sounds
local SFX_GateOpen     = "Gate_OpenClose"
local SFX_PlayerPickup = "Player_Pickup"
local SFX_ZombieFall   = "Zombie_SpawnFall"
local SFX_BoatEngine   = "Boat_Moving"
local SFX_BoatHorn     = "Boat_Horn"

-- Encik animation
local Encik_Idle         = "Idle"
local Encik_Interact     = "Interact"
local Encik_Jog          = "Jog"
local Encik_Point        = "Point"
local Encik_Struggle     = "Struggle"
local Encik_StruggleLoop = "StruggleLoop"
local Encik_Walk         = "Walk"
local Encik_Wave         = "Wave"

-- Event 1 : Recruit talk
local event0_startdelay = 0.1

-- Event 1 : Training dummy
local event1_startdelay = 0.1
local trainingDummies   = 3

-- Event 2 : Open gate to next section
local event2_startdelay = 1
local event2_fence1           = nil
local event2_fence2           = nil
local event2_fence1_Transform = nil
local event2_fence2_Transform = nil
local event2_collider         = nil

-- Event 3 : Look at gate
local event3_startdelay = 1
local slimeJail         = {}
local slimeInJail       = {}
local event3_fence1             = nil
local event3_fence2             = nil
local event3_fence1_Transform   = nil
local event3_fence2_Transform   = nil
local event3_collider           = nil
local event3_collider_Transform = nil

-- Event 4 : Introduction to traps
local event4_startdelay        = 0.1
local event4_trap1             = nil
local event4_trap2             = nil
local event4_trap3             = nil
local event5_Trigger_Transform = nil
local trapMarkers              = {}

-- Event 5 : Witness traps in action + open gate
local event5_startdelay       = 0.5
local core_Transform          = nil
local coreScale               = Vector3()
local corePos                 = Vector3()
local event5_fence1           = nil
local event5_fence1_Transform = nil

-- Event 6 : See chargers
local event6_startdelay = 0.1
local chargerCount      = 4
local chargers          = {}

-- Event 7 : Introduction to merlion + wave start
local event7_startdelay       = 0.5
local event7_fence1           = nil
local event7_fence1_Transform = nil
local killEncikSlime          = nil
local killEncikSlimeScript    = nil
local killEncikSlimeTransform = nil
local slimeSpawner            = nil

-- Event 8 : wave clear + End game sequence
local event8_startdelay = 0.5
local event8_fence1            = nil
local event8_fence1_Transform  = nil
local spawnerParticle          = nil
local spawnerParticleTransform = nil
local boatGO                   = nil
local boatTransform            = nil
local boatAudioEmitter         = nil

-- FUNCTIONS ===================================================================
-- Note: Constructor might call multiple copies, hence do not create stuff in
-- Constructor
function Constructor()
end

--ONLY CALLS ONCE
function MyConstructor()
  GameLogicScript = owner:GetLuaScript("GameLogic_VerticalSlice.lua")
  GameLogicScript:SetVariable("enableShootToStartWave", false)
  levelName   = PlayerPref_GetString      ("CurrentLevel")
  spawnPoints = PlayerPref_GetVector3Array("SpawnPositions", levelName)
  bossPoints = PlayerPref_GetVector3Array("BossPositions", levelName)
  
  -- Event 2
  event2_fence1   = CurrentLayer():GetObject("Event2_Fence1")
  event2_fence2   = CurrentLayer():GetObject("Event2_Fence2")
  event2_collider = CurrentLayer():GetObject("Event2_Collider")
  event2_fence1_Transform = event2_fence1:GetComponent("Transform")
  event2_fence2_Transform = event2_fence2:GetComponent("Transform")

  -- Event 3
  event3_fence1   = CurrentLayer():GetObject("Event3_Fence1")
  event3_fence2   = CurrentLayer():GetObject("Event3_Fence2")
  event3_collider = CurrentLayer():GetObject("Event3_Collider")
  event3_fence1_Transform = event3_fence1:GetComponent("Transform")
  event3_fence2_Transform = event3_fence2:GetComponent("Transform")
  event3_collider_Transform = event3_collider:GetComponent("Transform")
  
  -- Event 4
  event5_Trigger = CurrentLayer():GetObject("Trigger4")
  event5_Trigger_Transform = event5_Trigger:GetComponent("Transform")
  event4_trap1 = CurrentLayer():GetObject("EventTrap_1")
  event4_trap2 = CurrentLayer():GetObject("EventTrap_2")
  event4_trap3 = CurrentLayer():GetObject("EventTrap_3")
  trapMarkers  = CurrentLayer():GetObjectsListByName("TrapPlacing")
  
  -- Event 5
  core_GO                  = CurrentLayer():GetObject("Core")
  core_Transform           = core_GO:GetComponent("Transform")
  coreScale                = core_Transform:GetWorldScale()
  corePos                  = core_Transform:GetWorldPosition()
  event5_fence1            = CurrentLayer():GetObject("Event5_Fence1")
  event5_fence1_Transform  = event5_fence1:GetComponent("Transform")
  
  -- Event 7
  event7_fence1 = CurrentLayer():GetObject("Event7_Fence1")
  event7_fence1_Transform = event7_fence1:GetComponent("Transform")
  killEncikSlime = CurrentLayer():GetObject("EncikSlime")
  killEncikSlimeTransform = killEncikSlime:GetComponent("Transform")
  killEncikSlimeScript = killEncikSlime:GetLuaScript("KillEncik.lua")
  spawnerParticle = CreatePrefab("SpawnerParticleNoLight")
  spawnerParticleTransform = spawnerParticle:GetComponent("Transform")
  spawnerParticleTransform:SetWorldPosition(Vector3(0, -100, 0))
  
  -- Event 8
  event8_fence1   = CurrentLayer():GetObject("Event8_Fence1")
  event8_fence1_Transform = event8_fence1:GetComponent("Transform")
  boatGO           = CurrentLayer():GetObject("SingaporeBoat")
  boatTransform    = boatGO:GetComponent("Transform")
  boatAudioEmitter = boatGO:GetComponent("AudioEmitter")
  
  -- MISC
  camerGO           = CurrentLayer():GetObject("Camera")
  camera            = camerGO:GetComponent("Camera")
  cameraScript      = camerGO:GetLuaScript("PlayerCamera.lua")
  cameraTransform   = camerGO:GetComponent("Transform")
  instructionScript = camerGO:GetLuaScript("Instructions.lua")
  if(camerGO:GetLuaScript("IntroScreen.lua")) then
    camerGO:GetLuaScript("IntroScreen.lua"):SetVariable("TextTextureName", "Tekong_intro")
    camerGO:GetLuaScript("IntroScreen.lua"):CallFunction("ChangeIntro")
    camerGO:GetLuaScript("IntroScreen.lua"):SetVariable("BGTextureName", "TekongTrans")
    camerGO:GetLuaScript("IntroScreen.lua"):CallFunction("ChangeBackground")
  end
  DialogBoxScript   = camerGO:GetLuaScript("DialogBox.lua")
  originalPos       = cameraTransform:GetWorldPosition()
  originalLookAt    = camera:GetLookAt()
  
  playerGO        = CurrentLayer():GetObject("Player")
  playerScript    = playerGO:GetLuaScript("PlayerScript.lua")
  playerResource  = playerGO:GetLuaScript("PlayerResourceManagement.lua")
  playerTransform = playerGO:GetComponent("Transform")
  playerScript:SetVariable("lockModeChanging", true)
  
  encik          = CurrentLayer():GetObject("Encik")
  encikTransform = encik:GetComponent("Transform")
  encikScript    = encik:GetLuaScript("FollowPlayer.lua")
  encikAnimator  = encik:GetComponent("MeshAnimator")
  
  StartNextEvent()
end

function OnUpdate(dt)
  -- Call once
  if(IsKeyPressed(KEY_8) and currentEvent < 7) then
    if (inCutScene) then
      EndCutScene()
    end
    
    if (dialogState ==  1) then
      dialogState = 0
      DialogBoxScript:CallFunction("GoOut")
    end
    
    playerScript:SetVariable("lockModeChanging", false)
    
    CurrentLayer():GetObject("Trigger6"):Destroy()
    
    currentInstruction = 7
    currentEvent       = 7
    StartEvent()
    playerTransform:SetWorldPosition(Vector3(47, 0.0, -54.840))
  end

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
  
  if (dialogDelay > 0) then
    dialogDelay = dialogDelay - dt
  end
  
  -- Skip cut scene
  if(currentEvent == 0 and inCutScene and cutSceneState < 5) then
    if (ControllerPress("Shoot"))then
      cutSceneState = 5
      
      timer   = 0
      vector  = cameraTransform:GetWorldPosition()
      vector2 = camera:GetLookAt()
      
      originalPos    = Vector3(-57, 5, 38)
      originalLookAt = Vector3(0, -0.3, -0.9)
      CurrentLayer():GetObject("Player"):GetComponent("Transform"):SetWorldRotation(Vector3())
    end
  end
  
  -- Update events
  if (inEvent) then
    if (inCutScene) then
          if (currentEvent == 0) then UpdateCutScene0(dt)
      elseif (currentEvent == 1) then UpdateCutScene1(dt)
      elseif (currentEvent == 2) then UpdateCutScene2(dt)
      elseif (currentEvent == 3) then UpdateCutScene3(dt)
      elseif (currentEvent == 4) then UpdateCutScene4(dt)
      elseif (currentEvent == 5) then UpdateCutScene5(dt)
      elseif (currentEvent == 6) then UpdateCutScene6(dt)
      elseif (currentEvent == 7) then UpdateCutScene7(dt)
      elseif (currentEvent == 8) then UpdateCutScene8(dt)
      elseif (currentEvent == 9) then UpdateCutScene9(dt)
      elseif (currentEvent == 10) then UpdateCutScene10(dt)
      else write("Unexpected Event occured in Tekong") end
    else
          if (currentEvent == 0) then UpdateEvent0(dt)
      elseif (currentEvent == 1) then UpdateEvent1(dt)
      elseif (currentEvent == 2) then UpdateEvent2(dt)
      elseif (currentEvent == 3) then UpdateEvent3(dt)
      elseif (currentEvent == 4) then UpdateEvent4(dt)
      elseif (currentEvent == 5) then UpdateEvent5(dt)
      elseif (currentEvent == 6) then UpdateEvent6(dt)
      elseif (currentEvent == 7) then UpdateEvent7(dt)
      elseif (currentEvent == 8) then UpdateEvent8(dt)
      elseif (currentEvent == 9) then UpdateEvent9(dt)
      elseif (currentEvent == 10) then UpdateEvent10(dt)
      else write("Unexpected Event occured in Tekong") end
    end
  end
end

-- EVENTS ======================================================================
function StartNextEvent()
  write("Starting next Event", currentEvent + 1)
  
  currentEvent = currentEvent + 1
  inEvent      = false
  
      if (currentEvent == 0) then timer_EventStart = event0_startdelay
  elseif (currentEvent == 1) then timer_EventStart = event1_startdelay
  elseif (currentEvent == 2) then timer_EventStart = event2_startdelay
  elseif (currentEvent == 3) then timer_EventStart = event3_startdelay
  elseif (currentEvent == 4) then timer_EventStart = event4_startdelay
  elseif (currentEvent == 5) then timer_EventStart = event5_startdelay
  elseif (currentEvent == 6) then timer_EventStart = event6_startdelay
  elseif (currentEvent == 7) then timer_EventStart = event7_startdelay
  elseif (currentEvent == 8) then timer_EventStart = event8_startdelay
  else write("Unexpected Event occured in Tekong") end
end

function StartEvent()
  write("Starting Event", currentEvent)
  
  inEvent = true
  
      if (currentEvent == 0) then StartEvent0()
  elseif (currentEvent == 1) then StartEvent1()
  elseif (currentEvent == 2) then StartEvent2()
  elseif (currentEvent == 3) then StartEvent3()
  elseif (currentEvent == 4) then StartEvent4()
  elseif (currentEvent == 5) then StartEvent5()
  elseif (currentEvent == 6) then StartEvent6()
  elseif (currentEvent == 7) then StartEvent7()
  elseif (currentEvent == 8) then StartEvent8()
  else write("Unexpected Event occured in Tekong") end
end

-- EVENTS Start ================================================================ (Use to set up an event)
function StartEvent0()
  StartCutScene()
end

function StartEvent1()
  StartCutScene()
end

function StartEvent2()
  StartCutScene()
end

function StartEvent3()
  StartCutScene()
end

function StartEvent4()
  playerScript:SetVariable("lockModeChanging", false)
  playerScript:CallFunction("ToggleSpawnTrapOnGrid")
  StartCutScene()
end

function StartEvent5()
  playerScript:CallFunction("ToggleSpawnTrapOnGrid")
  StartCutScene()
end

function StartEvent6()
  chargerPosition = {Vector3(79, 0.5, -18), Vector3(76.5, 0.5, -18.5), Vector3(74.25, 0.5, -18), Vector3(77, 0.5, -19)}
  for i = 1, chargerCount do
    chargers[i] = CreatePrefab("Slime_Kamikaze")
    chargers[i]:GetComponent("Transform"):SetWorldPosition(chargerPosition[i])
    
    chargerTransform = chargers[i]:GetComponent("Transform")
    chargerTransform:SetWorldRotation(Vector3(0, 180, 0))
  end
  
  StartCutScene()
end

function StartEvent7()
  StartCutScene()
end

function StartEvent8()
  StartCutScene()
end

-- CUTSCENE Update =============================================================
function UpdateCutScene0(dt)
  EncikPlayAnimation(Encik_Wave)
  -- CAMERA PANNING
      if (cutSceneState == 1) then  timer   = 0
                                    vector  = Vector3(100, 50, -75)
                                    vector2 = Vector3(-0.75, -0.6, 0.5)
                                    cameraTransform:SetWorldPosition(vector)
                                    camera:SetLookAt(vector2)
                                    cutSceneState =  cutSceneState + 1
                                    
  elseif (cutSceneState == 2) then  timer   = timer + dt
                                    currPos = Vector3Lerp(vector , Vector3(-27 , 29  , -55.5), timer / 10)
                                    currlok = Vector3Lerp(vector2, Vector3(-0.7, -0.7, 0.2), timer / 10)
                                    cameraTransform:SetWorldPosition(currPos)
                                    camera:SetLookAt(currlok)
                                    
                                    if (timer >= 10) then
                                      cutSceneState =  cutSceneState + 1
                                      
                                      timer   = 0
                                      vector  = Vector3(-27 , 29  , -55.5)
                                      vector2 = Vector3(-0.7, -0.7, 0.2)
                                    end
                                    
  elseif (cutSceneState == 3) then  timer   = timer + dt
                                    currPos = Vector3Lerp(vector , Vector3(-53,  16 , -40), timer / 4)
                                    currlok = Vector3Lerp(vector2, Vector3(  0, -0.4,  1 ), timer / 4)
                                    cameraTransform:SetWorldPosition(currPos)
                                    camera:SetLookAt(currlok)
                                    
                                    if (timer >= 4) then
                                      cutSceneState =  cutSceneState + 1
                                      
                                      timer   = 0
                                      vector  = Vector3(-53,  16 , -40)
                                      vector2 = Vector3(  0, -0.4,  1 )
                                    end
                                    
  elseif (cutSceneState == 4) then  timer   = timer + dt
                                    currPos = Vector3Lerp(vector , Vector3(-53, 8, 33), timer / 4)
                                    currlok = Vector3Lerp(vector2, Vector3(-0.7, -0.8, 0), timer / 6)
                                    cameraTransform:SetWorldPosition(currPos)
                                    camera:SetLookAt(currlok)
                                    
                                    if (timer >= 4) then
                                      cutSceneState =  cutSceneState + 1
                                      
                                      timer   = 0
                                      vector  = cameraTransform:GetWorldPosition()
                                      vector2 = camera:GetLookAt()
                                      
                                      originalPos    = Vector3(-57, 5, 38)
                                      originalLookAt = Vector3(0, -0.3, -0.9)
                                      CurrentLayer():GetObject("Player"):GetComponent("Transform"):SetWorldRotation(Vector3())
                                    end
  
  elseif (cutSceneState == 5) then  timer   = timer + dt
                                    
                                    currPos = Vector3Lerp(vector , originalPos    , timer / 2)
                                    currlok = Vector3Lerp(vector2, originalLookAt , timer / 2)
                                    cameraTransform:SetWorldPosition(currPos)
                                    camera:SetLookAt(currlok)
                                    
                                    if (timer >= 2) then
                                      cutSceneState =  cutSceneState + 1
                                      
                                      timer   = 0
                                    end
                                    
  elseif (cutSceneState == 6) then  -- here add Dialogbox
                                    ClearDialogBox()
                                    DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT LEFT / PLAYER RGT
                                    DialogBoxScript:CallFunction("ChangeName")
                                    AddDialogBoxText("...")
                                    AddDialogBoxSound("")
                                    AddDialogBoxText("... ...")
                                    AddDialogBoxSound("")
                                    AddDialogBoxText("... ... !!??!")
                                    AddDialogBoxSound("")
                                    AddDialogBoxText("HEY!")
                                    AddDialogBoxSound("Tekong_E1_1")
                                    AddDialogBoxText("HEY! YOU!")
                                    AddDialogBoxSound("Tekong_E1_2")
                                    AddDialogBoxText("HEY! YOU! YES YOU!")
                                    AddDialogBoxSound("Tekong_E1_3")
                                    AddDialogBoxText("HEY! YOU! YES YOU! I can't believe someone \nactually made it!")
                                    AddDialogBoxSound("Tekong_E1_4")
                                    AddDialogBoxText("It's been so long since I've talked to someone!")
                                    AddDialogBoxSound("Tekong_E1_5")
                                    AddDialogBoxText("With all that's happening...")
                                    AddDialogBoxSound("Tekong_E1_6")
                                    AddDialogBoxText("With all that's happening... I was about to\ngive up!")
                                    AddDialogBoxSound("Tekong_E1_7")
                                    AddDialogBoxText("Ah!")
                                    AddDialogBoxSound("Tekong_E1_8")
                                    AddDialogBoxText("Ah! No worries!")
                                    AddDialogBoxSound("Tekong_E1_9")
                                    AddDialogBoxText("Ah! No worries! I was just mumbling to myself!")
                                    AddDialogBoxSound("Tekong_E1_10")
                                    AddDialogBoxText("Let's do a quick crash course to get you\nstarted!")
                                    AddDialogBoxSound("Tekong_E1_11")
                                    cutSceneState = cutSceneState + 1
                                    
  elseif (cutSceneState == 7) then LeftIn("Inchek")
                                   RightIn("Player")
                                   if (RunDialogBox()) then
                                      timer = 0
                                      cutSceneState = cutSceneState + 1
                                      
                                      speech_delay_counter = speech_delay
                                      ClearDialogBox()
                                      DialogBoxScript:SetVariable("NameText", "INSTRUCTIONS") -- JZ : NO IMAGE
                                      DialogBoxScript:CallFunction("ChangeName")
                                      if (GetControllerInput() == 0) then
                                        AddDialogBoxText("Move around with W/A/S/D and SPACE to jump.")
                                      else
                                        AddDialogBoxText("Move around with the left analog stick \nand A to jump")
                                      end
                                   end
                                   
  elseif (cutSceneState == 8) then  if (RunDialogBox()) then
                                      timer = 0
                                      cutSceneState = cutSceneState + 1
                                      
                                      speech_delay_counter = speech_delay
                                      ClearDialogBox()
                                      DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT LEFT / PLAYER RGT
                                      DialogBoxScript:CallFunction("ChangeName")
                                      AddDialogBoxText("Go on recruit, try moving!")
                                      AddDialogBoxSound("Tekong_E1_12")
                                   end
                                   
  elseif (cutSceneState == 9) then LeftIn("Inchek")
                                   RightIn("Player") 
                                   if (RunDialogBox()) then
                                      timer = 0
                                      cutSceneState = cutSceneState + 1
                                   end
                                   
  elseif (cutSceneState == 10) then  timer = timer + dt
                                    if (timer > 0.5) then
                                      EndCutScene()
                                      
  -- Set up positions
  event5Pos = event5_Trigger_Transform:GetWorldPosition()
  event5Pos.y = -5
  event5_Trigger_Transform:SetWorldPosition(event5Pos)

                                    end
  else end
end

function UpdateCutScene1(dt) -- See training dummy
  if     (cutSceneState == 1) then  timer   = 0
    vector  = cameraTransform:GetWorldPosition()
    vector2 = camera:GetLookAt()
    cutSceneState = cutSceneState + 1
      
  elseif (cutSceneState == 2) then  -- here add Dialogbox
      EncikPlayAnimation(Encik_Point)
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT LEFT / PLAYER RGT
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("Ah, so you've mastered the art of moving!")
      AddDialogBoxSound("Tekong_E2_1")
      AddDialogBoxText("Ah, so you've mastered the art of moving!\nWonderful!")
      AddDialogBoxSound("Tekong_E2_2")
      AddDialogBoxText("*mumbles* hiash...")
      AddDialogBoxSound("Tekong_E2_3")
      AddDialogBoxText("*mumbles* hiash... don't even know how to move...")
      AddDialogBoxSound("Tekong_E2_4")
      AddDialogBoxText("*mumbles* kids these days..")
      AddDialogBoxSound("Tekong_E2_5")
      AddDialogBoxText("...")
      AddDialogBoxSound("")
      AddDialogBoxText("... Ah!")
      AddDialogBoxSound("Tekong_E2_6")
      AddDialogBoxText("... Ah! Where were we? Yes yes right shooting!")
      AddDialogBoxSound("Tekong_E2_7")
      AddDialogBoxText("As you can see,")
      AddDialogBoxSound("Tekong_E2_8")
      AddDialogBoxText("As you can see, you are holding on to the ...")
      AddDialogBoxSound("Tekong_E2_9")
      AddDialogBoxText("uh...")
      AddDialogBoxSound("Tekong_E2_10")
      AddDialogBoxText("uh... what was it again...")
      AddDialogBoxSound("Tekong_E2_11")
      AddDialogBoxText("*mumbles* gahh...")
      AddDialogBoxSound("Tekong_E2_12")
      AddDialogBoxText("*mumbles* gahh... let me just give it a random\nname now...")
      AddDialogBoxSound("Tekong_E2_13")
      AddDialogBoxText("The ...")
      AddDialogBoxSound("Tekong_E2_14")
      AddDialogBoxText("The ... CB21!")
      AddDialogBoxSound("Tekong_E2_15")
      AddDialogBoxText("The ... CB21! Short for Cross Bow version model 21!")
      AddDialogBoxSound("Tekong_E2_16")
      AddDialogBoxText("Now the CB21 comes with an auto aim feature.")
      AddDialogBoxSound("Tekong_E2_17")
      AddDialogBoxText("It'll ensure that your shots have a higher\nchance of landing!")
      AddDialogBoxSound("Tekong_E2_18")
      AddDialogBoxText("Try shooting some arrows at the training\ndummies!")
      AddDialogBoxSound("Tekong_E2_19")
      AddDialogBoxText("*mumbles* ... ")
      AddDialogBoxSound("")
      AddDialogBoxText("*mumbles* ... because you guys suck at \nshooting ...")
      AddDialogBoxSound("Tekong_E2_20")
      cutSceneState = cutSceneState + 1
      
  elseif (cutSceneState == 3) then 
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
        
        ClearDialogBox()
        DialogBoxScript:SetVariable("NameText", "INSTRUCTIONS") -- JZ : NO IMAGE
        DialogBoxScript:CallFunction("ChangeName")
        if (GetControllerInput() == 0) then
          AddDialogBoxText("Shoot with the LEFT MOUSE.")
        else
          AddDialogBoxText("Shoot with the RIGHT TRIGGER.")
        end
    end
    
  elseif (cutSceneState == 4) then  if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
        
        ClearDialogBox()
        DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT LEFT / PLAYER RGT
        DialogBoxScript:CallFunction("ChangeName")
        AddDialogBoxText("Go on recruit, try shooting!")
        AddDialogBoxSound("Tekong_E2_21")
    end
    
  elseif (cutSceneState == 5) then
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
            EncikPlayAnimation(Encik_Idle)
    end
    
  elseif (cutSceneState == 6) then  timer        = timer + dt
      currCameraPos    = Vector3Lerp(vector, Vector3(-56, 4, -51), timer / 3)
      currCameraLookAt = Vector3Lerp(vector2, Vector3(0, 0, -1), timer / 3)
      cameraTransform:SetWorldPosition(currCameraPos)
      camera:SetLookAt(currCameraLookAt)
      
      if (timer >= 3) then
      cutSceneState =  cutSceneState + 1
      timer = 0
      end
      
  elseif (cutSceneState == 7) then  timer = timer + dt
      if (timer >= 0.5) then
      cutSceneState =  cutSceneState + 1
      end
      
  elseif (cutSceneState == 8) then  
      if (ShowInstructions()) then
        EndCutScene()
      end
  end
end

function UpdateCutScene2(dt) -- Open gate
    if    (cutSceneState == 1) then  -- here add Dialogbox
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : JZ : SGT LEFT / PLAYER RGT
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("WOW!")
      AddDialogBoxSound("Tekong_E3_1")
      AddDialogBoxText("WOW! we have a sharp shooter over here!")
      AddDialogBoxSound("Tekong_E3_2")
      AddDialogBoxText("WOW! we have a sharp shooter over here! \nSuch a natural!")
      AddDialogBoxSound("Tekong_E3_3")
      AddDialogBoxText("*mumbles* Phew ...")
      AddDialogBoxSound("Tekong_E3_4")
      AddDialogBoxText("*mumbles* Phew ... Thankfully we had that \nauto aim installed.")
      AddDialogBoxSound("Tekong_E3_5")
      AddDialogBoxText("*mumbles* Back in the old day,")
      AddDialogBoxSound("Tekong_E3_6")
      AddDialogBoxText("*mumbles* Back in the old day, we could hit \nbirds in the sky with only our slingshots ...")
      AddDialogBoxSound("Tekong_E3_7")
      AddDialogBoxText("... Ah!")
      AddDialogBoxSound("Tekong_E3_8")
      AddDialogBoxText("... Ah! Right! Back to you!")
      AddDialogBoxSound("Tekong_E3_9")
      AddDialogBoxText("As a reward for shooting down all the \ndummies ...")
      AddDialogBoxSound("Tekong_E3_10")
      AddDialogBoxText("As a reward for shooting down all the \ndummies ... I'll open the next gate for you!")
      AddDialogBoxSound("Tekong_E3_11")
      cutSceneState = cutSceneState + 1
      
  elseif (cutSceneState == 2) then 
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
    end
    
  elseif (cutSceneState == 3) then  timer = timer + dt
      if (timer >= 0.5) then
        cutSceneState =  cutSceneState + 1
        
        cameraTransform:SetWorldPosition(Vector3(-77.25, 17.5, -20.5))
        camera:SetLookAt(Vector3(0.7, -0.45, -0.6))
        vector  = encikTransform:GetWorldPosition()
        vector2 = encikTransform:GetWorldRotation()
        EncikPlayAnimation(Encik_Jog)
        timer = 0
      end
    
  elseif (cutSceneState == 4) then  timer = timer + dt
      pos = Vector3Lerp(vector, Vector3(-45, 0.5, -30), timer / 3)
      encikTransform:SetWorldPosition(pos)
      encikScript:SetVariable("lookAtTarget", false)
      
      rot = Vector3Lerp(vector2, Vector3(0, 90, 0), timer / 0.2)
      encikTransform:SetWorldRotation(rot)
      
      if (timer >= 3.5) then
      cutSceneState =  cutSceneState + 1
      timer = 0
      
      AudioSystem_PlayAudioAtLocation(SFX_GateOpen, Vector3(-43.75, 0.53, -29.5), 1, 50, 500)
      event2_collider:Destroy()
      number = event2_fence1_Transform:GetWorldScale():x()
      
      EncikPlayAnimation(Encik_Interact)
      end
    
  elseif (cutSceneState == 5) then timer = timer + dt
    vector = event2_fence1_Transform:GetWorldScale()
    vector.x = Lerp(number, 0.01, timer / 8)
    event2_fence1_Transform:SetWorldScale(vector)
    vector = event2_fence2_Transform:GetWorldScale()
    vector.x = Lerp(number, 0.01, timer / 8)
    event2_fence2_Transform:SetWorldScale(vector)
    
    if (timer >= 1.4) then
      EncikPlayAnimation(Encik_Idle)
    end
    
    if (timer >= 8) then
      cutSceneState = cutSceneState + 1
      timer = 0
      EncikPlayAnimation(Encik_Wave)
    end
    
  elseif (cutSceneState == 6) then timer = timer + dt
    encikScript:SetVariable("lookAtTarget", true)
    encikScript:CallFunction("Jump")
    
    if (timer >= 0.5) then
      cutSceneState = cutSceneState + 1
    end
    
  elseif (cutSceneState == 7) then  -- here add Dialogbox
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : JZ : SGT LEFT / PLAYER RGT
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("There!")
	  AddDialogBoxSound("Tekong_E3_12")
      AddDialogBoxText("There! All done!")
	  AddDialogBoxSound("Tekong_E3_13")
      AddDialogBoxText("There! All done! 1, 2, 1, 2! Hurry up recruit!")
	  AddDialogBoxSound("Tekong_E3_14")
      
      cutSceneState = cutSceneState + 1
      
  elseif (cutSceneState == 8) then 
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
        EncikPlayAnimation(Encik_Jog)
    end
    
  elseif (cutSceneState == 9) then  timer = timer + dt
      if (timer >= 0.5) then
        cutSceneState =  cutSceneState + 1
        
        encikScript:SetVariable("lookAtTarget", false)
        vector  = encikTransform:GetWorldPosition()
        vector2 = encikTransform:GetWorldRotation()
        timer = 0
      end
    
  elseif (cutSceneState == 10) then  timer = timer + dt
      
      pos = Vector3Lerp(vector, Vector3(-15, 0.5, -36.5), timer / 3)
      encikTransform:SetWorldPosition(pos)
      encikScript:SetVariable("lookAtTarget", false)
      
      rot = Vector3Lerp(vector2, Vector3(0, 90, 0), timer / 0.2)
      encikTransform:SetWorldRotation(rot)
      
      if (timer >= 3) then
      cutSceneState =  cutSceneState + 1
      timer = 0
      encikScript:SetVariable("lookAtTarget", true)
      encikScript:CallFunction("Jump")
      end
      
  elseif (cutSceneState == 11) then timer = timer + dt
      if (timer >= 0.35) then
      timer = 0
      cutSceneState =  cutSceneState + 1
      encikScript:CallFunction("Jump")
      EncikPlayAnimation(Encik_Point)
      end
      
  elseif (cutSceneState == 12) then timer = timer + dt
      if (timer >= 0.7) then
      EndCutScene()
      EncikPlayAnimation(Encik_Wave)
      end
  end
end

function UpdateCutScene3(dt) -- Open gate
  if (cutSceneState == 1) then  timer   = 0
      vector  = cameraTransform:GetWorldPosition()
      vector2 = camera:GetLookAt()
      
      cutSceneState = cutSceneState + 1
      
  elseif (cutSceneState == 2) then  timer            = timer + dt
      currCameraPos    = Vector3Lerp(vector, Vector3(-26.25, 4, -27.5), timer / 5)
      currCameraLookAt = Vector3Lerp(vector2, Vector3(0.55, -0.0, -0.8), timer / 5)
      cameraTransform:SetWorldPosition(currCameraPos)
      camera:SetLookAt(currCameraLookAt)
      
      if (timer >= 5) then
      cutSceneState = cutSceneState + 1
      timer = 0
      --number = event3_fence1_Transform:GetWorldScale():x()
      vector  = event3_fence1_Transform:GetWorldPosition()
      vector2 = event3_fence2_Transform:GetWorldPosition()
      
      end

  elseif (cutSceneState == 3) then timer = timer + dt
    if (timer >= 0.5) then
        -- here add Dialogbox
        ClearDialogBox()
        DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT LEFT / PLAYER RGT
        DialogBoxScript:CallFunction("ChangeName")
        AddDialogBoxText("Over here, we have our trap training facility!")
        AddDialogBoxSound("Tekong_E4_1")
        AddDialogBoxText("Now now.")
        AddDialogBoxSound("Tekong_E4_2")
        AddDialogBoxText("Now now. Don't be too excited.")
        AddDialogBoxSound("Tekong_E4_3")
        AddDialogBoxText("Now now. Don't be too excited. I know you're \nitching to try some traps.")
        AddDialogBoxSound("Tekong_E4_4")
        AddDialogBoxText("Now now. Don't be too excited. I know you're \nitching to try some traps. But.. you just \nain't ready.")
        AddDialogBoxSound("Tekong_E4_5")
        cutSceneState = cutSceneState + 1
        
        EncikPlayAnimation(Encik_Point)
    end
      
  elseif (cutSceneState == 4) then 
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
    end
    
  elseif (cutSceneState == 5) then  timer = timer + dt
      if (timer >= 0.5) then
      cutSceneState =  cutSceneState + 1
      timer = 0
      vector = encikTransform:GetWorldPosition()
      end
      
  elseif (cutSceneState == 6) then  timer = timer + dt
      
      pos = Vector3Lerp(vector, Vector3(-15, 0.5, -36.6), timer / 0.3)
      encikTransform:SetWorldPosition(pos)
      
      if (timer >= 0.2) then
        encikScript:SetVariable("lookAtTarget", false)
      end
    
      if (timer >= 0.4) then
        EncikPlayAnimation(Encik_Interact)
        cutSceneState =  cutSceneState + 1
        timer = 0
        
        vector = encikTransform:GetWorldRotation()
      end
      
  elseif (cutSceneState == 7) then  timer = timer + dt
      
      rot = Vector3Lerp(vector, Vector3(0, -160, 0), timer / 0.5)
      encikTransform:SetWorldRotation(rot)
      
      if (timer >= 1) then
      cutSceneState =  cutSceneState + 1
      timer = 0

      end
      
  elseif (cutSceneState == 8) then  timer = timer + dt
      
      rot = Vector3Lerp(Vector3(0, -160, 0), vector, timer / 0.3)
      encikTransform:SetWorldRotation(rot)
      
      if (timer >= 0.2) then
        EncikPlayAnimation(Encik_Idle)
      end
      
      if (timer >= 0.7) then
      cutSceneState =  cutSceneState + 1
      timer = 0
      end
      
  elseif (cutSceneState == 9) then  timer = timer + dt
      
      rot = Vector3Lerp(vector, Vector3(0, -160, 0), timer / 0.3)
      encikTransform:SetWorldRotation(rot)
      
      if (timer >= 0.7) then
      cutSceneState =  cutSceneState + 1
      timer = 0
      encikScript:CallFunction("Jump")
      end
      
  elseif (cutSceneState == 10) then timer = timer + dt
    if (timer >= 0.5) then
      cutSceneState =  cutSceneState + 1
      timer = 0
    end
    
  elseif (cutSceneState == 11) then  timer = timer + dt
      
      rot = Vector3Lerp(Vector3(0, -160, 0), vector, timer / 0.3)
      encikTransform:SetWorldRotation(rot)
      encikScript:CallFunction("Jump")
      
      if (timer >= 0.4) then
      cutSceneState =  cutSceneState + 1
      timer = 0
      encikScript:SetVariable("lookAtTarget", true)
      end
    
  elseif (cutSceneState == 12) then timer = timer + dt
    if (timer >= 0.5) then
        -- here add Dialogbox
        ClearDialogBox()
        DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT LEFT / PLAYER RGT
        DialogBoxScript:CallFunction("ChangeName")
        AddDialogBoxText("!!?!")
        AddDialogBoxSound("Tekong_E4_11")
        AddDialogBoxText("!!?! ARGHH!")
        AddDialogBoxSound("Tekong_E4_6")
        AddDialogBoxText("!!?! ARGHH! Noo!")
        AddDialogBoxSound("Tekong_E4_7")
        AddDialogBoxText("!!?! ARGHH! Noo! My hand slipped!")
        AddDialogBoxSound("Tekong_E4_8")
        AddDialogBoxText("!!?! ARGHH! Noo! My hand slipped! Don't go \nin Recruit!")
        AddDialogBoxSound("Tekong_E4_9")
        AddDialogBoxText("!!?! ARGHH! Noo! My hand slipped! Don't go \nin Recruit! I'm warning you!")
        AddDialogBoxSound("Tekong_E4_10")
        cutSceneState = cutSceneState + 1
    end
      
  elseif (cutSceneState == 13) then 
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
        AudioSystem_PlayAudioAtLocation(SFX_GateOpen, Vector3(-15.75, 3, -38), 1, 50, 500)
    end
    
  elseif (cutSceneState == 14) then  timer = timer + dt
      if (timer >= 0.5) then
      cutSceneState =  cutSceneState + 1
      
      vector = event3_fence1_Transform:GetWorldPosition()
      vector2 = event3_fence2_Transform:GetWorldPosition()
      
      colliderPos = event3_collider_Transform:GetWorldPosition()
      colliderPos.y = -10
      event3_collider_Transform:SetWorldPosition(colliderPos)
      end
      
  elseif (cutSceneState == 15) then  timer = timer + dt
      
      pos1 = Vector3Lerp(vector , Vector3(3   , 0.5, -37.75), timer / 8)
      pos2 = Vector3Lerp(vector2, Vector3(-34.5, 0.5, -37.75), timer / 8)
      
      event3_fence1_Transform:SetWorldPosition(pos1)
      event3_fence2_Transform:SetWorldPosition(pos2)
      
      if (timer >= 8) then
        cutSceneState = cutSceneState + 1
        timer = 0
        vector = encikTransform:GetWorldRotation()
      end
      
  elseif (cutSceneState == 16) then  timer = timer + dt
      
      rot = Vector3Lerp(vector, Vector3(0, -160, 0), timer / 0.5)
      encikTransform:SetWorldRotation(rot)
      
      if (timer >= 1) then
      cutSceneState =  cutSceneState + 1
      timer = 0
      end
      
  elseif (cutSceneState == 17) then  timer = timer + dt
      
      rot = Vector3Lerp(Vector3(0, -160, 0), vector, timer / 0.3)
      encikTransform:SetWorldRotation(rot)
      
      if (timer >= 0.7) then
      cutSceneState =  cutSceneState + 1
      timer = 0
      encikScript:CallFunction("Jump")
      end
      
  elseif (cutSceneState == 18) then timer = timer + dt
    if (timer >= 0.5) then
        -- here add Dialogbox
        ClearDialogBox()
        DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT LEFT / PLAYER RGT
        DialogBoxScript:CallFunction("ChangeName")
        AddDialogBoxText("...")
        AddDialogBoxSound("")
        AddDialogBoxText("... Well")
        AddDialogBoxSound("Tekong_E4_12")
        AddDialogBoxText("... Well, I guess what's done is done.")
        AddDialogBoxSound("Tekong_E4_13")
        AddDialogBoxText("... Well, I guess what's done is done. No point \ncrying over spilt milk.")
        AddDialogBoxSound("Tekong_E4_14")
        AddDialogBoxText("... Well, I guess what's done is done. No point \ncrying over spilt milk. Guess what recruit!")
        AddDialogBoxSound("Tekong_E4_15")
        AddDialogBoxText("... Well, I guess what's done is done. No point \ncrying over spilt milk. Guess what recruit! \nToday's your lucky day.")
        AddDialogBoxSound("Tekong_E4_16")
        AddDialogBoxText("... Well, I guess what's done is done. No point \ncrying over spilt milk. Guess what recruit! \nToday's your lucky day. Go ahead and \ntry those traps!")
        AddDialogBoxSound("Tekong_E4_17")
        cutSceneState = cutSceneState + 1
      end
      
  elseif (cutSceneState == 19) then 
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
    end
    
  elseif (cutSceneState == 20) then  timer = timer + dt
      if (timer > 0.5) then
        timer = 0
        cutSceneState = cutSceneState + 1
        
        vector = encikTransform:GetWorldPosition()
        vector2 = encikTransform:GetWorldRotation()
        encikScript:SetVariable("lookAtTarget", false)
        
        EncikPlayAnimation(Encik_Jog)
    end
    
  elseif (cutSceneState == 21) then  timer = timer + dt
      
      currPos = Vector3Lerp(vector, Vector3(-6, 0.5, -36.5), timer / 2)
      currRot = Vector3Lerp(vector2, Vector3(0, 90, 0), timer / 0.2)
      
      encikTransform:SetWorldPosition(currPos)
      encikTransform:SetWorldRotation(currRot)
      
      if (timer > 2) then
        timer = 0
        cutSceneState = cutSceneState + 1
        
        vector = encikTransform:GetWorldPosition()
        encikScript:SetVariable("lookAtTarget", true)
        
        EncikPlayAnimation(Encik_Idle)
    end
    
  elseif (cutSceneState == 22) then  timer = timer + dt
      if (timer > 0.5) then
        EndCutScene()
    end
  else end
end

function UpdateCutScene4(dt) -- Check for placing traps + introduction to traps + how to put traps
    if (cutSceneState == 1) then  event4_trap1:Destroy()
      event4_trap2:Destroy()
      event4_trap3:Destroy()
      AudioSystem_PlayAudioAtLocation(SFX_PlayerPickup, Vector3(-15.75, 3, -38), 1, 50, 500)
      cutSceneState = cutSceneState + 1
      
  elseif (cutSceneState == 2) then  if (ShowInstructions()) then
        cutSceneState = cutSceneState + 1
      end
      
  elseif (cutSceneState == 3) then  if (ShowInstructions()) then
        cutSceneState = cutSceneState + 1
      end
      
  elseif (cutSceneState == 4) then  if (ShowInstructions()) then
        cutSceneState = cutSceneState + 1
      end
      
  elseif (cutSceneState == 5) then  if (ShowInstructions()) then
        -- Add resources to player
        playerResource:SetVariable("amount", -10)
        playerResource:CallFunction("AddGold")
        cutSceneState = cutSceneState + 1
        timer = 0
      end
      
  elseif (cutSceneState == 6) then timer = timer + dt
    if (timer >= 0.5) then
        -- here add Dialogbox
        ClearDialogBox()
        DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT LEFT / PLAYER RGT
        DialogBoxScript:CallFunction("ChangeName")
        AddDialogBoxText("I've given you some coins to build some traps.")
        AddDialogBoxSound("Tekong_E5_1")
        AddDialogBoxText("I've given you some coins to build some traps. \nNote that every trap is unique so try to master \nthem all.")
        AddDialogBoxSound("Tekong_E5_2")
        AddDialogBoxText("Try building some traps recruit.")
        AddDialogBoxSound("Tekong_E5_3")
        AddDialogBoxText("Try building some traps recruit. When you're \ndone just step out of the training grounds.")
        AddDialogBoxSound("Tekong_E5_4")
        cutSceneState = cutSceneState + 1
      end
      
  elseif (cutSceneState == 7) then 
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
        
      for i = 1, #trapMarkers do
        t   = trapMarkers[i]:GetComponent("Transform")
        pos = t:GetWorldPosition()
        pos.y = 0.5
        t:SetWorldPosition(pos)
      end
    end
    
  elseif (cutSceneState == 8) then
    timer = timer + dt
    if (timer > 0.5) then
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Instructions")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("Try building some traps within the boxes.")
      AddDialogBoxSound("")
      cutSceneState = cutSceneState + 1
    end
    
  elseif (cutSceneState == 9) then 
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
    end
  
  elseif (cutSceneState == 10) then
    timer = timer + dt
    if (timer > 0.5) then
      EndCutScene()
      
      playerScript:SetVariable("respawn_Pos", playerTransform:GetWorldPosition())
    end
  end
end

function UpdateCutScene5(dt) -- Close gate + Show slimes + Test trap + Open gate
    if (cutSceneState == 1) then  
      timer   = 0
      cameraTransform:SetWorldPosition(Vector3(-16, 12, -24))
      camera:SetLookAt(Vector3(0, -0.5, -0.8))
      
      colliderPos = event3_collider_Transform:GetWorldPosition()
      colliderPos.y = 3
      event3_collider_Transform:SetWorldPosition(colliderPos)
      
      timer = 0
      
      
      cutSceneState = cutSceneState + 1
      
      slimeJail   = CurrentLayer():GetObjectsListByName("Box")
      slimeInJail = CurrentLayer():GetObjectsListByName("Slime")
      
      for i = 1, #slimeInJail do
        t = slimeInJail[i]:GetComponent("Transform")
        t:SetWorldRotation(Vector3(0, -90, 0))
      end
      
      for i = 1, #trapMarkers do
        t   = trapMarkers[i]:GetComponent("Transform")
        pos = t:GetWorldPosition()
        pos.y = -10
        t:SetWorldPosition(pos)
      end
      
  elseif (cutSceneState == 2) then timer = timer + dt
    if (timer >= 0.5) then
        -- here add Dialogbox
        ClearDialogBox()
        DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT LEFT / PLAYER RGT
        DialogBoxScript:CallFunction("ChangeName")
        AddDialogBoxText("Stand back recruit!")
        AddDialogBoxSound("Tekong_E6_1")
        AddDialogBoxText("Stand back recruit! I'm closing the gates!")
        AddDialogBoxSound("Tekong_E6_2")
        cutSceneState = cutSceneState + 1
        
        encikScript:SetVariable("lookAtTarget", false)
        vector = encikTransform:GetWorldPosition()
        vector2 = encikTransform:GetWorldRotation()
        timer = 0
    end
    
  elseif (cutSceneState == 3) then 
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      EncikPlayAnimation(Encik_Interact)
    end
    
  elseif (cutSceneState == 4) then  timer =  timer + dt
      
      currPos = Vector3Lerp(vector, Vector3(-6, 0.5, -37), timer / 1)
      currRot = Vector3Lerp(vector2, Vector3(0, 180, 0), timer / 0.2)
      encikTransform:SetWorldPosition(currPos)
      encikTransform:SetWorldRotation(currRot)
      
      if (timer >= 1.4) then
        EncikPlayAnimation(Encik_Idle)
      end
      
      if (timer >= 1.5) then
        cutSceneState = cutSceneState + 1
        encikScript:CallFunction("Jump")
        AudioSystem_PlayAudioAtLocation(SFX_GateOpen, Vector3(-15.75, 3, -38), 1, 50, 500)
      end
    
  elseif (cutSceneState == 5) then  timer = timer + dt
      if (timer >= 0.5) then
      cutSceneState =  cutSceneState + 1
      
      vector = event3_fence1_Transform:GetWorldPosition()
      vector2 = event3_fence2_Transform:GetWorldPosition()
      
      end
      
  elseif (cutSceneState == 6) then  timer = timer + dt
      
      pos1 = Vector3Lerp(vector , Vector3(-6.5, 0.5, -37.75), timer / 8)
      pos2 = Vector3Lerp(vector2, Vector3(-25 , 0.5, -37.75), timer / 8)
      
      event3_fence1_Transform:SetWorldPosition(pos1)
      event3_fence2_Transform:SetWorldPosition(pos2)
      
      if (timer >= 8) then
        cutSceneState = cutSceneState + 1
        timer = 0
        
        encikScript:SetVariable("lookAtTarget", true)
      end
      
  elseif (cutSceneState == 7) then timer = timer + dt
    if (timer >= 0.5) then
        -- here add Dialogbox
        ClearDialogBox()
        DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT LEFT / PLAYER RGT
        DialogBoxScript:CallFunction("ChangeName")
        AddDialogBoxText("Alright, now for the fun part.")
        AddDialogBoxSound("Tekong_E6_3")
        AddDialogBoxText("Alright, now for the fun part. Let's spawn in some \nslimes!")
        AddDialogBoxSound("Tekong_E6_4")
        AddDialogBoxText("Ah!")
        AddDialogBoxSound("Tekong_E6_5")
        AddDialogBoxText("Ah! Right!")
        AddDialogBoxSound("Tekong_E6_6")
        AddDialogBoxText("Ah! Right! I guess you didnt know but ...")
        AddDialogBoxSound("Tekong_E6_7")
        AddDialogBoxText("Ah! Right! I guess you didnt know but ... \nSingapore is going through a slime infestation.")
        AddDialogBoxSound("Tekong_E6_8")
        AddDialogBoxText("We have currently lost 99%")
        AddDialogBoxSound("Tekong_E6_9")
        AddDialogBoxText("We have currently lost 99.9%")
        AddDialogBoxSound("Tekong_E6_10")
        AddDialogBoxText("We have currently lost 99.99%")
        AddDialogBoxSound("Tekong_E6_11")
        AddDialogBoxText("We have currently lost 99.999%")
        AddDialogBoxSound("Tekong_E6_11")
        AddDialogBoxText("We have currently lost 99.9999%")
        AddDialogBoxSound("Tekong_E6_11")
        AddDialogBoxText("We have currently lost 99.9999.%")
        AddDialogBoxSound("Tekong_E6_11")
        AddDialogBoxText("We have currently lost 99.9999..%")
        AddDialogBoxSound("Tekong_E6_11")
        AddDialogBoxText("We have currently lost 99.9999...%")
        AddDialogBoxSound("Tekong_E6_11")
        AddDialogBoxText("We have currently lost 99.9999....% of land")
        AddDialogBoxSound("Tekong_E6_12")
        AddDialogBoxText("We have currently lost 99.9999....% of land \nand you're going to help us get rid of them all.")
        AddDialogBoxSound("Tekong_E6_13")
        AddDialogBoxText("We have currently lost 99.9999....% of land \nand you're going to help us get rid of them all. \nno pressure right?")
        AddDialogBoxSound("Tekong_E6_14")
        cutSceneState = cutSceneState + 1
    end
      
  elseif (cutSceneState == 8) then 
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
    end
    
  elseif (cutSceneState == 9) then  timer = timer + dt
      if (timer >= 0.5) then
        cutSceneState = cutSceneState + 1
        
        vector = cameraTransform:GetWorldPosition()
        vector2 = camera:GetLookAt()
        
        timer = 0
      end
      
  elseif (cutSceneState == 10) then  timer = timer + dt
      currCameraPos    = Vector3Lerp(vector, Vector3(-23.5, 7, -44), timer / 5)
      currCameraLookAt = Vector3Lerp(vector2, Vector3(-0.9, -0.3, -0.5), timer / 5)
      cameraTransform:SetWorldPosition(currCameraPos)
      camera:SetLookAt(currCameraLookAt)
      
      if (timer >= 5) then
        cutSceneState = cutSceneState + 1
      end
      
  elseif (cutSceneState == 11) then  timer = timer + dt
      if (timer >= 0.05) then
        cutSceneState = cutSceneState + 1
        timer = 0
        
        coreBalls = CurrentLayer():GetObjectsListByName("MerlionBalls")
        for i = 1, #coreBalls do
          t = coreBalls[i]:GetComponent("Transform")
          t:SetWorldScale(Vector3(0, 0, 0))
        end
        
        core_Transform:SetWorldPosition(Vector3(4, 0.25, -52))
        core_Transform:SetWorldScale(Vector3())
        
        for i = 1, #slimeJail do
          script = slimeJail[i]:GetLuaScript("Tekong_Jail.lua")
          script:CallFunction("Open")
        end
        
      end
      
  elseif (cutSceneState == 12) then  timer = timer + dt
      if (timer >= 1.5) then
        cutSceneState = cutSceneState + 1
      end
      
  elseif (cutSceneState == 13) then if (ShowInstructions()) then
        cutSceneState = cutSceneState + 1
        
        vector = cameraTransform:GetWorldPosition()
        vector2 = camera:GetLookAt()
        
        timer = 0
    end
    
  elseif (cutSceneState == 14) then timer = timer + dt
      currCameraPos    = Vector3Lerp(vector, Vector3(0.5, 5.5, -52), timer / 5)
      currCameraLookAt = Vector3Lerp(vector2, Vector3(-1, -0.25, 0), timer / 5)
      cameraTransform:SetWorldPosition(currCameraPos)
      camera:SetLookAt(currCameraLookAt)
      
      if (timer >= 5) then
        for i = 1, #slimeInJail do
          slimeInJail[i]:Destroy()
        end
        
        for i = 1, #slimeJail do
          t   = slimeJail[i]:GetComponent("Transform")
          pos = t:GetWorldPosition()
          pos.y = 0.9
          
          slime = CreatePrefab("Slime")
          trans = slime:GetComponent("Transform")
          trans:SetWorldPosition(pos)
          trans:SetWorldRotation(Vector3(0, -90, 0))
          
          slime:AddComponent("PathFinding")
          slime:AddComponent("LuaScript", "PathFindLogic.lua")
          slime:AddComponent("LuaScript", "EnemyBehavior.lua")
        end
        
        cutSceneState = cutSceneState + 1
        
        
        AISystem_SetPathChanged(true)
      end
      
  elseif (cutSceneState == 15) then
    killCount = GameLogicScript:GetVariable("enemiesKilled")
    if (killCount >= #slimeJail) then
      cutSceneState = cutSceneState + 1
      timer = 0
    end
    
  elseif (cutSceneState == 16) then timer = timer + dt
    if (timer >= 0.5) then
        -- here add Dialogbox
        ClearDialogBox()
        DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT LEFT / PLAYER RGT
        DialogBoxScript:CallFunction("ChangeName")
        AddDialogBoxText("Huh ...")
        AddDialogBoxSound("Tekong_E6_15")
        AddDialogBoxText("Huh ... seems like your traps were somewhat\neffective?")
        AddDialogBoxSound("Tekong_E6_16")
        AddDialogBoxText("Huh ... seems like your traps were somewhat\neffective? You'll have more chance to practise \nin the future,")
        AddDialogBoxSound("Tekong_E6_17")
        AddDialogBoxText("Huh ... seems like your traps were somewhat\neffective? You'll have more chance to practise \nin the future, but for now let's move on! \n")
        AddDialogBoxSound("Tekong_E6_18")
        AddDialogBoxText("Huh ... seems like your traps were somewhat\neffective? You'll have more chance to practise \nin the future, but for now let's move on! \nOpening the next gate!")
        AddDialogBoxSound("Tekong_E6_19")
        AddDialogBoxText("*mumbles* oh boy ...")
        AddDialogBoxSound("Tekong_E6_20")
        AddDialogBoxText("*mumbles* oh boy ... guess this one was a \nfailure too.")
        AddDialogBoxSound("Tekong_E6_21")
        AddDialogBoxText("*mumbles* oh boy ... guess this one was a \nfailure too. Why did we pick an otaku ...")
        AddDialogBoxSound("Tekong_E6_22")
        cutSceneState = cutSceneState + 1
    end
      
  elseif (cutSceneState == 17) then 
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
    end
    
  elseif (cutSceneState == 18) then timer = timer + dt
    if (timer >= 0.5) then
      cutSceneState = cutSceneState + 1
      timer   = 0
    
      colliderPos = event3_collider_Transform:GetWorldPosition()
      colliderPos.y = -10
      event3_collider_Transform:SetWorldPosition(colliderPos)
      
      number = event3_fence1_Transform:GetWorldScale():x()
      AudioSystem_PlayAudioAtLocation(SFX_GateOpen, Vector3(-15.75, 3, -38), 1, 50, 500)
      
      vector  = event3_fence1_Transform:GetWorldPosition()
      vector2 = event3_fence2_Transform:GetWorldPosition()
    end
    
  elseif (cutSceneState == 19) then 
      timer = timer + dt
      pos1 = Vector3Lerp(vector , Vector3( 3   , 0.5, -37.75), timer / 8)
      pos2 = Vector3Lerp(vector2, Vector3(-34.5, 0.5, -37.75), timer / 8)
      event3_fence1_Transform:SetWorldPosition(pos1)
      event3_fence2_Transform:SetWorldPosition(pos2)
      
      
      if (timer >= 8) then
        cutSceneState = cutSceneState + 1
        timer = 0
        
        coreBalls = CurrentLayer():GetObjectsListByName("MerlionBalls")
        for i = 1, #coreBalls do
          t = coreBalls[i]:GetComponent("Transform")
          t:SetWorldScale(Vector3(0.2, 0.2, 0.2))
        end
        
        core_Transform:SetWorldPosition(corePos)
        core_Transform:SetWorldScale(coreScale)
        AISystem_SetPathChanged(false)
      end
      
  elseif (cutSceneState == 20) then timer = timer + dt
    if (timer >= 0.5) then
      cutSceneState = cutSceneState + 1
    end
    
  elseif (cutSceneState == 21) then 
    vector  = cameraTransform:GetWorldPosition()
    vector2 = camera:GetLookAt()
    vector3 = encikTransform:GetWorldPosition()
    
    cutSceneState = cutSceneState + 1
    encikScript:SetVariable("lookAtTarget", false)
    encikTransform:SetWorldRotation(Vector3(0, 70, 0))
    EncikPlayAnimation(Encik_Jog)
    
  elseif (cutSceneState == 22) then  
    timer            = timer + dt
    currCameraPos    = Vector3Lerp(vector, Vector3(-9.5, 13, -25.5), timer / 5)
    currCameraLookAt = Vector3Lerp(vector2, Vector3(0.65, -0.5, 0.6), timer / 5)
    currPos          = Vector3Lerp(vector3, Vector3(4.5, 0.5, -15.5), timer / 4)
    
    cameraTransform:SetWorldPosition(currCameraPos)
    camera:SetLookAt(currCameraLookAt)
    
    encikTransform:SetWorldPosition(currPos)
    
    if (timer >= 4) then
      encikTransform:SetWorldRotation(Vector3(0, 90, 0))
      EncikPlayAnimation(Encik_Interact)
      --encikScript:CallFunction("Jump")
    end
    
    if (timer >= 5) then
    cutSceneState = cutSceneState + 1
    timer = 0
    end
    
  elseif (cutSceneState == 23) then timer = timer + dt
      if (timer >= 0.5) then
        EncikPlayAnimation(Encik_Idle)
        cutSceneState = cutSceneState + 1
        timer = 0
        number = event5_fence1_Transform:GetWorldScale():x()
        AudioSystem_PlayAudioAtLocation(SFX_GateOpen, event5_fence1_Transform:GetWorldPosition(), 1, 50, 500)
      end
      
  elseif (cutSceneState == 24) then timer = timer + dt
      vector = event5_fence1_Transform:GetWorldScale()
      vector.x = Lerp(number, 0, timer / 8)
      event5_fence1_Transform:SetWorldScale(vector)
      
      if (timer >= 8) then
        event5_fence1:Destroy()
        cutSceneState = cutSceneState + 1
        encikScript:SetVariable("lookAtTarget", true)
        EncikPlayAnimation(Encik_Wave)
      end
    
  elseif (cutSceneState == 25) then -- here add Dialogbox
        ClearDialogBox()
        DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT LEFT / PLAYER RGT
        DialogBoxScript:CallFunction("ChangeName")
        AddDialogBoxText("Come on recruit!")
        AddDialogBoxSound("Tekong_E6_23")
        AddDialogBoxText("Come on recruit! We ain't got all day let's go!")
        AddDialogBoxSound("Tekong_E6_24")
        cutSceneState = cutSceneState + 1
      
  elseif (cutSceneState == 26) then 
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
    end
    
  elseif (cutSceneState == 27) then timer = timer + dt
    if (timer >= 0.5) then
      EndCutScene()
    end
  else end
end

function UpdateCutScene6(dt) -- Kamikaze intro
    if     (cutSceneState == 1) then  timer   = 0
      vector  = cameraTransform:GetWorldPosition()
      vector2 = camera:GetLookAt()
      cutSceneState = cutSceneState + 1
      
  elseif (cutSceneState == 2) then  timer        = timer + dt
      currCameraPos    = Vector3Lerp(vector, Vector3(53, 6, -12), timer / 5)
      currCameraLookAt = Vector3Lerp(vector2, Vector3(1, -0.25,0), timer / 5)
      cameraTransform:SetWorldPosition(currCameraPos)
      camera:SetLookAt(currCameraLookAt)
      
      if (timer >= 5) then
      cutSceneState = cutSceneState + 1
      timer = 0
      end
      
  elseif (cutSceneState == 3) then  timer = timer + dt
      if (timer >= 0.5) then
      cutSceneState = cutSceneState + 1
      end
      
  elseif (cutSceneState == 4) then timer = timer + dt
    if (timer >= 0.5) then
        -- here add Dialogbox
        ClearDialogBox()
        DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT LEFT / PLAYER RGT
        DialogBoxScript:CallFunction("ChangeName")
        AddDialogBoxText("!!!")
        AddDialogBoxSound("Tekong_E7_1")
        AddDialogBoxText("!!! Who placed those barricades here!")
        AddDialogBoxSound("Tekong_E7_2")
        AddDialogBoxText("!!! Who placed those barricades here! I swear \nif it's someone pulling a prank on us I'll")
        AddDialogBoxSound("Tekong_E7_3")
        AddDialogBoxText("...")
        AddDialogBoxSound("Tekong_E7_4")
        AddDialogBoxText("... ...")
        AddDialogBoxSound("")
        AddDialogBoxText("... ... Wait a minute ...")
        AddDialogBoxSound("Tekong_E7_5")
        AddDialogBoxText("... ... Wait a minute ... do you hear somthing?")
        AddDialogBoxSound("Tekong_E7_6")
        cutSceneState = cutSceneState + 1
    end
      
  elseif (cutSceneState == 5) then 
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
    end
    
  elseif (cutSceneState == 6) then timer = timer + dt
                                   if (timer >= 0.5) then
                                    vector = Vector3(73, 3, -13.5)
                                    cameraTransform:SetWorldPosition(vector)
                                    camera:SetLookAt(Vector3(0.35, -0.3, -1))
                                    
                                    timer = 0
                                    
                                    cutSceneState = cutSceneState + 1
                                    
                                    AudioSystem_PlayAudioAtLocation("Zombie07_Growl02", Vector3(79   , 0.5, -18  ), 0.4)
                                    AudioSystem_PlayAudioAtLocation("Zombie07_Growl04", Vector3(76.5 , 0.5, -18.5), 0.6)
                                    AudioSystem_PlayAudioAtLocation("Zombie07_Growl06", Vector3(74.25, 0.5, -18  ), 0.7)
                                    AudioSystem_PlayAudioAtLocation("Zombie07_Growl12", Vector3(77   , 0.5, -19  ), 0.8)
                                   end
      
  elseif (cutSceneState == 7) then  timer = timer + dt
                                    currCameraPos = Vector3Lerp(vector, Vector3(78, 3, -13.5), timer / 5)
                                    cameraTransform:SetWorldPosition(currCameraPos)
                                    
                                    if (timer >= 2) then
                                      cutSceneState = cutSceneState + 1
                                      AudioSystem_PlayAudioAtLocation("Zombie07_Growl05", Vector3(79   , 0.5, -18  ), 0.4)
                                      AudioSystem_PlayAudioAtLocation("Zombie07_Growl13", Vector3(76.5 , 0.5, -18.5), 0.7)
                                      AudioSystem_PlayAudioAtLocation("Zombie07_Growl07", Vector3(74.25, 0.5, -18  ), 0.6)
                                      AudioSystem_PlayAudioAtLocation("Zombie07_Growl04", Vector3(77   , 0.5, -19  ), 0.3)
                                    end
                                    
  elseif (cutSceneState == 8) then  timer = timer + dt
                                    currCameraPos = Vector3Lerp(vector,Vector3(78, 3, -13.5), timer / 5)
                                    cameraTransform:SetWorldPosition(currCameraPos)
                                    
                                    if (timer >= 5) then
                                      cutSceneState = cutSceneState + 1
                                      
                                      cameraTransform:SetWorldPosition(Vector3(53, 6, -12))
                                      camera:SetLookAt(Vector3(1, -0.25,0))
                                      
                                      timer = 0
                                      AudioSystem_PlayAudioAtLocation("Zombie07_Growl08", Vector3(79   , 0.5, -18  ), 1, 1000, 10000)
                                      AudioSystem_PlayAudioAtLocation("Zombie07_Growl10", Vector3(76.5 , 0.5, -18.5), 1, 1000, 10000)
                                      AudioSystem_PlayAudioAtLocation("Zombie07_Growl04", Vector3(74.25, 0.5, -18  ), 1, 1000, 10000)
                                      AudioSystem_PlayAudioAtLocation("Zombie07_Growl02", Vector3(77   , 0.5, -19  ), 1, 1000, 10000)
                                    end
                                    
  elseif (cutSceneState == 9) then  timer = timer + dt
                                    if (timer >= 0.5) then
                                      cutSceneState = cutSceneState + 1
                                      
                                      for i = 1, chargerCount do
                                      chargers[i]:AddComponent("LuaScript", "Enemy_Kamikaze.lua")
                                      end
                                      
                                      timer = 0
                                    end
      
  elseif (cutSceneState == 10) then
    killCount = GameLogicScript:GetVariable("enemiesKilled")
    if (killCount >= #slimeJail + chargerCount) then
      cutSceneState = cutSceneState + 1
      timer = 0
    end
    
    --timer = timer + dt
    --if (timer >= 5) then
    --  cutSceneState = cutSceneState + 1
    --  
    --  timer = 0
    --end
    
  elseif (cutSceneState == 11) then
    timer = timer + dt
    if (timer >= 0.5) then
      cutSceneState = cutSceneState + 1
      timer = 0
    end
    
  elseif (cutSceneState == 12) then
    if (ShowInstructions()) then
      cutSceneState = cutSceneState + 1
      timer = 0
    end
    
  elseif (cutSceneState == 13) then timer = timer + dt
      if (timer >= 0.5) then
        -- here add Dialogbox
        ClearDialogBox()
        DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT LEFT / PLAYER RGT
        DialogBoxScript:CallFunction("ChangeName")
        AddDialogBoxText("...")
        AddDialogBoxSound("")
        AddDialogBoxText("... ...")
        AddDialogBoxSound("")
        AddDialogBoxText("... ...  THAT")
        AddDialogBoxSound("Tekong_E7_7")
        AddDialogBoxText("... ...  THAT WAS")
        AddDialogBoxSound("Tekong_E7_8")
        AddDialogBoxText("... ...  THAT WAS SO")
        AddDialogBoxSound("Tekong_E7_9")
        AddDialogBoxText("... ...  THAT WAS SO CUTE!!!")
        AddDialogBoxSound("Tekong_E7_10")
        AddDialogBoxText("... ...  THAT WAS SO CUTE!!! The way they run!")
        AddDialogBoxSound("Tekong_E7_11")
        AddDialogBoxText("... ...  THAT WAS SO CUTE!!! The way they run! \nKYAAAA!!!")
        AddDialogBoxSound("Tekong_E7_12")
        AddDialogBoxText("...")
        AddDialogBoxSound("")
        AddDialogBoxText("... *AHEM*")
        AddDialogBoxSound("Tekong_E7_13")
        AddDialogBoxText("... *AHEM* Carry on recruit.")
        AddDialogBoxSound("Tekong_E7_14")
        cutSceneState = cutSceneState + 1
      end
      
  elseif (cutSceneState == 14) then 
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
    end
    
  elseif (cutSceneState == 15) then timer = timer + dt
    if (timer >= 0.5) then
      EndCutScene()
      encikScript:SetVariable("followTarget", true)
    end
  else end
end

 -- Merlion Spawn wave
function UpdateCutScene7(dt)
  -- Set up encik
  if (cutSceneState == 1) then  
    timer         = 0
    vector        = cameraTransform:GetWorldPosition()
    vector2       = camera:GetLookAt()
    
    encikScript:SetVariable("followTarget", false)
    encikScript:SetVariable("lookAtTarget", false)
    encikTransform:SetWorldRotation(Vector3(0, -90, 0))
    EncikPlayAnimation(Encik_Jog)
    vector3 = encikTransform:GetWorldPosition()
    
    cutSceneState = cutSceneState + 1
  
  -- Camera moves to merlion and encik walks to merlion
  elseif (cutSceneState == 2) then  
    timer            = timer + dt
    currCameraPos    = Vector3Lerp(vector, Vector3(46.85, 4.5, -47), timer / 5)
    currCameraLookAt = Vector3Lerp(vector2, Vector3(-0.6, 0, -0.75), timer / 5)
    cameraTransform:SetWorldPosition(currCameraPos)
    camera:SetLookAt(currCameraLookAt)
    
    currPos = Vector3Lerp(vector3, Vector3(41, 0.5, -60), timer / 4)
    encikTransform:SetWorldPosition(currPos)
    
    if (timer >= 4) then
      EncikPlayAnimation(Encik_Idle)
    end
    
    
    if (timer >= 5) then
      timer = 0
      cutSceneState = cutSceneState + 1
    end
  
  -- Load Dialogbox
  elseif (cutSceneState == 3) then 
    timer = timer + dt
    encikScript:CallFunction("Jump")
    
    if (timer >= 0.5) then
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT RIGHT
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("Ah, finally.")
      AddDialogBoxSound("Tekong_E8_1")
      AddDialogBoxText("Ah, finally. The merlion, we meet again.")
      AddDialogBoxSound("Tekong_E8_2")
      cutSceneState = cutSceneState + 1
    end
  
  -- Dialogbox
  elseif (cutSceneState == 4) then 
    RightIn("Inchek")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
    end
  
  --  Dialogbox end + buffer
  elseif (cutSceneState == 5) then 
    timer = timer + dt
    if (timer >= 0.5) then
      cutSceneState = cutSceneState + 1
    end
  
  -- Show merlion instructions
  elseif (cutSceneState == 6) then
    if (ShowInstructions()) then
      timer = 0
      vector = encikTransform:GetWorldRotation()
      cutSceneState = cutSceneState + 1
    end
  
  -- Encik looks back + Encik slime exectue kill order
  elseif (cutSceneState == 7) then
    timer = timer + dt
    currRot = Vector3Lerp(vector, Vector3(0, 90, 0), timer / 0.2)
    encikTransform:SetWorldRotation(currRot)
    
    if (timer >= 0.2) then
      timer = 0
      killEncikSlimeScript:CallFunction("KillEncik")
      cutSceneState = cutSceneState + 1
    end
    
  -- Move Slime near enough to encik 
  elseif (cutSceneState == 8) then
    completed = killEncikSlimeScript:GetVariable("completed")
    
    if (completed) then
      pos = killEncikSlimeTransform:GetWorldPosition()
      AudioSystem_PlayAudioAtLocation("Zombie07_Growl08", pos, 1, 100, 500)
      
      timer = 0
      vector = camera:GetLookAt()
      cutSceneState = cutSceneState + 1
    end
  
  -- Look at slime
  elseif (cutSceneState == 9) then
    timer = timer + dt
    slimePos = killEncikSlimeTransform:GetWorldPosition()
    cameraPos = cameraTransform:GetWorldPosition()
    distance = slimePos - cameraPos
    currLookAt = Vector3Lerp(vector, distance:Normalize(), timer / 0.5)
    camera:SetLookAt(currLookAt)
    
    if (timer >= 0.5) then
      timer = 0
      cutSceneState = cutSceneState + 1
    end
  
  -- Camera follows slime to jump
  elseif (cutSceneState == 10) then
    slimePos = killEncikSlimeTransform:GetWorldPosition()
    cameraPos = cameraTransform:GetWorldPosition()
    distance = slimePos - cameraPos
    camera:SetLookAt(distance:Normalize())
    
    completed = killEncikSlimeScript:GetVariable("reachHighestPoint")
    if (completed) then
      EncikPlayAnimation(Encik_Struggle)
    end
    
    completed = killEncikSlimeScript:GetVariable("landedOnHead")
    if (completed) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      killEncikSlime:Destroy()
      object = encik:GetChild("Bip001_Head")
      meshR  = object:GetComponent("MeshRenderer")
      meshR:SetMesh("EncikSlimeHead")
      
      encikTransform:SetWorldPosition(encikTransform:GetWorldPosition() + Vector3(0, 0.2, 0))
      
      spawnerParticleTransform:SetWorldPosition(encikTransform:GetWorldPosition())
    end
  
  -- Dialogbox
  elseif (cutSceneState == 11) then
    timer = timer + dt
    if (timer >= 3) then
      pos = encikTransform:GetWorldPosition()
      AudioSystem_PlayAudioAtLocation("Zombie07_Growl07", pos, 1, 100, 500)
      
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Slime") -- JZ : SGT RIGHT / SLIME LEFT
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("Growlll!")
      AddDialogBoxSound("Tekong_E8_3")
      
      EncikPlayAnimation(Encik_StruggleLoop)
    end
  
  --  Dialogbox
  elseif (cutSceneState == 12) then 
    LeftIn("Slime")
    RightIn("Inchek")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
    end
  
  --  Dialogbox
  elseif (cutSceneState == 13) then  ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT RIGHT / SLIME LEFT
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("Where did you pop out from!")
      AddDialogBoxSound("Tekong_E8_4")
      AddDialogBoxText("Where did you pop out from! Stay away!")
      AddDialogBoxSound("Tekong_E8_5")
      cutSceneState = cutSceneState + 1
  
  --  Dialogbox
  elseif (cutSceneState == 14) then 
    LeftIn("Slime")
    RightIn("Inchek")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
    end
  
  -- Encik struggle
  elseif (cutSceneState == 15) then  ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Slime") -- JZ : SGT RIGHT / SLIME LEFT
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("Growlll!")
      AddDialogBoxSound("Tekong_E8_6")
      AddDialogBoxText("Growlll! NOM!")
      AddDialogBoxSound("")
      cutSceneState = cutSceneState + 1
  
  -- Encik struggle
  elseif (cutSceneState == 16) then 
    LeftIn("Slime")
    RightIn("Inchek")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
    end
  
  -- Encik struggle
  elseif (cutSceneState == 17) then  ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "SGT Jun Jie") -- JZ : SGT RIGHT
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("Urghh!")
      AddDialogBoxSound("Tekong_E8_7")
      AddDialogBoxText("Urghh! Gahh!")
      AddDialogBoxSound("Tekong_E8_8")
      AddDialogBoxText("Urghh! Gahh! I can feel the slime squirming \nin me!")
      AddDialogBoxSound("Tekong_E8_9")
      AddDialogBoxText("Urghh! Gahh! I can feel the slime squirming \nin me! Urghh ...")
      AddDialogBoxSound("")
      cutSceneState = cutSceneState + 1
  
  -- Encik struggle
  elseif (cutSceneState == 18) then 
    RightIn("Inchek")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
    end
  
  -- Conversation complete + Create encik slime
  elseif (cutSceneState == 19) then timer = timer + dt
    if (timer >= 0.5) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      encik:Destroy()
      
      slimeSpawner = CreatePrefab("Slime_Spawner")
      t = slimeSpawner:GetComponent("Transform")
      t:SetWorldPosition(Vector3(40, 0.75, -60))
      t:SetWorldRotation(Vector3(0, 90, 0))
      a = slimeSpawner:GetComponent("MeshAnimator")
      a:Play("StruggleLoop")
      
      AudioSystem_PlayAudioAtLocation("Zombie07_Growl07", Vector3(41, 0.5, -60), 1, 100, 500)
    end
  
  --  Dialogbox
  elseif (cutSceneState == 20) then
    timer = timer + dt
    if (timer >= 1.5) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Infected") -- JZ : INFECTED SGT  RIGHT
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("Growl!")
      AddDialogBoxSound("Tekong_E8_6")
      AddDialogBoxText("Growl! Growl!")
      AddDialogBoxSound("")
    end
  
  --  Dialogbox completed
  elseif (cutSceneState == 21) then 
    RightIn("Infected")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
        AudioSystem_PlayAudioAtLocation("Zombie07_Growl02", Vector3(41, 0.5, -60), 1, 100, 500)
    end
  
  -- Buffer
  elseif (cutSceneState == 22) then timer = timer + dt
    if (timer >= 0.5) then
      cutSceneState = cutSceneState + 1
    end
  
  -- Show instructions
  elseif (cutSceneState == 23) then if (ShowInstructions()) then
      cutSceneState = cutSceneState + 1
      timer = 0
    end
  
  -- Buffer
  elseif (cutSceneState == 24) then timer = timer + dt
    if (timer >= 0.5) then
      cutSceneState = cutSceneState + 1
      
    end
  
  -- Start wave
  elseif (cutSceneState == 25) then
      
      event7_fence1_Transform:SetWorldPosition(Vector3(81.4, 0.35, -53))
      
      newSlime = CreatePrefab("Slime_Kamikaze")
      newSlime:GetComponent("Transform"):SetWorldPosition(Vector3(96, 2, -74))
      newSlime:AddComponent("LuaScript", "Enemy_Kamikaze.lua")
      
      newSlime2 = CreatePrefab("Slime_Kamikaze")
      newSlime2:GetComponent("Transform"):SetWorldPosition(Vector3(90, 2, -70))
      newSlime2:AddComponent("LuaScript", "Enemy_Kamikaze.lua")
      
      vector = Vector3(40, 0.75, -56)
      spawnerParticleTransform:SetWorldPosition(vector)
      
      cameraTransform:SetWorldPosition(Vector3(52, 22, -75))
      camera:SetLookAt(Vector3(0.6, -0.6, 0.5))
      
      timer = 0
      
      AudioSystem_PlayAudioAtLocation(SFX_GateOpen, event7_fence1_Transform:GetWorldPosition(), 1, 50, 500)
      cutSceneState = cutSceneState + 1
  
  -- Gate open and kmaikaze enters
  elseif (cutSceneState == 26) then
      timer = timer + dt
      
      scale = event7_fence1_Transform:GetWorldScale()
      scale.x = Lerp(0, 0.115, timer / 8)
      event7_fence1_Transform:SetWorldScale(scale)
      event7_fence1_Transform:SetWorldScale(scale)
      
      if (timer < 4) then
        spawnerParticleTransform:SetWorldPosition(Vector3Lerp(vector , Vector3(77, 3, -56), timer / 3))
      elseif (timer < 6) then
        spawnerParticleTransform:SetWorldPosition(Vector3Lerp(Vector3(77, 3, -56), Vector3(77, 3, -72.5), (timer - 4) / 2))
      else
        spawnerParticleTransform:SetWorldPosition(Vector3Lerp(Vector3(77, 3, -72.5),  Vector3(96, 3, -72.5), (timer - 6) / 2))
      end
      
      if (timer >= 8) then
        cutSceneState = cutSceneState + 1
        timer = 0
        
        spawnerParticleTransform:SetWorldPosition(Vector3(0, -100, 0))
        slimeSpawner:GetComponent("Transform"):SetWorldPosition(bossPoints[1])
        slimeSpawner:GetComponent("Transform"):SetWorldRotation(Vector3(0, -90, 0))
      end
      
  elseif (cutSceneState == 27) then timer = timer + dt
      if (timer >= 0.5) then
        -- here add Dialogbox
        ClearDialogBox()
        DialogBoxScript:SetVariable("NameText", "Infected") -- JZ : INFECTED SGT  RIGHT
        DialogBoxScript:CallFunction("ChangeName")
        AddDialogBoxText("We ...")
        AddDialogBoxSound("Tekong_E8_10")
        AddDialogBoxText("We ... want")
        AddDialogBoxSound("Tekong_E8_11")
        AddDialogBoxText("We ... want OTAKUS !!!")
        AddDialogBoxSound("Tekong_E8_12")
        cutSceneState = cutSceneState + 1
      end
      
  elseif (cutSceneState == 28) then 
    RightIn("Infected")
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
        
        ClearDialogBox()
        DialogBoxScript:SetVariable("NameText", "Instructions") -- JZ : NONE
        DialogBoxScript:CallFunction("ChangeName")
        AddDialogBoxText("Defend the Merlion from the slimes!")
    end
    
  elseif (cutSceneState == 29) then 
    if (RunDialogBox()) then
        timer = 0
        cutSceneState = cutSceneState + 1
    end
    
  elseif (cutSceneState == 30) then timer = timer + dt
    if (timer >= 0.5) then
      cutSceneState = cutSceneState + 1
    end
      
  elseif (cutSceneState == 31) then
    if (ShowInstructions()) then
      EndCutScene()
      
      GameLogicScript:SetVariable("quitLevelWhenGameEnd", false)
      slimeSpawner:AddComponent("LuaScript", "Enemy_Spawner.lua")
      GameLogicScript:SetVariable("spawnStart", true)
      playerScript:SetVariable("respawn_Pos", Vector3(42, 2, -60))
    end
  end
end

-- Spawner flies away
function UpdateCutScene8(dt)
  -- Snap camera to spawner
  if (cutSceneState == 1) then
    cameraTransform:SetWorldPosition(Vector3(84, 5, -72.5))
    camera:SetLookAt(Vector3(1, -0.2, 0))
    
    timer = 0
    cutSceneState = cutSceneState + 1
    
    vector  = Vector3(96, 0.5, -72.5)
    spawnerParticleTransform:SetWorldPosition(vector)
    
  -- Create spawner particle
  elseif (cutSceneState == 2) then
    timer = timer + dt
    
    if (timer >= 0.3) then
      vector2 = camera:GetLookAt()
      
      timer = 0
      cutSceneState = cutSceneState + 1
    end
  
  -- Move spawner particle up + camera looks up
  elseif (cutSceneState == 3) then
    timer = timer + dt
    currPos = Vector3Lerp(vector, vector + Vector3(0, 400, 0), timer / 50)
    spawnerParticleTransform:SetWorldPosition(currPos)
    
    direction = currPos - cameraTransform:GetWorldPosition()
    currLookAt = Vector3Lerp(vector2, direction:Normalize(), timer / 3)
    camera:SetLookAt(currLookAt)
    
    if (timer >= 3) then
      cutSceneState = cutSceneState + 1
      vector2 = vector + Vector3(0, 400, 0)
      vector  = currPos
      timer   = 0
    end
  
  -- Particle continues moving up
  elseif (cutSceneState == 4) then
    timer = timer + dt
    currPos = Vector3Lerp(vector, vector2, timer / 5)
    spawnerParticleTransform:SetWorldPosition(currPos)
    
    if (ShowInstructions()) then
      cutSceneState = cutSceneState + 1
      --spawnerParticleTransform:SetWorldPosition(vector2)
    end
  
  -- Add Dialogbox
  elseif (cutSceneState == 5) then
    ClearDialogBox()
    DialogBoxScript:SetVariable("NameText", "Infected") -- JZ : INFECTED SGT LEFT / PLAYER RGT
    DialogBoxScript:CallFunction("ChangeName")
    AddDialogBoxText("Grrrr ... ")
    AddDialogBoxSound("Tekong_E9_1")
    AddDialogBoxText("Grrrr ... Run ...")
    AddDialogBoxSound("Tekong_E9_2")
    AddDialogBoxText("Grrrr ... Run ... Board the boat recruit ...")
    AddDialogBoxSound("Tekong_E9_3")
    cutSceneState = cutSceneState + 1
  
  -- DialogBox
  elseif (cutSceneState == 6) then
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
    end
  
  -- Boat enters
  elseif (cutSceneState == 7) then
    cameraTransform:SetWorldPosition(Vector3(115, 2, -115))
    camera:SetLookAt(Vector3(0.7, -0.05, -0.7))
    
    vector = Vector3(190, 1, -124)
    boatTransform:SetWorldPosition(vector)
    AudioSystem_PlayAudioAtLocation(SFX_BoatHorn, vector, 1, 50, 500)
    
    boatParticle = boatGO:GetChild("BoatParticle")
    boatParticle:GetLuaScript("TekongBoatParticle.lua"):SetVariable("activateParticle", true)
    
    timer = 0
    cutSceneState = cutSceneState + 1
    
  -- Boat horn
  elseif (cutSceneState == 8) then
    timer = timer + dt
    if (timer >= 0.2) then
      AudioSystem_PlayAudioAtLocation(SFX_BoatHorn, vector, 1, 50, 500)
      boatAudioEmitter:SetVolume(0.5)
      boatAudioEmitter:SetAndPlayAudioClip(SFX_BoatEngine)
      
      timer = 0
      cutSceneState = cutSceneState + 1
    end
  
  -- Boat moves
  elseif (cutSceneState == 9) then
    timer = timer + dt
    newPos = Vector3Lerp(vector, Vector3(105, 1, -124), timer / 5)
    boatTransform:SetWorldPosition(newPos)
    
    if (timer >= 5) then
      cutSceneState = cutSceneState + 1
      
      cameraTransform:SetWorldPosition(Vector3(48, 12, -50))
      camera:SetLookAt(Vector3(-0.4, -0.2, -0.9))
      
      number = event8_fence1_Transform:GetWorldScale():x()
      boatTransform:SetWorldPosition(Vector3(80, 1, -124))
      timer  = 0
      timer2 = 0
      
      playerTransform:SetWorldPosition(Vector3(50, 1.5, -65))
      playerTransform:SetWorldRotation(Vector3(0, 0, 0))
      
      AudioSystem_PlayAudioAtLocation(SFX_GateOpen, event8_fence1_Transform:GetWorldPosition(), 1, 50, 500)
    end
  
  -- Open gate + Boat moves to harbour
  elseif (cutSceneState == 10) then
    timer = timer + dt
    gateScale = event8_fence1_Transform:GetWorldScale()
    gateScale.x = Lerp(number, 0, timer / 8)
    event8_fence1_Transform:SetWorldScale(gateScale)
    
    if (timer >= 8) then
      event8_fence1:Destroy()
      cutSceneState = cutSceneState + 1
      timer         = 0
    end
    
    timer2 = timer2 + dt
    newPos = Vector3Lerp(Vector3(80, 1, -124), Vector3(40, 1, -124), timer2 / 4)
    boatTransform:SetWorldPosition(newPos)
    if (timer2 >= 3) then
      boatAudioEmitter:FadeOut(1)
      boatParticle = boatGO:GetChild("BoatParticle")
      boatParticle:GetLuaScript("TekongBoatParticle.lua"):SetVariable("activateParticle", false)
    end
  
  -- Add Dialogbox
  elseif (cutSceneState == 11) then
    timer = timer + dt
    
    if (timer >= 0.5) then
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Infected") -- JZ : INFECTED SGT LEFT / PLAYER RGT
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("Growl!")
      AddDialogBoxSound("Tekong_E9_4")
      AddDialogBoxText("Growl! Growl! ...")
      AddDialogBoxSound("Tekong_E9_5")
      AddDialogBoxText("Growl! Growl! ... Hurry! Run!")
      AddDialogBoxSound("Tekong_E9_6")
      cutSceneState = cutSceneState + 1
    end
  
  -- DialogBox
  elseif (cutSceneState == 12) then
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
  end
  
  -- Player move to boat
  elseif (cutSceneState == 13) then
    timer = timer + dt
    if (timer >= 0.5) then
      timer = 0
      cutSceneState = cutSceneState + 1
      playerScript:CallFunction("PlayWalkAnim")
    end
  elseif (cutSceneState == 14)then
    timer = timer + dt
    Pos = playerTransform:GetWorldPosition()
    Pos.z = Lerp(-65, -115, timer /4)
    playerTransform:SetWorldPosition(Pos)
    if (timer >= 4) then
      cutSceneState = cutSceneState + 1
      playerTransform:SetWorldRotation(Vector3(0, 90, 0))
      timer = 0
    end
  elseif (cutSceneState == 15) then
    timer = timer + dt
    Pos = playerTransform:GetWorldPosition()
    Pos.x = Lerp(50, 36.5, timer /1.5)
    playerTransform:SetWorldPosition(Pos)
    if (timer >= 1.5) then
      cutSceneState = cutSceneState + 1
      playerTransform:SetWorldRotation(Vector3(0, 0, 0))
      timer = 0
      
      playerGO:RemoveComponent("RigidBody")
    end
  elseif (cutSceneState == 16) then
    timer = timer + dt
    Pos = playerTransform:GetWorldPosition()
    Pos.z = Lerp(-115, -122, timer /1)
    Pos.y = Lerp(Pos:y(),2.3, timer /1)
    playerTransform:SetWorldPosition(Pos)
    if (timer >= 1) then
      cutSceneState = cutSceneState + 1
      timer = 0
      vector = Vector3(40, 1, -124)
      playerTransform:SetWorldRotation(Vector3(0, 90, 0))
    end
  
  -- Add Dialogbox
  elseif (cutSceneState == 17) then  timer = timer + dt
    if (timer >= 0.5) then
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Infected") -- JZ : INFECTED SGT LEFT /
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("... ")
      AddDialogBoxSound("")
      AddDialogBoxText("... Stop the infestation ... ")
      AddDialogBoxSound("Tekong_E9_7")
      AddDialogBoxText("... Stop the infestation ... please ...")
      AddDialogBoxSound("Tekong_E9_8")
      AddDialogBoxText("Growl!")
      AddDialogBoxSound("")
      AddDialogBoxText("Growl! Growl!")
      AddDialogBoxSound("")
      cutSceneState = cutSceneState + 1
      
      vector2 = Vector3(36.5, 2.3, -122)
      playerTransform:SetWorldPosition(vector2)
      playerScript:CallFunction("PlayIdleAnim")
      
      timer = 0
    end
  
  -- Player rides the boat + DialogBox
  elseif (cutSceneState == 18) then
    timer = timer + dt
    
    newPos = Vector3Lerp(vector , Vector3(-200, 1, -124), timer / 15)
    boatTransform:SetWorldPosition(newPos)
    
    boatParticle = boatGO:GetChild("BoatParticle")
    boatParticle:GetLuaScript("TekongBoatParticle.lua"):SetVariable("activateParticle", true)
    
    newPos = Vector3Lerp(vector2, Vector3(-203.5, 2.3, -122), timer / 15)
    playerTransform:SetWorldPosition(newPos)
    
    LeftIn("Infected")
    if (RunDialogBox()) then
      cutSceneState = cutSceneState + 1
      timer2 = 0
    end
  
  -- Player rides the boat
  elseif (cutSceneState == 19) then 
    timer = timer + dt
    timer2 = timer2 + dt
    newPos = Vector3Lerp(vector, Vector3(-200, 1, -124), timer / 15)
    boatTransform:SetWorldPosition(newPos)
    
    newPos = Vector3Lerp(vector2, Vector3(-203.5, 2.3, -122), timer / 15)
    playerTransform:SetWorldPosition(newPos)
    
    if (timer2 >= 2) then
      cutSceneState = cutSceneState + 1
      
      script = owner:GetLuaScript("fadeOut.lua")
      script:CallFunction("StartFadeOut")
    end
  
  -- Fade
  elseif (cutSceneState == 20) then 
    timer = timer + dt
    timer2 = timer2 + dt
    newPos = Vector3Lerp(vector, Vector3(-200, 1, -124), timer / 15)
    boatTransform:SetWorldPosition(newPos)
    
    newPos = Vector3Lerp(vector2, Vector3(-203.5, 2.3, -122), timer / 15)
    playerTransform:SetWorldPosition(newPos)
    
    if (timer2 >= 3) then
      timer = 0
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
function UpdateEvent0(dt)
  --encikScript:CallFunction("Jump")
end

function UpdateEvent1(dt)
  if (trainingDummies <= 0) then
    StartNextEvent()
  end
end

function UpdateEvent2(dt)
end

function UpdateEvent3(dt)
end

function UpdateEvent4(dt)
    event5Pos = event5_Trigger_Transform:GetWorldPosition()
    event5Pos.y = 2
    event5_Trigger_Transform:SetWorldPosition(event5Pos)
end

function UpdateEvent5(dt)
end

function UpdateEvent6(dt)
end

function UpdateEvent7(dt)
  slimeSpawner = CurrentLayer():GetObject("Slime_Spawner")
  if (slimeSpawner ~= nil) then
    slimeSpawner:GetComponent("Transform"):SetWorldRotation(Vector3(0, -90, 0))
  end
  
  gameEnded = GameLogicScript:GetVariable("gameEnded")
  gameLoose = GameLogicScript:GetVariable("gameLose")
  
  if (gameEnded)then
    if (gameLoose) then
      SceneLoad("LevelSelect")
      write(1)
    else
      PlayerPref_SetBool("ClearedTekong", true)
      StartNextEvent()
      write(2)
    end
  end
end

function UpdateEvent8(dt)
end

-- Updates =====================================================================
function ReduceDummyCount ()
  trainingDummies = trainingDummies - 1
end

function ActivateCharger()
  timer = 0
  StartNextEvent()
end

-- Instruction UI ==============================================================
function ShowInstructions()
  if (instructionState == 0) then
    currentInstruction = currentInstruction + 1
    write(currentInstruction)
    -- Shooting dummy
        if(currentInstruction == 1 ) then instructionScript:SetVariable("currTexture", "KILLALLDUMMIES")
    -- Barracde trap               
    elseif(currentInstruction == 2 ) then instructionScript:SetVariable("currTexture", "BARRICADETUT")
    -- coil trap                   
    elseif(currentInstruction == 3 ) then instructionScript:SetVariable("currTexture", "BARBEDWIRETUT")
    -- Firework trap               
    elseif(currentInstruction == 4 ) then instructionScript:SetVariable("currTexture", "FIRECRACKERTUT")
    -- How to use trap             
    elseif(currentInstruction == 5 ) then instructionScript:SetVariable("currTexture", "TOGGLEMODE")
    -- Slimes                      
    elseif(currentInstruction == 6 ) then instructionScript:SetVariable("currTexture", "SLIMETUT")
    -- Kamikaze slime              
    elseif(currentInstruction == 7 ) then instructionScript:SetVariable("currTexture", "KAMIKAZETUTNEW")
    -- Merlion                     
    elseif(currentInstruction == 8 ) then instructionScript:SetVariable("currTexture", "MERLIONTUTNEWEST")
    -- Encik                       
    elseif(currentInstruction == 9 ) then instructionScript:SetVariable("currTexture", "ENCIKTUT")
    -- Waves                       
    elseif(currentInstruction == 10) then instructionScript:SetVariable("currTexture", "PATHTUT")
    -- Encik goes chinatown        
    elseif(currentInstruction == 11) then instructionScript:SetVariable("currTexture", "ENDOFTEKONG")
    end
    
    instructionScript:CallFunction("Start")
    instructionState = 1
  elseif (instructionState == 1) then
    done = instructionScript:GetVariable("completed")
    if (done) then
      instructionState = 0
      return true
    end
  end
  
  return false
end

--  Dialogbox ===============================================================
function ClearDialogBox()
  
  --if (speech_delay_counter > 0) then
  --  speech_delay_counter = speech_delay_counter - dt
  --  if (speech_delay_counter <= 0) then
      DialogBoxScript:CallFunction("ClearText")
  --  end
  --end
  
end

function AddDialogBoxText(text)
  DialogBoxScript:SetVariable("LineOfText", text)
  DialogBoxScript:CallFunction("AddText")
end

function AddDialogBoxSound(clipName, pos)
  DialogBoxScript:SetVariable("AudioName", clipName)
  DialogBoxScript:SetVariable("AudioPos", pos)
  DialogBoxScript:CallFunction("AddAudio")
end

function RightIn(text)
  if(dialogRightTexture == 1) then return end
  up      = DialogBoxScript:GetVariable("up")
  if(up) then
    if(text == "Player") then
      DialogBoxScript:SetVariable("rightTexture", "Player_Dialog_w")
      DialogBoxScript:CallFunction("FadeInRight")
    elseif(text == "Slime") then
      DialogBoxScript:SetVariable("rightTexture", "NormalSlime_Right_Dialog_w")
      DialogBoxScript:CallFunction("FadeInRight")
    elseif(text == "Inchek") then
      DialogBoxScript:SetVariable("rightTexture", "inchek_dialog_newest")
      DialogBoxScript:CallFunction("FadeInRight")
    elseif(text == "Infected") then
      DialogBoxScript:SetVariable("rightTexture", "infected_dialog_w")
      DialogBoxScript:CallFunction("FadeInRight")
    end
    dialogRightTexture = dialogRightTexture + 1

  end
end

function LeftIn(text)
  if(dialogLeftTexture == 1) then return end
  up      = DialogBoxScript:GetVariable("up")
  if(up) then
    if(text == "Player") then
      DialogBoxScript:SetVariable("leftTexture", "Player_Dialog_w")
      DialogBoxScript:CallFunction("FadeInLeft")
    elseif(text == "Slime") then
      DialogBoxScript:SetVariable("leftTexture", "NormalSlime_Left_Dialog_w")
      DialogBoxScript:CallFunction("FadeInLeft")
    elseif(text == "Inchek") then
      DialogBoxScript:SetVariable("leftTexture", "inchek_dialog_newest")
      DialogBoxScript:CallFunction("FadeInLeft")
    elseif(text == "Infected") then
      DialogBoxScript:SetVariable("leftTexture", "infectedLeft_dialog_w")
      DialogBoxScript:CallFunction("FadeInLeft")
    end
    dialogLeftTexture = dialogLeftTexture + 1

  end
end

function PlayerRightIn()
  up      = DialogBoxScript:GetVariable("up")
  if(up) then
    DialogBoxScript:SetVariable("rightTexture", "Player_Dialog")
    DialogBoxScript:CallFunction("FadeInRight")
  end
end

function SlimeLeftIn()
  up      = DialogBoxScript:GetVariable("up")
  if(up) then
  DialogBoxScript:SetVariable("leftTexture", "Slime_Dialog")
  DialogBoxScript:CallFunction("FadeInLeft")
  end
end

function RunDialogBox()
  if (dialogState == 0) then
     DialogBoxScript:CallFunction("GoIn")
     dialogState = 1
     dialogDelay = 0.2
    return false
  else
    completed = DialogBoxScript:GetVariable("completed")
    done      = DialogBoxScript:GetVariable("done")
    
    if (not completed) then
      if (not done) then
        DialogBoxScript:CallFunction("GoIn")
      else
        if (ControllerPress("Shoot") or ControllerPress("Jump")) then
          if (dialogDelay <= 0) then
            DialogBoxScript:CallFunction("GoOut")
          end
        end
      end
      return false
    else
      dialogState = 0
      dialogLeftTexture = 0
      dialogRightTexture = 0
      return true
    end
    
  end
end

function EncikPlayAnimation (animation)
  if (encikCurrentAnimation ~= animation) then
    encikCurrentAnimation = animation
    encikAnimator:Play(animation)
  end
end