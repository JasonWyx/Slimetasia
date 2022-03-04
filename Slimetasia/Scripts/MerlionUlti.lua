local b = true
percentage = 0
local maxPercentage = 100
die = false
AmIinTekong = false

local startliaoma = false

local PlayerGO = nil
local PlayerXform = nil
local PlayerScript = nil
local camera_GO = nil
local cameraXform = nil
local camera = nil
local cameraScript = nil

local myXform = nil

local start = false
local UltiStopTimer = 3.0
local cinematic1 = true
local cinematic1setup = true
local cinematic2 = false

local magicCircle1 = nil
local magicCircle1Xform = nil
local magicCircle1Mesh = nil
local magicCircle1Audio = nil
local magicCircle1AudioXform = nil

local magicCircle2 = nil
local magicCircle2Xform = nil
local magicCircle2Mesh = nil
local magicCircle2Audio = nil
local magicCircle2AudioXform = nil

local magicCircle3 = nil
local magicCircle3Xform = nil
local magicCircle3Mesh = nil
local magicCircle3Audio = nil
local magicCircle3AudioXform = nil

local beam = nil
local beamXform = nil
local beamMesh = nil
local beamAudio = nil
local beamAudioXform = nil

local merlionUltiParticle_transform = nil
local merlionUltiParticle_particle = nil
local particleRate = 1
local particleRateLimit = 1000

local ImpactAudio = nil
local ImpactAudioXform = nil

local merlionVec = Vector3(-0.525, 4.116, -23.625)
local spawnerVec = Vector3(-0.48, 2.05, -14.23)
local spawnerview = false

local Scinematic1setup = true
local Stimer = 2.0

local beamdown = nil
local beamdownXform = nil
local beamdownMesh = nil
local beamdownAudio = nil
local beamdownAudioXform = nil

local rocksXform = {}
local rocksMesh = {}
local rocksTimer = {}
local rocksInit = {}
local rocksFinal = {}
local rocksLife = {}

local slimebypassTimer = 2.0

InMerlionUltiMode = false

function StartLiaoLe()
    startliaoma = true
end

local function InitRocks()
    for i = 1, 20 do
        r = owner:GetLayer():Create("rock")
        rocksXform[i] = r:AddComponent("Transform")
        tmp = RandomRange(0.1, 0.3)
        rocksXform[i]:SetWorldScale(Vector3(tmp, tmp, tmp))
        rocksMesh[i] = r:AddComponent("MeshRenderer")
        rocksMesh[i]:SetColor(Color(1,1,1,0))
        rocksMesh[i]:SetMesh("rock_4")
        rocksTimer[i] = 0.0
        rocksLife[i] = 0.0
        rocksInit[i] = Vector3()
        rocksFinal[i] = Vector3()
    end
    write("MerlionUlti.lua Creation of Rocks Completed")
end

local function ClearRocks()
    for i = 1, 20 do
        rocksMesh[i]:SetColor(Color(1,1,1,0))
        rocksTimer[i] = 0.0
        rocksLife[i] = 0.0
        rocksInit[i] = Vector3()
        rocksFinal[i] = Vector3()
    end 
end

local function UpdateRocks(dt, slimePos)
    for i = 1, 20 do
        rocksMesh[i]:SetColor(Color(1,1,1,1))
        if(rocksTimer[i] <= 0.0) then
            rocksTimer[i] = RandomRange(0.8, 1.2)
            rocksLife[i] = rocksTimer[i]
            rocksInit[i] = slimePos
            rocksXform[i]:SetWorldPosition(slimePos)
            tmp = RandomRange(0, 90)
            rocksXform[i]:SetWorldRotation(Vector3(tmp, tmp, tmp))
            rocksFinal[i].x = RandomRange(-6, 6)
            rocksFinal[i].y = RandomRange(2, 6)
            rocksFinal[i].z = RandomRange(-6, 6)
        else
            x = rocksFinal[i]:x() * dt / rocksLife[i]
            y = rocksFinal[i]:y() * dt / rocksLife[i]
            z = rocksFinal[i]:z() * dt / rocksLife[i]
            pos = rocksXform[i]:GetWorldPosition()
            rocksXform[i]:SetWorldPosition(pos:x() + x, pos:y() + y, pos:z() + z)
            rocksTimer[i] = rocksTimer[i] - dt
        end
    end 
