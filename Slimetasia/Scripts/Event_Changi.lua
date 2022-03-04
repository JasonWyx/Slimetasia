-- VARIABLES ===================================================================

currentEvent             = 0     --Used with UIIncomingWave
local currentInstruction = 0
local timer_EventStart   = 0
local inEvent            = false
inCutScene               = false --Used with UICutsceneHandler
local camerGO            = nil 
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
local timer3             = 0
local vector             = Vector3()
local vector2            = Vector3()
local vector3            = Vector3()
local vector4            = Vector3()
local number             = 0
local BGM_ENDING         = "CreditsLose"

-- Event 1 : pan around the world + look around merlion + zoom into player
local event1_startdelay = 0.01

local core_Transform    = nil
local current_Angle     = 0

local originalPos    = Vector3()
local originalLookAt = Vector3()

-- Event 2 : Spawn spawner
local event2_startdelay        = 0.01
local spawnerParticle          = nil
local spawnerParticleTransform = nil

-- Event 3 : Spawner flies away
local event3_startdelay = 0.01
local spawner           = nil
local spawnerAnimator   = nil
local spawnerTransform  = nil

-- Event 4 : Shoot incheck (Good end)
local event4_startdelay    = 0.01
local playerBullet         = nil
local playerBulletTranform = nil
local encik                = nil
local encikTransform       = nil
local encikAnimator        = nil
local encikSlime           = nil
local encikSlimeTranform   = nil
local encikParticle        = nil
local encikParticleEmitter = nil
local slimeDeath           = nil
local slimeDeathTransform  = nil

-- Event 5 : Shoot incheck (Bad end)
local event5_startdelay = 0.01

-- Dialog
local DialogBoxScript    = nil
local dialogState           = 0
local dialogLeftTexture  = 0
local dialogRightTexture = 0
local dialogDelay = 0

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
  
  if(camerGO:GetLuaScript("IntroScreen.lua")) then
    camerGO:GetLuaScript("IntroScreen.lua"):SetVariable("TextTextureName", "ChangiIntroScreen")
    camerGO:GetLuaScript("IntroScreen.lua"):CallFunction("ChangeIntro")
    camerGO:GetLuaScript("IntroScreen.lua"):SetVariable("BGTextureName", "TekongTrans")
    camerGO:GetLuaScript("IntroScreen.lua"):CallFunction("ChangeBackground")
  end

  playerGO        = CurrentLayer():GetObject("Player")
  playerScript    = playerGO:GetLuaScript("PlayerScript.lua")
  playerResource  = playerGO:GetLuaScript("PlayerResourceManagement.lua")
  playerTransform = playerGO:GetComponent("Transform")
  
  core_GO = CurrentLayer():GetObject("Core")
  core_Transform = core_GO:GetComponent("Transform")
  
  originalPos    = cameraTransform:GetWorldPosition()
  originalLookAt = camera:GetLookAt()
  
  DialogBoxScript = camerGO:GetLuaScript("DialogBox.lua")
  
  playerBullet = CurrentLayer():GetObject("CutSceneBullet")
  playerBulletTranform = playerBullet:GetComponent("Transform")
  
  encik = CurrentLayer():GetObject("Encik")
  encikTransform = encik:GetComponent("Transform")
  encikAnimator = encik:GetComponent("MeshAnimator")
  
  encikSlime = CurrentLayer():GetObject("EncikSlime")
  encikSlimeTranform = encikSlime:GetComponent("Transform")
  
  slimeDeath = CurrentLayer():GetObject("DeathParticle")
  slimeDeathTransform = slimeDeath:GetComponent("Transform")
  
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
  
  if (dialogDelay > 0) then
    dialogDelay = dialogDelay - dt
  end
  
  -- Skip cut scene
  if(currentEvent == 1 and inCutScene) then
    if (ControllerPress("Shoot"))then
      EndCutScene()
      AUDIOEMITTER = camerGO:GetComponent("AudioEmitter")
      AUDIOEMITTER:SetAudioClip("BGM_Changi_Loop_PlayAfterOpening")
      AUDIOEMITTER:SetLoop(true)
      AUDIOEMITTER:SetVolume(0.5)
      AUDIOEMITTER:Play()
    end
  end

  -- Update events
  if (inEvent) then
    if (inCutScene) then
          if (currentEvent == 1) then UpdateCutScene1(dt)
      elseif (currentEvent == 2) then UpdateCutScene2(dt)
      elseif (currentEvent == 3) then UpdateCutScene3(dt)
      elseif (currentEvent == 4) then UpdateCutScene4(dt)
      elseif (currentEvent == 5) then UpdateCutScene5(dt)
      else write("Unexpected Event occured in Tekong") end
    else
          if (currentEvent == 1) then UpdateEvent1(dt)
      elseif (currentEvent == 2) then UpdateEvent2(dt)
      elseif (currentEvent == 3) then UpdateEvent3(dt)
      elseif (currentEvent == 4) then UpdateEvent4(dt)
      elseif (currentEvent == 5) then UpdateEvent5(dt)
      else write("Unexpected Event occured in Tekong") end
    end
  end
end

-- EVENTS ======================================================================
function StartNextEvent()
  write("Starting next Event", currentEvent + 1)
  
  currentEvent = currentEvent + 1
  cutSceneState = 1
  inEvent      = false
  
      if (currentEvent == 1) then timer_EventStart = event1_startdelay
  elseif (currentEvent == 2) then timer_EventStart = event2_startdelay
  elseif (currentEvent == 3) then timer_EventStart = event3_startdelay
  elseif (currentEvent == 4) then timer_EventStart = event4_startdelay
  elseif (currentEvent == 5) then timer_EventStart = event5_startdelay
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
  StartCutScene()
