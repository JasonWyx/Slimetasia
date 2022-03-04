local start = false
maxTimer = 1.0
fadeinTimer = 1.0
local fadeOutLayer = nil
local GO = nil
local GOMesh = nil
local b = true
local fadeInStart = false

function StartFadeOut()
    start = true
end

function StartFadeIn()
    fadeInStart = true
end

function OnUpdate(dt)
    if(b) then
        fadeOutLayer = GetLayer("fadeOutLayer")
        if(fadeOutLayer == nil) then
            fadeOutLayer = CreateLayer("fadeOutLayer")
        end
        cam = fadeOutLayer:GetObject("MainCamera")
        camComp = cam:GetComponent("Camera")
        lookat = Vector3(0, 0, -1)
        camComp:SetLookAt(lookat)
        camComp:SetUICam(true)
        GO = fadeOutLayer:Create("Black")
        GOXform = GO:AddComponent("Transform")
        GOMesh = GO:AddComponent("MeshRenderer")
        GOMesh:SetMesh("planeMesh")
        scale = Vector3(0, 1, -1)
        GOXform:SetWorldPosition(scale)
        scale = Vector3(20, 10, 10)
        GOXform:SetWorldScale(scale)
        scale = Vector3(90,0,0)
        GOXform:SetWorldRotation(scale)
        c = Color(0, 0, 0, 0)
        GOMesh:SetColor(c)
        b = false
        write("fadeOut.lua : Creation Complete")
    end

    if(start) then
        alpha = GOMesh:GetColor():a()
        alpha = alpha + (dt / maxTimer)
        if(alpha >= 1.0) then
            alpha = 1.0
            start = false
        end
        c = Color(0, 0, 0, alpha)
        GOMesh:SetColor(c)
    elseif(fadeInStart) then
        alpha = GOMesh:GetColor():a()
        alpha = alpha - (dt / fadeinTimer)
        if(alpha <= 0.0) then
            alpha = 0.0
            fadeInStart = false
        end
        c = Color(0, 0, 0, alpha)
        GOMesh:SetColor(c)
    end

end