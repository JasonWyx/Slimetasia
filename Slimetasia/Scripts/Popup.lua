local GameObject = nil
local GOXform = nil
local GOMesh = nil
local b = true
local mTransform = nil

local start = false
local meshName = ""
meshOffset = 3.0
lifetime = 1.0
local scale = Vector3(0.18, 0.18, 0.18)
local randx = 0
local randy = 0
local popuptimer = 0.25
local startPos = 1.5
local grow = true

local decider = 0
local script = nil

local function Growing(dt)
    if(grow) then
        scale = GOXform:GetWorldScale()
        scale.x = scale:x() + dt / 2
        scale.y = scale:y() + dt / 2
        scale.z = scale:z() + dt / 2
        GOXform:SetWorldScale(scale)
        if(owner:Name() == "Slime_Spawner") then
            if(scale:x() >= 0.42) then
                grow = false
            end
        else
            if(scale:x() >= 0.2) then
                grow = false
            end
        end
    else
        scale = GOXform:GetWorldScale()
        scale.x = scale:x() - dt / 2
        scale.y = scale:y() - dt / 2
        scale.z = scale:z() - dt / 2
        GOXform:SetWorldScale(scale)
        if(owner:Name() == "Slime_Spawner") then
            if(scale:x() <= 0.38) then
                grow = true
            end
        else
            if(scale:x() <= 0.16) then
                grow = true
            end
        end
    end
end

function Start()
    start = true
    if(decider == 1) then 
        meshName = "ouch"
    else
        meshName = "pain"
    end
    GOMesh:SetMesh(meshName)
    GOMesh:SetColor(Color(1,0,0,1))
    GOMesh:SetEmissive(Color(1,0,0,1))
    ---write("Taking damage")
    if(popuptimer == 0.25) then
        pos = mTransform:GetWorldPosition()
        rot = mTransform:GetWorldRotation()
        rot.x = 0.0
        rot.z = 0.0
        y = pos:y() + startPos
        pos.y = y
        if(owner:Name() == "Slime_Spawner") then
            pos.y = pos:y() + 3.5
        else
            rot.y = rot:y() - 180
        end
        if(owner:Name() == "Slime_TourGuide") then
            pos.y = pos:y() + 0.5
            rot.y = rot:y() + 180
        end
        if(script ~= nil) then
            range = script:GetVariable("isRanged")
            buffed = script:GetVariable("buffUp")
            if(range == false and buffed == true) then
                pos.y = pos:y() + 1.1
                rot.y = rot:y() + 180
            end
        end
        GOXform:SetWorldPosition(pos)
        GOXform:SetWorldRotation(rot)
        GOXform:SetWorldScale(Vector3(0,0,0))
    end
end



function OnUpdate(dt)

if(b) then
    GameObject = owner:GetLayer():Create("PopUp")
    write("Popup : construction done")
    if(GameObject ~= nil) then
        GOXform = GameObject:AddComponent("Transform")
        GOMesh  = GameObject:AddComponent("MeshRenderer")
        --ID = owner:GetID()
        GOXform:SetWorldScale(scale)
    end
    if(owner:Name() == "Slime") then
        script = owner:GetLuaScript("EnemyBehavior.lua")
    end
    mTransform = owner:GetComponent("Transform")
    if(owner:Name() == "Slime_Spawner") then
        scale = Vector3(0.4, 0.4, 0.4)
    end
    b = false
end

if(start) then
    if(popuptimer > 0.0) then
        popuptimer = popuptimer - dt
        s = GOXform:GetWorldScale()
        s.x = s:x() + scale:x() * dt * 4.0
        s.y = s:x()
        s.z = s:x()
        GOXform:SetWorldScale(s)
        p = GOXform:GetWorldPosition()
        p.y = p:y() + (meshOffset - startPos) * dt * 4.0
        GOXform:SetWorldPosition(p)
    else
        lifetime = lifetime - dt
        if(lifetime < 0) then
            lifetime = 1.0
            start = false
            meshName = " "
            GOMesh:SetColor(Color(0,0,0,0))
            decider = decider + 1
            popuptimer = 0.25
            if(decider > 1) then decider = 0 end
        else
            pos = mTransform:GetWorldPosition()
            rot = mTransform:GetWorldRotation()
            rot.x = 0.0
            rot.z = 0.0
            y = pos:y() + meshOffset
            pos.y = y
            if(owner:Name() == "Slime_Spawner") then
                pos.y = pos:y() + 3.5
            else
                rot.y = rot:y() - 180
            end
            if(owner:Name() == "Slime_TourGuide") then
                pos.y = pos:y() + 0.5
                rot.y = rot:y() + 180
            end
            if(script ~= nil) then
                range = script:GetVariable("isRanged")
                buffed = script:GetVariable("buffUp")
                if(range == false and buffed == true) then
                    pos.y = pos:y() + 1.1
                    rot.y = rot:y() + 180
                end
            end
            GOXform:SetWorldPosition(pos)
            GOXform:SetWorldRotation(rot)
            Growing(dt)
        end
    end
end


end

function Destructor()
    GameObject:Destroy()
end