end

function StartEvent2()
  spawnerParticle = CreatePrefab("SummonerMovement")
  spawnerParticleTransform = spawnerParticle:GetComponent("Transform")
  
  StartCutScene()
end

function StartEvent3()
  -- REMOVE THIS LATER /*
  --spawnerParticle = CreatePrefab("SummonerMovement")
  --spawnerParticleTransform = spawnerParticle:GetComponent("Transform")
  --  */
  
  spawner          = CreatePrefab("Slime_Spawner")
  spawnerAnimator  = spawner:GetComponent("MeshAnimator")
  spawnerTransform = spawner:GetComponent("Transform")
  spawnerAnimator:Play("StruggleLoop")
  
  encikAnimator:Play("StruggleLoop")
  
  StartCutScene()
end

function StartEvent4()
end

function StartEvent5()
end

-- CUTSCENE Update =============================================================
function UpdateCutScene1(dt)
  -- Pan around world
  if     (cutSceneState == 1) then  timer   = 0
                                    
                                    vector = Vector3(37, 20, -50)
                                    vector2 = Vector3(-0.5, -0.5, 0.7)
                                    
                                    cameraTransform:SetWorldPosition(vector)
                                    camera:SetLookAt(vector2)
                                    cutSceneState = cutSceneState + 1
  -- Move to merlion
  elseif (cutSceneState == 2) then  timer        = timer + dt
                                    cameraPos    = Vector3Lerp(vector , Vector3(-37, 20, 123), timer / 10)
                                    cameraLookAt = Vector3Lerp(vector2, Vector3(0, -0.6, 0.7), timer / 10)
                                    cameraTransform:SetWorldPosition(cameraPos)
                                    camera:SetLookAt(cameraLookAt)
                                    
                                    if (timer >= 10) then
                                      vector  = cameraTransform:GetWorldPosition()
                                      vector2 = camera:GetLookAt()
                                      vector3 = cameraTransform:GetWorldPosition() - core_Transform:GetWorldPosition()
                                      
                                      timer = 0
                                      cutSceneState = cutSceneState + 1
                                    end
  -- Look around merlion
  elseif (cutSceneState == 3) then  timer          = timer + dt
                                    currCameraPos  = Vector3Lerp(vector , vector2, timer / 0.01)
                                    cameraTransform:SetWorldPosition(currCameraPos)
                                    
                                    cameraLookAt = core_Transform:GetWorldPosition() - cameraTransform:GetWorldPosition()
                                    camera:SetLookAt(cameraLookAt:Normalize())
                                    
                                    if (timer > 0.01) then
                                      current_Angle = current_Angle + 1
                                      
                                      if (current_Angle > 270) then
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
  elseif (cutSceneState == 4) then  timer        = timer + dt
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
  
  elseif (cutSceneState == 5) then  timer = timer + dt
                                    if (timer >= 0.5) then
                                      camerGO:GetLuaScript("Changi_intro.lua"):CallFunction("start")
                                      cutSceneState = cutSceneState + 1
                                      timer = 0
                                    end
									
  elseif (cutSceneState == 6) then  timer = timer + dt
                                    if (timer >= 5) then
                                      EndCutScene()
                                    end
  else end
end

