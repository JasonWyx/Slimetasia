executeOrder66    = false
completed         = false
landedOnHead      = false
reachHighestPoint = false

local encik = nil
local encikTransform = nil

local scaleMin = Vector3(0.08, 0.1, 0.08)
local scaleMax = Vector3(0.1 , 0.1, 0.1)
local scaleDur = 1
local dir      = -1

local jumpHeight = 15

local startingPosition      = Vector3()
local durationToReachTarget = 0.5

local ownerTransform = nil

local callOnce = true

local vector     = Vector3()
local vector2    = Vector3()
local vector3    = Vector3()
local timer      = 0
local scaleTimer = 0
local state      = -1

function Constructor()
end

function MyConstructor()
  if (callOnce) then
    callOnce = false
    
    -- GetTransform
    ownerTransform  = owner:GetComponent("Transform")
    
    -- Set variables
    startingPosition = ownerTransform:GetWorldPosition()
    
    -- Hide slime
    ownerTransform:SetWorldPosition(Vector3(0, -10, 0))
    
    -- Get encik
    encik = CurrentLayer():GetObject("Encik")
    encikTransform = encik:GetComponent("Transform")
  end
end

function OnUpdate(dt)
  MyConstructor()
  
  -- Growing and shrinking
  Grow(dt)
  Shrink(dt)
  
  -- Execute order
  if (executeOrder66) then
    -- Compute position to stop
	if (state == -1) then
	  timer = timer + dt
      currPos = Vector3Lerp(Vector3(27, 0.5, -68), Vector3(36.5, 0.5, -66), timer/ 1)
      ownerTransform:SetWorldPosition(currPos)
	  
	  if (timer >= 1) then
        state  = state + 1
        timer  = 0
        timer2 = 0
      end
	  
    elseif (state == 0) then
      encikPos = encikTransform:GetWorldPosition()
      myPos    = ownerTransform:GetWorldPosition()
      distance = encikPos - myPos
      vector   = myPos + distance * 0.7
      vector2  = myPos
      
      state    = state + 1
      timer    = 0
      timer2   = 0
    
    -- Move to encik
    elseif (state == 1) then
      timer = timer + dt
      
      currPos = Vector3Lerp(vector2, vector, timer/ durationToReachTarget)
      ownerTransform:SetWorldPosition(currPos)
      
      if (timer >= durationToReachTarget) then
        state  = state + 1
        timer  = 0
        timer2 = 0
        
        completed = true
      end
    
    -- Getting ready to jump
    elseif (state == 2) then
      timer = timer + dt
      
      if (timer >= 1) then
        state  = state + 1
        timer  = 0
        timer2 = 0
        
        encikPos    = encikTransform:GetWorldPosition()
        myPos       = ownerTransform:GetWorldPosition()
        distance    = encikPos - myPos
        vector      = myPos + distance * 0.5
        vector.y    = vector:y() + jumpHeight
        vector2     = myPos
        reachHighestPoint = true
      end
    
    -- jump
    elseif (state == 3) then
      timer  = timer  + dt
      
      currPos = Vector3Lerp(vector2, vector, timer/ 1.5)
      ownerTransform:SetWorldPosition(currPos)
      
      if (timer >= 1.5) then
        state  = state + 1
        timer  = 0
        
        encikPos    = encikTransform:GetWorldPosition()
        myPos       = ownerTransform:GetWorldPosition()
        vector      = encikPos
        vector.y    = vector:y() + 1.5
        vector2     = myPos
        vector3     = ownerTransform:GetWorldRotation()
      end
    
    -- Land on encik head
    elseif (state == 4) then
      timer  = timer  + dt
      
      currPos = Vector3Lerp(vector2, vector, timer / 0.8)
      ownerTransform:SetWorldPosition(currPos)
      
      currRot = Vector3Lerp(vector3, Vector3(-90, 90, 0), timer / 0.8)
      ownerTransform:SetWorldRotation(currRot)
      
      -- ChangeState
      if (timer >= 0.8) then
        state = state + 1
        
        landedOnHead = true
      end
    end
  end
end

function KillEncik()
  timer = 0;
  executeOrder66 = true;
end

-------------------------------------------------------------------------------
function Grow(dt)
  if (dir == 1) then
    scaleTimer = scaleTimer + dt
    currScale = Vector3Lerp(scaleMin, scaleMax, scaleTimer/0.75)
    ownerTransform:SetWorldScale(currScale)
    if (scaleTimer >= 0.75) then
      dir = -1
      scaleTimer = 0
    end
  end
end

function Shrink(dt) 
  if (dir == -1) then
    scaleTimer = scaleTimer + dt
    currScale = Vector3Lerp(scaleMax, scaleMin, scaleTimer/0.75)
    ownerTransform:SetWorldScale(currScale)
    if (scaleTimer >= 0.75) then
      dir = 1
      scaleTimer = 0
    end
  end
end

-------------------------------------------------------------------------------