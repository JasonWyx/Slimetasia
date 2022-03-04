--settings
rotateSpeed = 50.0
rotateLeft = false
activateRotate = true;

GO = nil
rb = nil
trans = nil

local increaseSpeedLeft = false
local increaseSpeedRight = false
local speedUpTimeOver = 2.0
local speedUpTime = 2.0

function Constructor()
  GO = owner
  trans = GO:GetComponent("Transform")  
  rb = GO:GetComponent("RigidBody")
  camObj = CurrentLayer():GetObject("Camera")
  
  if(GO:Name() == "UI_Bg") then
    rotateLeft = true
  end
  if(GO:Name() == "UI_Bg3") then
    rotateSpeed = 100.0
    rotateLeft = true
    speedUpTimeOver = 0.5
    increaseSpeedRight = true
  end
  if(GO:Name() == "UI_Bg4") then
    rotateSpeed = 100.0
    rotateLeft = false
    increaseSpeedLeft = true
  end
end

function OnUpdate(dt)
  
  if (increaseSpeedLeft == true) then
    if (ControllerPress("Shoot")) then
      rotateSpeed = 800.0
      speedUpTime = 0.0
    end
    if(speedUpTime >= speedUpTimeOver) then
      rotateSpeed = 100.0
    else
      speedUpTime = speedUpTime + dt
    end
  end
  
  if (increaseSpeedRight == true) then
    if (ControllerPress("SelectPrev")) then
      rotateSpeed = 800.0
      rotateLeft = false
      speedUpTime = 0.0
    end
    if (ControllerPress("SelectNext")) then
      rotateSpeed = 800.0
      rotateLeft = true
      speedUpTime = 0.0
    end
    if(speedUpTime >= speedUpTimeOver) then
      rotateSpeed = 100.0
      rotateLeft = true
    else
      speedUpTime = speedUpTime + dt
    end
  end
  if(activateRotate == true) then
    rotation = trans:GetWorldRotation()
    if(rotateLeft == true) then
      rotation.z =  rotation:z() - rotateSpeed*dt
    else
      rotation.z =  rotation:z() + rotateSpeed*dt
    end
    
    if(rotation:z() > 360) then
      rotation.z = rotation:z() - 360.0
    end
    if(rotation:z() < -360) then
      rotation.z = rotation:z() + 360.0
    end
    trans:SetWorldRotation(rotation)
  end
end