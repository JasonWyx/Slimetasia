--settings
local GO = nil
local TRANS = nil 
local cam = nil

disappear = false
function Constructor()
  GO = owner
  TRANS = GO:GetComponent("Transform")
  camObj = owner:GetLayer():GetObject("MainCamera")
  if (camObj ~= nil) then
    cam = camObj:GetComponent("Camera")
  end
end

function OnUpdate(dt)

  if(disappear == true) then
    --Get the position at the top of the screen
    screenPosY = 2.0
  else
    --Get the position at the center of the screen
    screenPosY = 0.5
  end


  vertSize = 10
  y = (screenPosY*vertSize)-(vertSize/2)
  if(disappear == true) then
    y = y + TRANS:GetWorldScale():z()
  end
  ratio = cam:GetViewporSize():x()/cam:GetViewporSize():y()
  horiSize = vertSize*ratio
  x = (0.5*horiSize)-(horiSize/2) 
  direction = Vector3(x, y, TRANS:GetWorldPosition():z()) - TRANS:GetWorldPosition()
  step = direction:Normalized() * dt * 50
  TRANS:SetWorldPosition(TRANS:GetWorldPosition()+step) 
end


function Disappear()
  disappear = true
end


function DisappearInstant()
  disappear = true
end


function Appear()
  disappear = false
end

function AppearInstant()
  disappear = false
end