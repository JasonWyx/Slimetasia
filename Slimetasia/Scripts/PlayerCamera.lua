-- VARIABLES ===================================================================
local disable = false
local safeLookAt = Vector3()
local callOnce = true

-- settings
sensitivity           = Vector3()
maxY                  =  40.0
minY                  = -15.0
lockRotation          = false
useAcceleration       = true
playingBGM            = false
currentControllerMode = true  -- true = keyboard / false = controller
currentPlayerMode     = true  -- true = shooting / false = trap building
currentRotation       = Vector3()
currentRotationY      = 0
notMoving             = true

-- BGM
local BGM_ChinaTown = "BGM_Chinatown"
local BGM_Tekong    = "BGM_Tekong"
local BGM_Yishun    = "BGM_Game.ogg"
local BGM_Changi    = "BGM_Changi_Opening"
local BGM_Changi_2  = "BGM_Changi_Loop_PlayAfterOpening"

-- camera current values
local epsilon                    = 1
local lookAt                     = Vector3() -- Camera's look at
local currRotation               = Vector3() -- What is the angle of rotation for the camera look at
local fakeRotation               = Vector3() -- The angle of rotation that slowly moves towards currRotation
local LookAtSpeedCurr            = 0.0
local LookAtSpeedMin             = 0.0
local LookAtSpeedMax             = 0.1
local LookAtAcceleration         = 0.001
local vectorToPlayer             = Vector3(0.0, 4.0,  5.0)
local vectorToPlayerLength       = 0

-- camera clipping
local distanceFromObstacles      = 0.2
local vectorToPlayerScaleMin     = 0.3
local vectorToPlayerScale        = 1
local vectorToPlayerScaleCurr    = 1
local speedVectorToPlayer        = 1
local epsilonClipping            = 0.1

-- gameobjects / components
local CAMERAGO        = nil
local CAMERA          = nil
local CAMERATRANSFORM = nil
local haveplayer      = true
local PLAYERGO        = nil
local PLAYERTRANSFORM = nil

-- keyboard Settings
local kSensitivityBase    = Vector3(1, 1, 1)
local kSensitivity        = Vector3()
local kLookAtMinSpeed     = 0.3
local kLookAtMaxSpeed     = 0.5
local kLookAtAcceleration = 0.05

-- controller settings
local cSensitivityBase    = Vector3(3, 4, 1)--Vector3(1.5, 2, 1.5)
local cSensitivity        = Vector3()
local cLookAtMinSpeed     = 0.3
local cLookAtMaxSpeed     = 0.5
local cLookAtAcceleration = 0.05 -- 0.1
                                                                                -- Debug
                                                                                local scriptOk = true
-- FUNCTIONS ===================================================================
function Constructor()
  ChangeSensitivity()
  
  CAMERAGO = owner
                                                                                if (CAMERAGO == nil) then scriptOk = false write ("Player camera : Missing camera"          ) end
  CAMERA   = CAMERAGO:GetComponent("Camera")
                                                                                if (CAMERA   == nil) then scriptOk = false write ("Player camera : Missing camera component") end
                                                                                
                                                                                if (scriptOk) then
                                                                                  write("Player camera : pass construction")
                                                                                else
                                                                                  write("Player camera : fail construction")
                                                                                end
  PLAYERGO = CurrentLayer():GetObject("Player")
  if (PLAYERGO == nil) then 
    haveplayer = false 
                                                                                write ("Player camera : Missing Player") 
  else
    PLAYERTRANSFORM = PLAYERGO:GetComponent("Transform")
    if (PLAYERTRANSFORM == nil) then
      haveplayer = false 
                                                                                write ("Player camera : Missing TRANSFORM in Player")
                                                                                else
                                                                                write ("Player camera : Found Player")
    end
  end
end

function MyConstuctor ()
    lookAt = CAMERA:GetLookAt()
    
    -- HARDCODED VALUES
    CAMERATRANSFORM =  CAMERAGO:GetComponent("Transform")
    CAMERATRANSFORM:SetWorldPosition(PLAYERTRANSFORM:GetWorldPosition() + vectorToPlayer)
    CAMERA:SetLookAt(Vector3(0.0, -0.3, -0.9))
    
    -- Set camera settings to fit current mode
    ChangeCameraSettings(false)
    vectorToPlayerLength = vectorToPlayer:Length()
    
    write("ok")
    
    callOnce = false
end

