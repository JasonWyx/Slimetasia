local GameObject = nil
local GOXform = nil
local GOMesh = nil
local b = true
local mTransform = nil

local start = false
local left = false
local meshName = "crossbow_UI"
local scale = Vector3(0.1, 0.1, 0.1)
meshOffset = 1.5
local randY = 0.0
local lifetime = 0.35
timer = lifetime
local lmeshName = "Lcrossbow_UI"
local rmeshName = "Rcrossbow_UI"

function Start()
    if(left) then
        GOMesh:SetMesh(lmeshName)       
    else
        GOMesh:SetMesh(rmeshName)
    end
    GOMesh:SetColor(Color(0,0.9,1,1))
    randY = RandomRange(0.0, 0.0)
    start = true
end

function End()
    timer = lifetime
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
        GameObject = owner:GetLayer():Create("shoot_UI")
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
        pos.x = x
        rot.x = 0
        rot.z = 0
        GOXform:SetWorldPosition(pos)
        GOXform:SetWorldRotation(rot)
        timer = timer - dt
        if(timer < 0) then
            End()
        end
    end
end

function Destructor()
    GameObject:Destroy()
end