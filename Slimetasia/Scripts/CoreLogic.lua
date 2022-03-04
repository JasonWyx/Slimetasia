-- VARIABLES ===================================================================
-- Health
CoreHealth     = 20
BaseCoreHealth = 20

-- Damage
kamikaze_Damage     = 5
truekamikaze_Damage = 5 -- WHATS THIS USE FOR (???)--KJJ : for tourguide buff LOR

-- Balls
BallIndex = 1 -- Used in MerlionBall
local balls         = {}
local balls_Active  = {}
local sphere_Prefab = "MerlionBalls"

-- Game logic
local GameLogicScript = nil

-- Vibration
local vibrateControl  = false
local vibrateTimer    = 0
local vibrateDuration = 0.2

-- Misc
local CallOnce = true

--Audio sfx
damage_sfx = nil

-- FUNCTIONS ===================================================================
function Constructor()

end

function MyConstructor()
  if (CallOnce) then
    CallOnce =  false
    
    -- Get game logic
    GameLogic_GO = CurrentLayer():GetObject("GameLogic")
    GameLogicScript = GameLogic_GO:GetLuaScript("GameLogic_VerticalSlice.lua")
    
    -- Create balls
    for i = 1, BaseCoreHealth do
      newObj = CreatePrefab(sphere_Prefab)
      balls[i] = newObj:GetLuaScript("MerlionBall.lua")
      balls_Active[i] = true
    end
	damage_sfx = 
	{
	  "MerlionHit1",
	  "MerlionHit2",
	  "MerlionHit3",
	  "MerlionHit4",
	  "MerlionHit5",
	  "MerlionHit6"
	}
  end
end

function OnUpdate(dt)
  -- Functions
  MyConstructor()
  
  -- Quick death
  if(IsKeyPressed(KEY_L)) then
    DealActualDamage(1)
  end
  
  -- Vibration
  if (vibrateControl) then
    if (vibrateTimer > 0) then
      ControllerVibrateBoth(1)
      vibrateTimer = vibrateTimer - dt
    else
      ControllerVibrateBoth(0)
      vibrateControl = false
    end
  end
  
end
-- Add health ==================================================================
function AddHealth()
  --write(BallIndex, " Have returned ")
  balls_Active[ToInt(BallIndex)] = true
  
  count = 0
    for j = 1, #balls_Active do
      if (balls_Active[j]) then
        count = count + 1
      end
  end
  
  if (CoreHealth < BaseCoreHealth) then
    CoreHealth = CoreHealth + 1
  end
  
  write("Merlion health ", CoreHealth)
end

-- Deal Core Damage ============================================================
function DealDamage()
  if(CoreHealth > 0) then
    DealActualDamage(1)
  end
end

function KamiKazeDealDamage()
  if(CoreHealth > 0) then
    DealActualDamage(kamikaze_Damage)
  end
end

function DealActualDamage(damage)
  -- Make balls go away
  ballsToGoAway = damage
  if (damage > CoreHealth) then
    ballsToGoAway = CoreHealth
  end
  
  for i = 1, ballsToGoAway do
    -- Get all free balls
    availableBallIndex = {}
    insertIndex = 1
    for j = 1, #balls_Active do
      if (balls_Active[j]) then
        availableBallIndex[insertIndex] = j
        insertIndex = insertIndex + 1
      end
    end

    -- Pick a random ball to go away
    ballSelectedIndex = RandomRangeInt(1, #availableBallIndex)
    chosenIndex = availableBallIndex[ballSelectedIndex]
    
    --write(#availableBallIndex, " Balls available ")
    --write(ballSelectedIndex  , " Selected ")
    --write(chosenIndex        , " Real index ")
    
    balls_Active[chosenIndex] = false
    script = balls[chosenIndex]:CallFunction("MoveToRandomPosition")
  end
  -- Deal damage
  CoreHealth = CoreHealth - damage
  AudioSystem_PlayAudioAtLocation(damage_sfx[RandomRangeInt(1, #damage_sfx)], owner:GetComponent("Transform"):GetWorldPosition())
  CheckCoreHealth()
end

function CheckCoreHealth()
  -- Debug
  write("Merlion health :", CoreHealth)
  
  -- Controller vibration
  vibrateControl = true
  vibrateTimer = vibrateDuration
  
  -- Call game over if required
  if (CoreHealth <= 0) then
    ControllerVibrateBoth(0)
    GameLogicScript:CallFunction("GameOver")
  end
end

-- Balls =======================================================================
function UpdateBallIndex()
  BallIndex = BallIndex + 1
end