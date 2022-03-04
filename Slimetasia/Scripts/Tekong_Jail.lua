-- VARIABLES ===================================================================
local playerTransform = nil
local ownerTransform = nil
local jailFenceTransform = nil
local slimeTransform = nil
local originalRotation = Vector3()
local owner_audioEmitter = nil

local callOnce = true
local minDist = 8
local minHeight = 0.2
local maxHeight = 0.5

local playerInRadius = false

local start = Vector3()
local dest  = Vector3()
local isJumping = false
local goingUp   = false
local timer     = 0

local opening = false
local scale = 0

local isOpened = false

local audio_SlimeDefaultNoise = 
{
  "Zombie07_Growl02",
  "Zombie07_Growl03",
  "Zombie07_Growl04",
  "Zombie07_Growl05",
  "Zombie07_Growl06",
  "Zombie07_Growl07",
  "Zombie07_Growl08",
  "Zombie07_Growl09",
  "Zombie07_Growl10",
  "Zombie07_Growl11",
  "Zombie07_Growl12",
  "Zombie07_Growl13"
}
local audio_timer = 3.0
local track_selector = 1

-- FUNCTIONS ===================================================================
function Start()
end

function OnUpdate(dt)
  ConstructOnce()
  
  if (not isOpened) then
    PlaySFX(dt)
    
    playerPos = playerTransform:GetWorldPosition()
    myPos     = ownerTransform:GetWorldPosition()
    distVec   = playerPos - myPos
    vec       = distVec:SquareLength()
    playerInRadius2 = vec <= minDist
    
    if (playerInRadius2) then
        owner_audioEmitter:SetVolume(1)
      -- Look at player
      vectorToTarget   = playerTransform:GetWorldPosition() - slimeTransform:GetWorldPosition()
      angle = VectorAngleWorldAxis(-vectorToTarget)
      rotationVector = slimeTransform:GetWorldRotation()
      rotationVector.y = angle
      slimeTransform:SetWorldRotation(rotationVector)
      
      if (not isJumping) then
        JumpUp()
      end
    else
        owner_audioEmitter:SetVolume(0.2)
      if (playerInRadius) then
      -- Look away from player
      slimeTransform:SetWorldRotation(originalRotation)
      end
    end
    
    playerInRadius = playerInRadius2
    
    
    if (isJumping) then
      UpdateJump(dt)
    end
    
    if (opening) then
      UpdateOpen(dt)
    end
  end
  
  if (IsKeyDown(KEY_L)) then
    Open()
  end
end

function ConstructOnce()
  if (callOnce) then
    callOnce = false
    
    playerTransform = CurrentLayer():GetObject("Player"):GetComponent("Transform")
    ownerTransform = owner:GetComponent("Transform")
    jailFence = owner:GetChild("BoxDoor")
    jailFenceTransform = jailFence:GetComponent("Transform")
    slime = owner:GetChild("Slime")
    slimeTransform = slime:GetComponent("Transform")
    originalRotation = slimeTransform:GetWorldRotation()
    
    owner_audioEmitter = slime:GetComponent("AudioEmitter")
    
    minDist = minDist * minDist
  end
end

function JumpUp ()
  isJumping = true
  goingUp   = true
  height    = RandomRange(minHeight, maxHeight)
  start     = ownerTransform:GetWorldPosition()
  dest      = ownerTransform:GetWorldPosition()
  dest.y    = height + currPos:y()
  timer     = 0
end

function UpdateJump(dt)
  timer = timer + dt
  if (goingUp) then
    currPos = Vector3Lerp(ownerTransform:GetWorldPosition(), dest, timer / 0.2)
    ownerTransform:SetWorldPosition(currPos)
    if (timer >= 0.2) then
      goingUp = false
      timer = 0
    end
  else
    currPos = Vector3Lerp(dest, start, timer / 0.1)
    ownerTransform:SetWorldPosition(currPos)
    
    if (timer >= 0.1) then
      isJumping = false
    end
  end
end

function Open()
  opening = true
  scale = jailFenceTransform:GetWorldScale():y()
  timer = 0
end

function UpdateOpen(dt)
  timer = timer + dt
  currScale = jailFenceTransform:GetWorldScale()
  currScale.y = Lerp(scale, 0, timer / 1)
  jailFenceTransform:SetWorldScale(currScale)
  
  if (timer >= 1) then
    opening = false
    isOpened = true
  end
end

function PlaySFX(dt)
  audio_timer = audio_timer - dt
  if(audio_timer < 0) then
    owner_audioEmitter:SetAndPlayAudioClip(audio_SlimeDefaultNoise[track_selector])
    audio_timer = 3.0
    track_selector =  track_selector + 1
    if(track_selector > 6) then 
      track_selector = 4
    end
  end
end