function OnUpdate(dt)
  -- NANI
  ChangeCameraSettings(true)
  
  if (callOnce) then
    MyConstuctor()
  end

  -- Update rotation
  CAMERATRANSFORM:SetWorldRotation(CAMERA:GetLookAt())
  
  -- Game bgm
  if (not playingBGM) then
    AUDIOEMITTER = CAMERAGO:GetComponent("AudioEmitter")
    if (AUDIOEMITTER ~= nil) then 
      currentLevel = PlayerPref_GetString("CurrentLevel")
      setloop = true
      if     (currentLevel == "Level_ChinaTown") then AUDIOEMITTER:SetAudioClip(BGM_ChinaTown)
      elseif (currentLevel == "Level_Tekong"   ) then AUDIOEMITTER:SetAudioClip(BGM_Tekong   )
      elseif (currentLevel == "Level_Yishun"   ) then AUDIOEMITTER:SetAudioClip(BGM_Yishun   )
      elseif (currentLevel == "Level_Changi"   ) then AUDIOEMITTER:SetAudioClip(BGM_Changi   ) setloop = false
      end
      AUDIOEMITTER:SetLoop(setloop)
      AUDIOEMITTER:SetVolume(0.5)
      AUDIOEMITTER:Play()
    end
    playingBGM = true
  end
  
  if (PlayerPref_GetString("CurrentLevel") == "Level_Changi" and AUDIOEMITTER:IsPlaying() == false) then
    AUDIOEMITTER:SetAudioClip(BGM_Changi_2)
    AUDIOEMITTER:SetLoop(true)
    AUDIOEMITTER:SetVolume(0.5)
    AUDIOEMITTER:Play()
  end
  
  -- Camera script
                                                                                if (scriptOk and not disable) then
  if (not lockRotation) then
    -- Looking around
    mouseDeltaX = -ControllerAxis("LookLeft") + ControllerAxis("LookRight")
    mouseDeltaY = -ControllerAxis("LookDown") + ControllerAxis("LookUp"   )
    currRotation.x = currRotation:x() - mouseDeltaX * sensitivity:x()
    currRotation.y = currRotation:y() + mouseDeltaY * sensitivity:y()
    notMoving = (mouseDeltaX == 0) and (mouseDeltaY == 0)
    
    -- Camera clamping
    if     (currRotation:y() > maxY) then currRotation.y = maxY
    elseif (currRotation:y() < minY) then currRotation.y = minY
    end
    
    -- Accelerated camera movement
    if (useAcceleration) then 
      UpdateLookAtLookAtAcceleration(dt)
    else 
      UpdateLookAt(currRotation:x(), currRotation:y())
    end
    
    -- Camera clipping
    UpdateCameraClipping()
    
    -- Update vector to player scale
    diff = vectorToPlayerScaleCurr - vectorToPlayerScale
    if (Abs(diff)>epsilonClipping) then
      direction = 1
      if (vectorToPlayerScaleCurr > vectorToPlayerScale) then
        direction = -1
      end
      
      vectorToPlayerScaleCurr = vectorToPlayerScaleCurr + direction * speedVectorToPlayer * dt
    end
  else
    -- Camera clamping
    if     (currRotation:y() > maxY) then currRotation.y = maxY
    elseif (currRotation:y() < minY) then currRotation.y = minY
    end
    write("HERE")
  end
                                                                                end
end

-- ROTATION ====================================================================
function UpdateLookAtLookAtAcceleration(dt)
  myDelta = currRotation - fakeRotation
                                                                                --write(myDelta)
  if (myDelta:Length() <= epsilon) then
    LookAtSpeedCurr = LookAtSpeedCurr - LookAtAcceleration * dt
    if (LookAtSpeedCurr < LookAtSpeedMin) then LookAtSpeedCurr = LookAtSpeedMin end
    
    if (notMoving) then
      fakeRotation = currRotation
    end
                                                                                --write("Smaller")
  else
    LookAtSpeedCurr = LookAtSpeedCurr + LookAtAcceleration * dt
    if (LookAtSpeedCurr > LookAtSpeedMax) then LookAtSpeedCurr = LookAtSpeedMax end
    
    fakeRotation = fakeRotation + myDelta * LookAtSpeedCurr
                                                                                --write("larger")
  end
  UpdateLookAt(fakeRotation:x(), fakeRotation:y())
end

function UpdateLookAt (rotationX, rotationY)
  CAMERA:SetLookAt(lookAt:Rotate("x", rotationY):Rotate("y", rotationX):Normalized())
  
  -- Player rotation with camera
  if (haveplayer) then
    playerRot        = PLAYERTRANSFORM:GetWorldRotation()
    xRotation        = rotationX - playerRot:x()
    playerRot.y      = xRotation
    currentRotation  = playerRot
    currentRotationY = currentRotation:y()
    
    -- If player is in shoot mode
    if (currentPlayerMode) then
      PLAYERTRANSFORM:SetWorldRotation(playerRot)
    end
    
    if (vectorToPlayerScaleCurr > 1) then
      vectorToPlayerScaleCurr =  1
    elseif(vectorToPlayerScaleCurr < vectorToPlayerScaleMin) then
      vectorToPlayerScaleCurr =  vectorToPlayerScaleMin
    end
    
    -- position of camera
    worldPosition = PLAYERTRANSFORM:GetWorldPosition()
    offsetVector  = vectorToPlayer * vectorToPlayerScaleCurr
    
    if (useAcceleration) then
      offsetVector = offsetVector:Rotate("x", fakeRotation:y())
      offsetVector = offsetVector:Rotate("y", fakeRotation:x())
    else
      offsetVector = offsetVector:Rotate("x", rotation:y())
      offsetVector = offsetVector:Rotate("y", rotation:x())
    end
    
    CAMERATRANSFORM:SetWorldPosition(worldPosition + offsetVector)
  end
