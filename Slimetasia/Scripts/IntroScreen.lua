local b = true
local introBG = nil
local introText = nil
local introBGXform = nil
local introBGMesh = nil
local introTextXform = nil
local introTextMesh = nil
local introLayer = nil

local BGFadedOut = false
local TextFadeIn = true

TextFadeInTimer = 10.0
TextFadeOutTimer = 4.0
BGFadeOutTimer = 14.0

BGTextureName = "transparency"
TextTextureName = "WhereTheChineseGather"

FadeOutSpeed = 1.0

function ChangeBackground()
    introBGMesh:SetDiffuseTexture(BGTextureName)
end

function ChangeIntro()
    introTextMesh:SetDiffuseTexture(TextTextureName)
end

function OnUpdate(dt)
    if(b) then
        introLayer = GetLayer("IntroLayer")
        if(introLayer == nil) then
            introLayer = CreateLayer("IntroLayer")
        end
        cam = introLayer:GetObject("MainCamera")
        camComp = cam:GetComponent("Camera")
        lookat = Vector3(0,0,-1)
        camComp:SetLookAt(lookat)
        camComp:SetUICam(true)
        introBG = introLayer:Create("BackGround")
        introText = introLayer:Create("StageName")
        introBGXform = introBG:AddComponent("Transform")
        introTextXform = introText:AddComponent("Transform")
        introTextMesh = introText:AddComponent("MeshRenderer")
        introBGMesh = introBG:AddComponent("MeshRenderer")
        rotation = Vector3(90, 0, 0)
        position = Vector3(0,0,-1)
        scale = Vector3(1.5, 1.00, 0.85)
        introTextXform:SetWorldRotation(rotation)
        introBGXform:SetWorldRotation(rotation)
        introTextXform:SetWorldPosition(position)
        position = Vector3(0,0,-1.0001)
        introBGXform:SetWorldPosition(position)
        introBGXform:SetWorldScale(scale)
        introTextXform:SetWorldScale(scale)
        introTextMesh:SetMesh("planeMesh")
        introBGMesh:SetMesh("planeMesh")
        introTextMesh:SetDiffuseTexture(TextTextureName)
        introBGMesh:SetDiffuseTexture(BGTextureName)
        color = introTextMesh:GetColor()
        color.a = 0
        introTextMesh:SetColor(color)
        b = false
        write("IntroScreen.lua : Creation Complete")
    end

    if(BGFadeOutTimer > 0.0) then
        BGFadeOutTimer = BGFadeOutTimer - dt
    else
        color = introBGMesh:GetColor()
        if(color:a() > 0.0) then
            color.a = color:a() - (dt * FadeOutSpeed)
            introBGMesh:SetColor(color)
        else
            BGFadedOut = true
        end
    end

    if(BGFadedOut) then
        if(TextFadeOutTimer > 0.0) then
            TextFadeOutTimer = TextFadeOutTimer - dt
        else
            color = introTextMesh:GetColor()
            if(color:a() > 0.0) then
                color.a = color:a() - (dt * FadeOutSpeed)
                introTextMesh:SetColor(color)
            end
        end
    end
    if(TextFadeIn) then
        if(TextFadeInTimer > 0.0) then
            TextFadeInTimer = TextFadeInTimer - dt
            color = introTextMesh:GetColor()
        else
            color = introTextMesh:GetColor()
            if(color:a() < 1.0) then
                color.a = color:a() + (dt * FadeOutSpeed)
                introTextMesh:SetColor(color)
            else
                color.a = 1.0
                introTextMesh:SetColor(color)
                TextFadeIn = false
            end
        end
    end
end