end

function myConstructor()
    if(PlayerPref_CheckExist("CurrentLevel", "Variables")) then
        currentLvl = PlayerPref_GetString("CurrentLevel", "Variables")
        if(currentLvl == "Level_Tekong") then 
            AmIinTekong = true 
        end
    end
    PlayerGO = owner:GetLayer():GetObject("Player")
    if(PlayerGO ~= nil) then
        PlayerScript = PlayerGO:GetLuaScript("PlayerScript.lua")
        PlayerXform = PlayerGO:GetComponent("Transform")
        write("here")
    end
    camera_GO = owner:GetLayer():GetObject("Camera")
    if(camera_GO ~= nil) then
        cameraXform = camera_GO:GetComponent("Transform")
        camera = camera_GO:GetComponent("Camera")
        cameraScript = camera_GO:GetLuaScript("PlayerCamera.lua")
    end
    myXform = owner:GetComponent("Transform")
    pos = myXform:GetWorldPosition()

    magicCircle1 = owner:GetLayer():Create("mc1")
    magicCircle1Xform = magicCircle1:AddComponent("Transform")
    magicCircle1Xform:SetWorldPosition(Vector3(pos:x() + 0.4, pos:y() + 7.00, pos:z()))
    magicCircle1Xform:SetWorldScale(Vector3(0,0,0))
    magicCircle1Mesh = magicCircle1:AddComponent("MeshRenderer")
    magicCircle1Mesh:SetMesh("magicCircle")
    magicCircle1Mesh:SetDiffuseTexture("testMagic_NoBorder_whiteness")
    magicCircle1Mesh:SetColor(Color(1,1,1,0))
    magicCircle1Mesh:SetEmissive(Color(1,1,1,1))
    tmp = owner:GetLayer():Create("sound1")
    magicCircle1AudioXform = tmp:AddComponent("Transform")
    magicCircle1Audio = tmp:AddComponent("AudioEmitter")

    magicCircle2 = owner:GetLayer():Create("mc2")
    magicCircle2Xform = magicCircle2:AddComponent("Transform")
    magicCircle2Xform:SetWorldPosition(Vector3(pos:x() + 0.4, pos:y() + 9.00, pos:z()))
    magicCircle2Xform:SetWorldScale(Vector3(0,0,0))
    magicCircle2Mesh = magicCircle2:AddComponent("MeshRenderer")
    magicCircle2Mesh:SetMesh("magicCircle")
    magicCircle2Mesh:SetDiffuseTexture("testMagic_NoBorder_whiteness")
    magicCircle2Mesh:SetColor(Color(1,1,1,0))
    magicCircle2Mesh:SetEmissive(Color(1,1,1,1))
    tmp = owner:GetLayer():Create("sound2")
    magicCircle2AudioXform = tmp:AddComponent("Transform")
    magicCircle2Audio = tmp:AddComponent("AudioEmitter")

    magicCircle3 = owner:GetLayer():Create("mc3")
    magicCircle3Xform = magicCircle3:AddComponent("Transform")
    magicCircle3Xform:SetWorldPosition(Vector3(pos:x() + 0.4, pos:y() + 12.00, pos:z()))
    magicCircle3Xform:SetWorldScale(Vector3(0,0,0))
    magicCircle3Mesh = magicCircle3:AddComponent("MeshRenderer")
    magicCircle3Mesh:SetMesh("magicCircle")
    magicCircle3Mesh:SetEmissive(Color(1,1,1,1))
    magicCircle3Mesh:SetDiffuseTexture("testMagic_NoBorder_whiteness")
    magicCircle3Mesh:SetColor(Color(1,1,1,0))
    tmp = owner:GetLayer():Create("sound3")
    magicCircle3AudioXform = tmp:AddComponent("Transform")
    magicCircle3Audio = tmp:AddComponent("AudioEmitter")

    beam = owner:GetLayer():Create("beam")
    beamXform = beam:AddComponent("Transform")
    beamXform:SetWorldPosition(Vector3(pos:x() + 0.4, pos:y() + 12.0, pos:z()))
    beamXform:SetWorldScale(Vector3(1,0,1))
    beamMesh = beam:AddComponent("MeshRenderer")
    beamMesh:SetMesh("beam")
    beamMesh:SetEmissive(Color(1,1,1,1))
    -- beamMesh:SetDiffuseTexture("")
    beamMesh:SetColor(Color(1,1,1,0))
    tmp = owner:GetLayer():Create("sound4")
    beamAudioXform = tmp:AddComponent("Transform")
    beamAudio = tmp:AddComponent("AudioEmitter")

    beamdown = owner:GetLayer():Create("beamdown")
    beamdownXform = beamdown:AddComponent("Transform")
    beamdownXform:SetWorldPosition(Vector3(pos:x() + 0.4, pos:y() + 12.0, pos:z()))
    beamdownXform:SetWorldScale(Vector3(1,0,1))
    beamdownMesh = beamdown:AddComponent("MeshRenderer")
    beamdownMesh:SetMesh("beam_down")
    beamdownMesh:SetEmissive(Color(1,1,1,1))
    beamdownMesh:SetColor(Color(1,1,1,0))
    tmp = owner:GetLayer():Create("sound5")
    beamdownAudioXform = tmp:AddComponent("Transform")
    beamdownAudio = tmp:AddComponent("AudioEmitter")

    tmp = owner:GetLayer():Create("sound6")
    ImpactAudioXform = tmp:AddComponent("Transform")
    ImpactAudio = tmp:AddComponent("AudioEmitter")
    
    merlionUltiParticle = CreatePrefab("MerlionUltiParticle")
    merlionUltiParticle_transform = merlionUltiParticle:GetComponent("Transform")
    merlionUltiParticle_particle  = merlionUltiParticle:GetComponent("ParticleEmitter_Circle")
    merlionUltiParticle_transform:SetWorldPosition(Vector3(0,-100,0))
    InitRocks()

    write("MerlionUlti.lua : Creation Complete")
