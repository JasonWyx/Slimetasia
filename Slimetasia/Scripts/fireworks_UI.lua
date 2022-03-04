local GameObject = nil
local GOXform = nil
local GOMesh = nil
local b = true
local mTransform = nil

local start = false
local meshName = "firework_UI"
local scale = Vector3(0.3, 0.3, 0.3)
meshOffset = 2.0
meshSide = 1.0
local left = false
timer = 0.5
local randY = 0.0


function Start()
    GOMesh:SetMesh(meshName)
    GOMesh:SetColor(Color(1,0,0,1))
    randY = RandomRange(0.0, 2.0)
    start = true
end

function End()
    if(left) then
        left = false        
    else
        left = true
    end
    start = false
    GOMesh:SetColor(Color(0,0,0,0))
end

function OnUpdate(dt)
    if(b) then
        GameObject = owner:GetLayer():Create("firecracker_UI")
        if(GameObject ~= nil) then
            GOXform = GameObject:AddComponent("Transform")
            GOMesh  = GameObject:AddComponent("MeshRenderer")
            GOXform:SetWorldScale(scale)
        end
        mTransform = owner:GetComponent("Transform")
        b = false
    end
    if(start) then
        pos = mTransform:GetWorldPosition()
        rot = mTransform:GetWorldRotation()
        y = pos:y() + meshOffset + randY
        x = pos:x()
        pos.y = y
        if(left) then
            x = x - meshSide
        else
            x = x + meshSide
        end
        pos.x = x
        rot.x = 0
        rot.z = 0
        GOXform:SetWorldPosition(pos)
        GOXform:SetWorldRotation(rot)
        timer = timer - dt
        if(timer < 0) then
            randY = RandomRange(0.0, 2.0)
            timer = 0.5
            if(left) then 
                left = false
            else
                left = true
            end
        end
    end
end

function Destructor()
    GameObject:Destroy()
end