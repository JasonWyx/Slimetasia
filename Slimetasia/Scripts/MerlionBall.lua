-- VARIABLES ===================================================================
-- misc
local callOnce  = true
local ballState = 1 -- 1 (Floating) / 2 (going away) / 3 (Waiting) / 4 (coming back)
local sfx_BallPickup = "MerlionBallsPickup"
-- Sequence(Going to random position on map)
local sequenceState                = 1
local minDist                      = 20
local flying_Height                = 5
local flying_DurationToReachHeight = 2
local flying_DurationToReachPos    = 3
local flying_DurationToFallToPos   = 1
local resting_Pos                  = Vector3()

-- Components
local owner_Transform  = nil
local script_CoreLogic = nil
local core_Transform   = nil

-- ball
local index = 0

local merlion_FloatingRadius = 10
local merlion_FloatingHeight = 9
local merlion_FloatingRaise  = 1
local merlion_DirectionRaise = 1
local merlion_DurationRaise  = 1
local merlion_Timer          = 0
local merlion_CurrentAngle   = 0
local merlion_FloatingSpeed  = 10
local merlion_CurrRaise      = 0
local merlion_BallPosition   = Vector3()

local moving_startPos = Vector3()
local moving_endPos   = Vector3()
local moving_duration = 0
local moving_timer    = 0


-- FUNCTIONS ===================================================================
function Constructor()
  owner_Transform = owner:GetComponent("Transform")
end

function MyConstructor()
  if (callOnce) then
    callOnce =  false
    
    -- Get core script
    Core_GO = CurrentLayer():GetObject("Core")
    script_CoreLogic = Core_GO:GetLuaScript("CoreLogic.lua")
    core_Transform = Core_GO:GetComponent("Transform")
    
    -- Get the index of the ball
    index = script_CoreLogic:GetVariable("BallIndex")
    script_CoreLogic:CallFunction("UpdateBallIndex")
    
    -- Set variables
    if (IsEven(index)) then
      merlion_DirectionRaise = -1
    end
    coreHealth = script_CoreLogic:GetVariable("BaseCoreHealth")
    anglePerHealth = 360 / coreHealth
    merlion_CurrentAngle = (index - 1) * anglePerHealth
    merlionBaseHeight = core_Transform:GetWorldPosition():y()
    flying_Height = merlionBaseHeight + merlion_FloatingHeight + merlion_FloatingRaise + flying_Height
  end
end

function OnUpdate(dt)
  -- Functions
  MyConstructor()
  UpdateMerlionPosition(dt)
  
  -- Floating around the merlion
      if (ballState == 1) then owner_Transform:SetWorldPosition(merlion_BallPosition)
  -- Flying to destination
  elseif (ballState == 2) then UpdateRandomPosition(dt)
  -- waiting for player to pickup
  -- elseif (ballState == 3) then 
  -- Flying to merlion
  elseif (ballState == 4) then UpdateMoveToMerlion(dt)
  end
end

function OnCollisionEnter(go)
  if (ballState == 3 and go:Tag() == "Player") then
    AudioSystem_PlayAudioAtLocation(sfx_BallPickup, merlion_BallPosition)
    MoveToMerlion()
  end
end

-- MERLION =====================================================================
function UpdateMerlionPosition(dt)
  merlion_Timer = merlion_Timer + dt
  
  -- Compute raise
  if (merlion_DirectionRaise == 1) then
    merlion_CurrRaise = Lerp(0, merlion_FloatingRaise, merlion_Timer / merlion_DurationRaise)
  else
    merlion_CurrRaise = Lerp(merlion_FloatingRaise, 0, merlion_Timer / merlion_DurationRaise)
  end
  
  if (merlion_Timer >= merlion_DurationRaise) then
    merlion_DirectionRaise = -merlion_DirectionRaise
    merlion_Timer = 0
  end
  
  -- Compute position
  merlion_CurrentAngle = merlion_CurrentAngle + merlion_FloatingSpeed * dt
  while merlion_CurrentAngle >= 360 do
    merlion_CurrentAngle = merlion_CurrentAngle - 360
  end
  vector = Vector3(merlion_FloatingRadius, 0, 0)
  vector = VectorRotate(vector, "y", merlion_CurrentAngle)
  
  corPos = core_Transform:GetWorldPosition()
  vector = corPos + vector
  
  vector.y = merlion_FloatingHeight + merlion_CurrRaise
  merlion_BallPosition = vector