end

local function ConversionOfLookatVectorSoWillLookProperlyIHope(vec)
    rot = myXform:GetWorldRotation()
    angle = ToRad(rot:y())
    costheta = Cos(angle)
    sintheta = Sin(angle)
    x = (costheta * vec:x()) + (sintheta * vec:z())
    z = (sintheta * vec:x()) - (costheta * vec:z())
    if(AmIinTekong) then
        x = (costheta * vec:x()) - (sintheta * vec:z())
        z = (sintheta * vec:x()) + (costheta * vec:z())
    end
    newVector = Vector3(x, vec:y(), z)
    return newVector
end

local function ConversionOfLookatVectorSoWillLookAtSlimeSpawnerProperlyIHope(vec)
    rot = slimeXform:GetWorldRotation()
    angle = ToRad(rot:y())
    costheta = Cos(angle)
    sintheta = Sin(angle)
    x = (costheta * vec:x()) + (sintheta * vec:z())
    z = (sintheta * vec:x()) - (costheta * vec:z())
    if(AmIinTekong) then
        x = (costheta * vec:x()) - (sintheta * vec:z())
        z = (sintheta * vec:x()) + (costheta * vec:z())
    end
    newVector = Vector3(x, vec:y(), z)
    return newVector
end

local function MerlionCinematics(dt)
    -- make the player and merlion god mode
    if(cinematic1) then
        if(cinematic1setup) then
            camera:SetLookAt(ConversionOfLookatVectorSoWillLookProperlyIHope(Vector3(0.052, -0.035, 0.998)))
            cinematic1setup = false
            return
        end
        lookat = camera:GetLookAt()
        -- write(lookat)
        direction = ConversionOfLookatVectorSoWillLookProperlyIHope(Vector3(-(-0.016 + 0.052), 0.431 + 0.035, (0.766 - 0.998)))
        ending = ConversionOfLookatVectorSoWillLookProperlyIHope(Vector3(0.016, 0.431, 0.766))
        lookat.x = lookat:x() + (dt * direction:x() / 2.0)
        lookat.y = lookat:y() + (dt * direction:y() / 2.0)
        lookat.z = lookat:z() + (dt * direction:z() / 2.0)
        if(Abs(lookat:x() - ending:x()) <= 0.001) then
            cinematic1 = false
            cinematic2 = true
        end
        camera:SetLookAt(lookat)
    elseif(cinematic2) then
        if(UltiStopTimer > 0) then
            if(UltiStopTimer > 0.0) then
                if(UltiStopTimer == 3.0) then
                    pos = cameraXform:GetWorldPosition()
                    magicCircle1AudioXform:SetWorldPosition(pos)
                    magicCircle1Audio:SetLoop(true)
                    magicCircle1Audio:SetAndPlayAudioClip("Merlion_Magic_Layer_1")
                end
                magicCircle1Mesh:SetColor(Color(1,1,1,1))
                scale = magicCircle1Xform:GetWorldScale()
                increment = 1.30 * dt / 3.0
                scale.x = scale:x() + increment
                if(scale:x() > 1.3) then 
                    scale.x = 1.3 
                    magicCircle1Audio:Stop()
                end
                scale.y = scale:x()
                scale.z = scale:x()
                magicCircle1Xform:SetWorldScale(scale)
            end
            if(UltiStopTimer < 2.0 and UltiStopTimer > 0.0) then
                if(UltiStopTimer + dt >= 2.0) then
                    pos = cameraXform:GetWorldPosition()
                    magicCircle2AudioXform:SetWorldPosition(pos)
                    magicCircle2Audio:SetLoop(true)
                    magicCircle2Audio:SetAndPlayAudioClip("Merlion_Magic_Layer_2")
                end
                magicCircle2Mesh:SetColor(Color(1,1,1,1))
                scale = magicCircle2Xform:GetWorldScale()
                increment = 1.00 * dt / 2.0
                scale.x = scale:x() + increment
                if(scale:x() > 1.0) then 
                    scale.x = 1.0 
                    magicCircle2Audio:Stop()
                end
                scale.y = scale:x()
                scale.z = scale:x()
                magicCircle2Xform:SetWorldScale(scale)
            end
            if(UltiStopTimer < 1.0 and UltiStopTimer > 0.0) then
                if(UltiStopTimer + dt >= 1.0) then
                    pos = cameraXform:GetWorldPosition()
                    magicCircle3AudioXform:SetWorldPosition(pos)
                    magicCircle3Audio:SetLoop(true)
                    magicCircle3Audio:SetAndPlayAudioClip("Merlion_Magic_Layer_3")
                end
                magicCircle3Mesh:SetColor(Color(1,1,1,1))
                scale = magicCircle3Xform:GetWorldScale()
                increment = 0.7 * dt
                scale.x = scale:x() + increment
                if(scale:x() > 0.7) then 
                    scale.x = 0.7 
                    magicCircle3Audio:Stop()
                end
                scale.y = scale:x()
                scale.z = scale:x()
                magicCircle3Xform:SetWorldScale(scale)
            end
            UltiStopTimer = UltiStopTimer - dt
            return
        end
        lookat = camera:GetLookAt()
        if(UltiStopTimer + dt >= 0 and UltiStopTimer < 0) then
            magicCircle3Audio:Stop()
            pos = cameraXform:GetWorldPosition()
            beamAudioXform:SetWorldPosition(pos)
            beamAudio:SetLoop(true)
            beamAudio:SetAndPlayAudioClip("Merlion_Beam_Loop")
            UltiStopTimer = UltiStopTimer - dt
        end
        direction = ConversionOfLookatVectorSoWillLookProperlyIHope(Vector3(-(-0.007 + 0.016), 0.643 - 0.431, (0.766 - 0.902)))
        ending = ConversionOfLookatVectorSoWillLookProperlyIHope(Vector3(0.007, 0.643, 0.766))
        lookat.x = lookat:x() + (dt * direction:x() / 4.0)
        lookat.y = lookat:y() + (dt * direction:y() / 4.0)
        lookat.z = lookat:z() + (dt * direction:z() / 4.0)
        beamMesh:SetColor(Color(1,1,1,1))
        scale = beamXform:GetWorldScale()
        beamXform:SetWorldScale(Vector3(1, scale:y() + (80 * dt), 1))
        diff = Abs(lookat:x() - ending:x())
        if(AmIinTekong) then
            diff = Abs(lookat:z() - ending:z())
        end
        if(diff <= 0.001) then
            beamAudio:Stop()
            magicCircle3Audio:Stop()
            magicCircle2Audio:Stop()
            magicCircle1Audio:Stop()
            cinematic1 = true
            cinematic2 = false
            cinematic1setup = true
            start = false
            spawnerview = true
            UltiStopTimer = 3.0
            magicCircle1Xform:SetWorldScale(Vector3(0,0,0))
            magicCircle2Xform:SetWorldScale(Vector3(0,0,0))
            magicCircle3Xform:SetWorldScale(Vector3(0,0,0))
            magicCircle1Mesh:SetColor(Color(1,1,1,0))
            magicCircle2Mesh:SetColor(Color(1,1,1,0))
            magicCircle3Mesh:SetColor(Color(1,1,1,0))
            beamXform:SetWorldScale(Vector3(1, 0, 1))
            beamMesh:SetColor(Color(1,1,1,0))
            if(AmIinTekong) then
                trans1 = owner:GetLayer():GetObjectByID(297):GetComponent("Transform"):GetWorldPosition()
                trans2 = owner:GetLayer():GetObjectByID(298):GetComponent("Transform"):GetWorldPosition()
                trans1.y = 0.354
                trans2.y = 0.354
                owner:GetLayer():GetObjectByID(298):GetComponent("Transform"):SetWorldPosition(trans2)
                owner:GetLayer():GetObjectByID(297):GetComponent("Transform"):SetWorldPosition(trans1)
            end
            return
        end
        camera:SetLookAt(lookat)
    end