-- Spawn spawnner
function UpdateCutScene2(dt)
  -- Set camera position
  if (cutSceneState == 1) then
    cameraTransform:SetWorldPosition(Vector3(-76, 7, 53))
    vector = Vector3(0, 0.5, -1)
    camera:SetLookAt(vector)
    
    vector2 = Vector3(-76, 100, 29)
    spawnerParticleTransform:SetWorldPosition(vector2)
    
    timer  = 0
    cutSceneState = cutSceneState + 1
  
  -- Particle comes down
  elseif (cutSceneState == 2) then
    timer = timer + dt
    
    lookAt = Vector3Lerp(vector, Vector3(0, 0, -1), timer / 2)
    camera:SetLookAt(lookAt)
    
    pos = Vector3Lerp(vector2, Vector3(-76, 0, 29), timer / 2)
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
  -- Snap camera position
  if (cutSceneState == 1) then
    cameraTransform:SetWorldPosition(Vector3(-15, 6, 100))
    camera:SetLookAt(Vector3(-0.4, -0.05, -0.9))
    
    vector = Vector3(3.25, 5, 75)
    spawnerParticleTransform:SetWorldPosition(vector)
    
    light = CurrentLayer():GetObject("CutSceneLight")
    lightTrans = light:GetComponent("Transform")
    lightTrans:SetWorldPosition(Vector3(-19, 10, 82))
    
    timer = 0
    cutSceneState = cutSceneState + 1
  
  -- Spawner flies in
  elseif (cutSceneState == 2) then
    timer = timer + dt
    
    currPos = Vector3Lerp(vector, Vector3(-27, 0, 75), timer / 4)
    spawnerParticleTransform:SetWorldPosition(currPos)
    
    if (timer >= 4 ) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      vector = Vector3(-27, 0.2, 75)
      spawnerParticleTransform:SetWorldPosition(Vector3(0, -100, 0))
      spawnerTransform:SetWorldPosition(vector)
      spawnerTransform:SetWorldRotation(Vector3(0, -90, 0))
      
      vector2 = Vector3(5, 1.1, 75)
      playerTransform:SetWorldPosition(vector2)
      playerTransform:SetWorldRotation(Vector3(0, 90, 0))
      playerScript:CallFunction("PlayWalkAnim")
      
      rgbd = playerGO:GetComponent("RigidBody")
      rgbd:SetVelocity(Vector3())
    end
  
  -- Spawner crawls to position
  elseif (cutSceneState == 3) then
    timer = timer + dt
    
    currPos = Vector3Lerp(vector, Vector3(-29.5, 0.2, 75), timer / 4)
    spawnerTransform:SetWorldPosition(currPos)
    
    currPos = Vector3Lerp(vector2, Vector3(-15, 1.1, 75), timer / 4)
    playerTransform:SetWorldPosition(currPos)
    
    if (timer >= 4) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      playerScript:CallFunction("PlayIdleAnim")
    end
  
  -- Dialog
  elseif (cutSceneState == 4) then
    ClearDialogBox()
    DialogBoxScript:SetVariable("NameText", "Player")
    DialogBoxScript:CallFunction("ChangeName")
    AddDialogBoxText("ENCIK! STOP!")
    AddDialogBoxSound("Changi_E3p_1")
    AddDialogBoxText("I KNOW YOU CAN HEAR ME!")
    AddDialogBoxSound("Changi_E3p_2")
    AddDialogBoxText("I KNOW YOU CAN HEAR ME! SNAP OUT OF IT!")
    AddDialogBoxSound("Changi_E3p_3")
    cutSceneState = cutSceneState + 1
  
  -- Dialog
  elseif (cutSceneState == 5) then
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      vector = spawnerTransform:GetWorldRotation()
    end
  
  -- Spawner rotates to face player
  elseif (cutSceneState == 6) then
    timer = timer + dt
    
    currRot = Vector3Lerp(vector, Vector3(0, 90, 0), timer / 2)
    spawnerTransform:SetWorldRotation(currRot)
    
    if (timer >= 2.5) then
      timer = 0
      cutSceneState = cutSceneState + 1
    end
  
  -- Dialog
  elseif (cutSceneState == 7) then
    ClearDialogBox()
    DialogBoxScript:SetVariable("NameText", "Infected")
    DialogBoxScript:CallFunction("ChangeName")
    AddDialogBoxText("GROWL! GROWL! ...")
    AddDialogBoxSound("Changi_E3_1")
    AddDialogBoxText("GROWL! GROWL! ... GRRRR")
    AddDialogBoxSound("Changi_E3_3")
    AddDialogBoxText("GROWL! GROWL! ... GRRRR ARGHH!")
    AddDialogBoxSound("Changi_E3_1")
    AddDialogBoxText("YOU MADE IT ...")
    AddDialogBoxSound("Changi_E3_5")
    AddDialogBoxText("YOU MADE IT ... SON ...")
    AddDialogBoxSound("Changi_E3_6")
    
    cutSceneState = cutSceneState + 1
    
  -- Dialog
  elseif (cutSceneState == 8) then
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Player")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("?!?")
      AddDialogBoxSound("Changi_E3p_4")
      AddDialogBoxText("?!? WHAT? ...")
      AddDialogBoxSound("Changi_E3p_5")
      AddDialogBoxText("?!? WHAT? ... WHAT ARE YOU MUMBLING ABOUT?")
      AddDialogBoxSound("Changi_E3p_6")
      AddDialogBoxText("?!? WHAT? ... WHAT ARE YOU MUMBLING ABOUT? \nGET A GRIP ON YOURSELF!")
      AddDialogBoxSound("Changi_E3p_7")
    end
  
  -- Dialog
  elseif (cutSceneState == 9) then
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Infected")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("LISTEN TO ME ...")
      AddDialogBoxSound("Changi_E3_7")
      AddDialogBoxText("LISTEN TO ME ... I DON'T HAVE MUCH TIME.")
      AddDialogBoxSound("Changi_E3_8")
      AddDialogBoxText("LISTEN TO ME ... I DON'T HAVE MUCH TIME. \nI DON'T KNOW WHEN THE SLIME WILL TAKE CONTROL AGAIN.")
      AddDialogBoxSound("Changi_E3_9")
      AddDialogBoxText("IT ALL BEGIN WHEN YOU WERE JUST A BABY ...")
      AddDialogBoxSound("Changi_E3_10")
    end
  
  -- Dialog
  elseif (cutSceneState == 10) then
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Player")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("OH BOY TRAGIC BACK STORY TIME ...")
      AddDialogBoxSound("Changi_E3p_8")
      AddDialogBoxText("OH BOY TRAGIC BACK STORY TIME ... WHAT SHOULD I DO?")
      AddDialogBoxSound("Changi_E3p_9")
    end
  
  -- Dialog choice
  elseif (cutSceneState == 11) then
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Instructions")
      DialogBoxScript:CallFunction("ChangeName")
      if (GetControllerInput() == 0) then
        AddDialogBoxText("> SHOOT TO CONTINUE LISTENING TO ENCIK'S STORY \n> PRESS F TO SHOOT ENCIK")
      else
        AddDialogBoxText("> PRESS A CONTINUE LISTENING TO ENCIK'S STORY \n> PRESS B TO SHOOT ENCIK")
      end
      AddDialogBoxSound("")
    end
  
  -- Dialog
  elseif (cutSceneState == 12) then
    -- Check for input
    if (ControllerUp("SwitchMode")) then
      DialogBoxScript:CallFunction("GoOut")
      dialogState = 0
      dialogLeftTexture = 0
      dialogRightTexture = 0
      StartNextEvent()
    elseif (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Infected")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("YOU WERE JUST BORN INTO THE WORLD AND")
      AddDialogBoxSound("Changi_E3_11")
      AddDialogBoxText("YOU WERE JUST BORN INTO THE WORLD AND I WAS \nOFFERED A PROMOTION.")
      AddDialogBoxSound("Changi_E3_12")
      AddDialogBoxText("IT WAS AN ENTICING OFFER BUT THERE WAS A CATCH.")
      AddDialogBoxSound("Changi_E3_13")
      AddDialogBoxText("I WAS TO BE STATIONED IN TEKONG FOR A LONG PERIOD \nOF TIME.")
      AddDialogBoxSound("Changi_E3_14")
      AddDialogBoxText("BEING YOUNG AND FOOLISH,")
      AddDialogBoxSound("Changi_E3_15")
      AddDialogBoxText("BEING YOUNG AND FOOLISH, MONEY WAS THE ONLY \nTHING I COULD THINK OF.")
      AddDialogBoxSound("Changi_E3_16")
      AddDialogBoxText("I TOOK THE OFFER IMMEDIATELY.")
      AddDialogBoxSound("Changi_E3_17")
      AddDialogBoxText("I TOOK THE OFFER IMMEDIATELY. YOUR MOTHER WAS \nDISAPOINTED IN ME.")
      AddDialogBoxSound("Changi_E3_18")
      AddDialogBoxText("THE FACT THAT I DIDN'T CONSULT HER BEFORE COMING \nTO A DECISION,")
      AddDialogBoxSound("Changi_E3_19")
      AddDialogBoxText("THE FACT THAT I DIDN'T CONSULT HER BEFORE COMING \nTO A DECISION, CRUSHED HER HEART.")
      AddDialogBoxSound("Changi_E3_20")
      AddDialogBoxText("THAT DAY,")
      AddDialogBoxSound("Changi_E3_21")
      AddDialogBoxText("THAT DAY, SHE TOOK YOU AND LEFT WITHOUT SAYING A WORD.")
      AddDialogBoxSound("Changi_E3_22")
      AddDialogBoxText("THAT DAY,")
      AddDialogBoxSound("Changi_E3_21")
      AddDialogBoxText("THAT DAY, I LOST EVERYTHING.")
      AddDialogBoxSound("Changi_E3_23")
    end
  
    -- Dialog
  elseif (cutSceneState == 13) then
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Player")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("HUH ...")
      AddDialogBoxSound("Changi_E3p_10")
      AddDialogBoxText("HUH ... I HAVE NEVER SEEN MY DAD BEFORE AND")
      AddDialogBoxSound("Changi_E3p_11")
      AddDialogBoxText("HUH ... I HAVE NEVER SEEN MY DAD BEFORE AND \nMUM TOLD ME DAD PASSED AWAY A LONG TIME AGO ...")
      AddDialogBoxSound("Changi_E3p_12")
      AddDialogBoxText("COULD HE BE TELLING THE TRUTH?")
      AddDialogBoxSound("Changi_E3p_13")
    end
    
  -- Dialog choice
  elseif (cutSceneState == 14) then
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Instructions")
      DialogBoxScript:CallFunction("ChangeName")
      if (GetControllerInput() == 0) then
        AddDialogBoxText("> SHOOT TO CONTINUE LISTENING TO ENCIK'S STORY \n> PRESS F TO SHOOT ENCIK")
      else
        AddDialogBoxText("> PRESS A CONTINUE LISTENING TO ENCIK'S STORY \n> PRESS B TO SHOOT ENCIK")
      end
      AddDialogBoxSound("")
    end
  
  -- Dialog
  elseif (cutSceneState == 15) then
    -- Check for input
    if (ControllerUp("SwitchMode")) then
      DialogBoxScript:CallFunction("GoOut")
      dialogState = 0
      dialogLeftTexture = 0
      dialogRightTexture = 0
      StartNextEvent()
    elseif (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Infected")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("DURING MY TIME IN TEKONG,")
      AddDialogBoxSound("Changi_E3_24")
      AddDialogBoxText("DURING MY TIME IN TEKONG, I HAVE SEEN MANY YOUNG \nBOY BECOME MEN.")
      AddDialogBoxSound("Changi_E3_25")
      AddDialogBoxText("I KEPT WONDERING HOW YOU AND YOUR MUM ARE DOING.")
      AddDialogBoxSound("Changi_E3_26")
      AddDialogBoxText("THERE WAS A TIME WHEN I DECIDED TO SNEAK OUT")
      AddDialogBoxSound("Changi_E3_27")
      AddDialogBoxText("THERE WAS A TIME WHEN I DECIDED TO SNEAK OUT \nTO FIND YOU BOTH.")
      AddDialogBoxSound("Changi_E3_28")
      AddDialogBoxText("THERE WAS A TIME WHEN I DECIDED TO SNEAK OUT \nTO FIND YOU BOTH. BUT I WAS CAUGHT AT THE HARBOR.")
      AddDialogBoxSound("Changi_E3_29")
      AddDialogBoxText("I WAS PUNISHED BY MY SUPERIORS")
      AddDialogBoxSound("Changi_E3_30")
      AddDialogBoxText("I WAS PUNISHED BY MY SUPERIORS AND WAS TO STAY \nIN TEKONG FOR THE REST OF MY LIFE.")
      AddDialogBoxSound("Changi_E3_31")
      AddDialogBoxText("THAT DAY WHEN YOU FIRST SET FOOT ON TEKONG,")
      AddDialogBoxSound("Changi_E3_32")
      AddDialogBoxText("THAT DAY WHEN YOU FIRST SET FOOT ON TEKONG, \nI SUSPECTED YOU WERE MY SON ...")
      AddDialogBoxSound("Changi_E3_33")
      AddDialogBoxText("BUT I WASN'T SURE.")
      AddDialogBoxSound("Changi_E3_34")
      AddDialogBoxText("BUT I WASN'T SURE. BUT NOW I'M CERTAIN YOU ARE MY SON.")
      AddDialogBoxSound("Changi_E3_35")
      AddDialogBoxText("SON.")
      AddDialogBoxSound("Changi_E3_36")
    end
    
    -- Dialog
  elseif (cutSceneState == 16) then
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Player")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("YOU LIE!")
      AddDialogBoxSound("Changi_E3p_14")
    end
    
  -- Dialog choice
  elseif (cutSceneState == 17) then
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Instructions")
      DialogBoxScript:CallFunction("ChangeName")
      if (GetControllerInput() == 0) then
        AddDialogBoxText("> SHOOT TO CONTINUE LISTENING TO ENCIK'S STORY \n> PRESS F TO SHOOT ENCIK")
      else
        AddDialogBoxText("> PRESS A CONTINUE LISTENING TO ENCIK'S STORY \n> PRESS B TO SHOOT ENCIK")
      end
      AddDialogBoxSound("")
    end
    
  -- Dialog
  elseif (cutSceneState == 18) then
    -- Check for input
    if (ControllerUp("SwitchMode")) then
      DialogBoxScript:CallFunction("GoOut")
      dialogState = 0
      dialogLeftTexture = 0
      dialogRightTexture = 0
      StartNextEvent()
    elseif (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Infected")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("YOUR MUM DIDN'T TELL YOU?")
      AddDialogBoxSound("Changi_E3_36")
    end
    
    -- Dialog
  elseif (cutSceneState == 19) then
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Player")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("SHE TOLD ME ENOUGH!")
      AddDialogBoxSound("Changi_E3p_15")
      AddDialogBoxText("SHE TOLD ME ENOUGH! SHE TOLD ME HE IS DEAD!")
      AddDialogBoxSound("Changi_E3p_16")
    end
    
    -- Dialog
  elseif (cutSceneState == 20) then
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Infected")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("IF ONLY YOU KNEW THE TRUTH.")
      AddDialogBoxSound("Changi_E3_38")
      AddDialogBoxText("SEARCH YOUR FEELINGS, YOU KNOW IT TO BE TRUE.")
      AddDialogBoxSound("Changi_E3_39")
    end
    
    -- Dialog
  elseif (cutSceneState == 21) then
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Player")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("NOOOOOOOO!")
      AddDialogBoxSound("Changi_E3p_17")
      AddDialogBoxText("NOOOOOOOO! IT CAN'T BE TRUE!")
      AddDialogBoxSound("Changi_E3p_18")
    end
    
    -- Dialog
  elseif (cutSceneState == 22) then
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      DialogBoxScript:CallFunction("GoOut")
      dialogState = 0
      dialogLeftTexture = 0
      dialogRightTexture = 0
      StartNextEvent()
    end
  end
end

-- Shoot incheck
function UpdateCutScene4(dt)
  -- Dialog
  if (cutSceneState == 1) then
    timer = timer + dt
    
    if (timer > 1) then
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Player")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("DIE!")
      AddDialogBoxSound("Changi_E4p_0")
      
      timer = 0
      cutSceneState = cutSceneState + 1
    end
    
  -- Snap Camera Pos
  elseif (cutSceneState == 2) then
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      vector  = cameraTransform:GetWorldPosition()
      vector2 = camera:GetLookAt()
    end
  
  -- Move Camera
  elseif (cutSceneState == 3) then
    timer = timer + dt
    
    currPos = Vector3Lerp(vector, Vector3(-15.2, 3, 75), timer / 2)
    cameraTransform:SetWorldPosition(currPos)
    currPos = Vector3Lerp(vector2, Vector3(-1, -0.3, 0), timer / 2)
    camera:SetLookAt(currPos)
    
    if (timer >= 2.5) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "Instructions")
      DialogBoxScript:CallFunction("ChangeName")
      if (GetControllerInput() == 0) then
        AddDialogBoxText("> SHOOT TO SHOOT ENCIK'S HEAD \n> PRESS F TO SHOOT ENCIK'S BODY")
      else
        AddDialogBoxText("> PRESS A TO SHOOT ENCIK'S HEAD \n> PRESS B TO SHOOT ENCIK'S BODY")
      end
      AddDialogBoxSound("")
    end
  
  -- Make Choice
  elseif (cutSceneState == 4) then
    -- Check for input
    if (ControllerUp("SwitchMode")) then
      DialogBoxScript:CallFunction("GoOut")
      dialogState = 0
      dialogLeftTexture = 0
      dialogRightTexture = 0
      StartNextEvent()
      
      vector = Vector3(-17, 0.5, 75)
      playerBulletTranform:SetWorldPosition(vector)
      vector2 = cameraTransform:GetWorldPosition()
      vector3 = spawnerTransform:GetWorldRotation()
      vector4 = spawnerTransform:GetWorldPosition()
    elseif (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      vector = Vector3(-17, 0.5, 75)
      playerBulletTranform:SetWorldPosition(vector)
      vector2 = cameraTransform:GetWorldPosition()
      vector3 = spawnerTransform:GetWorldRotation()
      vector4 = spawnerTransform:GetWorldPosition()
    end
  
  -- Shoot incheck head
  elseif (cutSceneState == 5) then
    timer = timer + dt
    
    currPos = Vector3Lerp(vector, Vector3(-31.75, 3.25, 75), timer / 9.5)
    playerBulletTranform:SetWorldPosition(currPos)
    
    currPos = Vector3Lerp(vector2, Vector3(-25, 4, 75), timer / 7)
    cameraTransform:SetWorldPosition(currPos)
    
    currPos = Vector3Lerp(vector3, Vector3(0, 90, 60), timer / 7)
    spawnerTransform:SetWorldRotation(currPos)
    
    currPos = Vector3Lerp(vector4, vector4 + Vector3(0, 0.2, 0), timer / 7)
    spawnerTransform:SetWorldPosition(currPos)
    
    if (timer >= 7) then
      cutSceneState = cutSceneState + 1
      encikParticle = CreatePrefab("SpawnerParticleNoLight")
    end
  
  -- Spawn slime and enick
  elseif (cutSceneState == 6) then
    timer = timer + dt
    
    encikParticleTransform = encikParticle:GetComponent("Transform")
    encikParticleTransform:SetWorldPosition(Vector3(-30, 2, 75))
    
    currPos = Vector3Lerp(vector, Vector3(-31.75, 3.25, 75), timer / 9.5)
    playerBulletTranform:SetWorldPosition(currPos)
    
    if (timer > 7.2) then
      cutSceneState = cutSceneState + 1
      spawnerAnimator:Stop()
      
      encikTransform:SetWorldPosition(Vector3(-30, 0.2, 75))
      encikTransform:SetWorldRotation(Vector3(0, 90, 60))
      
      vector2 = Vector3(-30, 2.5, 75)
      encikSlimeTranform:SetWorldPosition(vector2)
      encikSlimeTranform:SetWorldRotation(Vector3(-90, 90, 0))
      
      spawnerTransform:SetWorldPosition(Vector3(0, -100, 0))
      
      encikParticleEmitter = encikParticle:GetComponent("ParticleEmitter_Circle")
      encikParticleEmitter:SetEmitRate(0)
    end
  
  -- Hit slime
  elseif (cutSceneState == 7) then
    timer = timer + dt
    
    currPos = Vector3Lerp(vector, Vector3(-31.75, 3.25, 75), timer / 9.5)
    playerBulletTranform:SetWorldPosition(currPos)
    
    currPos = Vector3Lerp(vector2, vector2 + Vector3(0, 0.2, 0), (timer - 7.2) / 0.4)
    encikSlimeTranform:SetWorldPosition(currPos)
    
    if (timer >= 7.6) then
      vector2 = encikSlimeTranform:GetWorldPosition()
      cutSceneState = cutSceneState + 1
    end
  
  -- Move slime and arrow
  elseif (cutSceneState == 8) then
    timer = timer + dt
    
    currPos = Vector3Lerp(vector, Vector3(-31.75, 3.25, 75), timer / 9.5)
    playerBulletTranform:SetWorldPosition(currPos)
    
    currPos = Vector3Lerp(vector2, Vector3(-33, 2.7, 75), (timer - 7.6) / 2.5)
    encikSlimeTranform:SetWorldPosition(currPos)
    
    if (timer >= 9.5) then
      cutSceneState = cutSceneState + 1
      
      script = encikSlime:GetLuaScript("GrowAndShrink.lua")
      script:CallFunction("FadeEffect")
    end
  
  -- Snap camera position
  elseif (cutSceneState == 9) then
    cameraTransform:SetWorldPosition(Vector3(-15, 6, 100))
    camera:SetLookAt(Vector3(-0.4, -0.05, -0.9))
    timer = 0
    cutSceneState = cutSceneState + 1
    slimeDeathTransform:SetWorldPosition( Vector3(-31.75, 3.25, 75))
  
  -- Continue dialog
  elseif (cutSceneState == 10) then
    ClearDialogBox()
    DialogBoxScript:SetVariable("NameText", "PLAYER")
    DialogBoxScript:CallFunction("ChangeName")
    AddDialogBoxText("ENCIK!")
    AddDialogBoxSound("Changi_E4p_1")
    AddDialogBoxText("ENCIK! ARE YOU OK?")
    AddDialogBoxSound("Changi_E4p_2")
    cutSceneState = cutSceneState + 1
      
  elseif (cutSceneState == 11) then
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "SGT JUN JIE")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("...")
      AddDialogBoxSound("")
      AddDialogBoxText("... WHERE AM I?")
      AddDialogBoxSound("Changi_E4_1")
      AddDialogBoxText("... WHERE AM I? WHAT'S GOING ON?")
      AddDialogBoxSound("Changi_E4_2")
    end
  
  elseif (cutSceneState == 12) then
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "PLAYER")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("WE ARE IN CHANGI!")
      AddDialogBoxSound("Changi_E4p_3")
      AddDialogBoxText("WE ARE IN CHANGI! I CHASED YOU ALL THE WAY FROM TEKONG!")
      AddDialogBoxSound("Changi_E4p_4")
      AddDialogBoxText("DO YOU REMEMBER ANYTHING?")
      AddDialogBoxSound("Changi_E4p_5")
    end

  elseif (cutSceneState == 13) then
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "SGT JUN JIE")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("URGHH MY HEAD ...")
      AddDialogBoxSound("Changi_E4_3")
      AddDialogBoxText("URGHH MY HEAD ... I REMEMBER ...")
      AddDialogBoxSound("Changi_E4_4")
      AddDialogBoxText("URGHH MY HEAD ... I REMEMBER ... NOTHING ...")
      AddDialogBoxSound("Changi_E4_5")
    end
  
  elseif (cutSceneState == 14) then
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "PLAYER")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("YOU MENTIONED SOMETHING ABOUT ME")
      AddDialogBoxSound("Changi_E4p_6")
      AddDialogBoxText("YOU MENTIONED SOMETHING ABOUT ME BEING YOUR SON.")
      AddDialogBoxSound("Changi_E4p_7")
      AddDialogBoxText("IS THAT TRUE?")
      AddDialogBoxSound("Changi_E4p_8")
    end
  
  elseif (cutSceneState == 15) then
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
      cutSceneState = cutSceneState + 1
      encikAnimator:Play("Idle")
      encikTransform:SetWorldRotation(Vector3(0, 90, 0))
      currPos = encikTransform:GetWorldPosition()
      currPos.y = 0.1
      encikTransform:SetWorldPosition(currPos)
        
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "SGT JUN JIE")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("!!?!")
      AddDialogBoxSound("Changi_E4_6")
      AddDialogBoxText("!!?! OH LOOK AT THE TIME,")
      AddDialogBoxSound("Changi_E4_7")
      AddDialogBoxText("!!?! OH LOOK AT THE TIME, WE BETTER GET GOING.")
      AddDialogBoxSound("Changi_E4_8")
      AddDialogBoxText("!!?! OH LOOK AT THE TIME, WE BETTER GET GOING. \nIT'S NOT SAFE TO BE OUT HERE!")
      AddDialogBoxSound("Changi_E4_9")
    end
  
  elseif (cutSceneState == 16) then
    LeftIn("Inchek")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      vector = encikTransform:GetWorldPosition()
      vector2 = encikTransform:GetWorldRotation()
      vector3 = playerTransform:GetWorldRotation()
      
      encikAnimator:Play("Jog")
    end
  
  -- Enchik walks away
  elseif (cutSceneState == 17) then
    timer = timer + dt
    
    currPos = Vector3Lerp(vector, vector + Vector3(0, 0, 30), timer / 15)
    encikTransform:SetWorldPosition(currPos)
    
    currPos = Vector3Lerp(vector2, Vector3(0, 0, 0), timer / 0.2)
    encikTransform:SetWorldRotation(currPos)
    
    currPos = Vector3Lerp(vector3, vector3 + Vector3(0, 60, 0), timer / 15)
    playerTransform:SetWorldRotation(currPos)
    
    if (timer >= 3.5) then
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "PLAYER")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("HEY!")
      AddDialogBoxSound("Changi_E4p_9")
      AddDialogBoxText("WE AIN'T DONE!")
      AddDialogBoxSound("Changi_E4p_10")
      AddDialogBoxText("GET BACK HERE!")
      AddDialogBoxSound("Changi_E4p_11")
    end
    
  -- Continue dialog
  elseif (cutSceneState == 18) then
    timer = timer + dt
    
    currPos = Vector3Lerp(vector, vector + Vector3(0, 0, 30), timer / 15)
    encikTransform:SetWorldPosition(currPos)
    
    currPos = Vector3Lerp(vector3, vector3 + Vector3(0, 60, 0), timer / 15)
    playerTransform:SetWorldRotation(currPos)
    
    RightIn("Player")
    if (RunDialogBox()) then
      timer2 = 0
      timer3 = 0
      cutSceneState = cutSceneState + 1
      vector2 = playerTransform:GetWorldPosition()
      
      playerScript:CallFunction("PlayWalkAnim")
    end
    
  -- Player chases inchek
  elseif (cutSceneState == 19) then
    timer  = timer  + dt
    timer2 = timer2 + dt
    timer3 = timer3 + dt
    
    currPos = Vector3Lerp(vector, vector + Vector3(0, 0, 30), timer / 15)
    encikTransform:SetWorldPosition(currPos)
    
    currPos = Vector3Lerp(vector2, encikTransform:GetWorldPosition(), timer2 / 7)
    playerTransform:SetWorldPosition(currPos)
    
    currPos = Vector3Lerp(vector3, vector3 + Vector3(0, 60, 0), timer / 15)
    playerTransform:SetWorldRotation(currPos)
    
    if (timer3 >= 1.5) then
      timer3 = 0
      cutSceneState = cutSceneState + 1
      vector4 = camera:GetLookAt()
    end
    
  -- Camera pan up
  elseif (cutSceneState == 20) then
    timer  = timer  + dt
    timer2 = timer2 + dt
    timer3 = timer3 + dt
    
    currPos = Vector3Lerp(vector, vector + Vector3(0, 0, 30), timer / 15)
    encikTransform:SetWorldPosition(currPos)
    
    currPos = Vector3Lerp(vector2, encikTransform:GetWorldPosition(), timer2 / 7)
    playerTransform:SetWorldPosition(currPos)
    
    currPos = Vector3Lerp(vector3, vector3 + Vector3(0, 60, 0), timer / 15)
    playerTransform:SetWorldRotation(currPos)
    
    currPos = Vector3Lerp(vector4, vector4 + Vector3(0, 2, 0), timer3 / 6)
    camera:SetLookAt(currPos)
    
    if (timer3 >= 6.5) then
      cutSceneState = cutSceneState + 1
    end
    
  -- End game
  elseif (cutSceneState == 21) then
    PlayerPref_SetInteger("WinState", 1)
    SceneLoad("Level_Credits")
  end
