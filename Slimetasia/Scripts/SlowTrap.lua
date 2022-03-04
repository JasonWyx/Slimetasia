-- VARIABLES ===================================================================

-- settings

-- gameobjects / components
local GO        = nil
local TRANSFORM = nil
local rayData = nil
local startpt = Vector3()
local endpt   = Vector3()
                                                                                
                                                                                
-- FUNCTIONS ===================================================================
function Constructor()
  GO = owner

    TRANSFORM = GO:GetComponent("Transform")
    startpt = TRANSFORM:GetWorldPosition()
    -------get raycast data shld be array of results
-----
    
    ---------Get length of position to raycast object
    --Multiply with beam vector and 
  
    ---assuming the trap facing -ve z
    
    beamVector = TRANSFORM:ForwardVector()
    rayDir = beamVector
    rayDir.x = beamVector:x() * -1.0
    rayDir.y  = beamVector:y() * -1.0
    rayDir.z  = beamVector:z() * -1.0
    raylimit = rayDir
    raylimit.x  = raylimit:x() * 50.0
    raylimit.y  = raylimit:y() * 50.0
    raylimit.z  = raylimit:z() * 50.0
    endpt = startpt + raylimit
    hittime = 0.0
    write("Before casting")
   -- rayData = RayCast(startpt, endpt,hittime)
   -- write("After casting", #rayData)
   -- write(rayData[0].hitFrac)
end

function OnUpdate(dt)                                         
  DebugDrawLine(startpt, endpt)
  write("Draw")
end