end

local function SpawnerCinematics(dt)
    slime = owner:GetLayer():GetObject("Slime_Spawner")
    if(slime ~= null) then
        if(Scinematic1setup) then
            slimeXform = slime:GetComponent("Transform")
            if(slimeXform == null) then
                spawnerview = false
                PlayerScript:CallFunction("Resume")
                cameraScript:CallFunction("Resume")
                return
            end
            rot = slimeXform:GetWorldRotation()
            angle = ToRad(rot:y())
            costheta = Cos(angle)
            sintheta = Sin(angle)
            x = (costheta * spawnerVec:x()) + (sintheta * spawnerVec:z())
            z = (sintheta * spawnerVec:x()) - (costheta * spawnerVec:z())
            if(AmIinTekong) then
                x = (costheta * spawnerVec:x()) - (sintheta * spawnerVec:z())
                z = (sintheta * spawnerVec:x()) + (costheta * spawnerVec:z())
            end
            SlimePos = slimeXform:GetWorldPosition()
            cameraXform:SetWorldPosition(Vector3(SlimePos:x() + x, SlimePos:y() + spawnerVec:y(), SlimePos:z() + z))
            camera:SetLookAt(ConversionOfLookatVectorSoWillLookAtSlimeSpawnerProperlyIHope(Vector3(-0.011, 0.772, 0.636)))
            -- beam down setup
            Spos = slimeXform:GetWorldPosition()
            beamdownXform:SetWorldPosition(Vector3(Spos:x(), 60.0, Spos:z()))
            -- beamdownXform:SetWorldScale(Vector3(1,1,1))
            beamdownMesh:SetColor(Color(1,1,1,1))
            Scinematic1setup = false
            pos = cameraXform:GetWorldPosition()
            beamdownAudioXform:SetWorldPosition(pos)
            beamdownAudio:SetLoop(true)
            beamdownAudio:SetAndPlayAudioClip("Merlion_Beam_Loop")            
            return
        end
        lookat = camera:GetLookAt()
        -- write(lookat)
        direction = ConversionOfLookatVectorSoWillLookAtSlimeSpawnerProperlyIHope(Vector3(0.0 + 0.011, 0.0 - 0.772, 1.0 - 0.636))
        ending = ConversionOfLookatVectorSoWillLookAtSlimeSpawnerProperlyIHope(Vector3(0.0, 0.0, 1.0))
        lookat.x = lookat:x() + (dt * direction:x() / 2.0)
        lookat.y = lookat:y() + (dt * direction:y() / 2.0)
        lookat.z = lookat:z() + (dt * direction:z() / 2.0)
        slimeTransform = owner:GetLayer():GetObject("Slime_Spawner"):GetComponent("Transform"):GetWorldPosition()
        UpdateRocks(dt, slimeTransform)
        difference = Abs(lookat:x() - ending:x())
        if(AmIinTekong == true) then
            difference = Abs(lookat:z() - ending:z())
        end
        if(difference <= 0.001 or slimebypassTimer <= 0.0) then
            if(Stimer > 0.0) then 
                if(Stimer == 2.0) then
                    pos = cameraXform:GetWorldPosition()
                    ImpactAudioXform:SetWorldPosition(pos)
                    ImpactAudio:SetLoop(true)
                    ImpactAudio:SetAndPlayAudioClip("Merlion_Beam_Impact")
                end
                die = true
                Stimer = Stimer - dt 
                hp = slime:GetLuaScript("Health.lua")
                if(hp ~= nil) then

                    fullhp = hp:GetVariable("fullHP")
                    hp:SetVariable("damage", ((fullhp * 0.2) * dt / 2.0))
                    hp:SetVariable("immune",false)
                    hp:CallFunction("DealDamge")
                    write("I am DAMAGING THE SPAWNER")
                    merlionUltiParticle_transform:SetWorldPosition(slime:GetComponent("Transform"):GetWorldPosition())
                    if (particleRate < particleRateLimit) then
                      particleRate = particleRate * 2
                    end
                    merlionUltiParticle_particle:SetEmitRate(particleRate)
                    merlionUltiParticle_particle:SetFloorHeight(slime:GetComponent("Transform"):GetWorldPosition():y())
                    position = cameraXform:GetWorldPosition()
                    position = Vector3(position:x() + RandomRange(-0.1,0.1), position:y() + RandomRange(-0.1,0.1), position:z() + RandomRange(-0.1,0.1))
                    cameraXform:SetWorldPosition(position)
                end
            else
                slimebypassTimer = 2.0
                die = false
                ClearRocks()
                Scinematic1setup = true
                Stimer = 2.0
                spawnerview = false
                beamdownXform:SetWorldScale(Vector3(1,0,1))
                beamdownMesh:SetColor(Color(1,1,1,0))
                beamdownAudio:Stop()
                ImpactAudio:Stop()
                PlayerScript:CallFunction("Resume")
                cameraScript:CallFunction("Resume")
                InMerlionUltiMode = false
                merlionUltiParticle_transform:SetWorldPosition(Vector3(0,-100,0))
                merlionUltiParticle_particle:SetEmitRate(0)
                particleRate = 1
                return
            end
        else
            slimebypassTimer = slimebypassTimer - dt
            scale = beamdownXform:GetWorldScale()
            beamdownXform:SetWorldScale(Vector3(1, scale:y() + (dt * 80), 1))
            camera:SetLookAt(lookat)
        end
    else
        write("cannot find spawner")
        slimebypassTimer = 2.0
        die = false
        ClearRocks()
        Scinematic1setup = true
        Stimer = 2.0
        spawnerview = false
        beamdownXform:SetWorldScale(Vector3(1,0,1))
        beamdownMesh:SetColor(Color(1,1,1,0))
        beamdownAudio:Stop()
        ImpactAudio:Stop()
        PlayerScript:CallFunction("Resume")
        cameraScript:CallFunction("Resume")
        InMerlionUltiMode = false
        merlionUltiParticle_transform:SetWorldPosition(Vector3(0,-100,0))
        merlionUltiParticle_particle:SetEmitRate(0)
        particleRate = 1
        PlayerScript:CallFunction("Resume")
        cameraScript:CallFunction("Resume")
    end
