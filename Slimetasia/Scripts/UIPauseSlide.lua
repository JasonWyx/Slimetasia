--settings
local GO = nil
local trans = nil 
local cam = nil
slideIn = 0
currentX = 0
function Constructor()
  GO = owner
  trans = GO:GetComponent("Transform")
  camObj = owner:GetLayer():GetObject("MainCamera")
  if (camObj ~= nil) then
    cam = camObj:GetComponent("Camera")
  end
end

function OnUpdate(dt)
  if(slideIn == 0) then
    currentX = currentX - dt
    if(currentX < -1) then
      currentX = -1
    end
  else
    currentX = currentX + dt
    if(currentX > 0) then
      currentX = 0
    end
  end
  UpdatePosition(currentX, 0.0, 0.0, 0.5)
end


function UpdatePosition(screenPosX,screenPosY,pivotX,pivotY)
  vertSize = 10
  --Get the new position as thought it's 0.5 at pivot
  y = (screenPosY*vertSize)-(vertSize/2)
  ratio = cam:GetViewporSize():x()/cam:GetViewporSize():y()
  horiSize = vertSize*ratio
  x = (screenPosX*horiSize)-(horiSize/2)
  v = Vector3(x, y, trans:GetWorldPosition():z())

  --offset the new position base on the pivot
  pivotOffset = Vector3( pivotX - 0.5,  
                         pivotY - 0.5, 
                         0.0)
  x =  pivotOffset:x() * trans:GetWorldScale():x()
  y =  pivotOffset:y() * trans:GetWorldScale():z()
  z =  pivotOffset:z() * trans:GetWorldScale():y()
  pivotOffset = Vector3( x, y, z)
  v = v + pivotOffset
  trans:SetWorldPosition(v)
end