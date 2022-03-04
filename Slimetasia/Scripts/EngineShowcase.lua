local camera = nil
local cameraXForm = nil

local current_Angle = 0.0
local originalPos = Vector3()
local pointOfFocus = Vector3()
local distance = Vector3()

function Constructor()
  camera = owner:GetComponent("Camera")
  cameraXForm = owner:GetComponent("Transform")
  originalPos = cameraXForm:GetWorldPosition()
  pointOfFocus = Vector3(0.0, 2.0, 0.0)
  distance = originalPos - pointOfFocus
end

function OnUpdate(dt)

  write("camcmamcamc")
  current_Angle = current_Angle + dt * 5
  
  if (current_Angle > 360) then
    current_Angle = 0
  end
  
  cameraPos = pointOfFocus + distance:Rotate("y", current_Angle)

  cameraXForm:SetWorldPosition(cameraPos)
  
  lookDirection = pointOfFocus - cameraPos
  
  camera:SetLookAt(lookDirection:Normalize())

end