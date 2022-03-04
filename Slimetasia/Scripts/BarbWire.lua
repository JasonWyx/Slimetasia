HP = 5
local textRenderer = nil
local hpString = ""

local lifeCountDownTimer = 
{
  timer = 3.0
}

local tmp = 1
local testArray = {}
local it = 1

local b = true

function Constructor()
textRenderer = owner:GetComponent("TextRenderer")
end

function OnUpdate(dt)
for i = 1, #testArray, 1 do
  if(testArray[i] ~= nil) then
      timeLeft = testArray[i].timer - dt
      testArray[i].timer = timeLeft
      if(timeLeft < 0) then
        write("BarbWire Taking Damage")
        testArray[i] = nil
        TakeDamage()
        if(HP < 1) then
          write("BarbWire dies")
          CurrentLayer():GetObject("Player"):GetLuaScript("PlayerController.lua"):CallFunction("ExitBarbedWire")
          owner:Destroy()
        end
      end
  end
end

hpString = HP .. "\n"
textRenderer:SetText(hpString)
end

function TakeDamage()
  HP = HP - 1
end

function StartTimer()
  testArray[it] = lifeCountDownTimer
  it = it + 1
  write("Start Countdown to take damage")
end