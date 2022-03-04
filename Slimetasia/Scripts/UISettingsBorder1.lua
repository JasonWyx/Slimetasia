--settings

local camObj = nil
local cam = nil


GO = nil
rb = nil
trans = nil
speed = 1.0
totalDt = 0.0
xAnchor = 1.0
xPivot = 0.0
anchorPosition = Vector3()
function Constructor()
  GO = owner
  trans = GO:GetComponent("Transform")
  
  camObj = GO:GetLayer():GetObject("MainCamera")
  if(camObj ~= nil) then
    cam = camObj:GetComponent("Camera")
  end
  
  if(GO:Name() == "Border1") then
    speed = 3
    xPivot = 0.0
    xAnchor = 0.97
  elseif(GO:Name() == "Border2") then
    speed = 2
    xPivot = 0.0
    xAnchor = 0.925
  elseif(GO:Name() == "Border3") then
    speed = 3
    xPivot = 1.0
    xAnchor = 0.03
  elseif(GO:Name() == "Border4") then
    speed = 2
    xPivot = 1.0
    xAnchor = 0.075
  end
end

function OnUpdate(dt)
  if(camObj:GetActive() == false) then
    totalDt = totalDt - UnscaledDT() * speed
    if(totalDt < 0 )then totalDt = 0.0 end
  else
    totalDt = totalDt + UnscaledDT() * speed
    if(totalDt > 1.0)then totalDt = 1.0 end
  end
    s = trans:GetWorldScale() 
    vertSize = 10
    s.x = 0.1
    s.z = vertSize
    trans:SetWorldScale(s)
    CalculateAnchoredPosition(xAnchor, totalDt, xPivot, 0.0, trans)
    trans:SetWorldPosition(anchorPosition)
end


function CalculateAnchoredPosition(screenPosX,screenPosY,pivotX,pivotY, t)
  vertSize = 10
  
  --Get the new position as thought it's 0.5 at pivot
  y = (screenPosY*vertSize)-(vertSize/2)
  ratio = cam:GetViewporSize():x()/cam:GetViewporSize():y()
  horiSize = vertSize*ratio
  x = (screenPosX*horiSize)-(horiSize/2)
  v = Vector3(x, y, t:GetWorldPosition():z())

  --offset the new position base on the pivot
  pivotOffset = Vector3( pivotX - 0.5,  
                         pivotY - 0.5, 
                         0.0)
  x =  pivotOffset:x() * t:GetWorldScale():x()
  y =  pivotOffset:y() * t:GetWorldScale():z()
  z =  0-- pivotOffset:z() * t:GetWorldScale():z()
  pivotOffset = Vector3( x, y, z)
  v = v + pivotOffset
  anchorPosition =  v
end