end

-- Bad end
function UpdateCutScene5(dt)
 -- Shoot incheck body
  if (cutSceneState == 1) then
    timer = timer + dt
    
    currPos = Vector3Lerp(vector, Vector3(-31.75, 0.75, 75), timer / 4)
    playerBulletTranform:SetWorldPosition(currPos)
    
    currPos = Vector3Lerp(vector2, Vector3(-25, 4, 75), timer / 3)
    cameraTransform:SetWorldPosition(currPos)
    
    currPos = Vector3Lerp(vector3, Vector3(0, 0, 0), timer / 3)
    spawnerTransform:SetWorldRotation(currPos)
    
    if (timer >= 3) then
      cutSceneState = cutSceneState + 1
      
      vector2 = spawnerTransform:GetWorldRotation()
      vector3 = spawnerTransform:GetWorldPosition()
    end
  
  -- Hit inchek and move back
  elseif (cutSceneState == 2) then
    timer = timer + dt
    
    currPos = Vector3Lerp(vector, Vector3(-31.75, 0.75, 75), timer / 4)
    playerBulletTranform:SetWorldPosition(currPos)
    
    currPos = Vector3Lerp(vector2, vector2 + Vector3(0, 0, 40), (timer - 3) / 0.5)
    spawnerTransform:SetWorldRotation(currPos)
    
    currPos = Vector3Lerp(vector3, vector3 + Vector3(-2, 0, 0), (timer - 3) / 0.7)
    spawnerTransform:SetWorldPosition(currPos)
    
    if (timer > 4.2) then
      cutSceneState = cutSceneState + 1
      
      cameraTransform:SetWorldPosition(Vector3(-15, 6, 100))
      camera:SetLookAt(Vector3(-0.4, -0.05, -0.9))
      timer = 0
      cutSceneState = cutSceneState + 1
      slimeDeathTransform:SetWorldPosition( Vector3(-31.75, 3.25, 75))
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "INFECTED")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("*BLEH!*")
      AddDialogBoxSound("Changi_E5_1")
      AddDialogBoxText("*BLEH!* *COUGH* *COUGH*")
      AddDialogBoxSound("Changi_E5_2")
      
      vector = playerTransform:GetWorldPosition()
      playerScript:CallFunction("PlayWalkAnim")
    end
  
  -- Player checks on Inchek + dialogue
  elseif (cutSceneState == 3) then
    timer = timer + dt
    currPos = Vector3Lerp(vector, vector + Vector3(-10, 0, 0) , timer / 2)
    playerTransform:SetWorldPosition(currPos)
    if (timer >= 2) then
      playerScript:CallFunction("PlayIdleAnim")
    end
    
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "PLAYER")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("TELL ME THE TRUTH!")
      AddDialogBoxSound("Changi_E5p_1")
      AddDialogBoxText("TELL ME THE TRUTH! ARE YOU REALLY MY DAD?")
      AddDialogBoxSound("Changi_E5p_2")
    end
    
  elseif (cutSceneState == 4) then
    timer = timer + dt
    currPos = Vector3Lerp(vector, vector + Vector3(-10, 0, 0) , timer / 2)
    playerTransform:SetWorldPosition(currPos)
    if (timer >= 2) then
      playerScript:CallFunction("PlayIdleAnim")
    end
    
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      cutSceneState = cutSceneState + 1
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "INFECTED")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("SON ...")
      AddDialogBoxSound("Changi_E5_3")
      AddDialogBoxText("SON ... I ...")
      AddDialogBoxSound("Changi_E5_4")
      AddDialogBoxText("SON ... I ... LOVE ...")
      AddDialogBoxSound("Changi_E5_5")
      AddDialogBoxText("*BLEH*")
      AddDialogBoxSound("Changi_E5_6")
    end
  
  elseif (cutSceneState == 5) then
    timer = timer + dt
    currPos = Vector3Lerp(vector, vector + Vector3(-10, 0, 0) , timer / 2)
    playerTransform:SetWorldPosition(currPos)
    if (timer >= 2) then
      playerScript:CallFunction("PlayIdleAnim")
    end
    
    currPos = Vector3Lerp(vector, Vector3(-5, 0, 0), timer / 9.5)
    playerBulletTranform:SetWorldPosition(currPos)
    
    LeftIn("Infected")
    RightIn("Player")
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
      
      vector = camera:GetLookAt()
      
      ClearDialogBox()
      DialogBoxScript:SetVariable("NameText", "PLAYER")
      DialogBoxScript:CallFunction("ChangeName")
      AddDialogBoxText("NOOOOOOOOOOOOOOO!")
      AddDialogBoxSound("Changi_E5p_3")
    end
    
  -- Camera pan up
  elseif (cutSceneState == 6) then
    if (RunDialogBox()) then
      timer = 0
      cutSceneState = cutSceneState + 1
    end
    
  elseif (cutSceneState == 7) then
    timer  = timer  + dt
    currPos = Vector3Lerp(vector, vector + Vector3(0, 2, 0), timer / 6)
    camera:SetLookAt(currPos)
    
    if (timer >= 6.5) then
      cutSceneState = cutSceneState + 1
    end
    
  -- End game
  elseif (cutSceneState == 8) then
    PlayerPref_SetInteger("WinState", 2)
    SceneLoad("Level_Credits")
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
      write("GG")
      SceneLoad("LevelSelect")
    else
      write("win")
      PlayerPref_SetBool("ClearedChangi", true)
      AUDIOEMITTER = camerGO:GetComponent("AudioEmitter")
      AUDIOEMITTER:SetAudioClip(BGM_ENDING)
      AUDIOEMITTER:SetLoop(true)
      AUDIOEMITTER:SetVolume(0.75)
      AUDIOEMITTER:Play()
      StartNextEvent()
    end
  end
end

function UpdateEvent3(dt)
end

function UpdateEvent4(dt)
end

function UpdateEvent5(dt)
end

--  Dialogbox ===============================================================
function ClearDialogBox()
  DialogBoxScript:CallFunction("ClearText")
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
  up = DialogBoxScript:GetVariable("up")
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
  up = DialogBoxScript:GetVariable("up")
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
     dialogDelay = 0.4
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