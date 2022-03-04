--settings

local isPaused = false
local state = 0

local resumeObj = nil
local selectLevel = nil
local setting = nil 
local c = nil

disappear = false
function Constructor()
  c = owner:GetComponent("MeshRenderer")
end

function OnUpdate(dt)
  newColor = c:GetColor()
  speed = UnscaledDT()*2
  if(  disappear == true ) then
    newColor.a = newColor:a() - speed
  else
    newColor.a = newColor:a() + speed
  end
  
  if(  newColor:a()  > 1.0 ) then
    newColor.a = 1.0
  elseif(  newColor:a()  < 0.0 ) then
    newColor.a = 0.0
  end
  c:SetColor(newColor)
end


function Disappear()
  disappear = true
end


function DisappearInstant()
  disappear = true
  nextColor = Color(0,0,0,0)
  c:SetColor(nextColor)
end


function Appear()
  disappear = false
end

function AppearInstant()
  disappear = false
  nextColor = Color(0,0,0,1)
  c:SetColor(nextColor)
end