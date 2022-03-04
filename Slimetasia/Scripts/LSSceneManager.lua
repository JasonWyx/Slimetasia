--settings

totalDt = 0

firstUpdate = true
entering = true
transitting = false
levelName = ""
levelPref = ""
cam = nil

function Constructor()
  GO = owner
  transit = GetLayer("Transition"):GetObject("Transition"):GetLuaScript("UIFade.lua")
  camObj = owner:GetLayer():GetObject("MainCamera")
  if (camObj ~= nil) then
    cam = camObj:GetComponent("Camera")
  end
end

function OnUpdate(dt)

if(firstUpdate) then
  firstUpdate = false
  return
end

if(entering) then
  transit:CallFunction("Disappear")
end

if(transitting) then
  totalDt = totalDt + UnscaledDT()  
  transit:CallFunction("Appear")
  if(totalDt > 3) then
    PlayerPref_SetString("CurrentLevel",levelPref)
    SceneLoad(levelName)
  end
  return
end

end

function TransitToNextLevel()
  transitting = true
  entering = false
end
