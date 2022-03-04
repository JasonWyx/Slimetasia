-- VARIABLES ===================================================================


-- gameobjects / components
local CAMERAGO           = nil
local CAMERA             = nil
local CAMERATRANSFORM    = nil
local PLAYERGO           = nil
local PLAYERTRANSFORM    = nil


-- FUNCTIONS ===================================================================
function Constructor()
  CAMERAGO = owner
  CAMERA   = CAMERAGO:GetComponent("Camera")
  CAMERATRANSFORM    =  CAMERAGO:GetComponent("Transform")
  PLAYERGO = owner:GetLayer():GetObject("Player")
  PLAYERTRANSFORM = PLAYERGO:GetComponent("Transform")
end

function OnUpdate(dt)
  pos = PLAYERTRANSFORM:GetWorldPosition()
  pos = pos+Vector3(0,70,70)
  CAMERATRANSFORM:SetWorldPosition(pos)
end
