local Xform = nil
local cam = nil
local cameraScript = nil
local playerScript = nil
local b = true

local cameraMove = false
local startCutScene = false

local vec = Vector3(0 ,0, -44.54)

local JiangShi = nil
local JiangShiMesh = nil
local JiangShiXform = nil
local JiangShiAnimator = nil

local BG = nil
local BGMesh = nil
local BGXform = nil

local Text1 = nil
local Text1Mesh = nil
local Text1Xform = nil

local Text2 = nil
local Text2Mesh = nil
local Text2Xform = nil

local Text3 = nil
local Text3Mesh = nil
local Text3Xform = nil

local light = nil
local lightXform = nil
local lightComp = nil

local timer = 5.0

local function myConstructor()
    Xform = owner:GetComponent("Transform")
    cam = owner:GetComponent("Camera")
    cameraScript = owner:GetLuaScript("PlayerCamera.lua")
    playerScript = owner:GetLayer():GetObject("Player"):GetLuaScript("PlayerScript.lua")
    b = false
    -- Jiang Shi
    JiangShi = owner:GetLayer():Create("JiangShi")
    JiangShiXform = JiangShi:AddComponent("Transform")
    JiangShiMesh = JiangShi:AddComponent("MeshRenderer")
    JiangShiXform:SetWorldScale(Vector3(0.03,0.03,0.03))
    JiangShiXform:SetWorldPosition(Vector3(-63.408, -1000, -25.047))
    JiangShiXform:SetWorldRotation(Vector3(-90.0, -255.0, 0.0))
    JiangShiMesh:SetMesh("ZombieJiangShiAnimated")
    JiangShiMesh:SetDiffuseTexture("Enemy_jiangshi_D")
    JiangShiMesh:SetEmissiveTexture("Enemy_jiangshi_D_EM")
    JiangShiMesh:SetEmissive(Color(1.0, 1.0, 1.0, 1.0))
    JiangShiMesh:SetColor(Color(1,1,1,0))
    JiangShiAnimator = JiangShi:AddComponent("MeshAnimator")
    myAnim = GetResource_Animation("ZombieJSAnim")
    JiangShiAnimator:SetAnimation(myAnim)
    

    -- BG
    BG = owner:GetLayer():Create("BG")
    BGXform = BG:AddComponent("Transform")
    BGMesh = BG:AddComponent("MeshRenderer")
    BGXform:SetWorldScale(Vector3(13,13,7))
    BGXform:SetWorldPosition(Vector3(-65.60, 3.0, -25.447))
    BGXform:SetWorldRotation(Vector3(90.0, 0.0, 0.0))
    BGMesh:SetMesh("planeMesh")
    BGMesh:SetColor(Color(1,1,1,0))
    BGMesh:SetDiffuseTexture("bgTextureNewest")
    -- BGMesh:SetEmissive(Color(0.5, 0.5, 0.5, 0.5))

    -- Text1
    Text1 = owner:GetLayer():Create("Text1")
    Text1Xform = Text1:AddComponent("Transform")
    Text1Mesh = Text1:AddComponent("TextRenderer")
    Text1Xform:SetWorldScale(Vector3(1.25,1.25,1.25))
    Text1Xform:SetWorldRotation(Vector3(0.0, 0.0, 10.6))
    Text1Xform:SetWorldPosition(Vector3(-70.320, -1000, -25.347))
    Text1Mesh:SetText("JIANGSHI")
    Text1Mesh:SetFont("BD_CARTOON_SHOUT")
    Text1Mesh:SetColor(Color(1,0,0,1))
    Text1Mesh:SetOutline(true)
    Text1Mesh:SetOutlineSize(0.7)


    -- Text2
    Text2 = owner:GetLayer():Create("Text2")
    Text2Xform = Text2:AddComponent("Transform")
    Text2Mesh  = Text2:AddComponent("TextRenderer")
    Text2Xform:SetWorldScale(Vector3(0.5, 0.5, 0.5))
    Text2Xform:SetWorldRotation(Vector3(0.0, 0.0, 5.626))
    Text2Xform:SetWorldPosition(Vector3(-70.026, -1000, -25.347))
    Text2Mesh:SetText("HE SPITS BALLS")
    Text2Mesh:SetFont("BD_CARTOON_SHOUT")
    Text2Mesh:SetColor(Color(1,1,1,1))
    Text2Mesh:SetOutline(true)
    Text2Mesh:SetOutlineSize(0.7)

    -- Text3
    Text3 = owner:GetLayer():Create("Text3")
    Text3Xform = Text3:AddComponent("Transform")
    Text3Mesh  = Text3:AddComponent("TextRenderer")
    Text3Xform:SetWorldScale(Vector3(0.5, 0.5, 0.5))
    Text3Xform:SetWorldRotation(Vector3(0.0, 0.0, 5.626))
    Text3Xform:SetWorldPosition(Vector3(-69.928, -1000, -25.347))
    Text3Mesh:SetText("(ALSO, THEY \n STICK ON YOU)")
    Text3Mesh:SetFont("BD_CARTOON_SHOUT")
    Text3Mesh:SetColor(Color(1,1,1,1))
    Text3Mesh:SetOutline(true)
    Text3Mesh:SetOutlineSize(0.7)

    write("Construction Completed : Chinatown_newCreep.lua")