end

-- MOVE TO RANDOM MAP POSITION =================================================
function MoveToRandomPosition()
  -- Set sequence and state
  ballState = 2
  sequenceState = 1
  
  -- Find random position that the merlion is going to land
  corePos = core_Transform:GetWorldPosition()
  resting_Pos = AISystem_GetAvilableGridOutsidePos(corePos, minDist)
  write("ball ball moving to : ", resting_Pos)
end

function UpdateRandomPosition(dt)
      if (sequenceState == 1) then  -- Set position
                                    moving_startPos = owner_Transform:GetWorldPosition()
                                    moving_endPos   = owner_Transform:GetWorldPosition()
                                    moving_endPos.y = flying_Height
                                    moving_timer    = 0
                                    moving_duration = flying_DurationToReachHeight
                                    sequenceState   = sequenceState + 1
                                    
  elseif (sequenceState == 2) then  -- Move to flying level
                                    if (MoveToDestination(dt)) then
                                      moving_startPos = owner_Transform:GetWorldPosition()
                                      moving_endPos   = owner_Transform:GetWorldPosition()
                                      moving_endPos.x = resting_Pos:x()
                                      moving_endPos.z = resting_Pos:z()
                                      moving_timer    = 0
                                      moving_duration = flying_DurationToReachPos
                                      sequenceState   = sequenceState + 1
                                    end
                                    
  elseif (sequenceState == 3) then  -- Move to resting position
                                    if (MoveToDestination(dt)) then
                                      moving_startPos = owner_Transform:GetWorldPosition()
                                      moving_endPos   = resting_Pos
                                      moving_timer    = 0
                                      moving_duration = flying_DurationToFallToPos
                                      sequenceState   = sequenceState + 1
                                    end
                                    
  elseif (sequenceState == 4) then  -- Move to resting position
                                    if (MoveToDestination(dt)) then
                                      ballState = 3
                                    end
  end
end

-- MOVE TO MERLION =============================================================
function MoveToMerlion()
  -- Set sequence and state
  ballState = 4
  sequenceState = 1
end

function UpdateMoveToMerlion(dt)
      if (sequenceState == 1) then  -- Set position
                                    moving_startPos = owner_Transform:GetWorldPosition()
                                    moving_endPos   = owner_Transform:GetWorldPosition()
                                    moving_endPos.y = flying_Height
                                    moving_timer    = 0
                                    moving_duration = flying_DurationToReachHeight
                                    sequenceState   = sequenceState + 1
                                    
  elseif (sequenceState == 2) then   -- Move to flying level
                                    if (MoveToDestination(dt)) then
                                      moving_startPos = owner_Transform:GetWorldPosition()
                                      moving_endPos   = core_Transform:GetWorldPosition()
                                      moving_endPos.y = flying_Height
                                      moving_timer    = 0
                                      moving_duration = flying_DurationToReachPos
                                      sequenceState   = sequenceState + 1
                                    end
                                    
  elseif (sequenceState == 3) then  -- Move back to merlion
                                    if (MoveToDestination(dt)) then
                                      moving_startPos = owner_Transform:GetWorldPosition()
                                      moving_endPos   = merlion_BallPosition
                                      moving_timer    = 0
                                      moving_duration = flying_DurationToFallToPos
                                      sequenceState   = sequenceState + 1
                                    end
                                    
  elseif (sequenceState == 4) then  -- Move to merlion ball position
                                    if (MoveToDestination(dt)) then
                                      ballState = 1
                                      script_CoreLogic:SetVariable("BallIndex", index)
                                      script_CoreLogic:CallFunction("AddHealth")
                                    end
  end
end

-- MOVING ======================================================================
function MoveToDestination(dt)
  moving_timer = moving_timer + dt
  currPos = Vector3Lerp(moving_startPos, moving_endPos, moving_timer / moving_duration)
  owner_Transform:SetWorldPosition(currPos)
  if (moving_timer >= moving_duration) then
    return true
  end
  
  return false
end