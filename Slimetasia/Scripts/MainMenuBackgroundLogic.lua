--settings

local isPaused = false
local state = 0

local resumeObj = nil
local selectLevel = nil
local setting = nil 
local c = nil
nextColor = Color(1,1,1,1)
function Constructor()
  c = owner:GetComponent("MeshRenderer")
end

function OnUpdate(dt)
  prevColor = c:GetColor()
  newColor = Color(0,0,0,1)
  speed = dt * 5.0
  newColor.r = prevColor:r() + (nextColor:r() - prevColor:r())*speed
  newColor.g = prevColor:g() + (nextColor:g() - prevColor:g())*speed
  newColor.b = prevColor:b() + (nextColor:b() - prevColor:b())*speed
  c:SetColor(newColor)
end