end

function start()
    -- owner:GetLayer():GetObject("GameLogic"):GetLuaScript("Event_Chinatown.lua"):SetVariable("inCutScene", true)
    if(Xform ~= nil) then
        Xform:SetWorldPosition(Vector3(-65.60, 2.617, 21.640))
    end
    if(cam ~= nil) then
        cam:SetLookAt(Vector3(0,0,-1))
    end
    cameraMove = true
    JiangShiXform:SetWorldPosition(Vector3(-63.408, 0.0, -25.047))
    JiangShiMesh:SetColor(Color(1,1,1,1))
    owner:GetLayer():GetObject("Sound"):GetComponent("AudioEmitter"):SetAndPlayAudioClip("Whoosh_Zombie_intro.ogg")
    --playerScript:CallFunction("Pause")
    --cameraScript:CallFunction("Pause")
    --
end

function OnUpdate(dt)
    if(b == true) then
        myConstructor()
    end
    if(cameraMove == true) then
        if(Xform ~= nil) then
            pos = Xform:GetWorldPosition()
            pos.x = pos:x() + (vec:x() * dt * 2.0)
            pos.y = pos:y() + (vec:y() * dt * 2.0)
            pos.z = pos:z() + (vec:z() * dt * 2.0)
            if(pos:z() <= -20.910) then
                pos.z = -20.910
                cameraMove = false
                startCutScene = true
                -- cam:SetUICam(true)
                BGMesh:SetColor(Color(1,1,1,1))
                Text2Xform:SetWorldPosition(Vector3(-69.526, 2.194, -25.347))
                Text3Xform:SetWorldPosition(Vector3(-69.428, 1.615, -25.347))
                Text1Xform:SetWorldPosition(Vector3(-69.820, 3.815, -25.347))
                cam:SetLightDirection(Vector3(0,0, -1))
                cam:SetLightIntensity(1.0)
                cam:SetShadowCasted(false)
                cam:SetColor(Color(1,1,1,1))
                owner:GetLayer():GetObject("Sound"):GetComponent("AudioEmitter"):Stop()
            end
            Xform:SetWorldPosition(pos)
        end
    end

    if(startCutScene == true) then
        timer = timer - dt
        if(timer < 0.0) then
            startCutScene = false
            timer = 5.0
            cam:SetLightDirection(Vector3(0.5, -1.0, 0.20))
            cam:SetLightIntensity(0.6)
            cam:SetShadowCasted(true)
            cam:SetColor(Color(0, 0.4, 0.6))
            JiangShiMesh:SetColor(Color(1,1,1,0))
            Text2Xform:SetWorldPosition(Vector3(-69.526, -1000, -25.347))
            Text3Xform:SetWorldPosition(Vector3(-69.428, -1000, -25.347))
            Text1Xform:SetWorldPosition(Vector3(-69.820, -1000, -25.347))
            JiangShiXform:SetWorldPosition(Vector3(-63.408, -1000, -25.047))
            BGMesh:SetColor(Color(1,1,1,0))
            --playerScript:CallFunction("Resume")
            --cameraScript:CallFunction("Resume")
        end
    end

    if(IsKeyPressed(KEY_A)) then
        --start()
    end

end