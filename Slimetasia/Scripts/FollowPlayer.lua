followTarget    = false
lookAtTarget    = true
targetTransform = CurrentLayer():GetObject("Player"):GetComponent("Transform")

local speed          = 12
local ownerTransform = nil
local stoppingDist   = 5
local callOnce       = true
local Y_Height       = 0

local isJumping = false
local goingUp   = false
local timer     = 0

local ownerAnimator    = nil
local currentAnimation = ""

function Constructor()
end

function MyConstructor()
  if (callOnce) then
    callOnce = false
    
    -- GetTransform
    ownerTransform  = owner:GetComponent("Transform")
    
    -- GetAnimator
    ownerAnimator = owner:GetComponent("MeshAnimator")
    
    -- Set variables
    stoppingDist = stoppingDist * stoppingDist
    Y_Height     = ownerTransform:GetWorldPosition():y()
  end
end

function OnUpdate(dt)
  -- Constructor
  MyConstructor()
  
  -- Position
  targetPos      = targetTransform:GetWorldPosition()
  ownerPos       = ownerTransform:GetWorldPosition()
  vectorToPlayer = targetPos - ownerPos
  
  -- Following target
  if (followTarget) then
    squaredDist    = vectorToPlayer:SquareLength()
    if (squaredDist >= stoppingDist) then
      direction  = vectorToPlayer:Normalized()
      finalDes   = ownerPos + direction * speed * dt
      finalDes.y = Y_Height
      ownerTransform:SetWorldPosition(finalDes)
      PlayAnimation("Jog")
      timer = 0.5
    else
      timer = timer - dt
      if (timer < 0) then
        PlayAnimation("Idle")
      end
    end
  end
  
  -- Look at target
  if (lookAtTarget) then
      vectorToPlayer.y = 0
      angle = VectorAngleWorldAxis(vectorToPlayer)
      rotationVector = ownerTransform:GetWorldRotation()
      rotationVector.y = angle
      ownerTransform:SetWorldRotation(rotationVector)
  end
  
  -- Jumping
  if (isJumping) then
    UpdateJump(dt)
  end
end

function Jump ()
  if (not isJumping) then
    isJumping = true
    goingUp   = true
    timer     = 0
  end
end

function UpdateJump (dt)
  timer = timer + dt
  if (goingUp) then
    duration = 0.2
    currY = Lerp(Y_Height, Y_Height + 0.5, timer / duration)
    pos   = ownerTransform:GetWorldPosition()
    pos.y = currY
    ownerTransform:SetWorldPosition(pos)
    if (timer >= duration) then
      goingUp = false
      timer = 0
    end
  else
    duration = 0.1
    currY = Lerp(Y_Height + 0.5, Y_Height, timer / duration)
    pos   = ownerTransform:GetWorldPosition()
    pos.y = currY
    ownerTransform:SetWorldPosition(pos)
    if (timer >= duration) then
      isJumping = false
    end
  end
end

function PlayAnimation (animation)
  if (ownerAnimator ~= nil) then
    if (currentAnimation ~= animation) then
      currentAnimation = animation
      ownerAnimator:Play(animation)
    end
  end
end