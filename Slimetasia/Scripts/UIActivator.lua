
GO = nil
trans = nil
meshR = nil

timer = 0.0
maxTime = 1.0

appeared = false
transition = false
function Constructor()
  GO = owner 
  trans = GO:GetComponent("Transform")
  meshR =  GO:GetComponent("MeshRenderer")  
  originalZ = trans:GetWorldPosition():z()
end

function OnUpdate(dt)
  c = meshR:GetColor()
  if(appeared == true) then
    c = Color(1.0,1.0,1.0, 1.0)
  else
    c = Color(1.0,1.0,1.0, 0.0)
  end
  meshR:SetColor(c)
end

function Activate()
if(appeared == true) then
 return
end
transition = true
appeared = true
end

function Deactivate()
if(appeared == false) then
 return
end
transition = true
appeared = false
end

function DeactivateImmediate()
if(appeared == false) then
 return
end
transition = true
appeared = false
timer = MaxTime
end
