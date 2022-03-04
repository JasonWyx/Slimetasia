local GameObject = nil
local GOXform = nil
local GOMesh = nil
local b = true
local mTransform = nil

local start = false
local meshName = "stars"
local texName = "stars_t"
meshOffset = 2.5
meshForward = 0.35
local scale = Vector3(0.3, 0.3, 0.3)

local yRot = 0.0
local speed = 0.5
local shakeSpeed = 2.0
local shake = 0.0
local add = false


function Start()
    start = true
    GOMesh:SetMesh(meshName)
    GOMesh:SetDiffuseTexture(texName)
    GOMesh:SetColor(Color(1,1,1,1))
    -- write("Started Stun")
end

function End()
    yRot = 0.0
    start = false
    GOMesh:SetColor(Color(0,0,0,0))
    -- write("Ended Stun")
end


function OnUpdate(dt)
    if(b) then
        GameObject = owner:GetLayer():Create("Stun")
        if(GameObject ~= nil) then
            GOXform = GameObject:AddComponent("Transform")
            GOMesh  = GameObject:AddComponent("MeshRenderer")
            GOXform:SetWorldScale(scale)
        end
        mTransform = owner:GetComponent("Transform")
        b = false
    end

    if(start) then
        if(add) then
            shake = shake + (15.0 * shakeSpeed * dt)
            if(shake > 5.0) then add = false end
        else
            shake = shake - (15.0 * shakeSpeed * dt)
            if(shake < -5.0) then add = true end
        end
        yRot = yRot + (360.0 * dt * speed)
        if(yRot > 360) then yRot = yRot - 360 end
        pos = mTransform:GetWorldPosition()
        rot = mTransform:GetWorldRotation()
        y = pos:y() + meshOffset 
        z = pos:z() - meshForward
        pos.y = y
        pos.z = z
        -- rz = rot:z() - 180
        -- rx = rot:x() - 180
        rot.z = shake
        rot.y = yRot
        rot.x = 0
        GOXform:SetWorldPosition(pos)
        GOXform:SetWorldRotation(rot)
    end

end

function Destructor()
    GameObject:Destroy()
end