end

-- CAMERA SETTINGS FOR DIFFERENT CONTROLLER ====================================
function ChangeCameraSettings(check)
  systemControllerMode = (GetControllerInput() == 0)
  
  if (not check or currentControllerMode ~= systemControllerMode) then
    currentControllerMode = systemControllerMode
    if (currentControllerMode) then
      sensitivity.x      = kSensitivity:x()
      sensitivity.y      = kSensitivity:y()
      sensitivity.z      = kSensitivity:z()
      LookAtSpeedMin     = kLookAtMinSpeed
      LookAtSpeedMax     = kLookAtMaxSpeed
      LookAtAcceleration = kLookAtAcceleration
    else
      sensitivity.x      = cSensitivity:x()
      sensitivity.y      = cSensitivity:y()
      sensitivity.z      = cSensitivity:z()
      LookAtSpeedMin     = cLookAtMinSpeed
      LookAtSpeedMax     = cLookAtMaxSpeed
      LookAtAcceleration = cLookAtAcceleration
    end
    
    write("SET")
    LookAtSpeedCurr = 0.0
  end
end

-- CAMERA CHANGE MODE (BUILDING TRAPS / SHOOTING) ==============================
function ChangeCameraMode()
  currentPlayerMode = not currentPlayerMode
end

-- CAMERA CLIPPING =============================================================
function UpdateCameraClipping()
  playerPos = PLAYERTRANSFORM:GetWorldPosition()
  cameraPos = CAMERATRANSFORM:GetWorldPosition()
  
  -- raycast from player to camera
  direction = cameraPos - playerPos
  hitInfo   = RayCast(playerPos, direction, vectorToPlayerLength, "Player", "Bullet", "Coin", "Slime", "Trap")
  
  if (hitInfo:GameObject() ~= nil) then
    -- compute distance from obstacle

    vectorToObstacle = hitInfo:Point() - playerPos
    idealDistance    = vectorToObstacle:Length() - distanceFromObstacles
    
    -- set max camera distance from player
    vectorToPlayerScale = idealDistance / vectorToPlayerLength
  else
    vectorToPlayerScale = 1
  end
  
  -- Clamp scale
  if (vectorToPlayerScale < vectorToPlayerScaleMin) then
    vectorToPlayerScale = vectorToPlayerScaleMin
  elseif (vectorToPlayerScale > 1) then
    vectorToPlayerScale = 1
  end
end

-- PAUSE/RESUME ================================================================
function Pause()
 if (callOnce) then
    MyConstuctor()
  end
  
  disable = true
  safeLookAt = CAMERA:GetLookAt()
end

function Resume()
 if (callOnce) then
    MyConstuctor()
  end
  
  disable = false;
  CAMERA:SetLookAt(safeLookAt);
end

-- Controller ================================================================
function ChangeSensitivity()
  cSensitivityPP = PlayerPref_GetFloat("SensitivityController", "Settings_Player")
  kSensitivityPP = PlayerPref_GetFloat("SensitivityKeyBoard", "Settings_Player")
  
  newSenseK = kSensitivityBase * kSensitivityPP
  newSenseC = cSensitivityBase * cSensitivityPP
  
  cSensitivity.x = newSenseC:x()
  cSensitivity.y = newSenseC:y()
  cSensitivity.z = newSenseC:z()
  
  kSensitivity.x = newSenseK:x()
  kSensitivity.y = newSenseK:y()
  kSensitivity.z = newSenseK:z()
  

  if (currentControllerMode) then
    sensitivity.x      = kSensitivity:x()
    sensitivity.y      = kSensitivity:y()
    sensitivity.z      = kSensitivity:z()
    LookAtSpeedMin     = kLookAtMinSpeed
    LookAtSpeedMax     = kLookAtMaxSpeed
    LookAtAcceleration = kLookAtAcceleration
  else
    sensitivity.x      = cSensitivity:x()
    sensitivity.y      = cSensitivity:y()
    sensitivity.z      = cSensitivity:z()
    LookAtSpeedMin     = cLookAtMinSpeed
    LookAtSpeedMax     = cLookAtMaxSpeed
    LookAtAcceleration = cLookAtAcceleration
  end
  
  write("==========")
  write(cSensitivity)
  write(kSensitivity)
end