end

function OnUpdate(dt)
    if(b) then
        myConstructor()
        b = false
    end

    if(startliaoma == true) then
        percentage = percentage + dt
    end

    if(IsKeyPressed(KEY_5) and startliaoma == true) then
        percentage = 100
    end

    if(percentage >= 100) then
        percentage = 100
        -- write("Max Ulti Liao")
        if(PlayerXform ~= nil) then
            pos = PlayerXform:GetWorldPosition()
            myPos = myXform:GetWorldPosition()
            x = myPos:x() - pos:x()
            x = x * x
            y = myPos:y() - pos:y()
            y = y * y
            z = myPos:z() - pos:z()
            z = z * z
            dist = x + y + z
            if(dist <= 49 and ControllerPress("Ultimate")) then
                slime = owner:GetLayer():GetObject("Slime_Spawner")
                if(slime ~= nil) then
                    slimeScript = slime:GetLuaScript("Enemy_Spawner.lua")
                    if(slimeScript ~= nil) then
                        idle = slimeScript:GetVariable("idleSpawner")
                        if(idle == false) then
                            InMerlionUltiMode = true
                            write("Ulti Bitches")
                            PlayerScript:CallFunction("Pause")
                            cameraScript:CallFunction("Pause")
                            rot = myXform:GetWorldRotation()
                            angle = ToRad(rot:y())
                            costheta = Cos(angle)
                            sintheta = Sin(angle)
                            x = (costheta * merlionVec:x()) + (sintheta * merlionVec:z())
                            z = (sintheta * merlionVec:x()) - (costheta * merlionVec:z())
                            if(AmIinTekong) then
                                x = (costheta * merlionVec:x()) - (sintheta * merlionVec:z())
                                z = (sintheta * merlionVec:x()) + (costheta * merlionVec:z())
                                trans1 = owner:GetLayer():GetObjectByID(297):GetComponent("Transform"):GetWorldPosition()
                                trans2 = owner:GetLayer():GetObjectByID(298):GetComponent("Transform"):GetWorldPosition()
                                trans1.y = -1000
                                trans2.y = -1000
                                owner:GetLayer():GetObjectByID(298):GetComponent("Transform"):SetWorldPosition(trans2)
                                owner:GetLayer():GetObjectByID(297):GetComponent("Transform"):SetWorldPosition(trans1)
                            end
                            MerlionPos = myXform:GetWorldPosition()
                            cameraXform:SetWorldPosition(Vector3(MerlionPos:x() + x, MerlionPos:y() + merlionVec:y(), MerlionPos:z() + z))
                            start = true
                            percentage = 0
                        end
                    end
                end
            end
        end
    
    end

    if(start) then
        MerlionCinematics(dt)
    end

    if(spawnerview) then
        SpawnerCinematics(dt)
    end

end