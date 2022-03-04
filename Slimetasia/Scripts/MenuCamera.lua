local StartPos = Vector3(0, 0, 0)
local EndPos = Vector3(22.1, 5.5, 7.5)
local mTransform = nil
local b = true
local LookAt = Vector3(-3.00, 500.0, 0.0)
local FinalLookAt = Vector3(-3.00, 6.300, 0.0)
local startMoving = false
local Camera = nil
local startY = 50.0
local endY = 6.41
local yDiff = startY - endY
local done = false
local zoomInPos = Vector3(22.975, 7.051, -11.880)
local zoomVector = zoomInPos - EndPos
local waitTimer = 3.0
local lightIntensity = 1.00
local ArrayOfButtons = {}
local TextureOfButtons = { 
                         "StartGame_king",
                         "Options_king",
                         "ExitGame_king"
                       }
local TextureOfSelectedButtons = { 
                                 "StartGameS",
                                 "OptionsS",
                                 "ExitGameS"
                               }
local Selected = 1

local Start = nil
local StartXform = nil
local StartMesh = nil
local Options = nil
local OptionsXform = nil
local OptionsMesh = nil
local Exit = nil
local ExitXform = nil
local ExitMesh = nil

local intensity = lightIntensity
local transition = false
local InstructionsLayer = nil

local PaperMesh = nil

local BGMSoundtrack = "BGM_MainMenu"
local ButtonChangeSFX = "Game_Start_play"
local SelectButtonSound = "Menu_Select"

local startChange = true
local optionChange = true
local exitChange = true

local OptionsMove = false
local BackMove = false
local InOptions = false
local LookAtOptions = Vector3(0, 1, -0.01)
local OptionsUp = Vector3(-0.0001, 700, 1)
local OptionsPos = Vector3(-20.673, -11.719, -6.447)

local OptionsUpVector = OptionsUp - LookAt
local OptionsLookAtVector = LookAtOptions - Vector3(0,0,-1)
local UpVector = Vector3(-3.00, 700.0, 0.0)

-- Options Mode Variables

local ArrayOfOptionsButton = {}
local OptionsSelected = 1
local OptionsWin = nil
local OptionsWinMesh = nil
local OptionsWinXform = nil
local FullScreenMode = 1

local OptionsBack = nil
local OptionsBackMesh = nil
local OptionsBackXform = nil

local SFX = nil
local SFXMesh = nil
local SFXXform = nil

local BGM = nil
local BGMMesh = nil
local BGMXform = nil

local Sensitivity = nil
local SensitivityMesh = nil
local SensitivityXform = nil

local SFXBar = nil
local SFXBarMesh = nil
local SFXBarXform = nil

local SFXBarBG = nil
local SFXBarBGMesh = nil
local SFXBarBGXform = nil

local BGMBar = nil
local BGMBarMesh = nil
local BGMBarXform = nil

local BGMBarBG = nil
local BGMBarBGMesh = nil
local BGMBarBGXform = nil

local SensitivityBar = nil
local SensitivityBarMesh = nil
local SensitivityBarXform = nil

local SensitivityBGBar = nil
local SensitivityBGBarMesh = nil
local SensitivityBGBarXform = nil

local inConfirmationMode = false
local ConfirmationSelect = 1

local AreYouSure = nil
local AreYouSureMesh = nil
local AreYouSureXform = nil

local Yes = nil
local YesXform = nil
local YesMesh = nil
local YesChange = false

local No = nil
local NoXform = nil
local NoMesh = nil
local NoChange = false

local CreditsMove = false
local InCredits = false
local CreditsBack = false

local OptionsHeader = nil

local SFXEmitter = nil
local ButtonChangeSFXEmitter = nil
local TickTimer = 0.3

local function CreditsMode(dt)
    if(GetControllerInput() == 0) then
        if(IsKeyPressed(KEY_SPACE)) then
            CreditsBack = true
        end
    else
        if(ControllerPress("Shoot") or ControllerPress("Jump")) then
            CreditsBack = true
        end
    end
end

local function OptionsMode(dt)
    OptionsMove = false

    if(OptionsSelected == 2) then
        SFXBarMesh:SetColor(Color(1,1,1,1))
        SensitivityBarMesh:SetColor(Color(1,1,1,0))
        BGMBarMesh:SetColor(Color(1,1,1,0))
        SensitivityBGBarMesh:SetColor(Color(0.1,0.1,0.1,0.0))
        BGMBarBGMesh:SetColor(Color(0.1,0.1,0.1,0.0))
        SFXBarBGMesh:SetColor(Color(0.1,0.1,0.1,1.0))
        -- start Update
        vol = AudioSystem_GetChannelGrpVolume("SFX")
        if(ControllerDown("Left")) then
            vol = vol - (0.1 * dt);
            if(vol < 0.0) then vol = 0.0 end
            AudioSystem_SetChannelGrpVolume("SFX", vol)
            if(not ControllerDown("Right")) then
                TickTimer = TickTimer - dt
            end
        end
        if(ControllerDown("Right")) then
            vol = vol + (0.1 * dt)
            if(vol > 1.0) then vol = 1.0 end
            AudioSystem_SetChannelGrpVolume("SFX", vol)
            if(not ControllerDown("Left")) then
                TickTimer = TickTimer - dt
            end
        end

        if(ControllerUp("Left") and not ControllerDown("Right")) then
            SFXEmitter:SetAndPlayAudioClip("Tekong_E1_1")
            TickTimer = 0.3
        end
        
        if(ControllerUp("Right") and not ControllerDown("Left")) then
            SFXEmitter:SetAndPlayAudioClip("Tekong_E1_1")
            TickTimer = 0.3
        end

        if(TickTimer <= 0.0) then
            AudioSystem_PlayAudioAtLocation("TickSound", Vector3(-20.689, -11.728, -6.452))
            TickTimer = 0.3
        end

        p = vol
        nScale = p * 8.0
        nOffset = (1.0 - p) * 4.0
        pos = SFXBarXform:GetWorldPosition()
        scale = SFXBarXform:GetWorldScale()
        scale.x = nScale
        pos.x = -19.3 - nOffset
        SFXBarXform:SetWorldPosition(pos)
        SFXBarXform:SetWorldScale(scale)
        -- end update
    elseif(OptionsSelected == 3) then
        SFXBarMesh:SetColor(Color(1,1,1,0))
        SensitivityBarMesh:SetColor(Color(1,1,1,0))
        BGMBarMesh:SetColor(Color(1,1,1,1))
        SensitivityBGBarMesh:SetColor(Color(0.1,0.1,0.1,0.0))
        BGMBarBGMesh:SetColor(Color(0.1,0.1,0.1,1.0))
        SFXBarBGMesh:SetColor(Color(0.1,0.1,0.1,0))
        -- start Update
        vol = AudioSystem_GetChannelGrpVolume("BGM")
        if(ControllerDown("Left")) then
            vol = vol - (0.1 * dt);
            if(vol < 0.0) then vol = 0.0 end
            AudioSystem_SetChannelGrpVolume("BGM", vol)
            if(not ControllerDown("Right")) then
                TickTimer = TickTimer - dt
            end
        end
        if(ControllerDown("Right")) then
            vol = vol + (0.1 * dt)
            if(vol > 1.0) then vol = 1.0 end
            AudioSystem_SetChannelGrpVolume("BGM", vol)
            if(not ControllerDown("Left")) then
                TickTimer = TickTimer - dt
            end
        end

        if(ControllerUp("Left") and not ControllerDown("Right")) then
            TickTimer = 0.3
        end
        
        if(ControllerUp("Right") and not ControllerDown("Left")) then
            TickTimer = 0.3
        end

        if(TickTimer <= 0.0) then
            AudioSystem_PlayAudioAtLocation("TickSound", Vector3(-20.689, -11.728, -6.452))
            TickTimer = 0.3
        end

        p = vol
        nScale = p * 8.0
        nOffset = (1.0 - p) * 4.0
        pos = BGMBarXform:GetWorldPosition()
        scale = BGMBarXform:GetWorldScale()
        scale.x = nScale
        pos.x = -19.3 - nOffset
        BGMBarXform:SetWorldPosition(pos)
        BGMBarXform:SetWorldScale(scale)
        -- end update
    elseif(OptionsSelected == 4) then -- sensitivity
        SFXBarMesh:SetColor(Color(1,1,1,0))
        SensitivityBarMesh:SetColor(Color(1,1,1,1))
        BGMBarMesh:SetColor(Color(1,1,1,0))
        SensitivityBGBarMesh:SetColor(Color(0.1,0.1,0.1,1.0))
        BGMBarBGMesh:SetColor(Color(0.1,0.1,0.1,0.0))
        SFXBarBGMesh:SetColor(Color(0.1,0.1,0.1,0))
        -- start update
        sens = 0.0

        if(GetControllerInput() == 1) then
            if(PlayerPref_CheckExist("SensitivityController", "Settings_Player")) then
                sens = PlayerPref_GetFloat("SensitivityController", "Settings_Player")
                if(ControllerDown("Left")) then
                    sens = sens - (0.3 * dt);
                    if(sens < 0.1) then sens = 0.1 end
                    PlayerPref_SetFloat("SensitivityController", sens, "Settings_Player")
                    if(not ControllerDown("Right")) then
                        TickTimer = TickTimer - dt
                    end
                end
                if(ControllerDown("Right")) then
                    sens = sens + (0.3 * dt)
                    if(sens > 2.0) then sens = 2.0 end
                    PlayerPref_SetFloat("SensitivityController", sens, "Settings_Player")
                    if(not ControllerDown("Left")) then
                        TickTimer = TickTimer - dt
                    end
                end
            end
        elseif(GetControllerInput() == 0) then
            if(PlayerPref_CheckExist("SensitivityKeyBoard", "Settings_Player")) then
                sens = PlayerPref_GetFloat("SensitivityKeyBoard", "Settings_Player")
                if(ControllerDown("Left")) then
                    sens = sens - (0.3 * dt);
                    if(sens < 0.1) then sens = 0.1 end
                    PlayerPref_SetFloat("SensitivityKeyBoard", sens, "Settings_Player")
                    if(not ControllerDown("Right")) then
                        TickTimer = TickTimer - dt
                    end
                end
                if(ControllerDown("Right")) then
                    sens = sens + (0.3 * dt)
                    if(sens > 2.0) then sens = 2.0 end
                    PlayerPref_SetFloat("SensitivityKeyBoard", sens, "Settings_Player")
                    if(not ControllerDown("Left")) then
                        TickTimer = TickTimer - dt
                    end
                end
            end
        end

        if(ControllerUp("Left") and not ControllerDown("Right")) then
            TickTimer = 0.3
        end
        
        if(ControllerUp("Right") and not ControllerDown("Left")) then
            TickTimer = 0.3
        end

        if(TickTimer <= 0.0) then
            AudioSystem_PlayAudioAtLocation("TickSound", Vector3(-20.689, -11.728, -6.452))
            TickTimer = 0.3
        end

        p = (sens - 0.1) / 1.9
        nScale = p * 8.0
        nOffset = (1.0 - p) * 4.0
        pos = SensitivityBarXform:GetWorldPosition()
        scale = SensitivityBarXform:GetWorldScale()
        scale.x = nScale
        pos.x = -19.3 - nOffset
        SensitivityBarXform:SetWorldPosition(pos)
        SensitivityBarXform:SetWorldScale(scale)
        -- end update
    else
        SFXBarMesh:SetColor(Color(1,1,1,0))
        SensitivityBarMesh:SetColor(Color(1,1,1,0))
        BGMBarMesh:SetColor(Color(1,1,1,0))
        SensitivityBGBarMesh:SetColor(Color(0.1,0.1,0.1,0.0))
        BGMBarBGMesh:SetColor(Color(0.1,0.1,0.1,0.0))
        SFXBarBGMesh:SetColor(Color(0.1,0.1,0.1,0))
    end
    if(ControllerPress("Up") and not ControllerDown("Left") and not ControllerDown("Right")) then
        ButtonChangeSFXEmitter:SetAndPlayAudioClip(ButtonChangeSFX)
        OptionsSelected = OptionsSelected - 1
        if(OptionsSelected < 1) then OptionsSelected = OptionsSelected + 5 end
        if(OptionsSelected == 1) then
            OptionsWinMesh:SetColor(Color(1,0,0,1))
            OptionsBackMesh:SetColor(Color(1,1,1,1))
            SFXMesh:SetColor(Color(1,1,1,1))
            BGMMesh:SetColor(Color(1,1,1,1))
            SensitivityMesh:SetColor(Color(1,1,1,1))
        end
        if(OptionsSelected == 2) then
            OptionsWinMesh:SetColor(Color(1,1,1,1))
            OptionsBackMesh:SetColor(Color(1,1,1,1))
            SFXMesh:SetColor(Color(1,0,0,1))
            BGMMesh:SetColor(Color(1,1,1,1))
            SensitivityMesh:SetColor(Color(1,1,1,1))
        end
        if(OptionsSelected == 3) then
            OptionsWinMesh:SetColor(Color(1,1,1,1))
            OptionsBackMesh:SetColor(Color(1,1,1,1))
            SFXMesh:SetColor(Color(1,1,1,1))
            BGMMesh:SetColor(Color(1,0,0,1))
            SensitivityMesh:SetColor(Color(1,1,1,1))
        end
        if(OptionsSelected == 4) then
            OptionsWinMesh:SetColor(Color(1,1,1,1))
            OptionsBackMesh:SetColor(Color(1,1,1,1))
            SFXMesh:SetColor(Color(1,1,1,1))
            BGMMesh:SetColor(Color(1,1,1,1))
            SensitivityMesh:SetColor(Color(1,0,0,1))
        end
        if(OptionsSelected == 5) then
            OptionsWinMesh:SetColor(Color(1,1,1,1))
            OptionsBackMesh:SetColor(Color(1,0,0,1))
            SFXMesh:SetColor(Color(1,1,1,1))
            BGMMesh:SetColor(Color(1,1,1,1))
            SensitivityMesh:SetColor(Color(1,1,1,1))
        end
    end

    if(ControllerPress("Down") and not ControllerDown("Left") and not ControllerDown("Right")) then
        ButtonChangeSFXEmitter:SetAndPlayAudioClip(ButtonChangeSFX)
        OptionsSelected = OptionsSelected + 1
        if(OptionsSelected > 5) then OptionsSelected = OptionsSelected - 5 end
        if(OptionsSelected == 1) then
            OptionsWinMesh:SetColor(Color(1,0,0,1))
            OptionsBackMesh:SetColor(Color(1,1,1,1))
            SFXMesh:SetColor(Color(1,1,1,1))
            BGMMesh:SetColor(Color(1,1,1,1))
            SensitivityMesh:SetColor(Color(1,1,1,1))
        end
        if(OptionsSelected == 2) then
            OptionsWinMesh:SetColor(Color(1,1,1,1))
            OptionsBackMesh:SetColor(Color(1,1,1,1))
            SFXMesh:SetColor(Color(1,0,0,1))
            BGMMesh:SetColor(Color(1,1,1,1))
            SensitivityMesh:SetColor(Color(1,1,1,1))
        end
        if(OptionsSelected == 3) then
            OptionsWinMesh:SetColor(Color(1,1,1,1))
            OptionsBackMesh:SetColor(Color(1,1,1,1))
            SFXMesh:SetColor(Color(1,1,1,1))
            BGMMesh:SetColor(Color(1,0,0,1))
            SensitivityMesh:SetColor(Color(1,1,1,1))
        end
        if(OptionsSelected == 4) then
            OptionsWinMesh:SetColor(Color(1,1,1,1))
            OptionsBackMesh:SetColor(Color(1,1,1,1))
            SFXMesh:SetColor(Color(1,1,1,1))
            BGMMesh:SetColor(Color(1,1,1,1))
            SensitivityMesh:SetColor(Color(1,0,0,1))
        end
        if(OptionsSelected == 5) then
            OptionsWinMesh:SetColor(Color(1,1,1,1))
            OptionsBackMesh:SetColor(Color(1,0,0,1))
            SFXMesh:SetColor(Color(1,1,1,1))
            BGMMesh:SetColor(Color(1,1,1,1))
            SensitivityMesh:SetColor(Color(1,1,1,1))
        end
    end

    if(GetControllerInput() == 0) then
        if(IsKeyPressed(KEY_SPACE)) then
            if(OptionsSelected == 1) then
                ButtonChangeSFXEmitter:SetAndPlayAudioClip(ButtonChangeSFX)
                if(FullScreenMode == 1) then -- FullScreen
                    OptionsWinXform:SetWorldPosition(Vector3(-26.383, 2.399, -4.460))
                    OptionsWinXform:SetWorldRotation(Vector3(180, 0, 0))
                    OptionsWinXform:SetWorldScale(Vector3(5.0, 0.75, 0.75))
                    OptionsWinMesh:SetDiffuseTexture("Fullscreen")
                    FullScreenMode = 0
                    ToggleFullscreen(false)
                elseif(FullScreenMode == 0) then -- Windowed
                    OptionsWinXform:SetWorldPosition(Vector3(-26.973, 2.399, -4.460))
                    OptionsWinXform:SetWorldRotation(Vector3(180, 0, 0))
                    OptionsWinXform:SetWorldScale(Vector3(4.0, 0.75, 0.75))
                    OptionsWinMesh:SetDiffuseTexture("window")
                    FullScreenMode = 1
                    ToggleFullscreen(true)
                end
            end
            if(OptionsSelected == 5) then
                ButtonChangeSFXEmitter:SetAndPlayAudioClip(ButtonChangeSFX)
                BackMove = true
                InOptions = false
            end
        end
    else
        if(ControllerPress("Shoot") or ControllerPress("Jump")) then
            if(OptionsSelected == 1) then
                ButtonChangeSFXEmitter:SetAndPlayAudioClip(ButtonChangeSFX)
                if(FullScreenMode == 1) then -- FullScreen
                    OptionsWinXform:SetWorldPosition(Vector3(-26.383, 2.399, -4.460))
                    OptionsWinXform:SetWorldRotation(Vector3(180, 0, 0))
                    OptionsWinXform:SetWorldScale(Vector3(5.0, 0.75, 0.75))
                    OptionsWinMesh:SetDiffuseTexture("Fullscreen")
                    FullScreenMode = 0
                    ToggleFullscreen(false)
                elseif(FullScreenMode == 0) then -- Windowed
                    OptionsWinXform:SetWorldPosition(Vector3(-26.973, 2.399, -4.460))
                    OptionsWinXform:SetWorldRotation(Vector3(180, 0, 0))
                    OptionsWinXform:SetWorldScale(Vector3(4.0, 0.75, 0.75))
                    OptionsWinMesh:SetDiffuseTexture("window")
                    FullScreenMode = 1
                    ToggleFullscreen(true)
                end
            end
            if(OptionsSelected == 5) then
                ButtonChangeSFXEmitter:SetAndPlayAudioClip(ButtonChangeSFX)
                BackMove = true
                InOptions = false
            end
        end
    end
end

local function StartScaling(dt)
    scale = StartXform:GetWorldScale()
    if(startChange) then
        scale.x = scale:x() + dt / 2
        scale.y = scale:y() + dt / 2
        scale.z = scale:z() + dt / 2
        StartXform:SetWorldScale(scale)
        if(scale:x() >= 5.6) then
            startChange = false
        end
    else
        scale.x = scale:x() - dt / 2
        scale.y = scale:y() - dt / 2
        scale.z = scale:z() - dt / 2 
        StartXform:SetWorldScale(scale)
        if(scale:x() <= 5.4) then
            startChange = true
        end
    end
    if(scale:y() > 2.00) then
    StartXform:SetWorldScale(Vector3(5.5, 0.75, 0.75))
   end
    -- StartXform:SetWorldScale(Vector3(5.5, 0.75, 0.75))
end

local function OptionScaling(dt)

    if(optionChange) then
        scale = OptionsXform:GetWorldScale()
        scale.x = scale:x() + dt / 2
        scale.y = scale:y() + dt / 2
        scale.z = scale:z() + dt / 2
        OptionsXform:SetWorldScale(scale)
        if(scale:x() >= 3.6) then
            optionChange = false
        end
    else
        scale = OptionsXform:GetWorldScale()
        scale.x = scale:x() - dt / 2
        scale.y = scale:y() - dt / 2
        scale.z = scale:z() - dt / 2 
        OptionsXform:SetWorldScale(scale)
        if(scale:x() <= 3.4) then
            optionChange = true
        end
    end
    -- OptionsXform:SetWorldScale(Vector3(3.50, 0.750, 0.750))
end

local function ExitScaling(dt)

    if(exitChange) then
        scale = ExitXform:GetWorldScale()
        scale.x = scale:x() + dt / 2
        scale.y = scale:y() + dt / 2
        scale.z = scale:z() + dt / 2
        ExitXform:SetWorldScale(scale)
        if(scale:x() >= 4.6) then
            exitChange = false
        end
    else
        scale = ExitXform:GetWorldScale()
        scale.x = scale:x() - dt / 2
        scale.y = scale:y() - dt / 2
        scale.z = scale:z() - dt / 2 
        ExitXform:SetWorldScale(scale)
        if(scale:x() <= 4.4) then
            exitChange = true
        end
    end
    -- ExitXform:SetWorldScale(Vector3(4.500, 0.750, 0.750))
end

local function YesScaling(dt)
    if(YesChange) then
        scale = YesXform:GetWorldScale()
        scale.x = scale:x() + dt / 2
        scale.y = scale:y() + dt / 2
        scale.z = scale:z() + dt / 2
        YesXform:SetWorldScale(scale)
        if(scale:x() >= 2.1) then
            YesChange = false
        end
    else
        scale = YesXform:GetWorldScale()
        scale.x = scale:x() - dt / 2
        scale.y = scale:y() - dt / 2
        scale.z = scale:z() - dt / 2 
        YesXform:SetWorldScale(scale)
        if(scale:x() <= 1.9) then
            YesChange = true
        end
    end
end

local function NoScaling(dt)
    if(NoChange) then
        scale = NoXform:GetWorldScale()
        scale.x = scale:x() + dt / 2
        scale.y = scale:y() + dt / 2
        scale.z = scale:z() + dt / 2
        NoXform:SetWorldScale(scale)
        if(scale:x() >= 1.6) then
            NoChange = false
        end
    else
        scale = NoXform:GetWorldScale()
        scale.x = scale:x() - dt / 2
        scale.y = scale:y() - dt / 2
        scale.z = scale:z() - dt / 2 
        NoXform:SetWorldScale(scale)
        if(scale:x() <= 1.4) then
            NoChange = true
        end
    end
end

function OnUpdate(dt)
if(b) then
    PlayerPref_SetString ("CurrentLevel", "Level_Tekong")

    FadeToBlackMesh = owner:GetComponent("DirectionalLight")
    mTransform = owner:GetComponent("Transform")
    Camera = owner:GetComponent("Camera")
    Camera:SetUp(LookAt)
    b = false
    
    -- Start Game Button
    Start = owner:GetLayer():Create("StartGame")
    StartXform = Start:AddComponent("Transform")
    StartMesh = Start:AddComponent("MeshRenderer")
    StartXform:SetWorldPosition(Vector3(-3.559, -2.627, -15.809))
    StartXform:SetWorldRotation(Vector3(90.00, 0.000, -4.646))
    StartXform:SetWorldScale(Vector3(5.5, 0.75, 0.75))
    StartMesh:SetMesh("planeMesh")
    StartMesh:SetDiffuseTexture("StartGameS")
    ArrayOfButtons[1] = StartMesh
    -- Options Button
    Options = owner:GetLayer():Create("Options")
    OptionsXform = Options:AddComponent("Transform")
    OptionsMesh = Options:AddComponent("MeshRenderer")
    OptionsXform:SetWorldPosition(Vector3(-4.630, -3.988, -15.809))
    OptionsXform:SetWorldRotation(Vector3(90.00, 0.000, -4.646))
    OptionsXform:SetWorldScale(Vector3(3.50, 0.750, 0.750))
    OptionsMesh:SetMesh("planeMesh")
    OptionsMesh:SetDiffuseTexture("Options_king")
    ArrayOfButtons[2] = Options
    -- ExitGame Button
    Exit = owner:GetLayer():Create("Options")
    ExitXform = Exit:AddComponent("Transform")
    ExitMesh = Exit:AddComponent("MeshRenderer")
    ExitXform:SetWorldPosition(Vector3(-4.275, -5.445, -15.809))
    ExitXform:SetWorldRotation(Vector3(90.00, 0.000, -4.646))
    ExitXform:SetWorldScale(Vector3(4.500, 0.750, 0.750))
    ExitMesh:SetMesh("planeMesh")
    ExitMesh:SetDiffuseTexture("ExitGame_king")
    ArrayOfButtons[3] = Exit

    ShowMouseCursor(false)

    PaperMesh = owner:GetLayer():GetObject("Paper"):GetComponent("MeshRenderer")

    owner:GetComponent("AudioEmitter"):SetLoop(true)
    owner:GetComponent("AudioEmitter"):SetChannelGroup("BGM")
    owner:GetComponent("AudioEmitter"):SetAndPlayAudioClip(BGMSoundtrack)

    SFXEmitter = owner:AddComponent("AudioEmitter")
    SFXEmitter:SetChannelGroup("SFX")
    SFXEmitter:SetLoop(false)
    
    ButtonChangeSFXEmitter = owner:AddComponent("AudioEmitter")
    ButtonChangeSFXEmitter:SetChannelGroup("SFX")
    ButtonChangeSFXEmitter:SetLoop(false)

    Selected = 1

    OptionsSelected = 1

    OptionsBack = owner:GetLayer():Create("Back")
    OptionsBackXform = OptionsBack:AddComponent("Transform")
    OptionsBackMesh = OptionsBack:AddComponent("MeshRenderer")
    OptionsBackXform:SetWorldPosition(Vector3(-27.816, 2.399, -9.992))
    OptionsBackXform:SetWorldRotation(Vector3(180, 0, 0))
    OptionsBackXform:SetWorldScale(Vector3(2.0, 0.75, 0.75))
    OptionsBackMesh:SetMesh("planeMesh")
    OptionsBackMesh:SetDiffuseTexture("Back")

    OptionsWin = owner:GetLayer():Create("Window")
    OptionsWinXform = OptionsWin:AddComponent("Transform")
    OptionsWinMesh = OptionsWin:AddComponent("MeshRenderer")
    OptionsWinXform:SetWorldPosition(Vector3(-26.973, 2.399, -4.460))
    OptionsWinXform:SetWorldRotation(Vector3(180, 0, 0))
    OptionsWinXform:SetWorldScale(Vector3(4.0, 0.75, 0.75))
    OptionsWinMesh:SetMesh("planeMesh")
    OptionsWinMesh:SetDiffuseTexture("window")
    OptionsWinMesh:SetColor(Color(1,0,0,1))
    FullScreenMode = 1 -- 1 == FUllsCreen then 0 == window

    SFX = owner:GetLayer():Create("SFX")
    SFXXform = SFX:AddComponent("Transform")
    SFXMesh = SFX:AddComponent("MeshRenderer")
    SFXXform:SetWorldPosition(Vector3(-27.851, 2.399, -5.867))
    SFXXform:SetWorldRotation(Vector3(180, 0, 0))
    SFXXform:SetWorldScale(Vector3(2.0, 0.75, 0.75))
    SFXMesh:SetMesh("planeMesh")
    SFXMesh:SetDiffuseTexture("SFX")
    
    SFXBar = owner:GetLayer():Create("SFXBar")
    SFXBarXform = SFXBar:AddComponent("Transform")
    SFXBarMesh = SFXBar:AddComponent("MeshRenderer")
    SFXBarXform:SetWorldPosition(Vector3(-19.300, 2.3, -5.800))
    SFXBarXform:SetWorldRotation(Vector3(180.0, 0.0, 0.0))
    SFXBarXform:SetWorldScale(Vector3(8.0, 1.0, 0.75))
    SFXBarMesh:SetMesh("planeMesh")
    SFXBarMesh:SetColor(Color(1,1,1,0))

    SFXBarBG = owner:GetLayer():Create("SFXBarBG")
    SFXBarBGXform = SFXBarBG:AddComponent("Transform")
    SFXBarBGMesh = SFXBarBG:AddComponent("MeshRenderer")
    SFXBarBGXform:SetWorldPosition(Vector3(-19.300, 2.399, -5.800))
    SFXBarBGXform:SetWorldRotation(Vector3(180.0, 0.0, 0.0))
    SFXBarBGXform:SetWorldScale(Vector3(8.0, 1.0, 0.75))
    SFXBarBGMesh:SetMesh("planeMesh")
    SFXBarBGMesh:SetColor(Color(0.1,0.1,0.1,0))

    BGM = owner:GetLayer():Create("BGM")
    BGMXform = BGM:AddComponent("Transform")
    BGMMesh = BGM:AddComponent("MeshRenderer")
    BGMXform:SetWorldPosition(Vector3(-27.851, 2.399, -7.267))
    BGMXform:SetWorldRotation(Vector3(180, 0, 0))
    BGMXform:SetWorldScale(Vector3(2.0, 0.75, 0.75))
    BGMMesh:SetMesh("planeMesh")
    BGMMesh:SetDiffuseTexture("BGM_NEW")

    BGMBar = owner:GetLayer():Create("BGMBar")
    BGMBarXform = BGMBar:AddComponent("Transform")
    BGMBarMesh = BGMBar:AddComponent("MeshRenderer")
    BGMBarXform:SetWorldPosition(Vector3(-19.300, 2.3, -7.267))
    BGMBarXform:SetWorldRotation(Vector3(180.0, 0.0, 0.0))
    BGMBarXform:SetWorldScale(Vector3(8.0, 1.0, 0.75))
    BGMBarMesh:SetMesh("planeMesh")
    BGMBarMesh:SetColor(Color(1,1,1,0))

    BGMBarBG = owner:GetLayer():Create("BGMBarBG")
    BGMBarBGXform = BGMBarBG:AddComponent("Transform")
    BGMBarBGMesh = BGMBarBG:AddComponent("MeshRenderer")
    BGMBarBGXform:SetWorldPosition(Vector3(-19.300, 2.399, -7.267))
    BGMBarBGXform:SetWorldRotation(Vector3(180.0, 0.0, 0.0))
    BGMBarBGXform:SetWorldScale(Vector3(8.0, 1.0, 0.75))
    BGMBarBGMesh:SetMesh("planeMesh")
    BGMBarBGMesh:SetColor(Color(0.1,0.1,0.1,0.0))

    Sensitivity = owner:GetLayer():Create("Sensitivity")
    SensitivityXform = Sensitivity:AddComponent("Transform")
    SensitivityMesh = Sensitivity:AddComponent("MeshRenderer")
    SensitivityXform:SetWorldPosition(Vector3(-26.351, 2.399, -8.667))
    SensitivityXform:SetWorldRotation(Vector3(180, 0, 0))
    SensitivityXform:SetWorldScale(Vector3(5.0, 0.75, 0.75))
    SensitivityMesh:SetMesh("planeMesh")
    SensitivityMesh:SetDiffuseTexture("Sensitivity_NEW")

    SensitivityBar = owner:GetLayer():Create("SensitivityBar")
    SensitivityBarXform = SensitivityBar:AddComponent("Transform")
    SensitivityBarMesh = SensitivityBar:AddComponent("MeshRenderer")
    SensitivityBarXform:SetWorldPosition(Vector3(-19.300, 2.3, -8.716))
    SensitivityBarXform:SetWorldRotation(Vector3(180.0, 0.0, 0.0))
    SensitivityBarXform:SetWorldScale(Vector3(8.0, 1.0, 0.75))
    SensitivityBarMesh:SetMesh("planeMesh")
    SensitivityBarMesh:SetColor(Color(1,1,1,0))

    SensitivityBGBar = owner:GetLayer():Create("SensitivityBGBar")
    SensitivityBGBarXform = SensitivityBGBar:AddComponent("Transform")
    SensitivityBGBarMesh = SensitivityBGBar:AddComponent("MeshRenderer")
    SensitivityBGBarXform:SetWorldPosition(Vector3(-19.300, 2.399, -8.716))
    SensitivityBGBarXform:SetWorldRotation(Vector3(180.0, 0.0, 0.0))
    SensitivityBGBarXform:SetWorldScale(Vector3(8.0, 1.0, 0.75))
    SensitivityBGBarMesh:SetMesh("planeMesh")
    SensitivityBGBarMesh:SetColor(Color(0.1,0.1,0.1,0.0))
    
    AreYouSure = owner:GetLayer():Create("AreYouSure")
    AreYouSureXform = AreYouSure:AddComponent("Transform")
    AreYouSureMesh = AreYouSure:AddComponent("MeshRenderer")
    AreYouSureXform:SetWorldPosition(Vector3(-2.272, -2.672, -15.000))
    AreYouSureXform:SetWorldRotation(Vector3(90, 0, -5.0))
    AreYouSureXform:SetWorldScale(Vector3(7.8, 1.0, 1.0))
    AreYouSureMesh:SetMesh("planeMesh")
    AreYouSureMesh:SetDiffuseTexture("AreYouSure")
    AreYouSureMesh:SetColor(Color(1.0,1.0,0.0,0.0))

    Yes = owner:GetLayer():Create("Yes")
    YesXform = Yes:AddComponent("Transform")
    YesMesh = Yes:AddComponent("MeshRenderer")
    YesXform:SetWorldPosition(Vector3(-4.474, -4.337, -15.000))
    YesXform:SetWorldRotation(Vector3(90, 0, -5.0))
    YesXform:SetWorldScale(Vector3(2.0, 1.0, 1.0))
    YesMesh:SetMesh("planeMesh")
    YesMesh:SetDiffuseTexture("Yes")
    YesMesh:SetColor(Color(1.0,1.0,1.0,0.0))

    No = owner:GetLayer():Create("No")
    NoXform = No:AddComponent("Transform")
    NoMesh = No:AddComponent("MeshRenderer")
    NoXform:SetWorldPosition(Vector3(-0.625, -4.689, -15.000))
    NoXform:SetWorldRotation(Vector3(90, 0, -5.0))
    NoXform:SetWorldScale(Vector3(1.50, 1.0, 1.0))
    NoMesh:SetMesh("planeMesh")
    NoMesh:SetDiffuseTexture("No")
    NoMesh:SetColor(Color(1.0,1.0,1.0,0.0))

    optionsGO = owner:GetLayer():GetObject("OptionsHeader")
    if(optionsGO) then
        OptionsHeader = optionsGO:GetComponent("MeshRenderer")
    end

    write("MenuCamera.lua : Creation Complete")
end

if(inConfirmationMode == false) then

    if(ControllerPress("SwitchMode") and not transition) then
        SensitivityMesh:SetColor(Color(0,0,0,0))
        BGMMesh:SetColor(Color(0,0,0,0))
        SFXMesh:SetColor(Color(0,0,0,0))
        OptionsBackMesh:SetColor(Color(0,0,0,0))
        OptionsWinMesh:SetColor(Color(0,0,0,0))
        if(OptionsHeader ~= nil) then
            OptionsHeader:SetColor(Color(1,1,0,0))
        end
        if(GetControllerInput() == 0) then
            owner:GetLayer():GetObject("monitor"):GetComponent("MeshRenderer"):SetDiffuseTexture("CreditsMenu")
        else
            owner:GetLayer():GetObject("monitor"):GetComponent("MeshRenderer"):SetDiffuseTexture("CreditsMenuCon")
        end
        CreditsMove = true
        transition = true
    end

    if(CreditsMove and transition) then
        tmp1 = true
        tmp2 = true
        tmp3 = true
        currPos = mTransform:GetWorldPosition()    
        if(currPos:x() > OptionsPos:x() or currPos:y() > OptionsPos:y() or currPos:z() > OptionsPos:z()) then
            x = currPos:x() + (OptionsPos:x() * dt)
            y = currPos:y() + (OptionsPos:y() * dt)
            z = currPos:z() + (OptionsPos:z() * dt)
            mTransform:SetWorldPosition(Vector3(x,y,z))
            if(currPos:x() < OptionsPos:x() or currPos:y() < OptionsPos:y() or currPos:z() < OptionsPos:z()) then
                mTransform:SetWorldPosition(OptionsPos)
            end
        else
            tmp1 = false
        end
        LA = Camera:GetLookAt()
        if(LA:y() < LookAtOptions:y() or LA:z() < LookAtOptions:z()) then
            x = LA:x() + (OptionsLookAtVector:x() * dt)
            y = LA:y() + (OptionsLookAtVector:y() * dt)
            z = LA:z() + (OptionsLookAtVector:z() * dt)
            if(LA:y() > LookAtOptions:y() or LA:z() > LookAtOptions:z()) then
                x = 0
                y = 1
                z = -0.01
            end
            Camera:SetLookAt(Vector3(x,y,z))
        else
            Camera:SetLookAt(Vector3(0,1,-0.001))
            tmp2 = false
        end
        if(UpVector:x() < OptionsUp:x() or UpVector:z() > OptionsUp:z()) then
            x = UpVector:x() + (OptionsUpVector:x() * dt)
            y = UpVector:y() + (OptionsUpVector:y() * dt)
            z = UpVector:z() + (OptionsUpVector:z() * dt)
            UpVector.x = x
            UpVector.y = y
            UpVector.z = z
            if(UpVector:x() > OptionsUp:x() or UpVector:z() < OptionsUp:z()) then
            UpVector = Vector3(-0.0001, 700, 0.01)
            write(UpVector)
            end
            Camera:SetUp(UpVector)
        else
            tmp3 = false
        end

        if(tmp1 == false and tmp2 == false and tmp3 == false) then
            InCredits = true
        end

    end

    if(InCredits) then
        CreditsMove = false
        CreditsMode(dt)
    end

    if(CreditsBack) then
        InCredits = false
        tmp1 = true
        tmp2 = true
        tmp3 = true
        currPos = mTransform:GetWorldPosition()    
        if(currPos:x() < 0 or currPos:y() < 0 or currPos:z() < 0) then
            x = currPos:x() - (OptionsPos:x() * dt)
            y = currPos:y() - (OptionsPos:y() * dt)
            z = currPos:z() - (OptionsPos:z() * dt)
            mTransform:SetWorldPosition(Vector3(x,y,z))
            if(currPos:x() > 0 or currPos:y() > 0 or currPos:z() > 0) then
            mTransform:SetWorldPosition(Vector3(0,0,0))
            end
        else
            tmp1 = false
        end
        LA = Camera:GetLookAt()
        if(LA:x() < 0 or LA:y() > 0 or LA:z() > -1) then
            x = LA:x() - (OptionsLookAtVector:x() * dt)
            y = LA:y() - (OptionsLookAtVector:y() * dt)
            z = LA:z() - (OptionsLookAtVector:z() * dt)
            if(LA:x() > 0 or LA:y() < 0 or LA:z() < -1) then
                x = 0
                y = 0
                z = -1
            end
            Camera:SetLookAt(Vector3(x,y,z))
        else
            Camera:SetLookAt(Vector3(0,0,-1))
            tmp2 = false
        end
        if(UpVector:x() > LookAt:x() or UpVector:z() < LookAt:z()) then
            x = UpVector:x() - (OptionsUpVector:x() * dt)
            y = UpVector:y() - (OptionsUpVector:y() * dt)
            z = UpVector:z() - (OptionsUpVector:z() * dt)
            UpVector.x = x
            UpVector.y = y
            UpVector.z = z

            if(UpVector:x() < LookAt:x() or UpVector:z() > LookAt:z()) then
            UpVector = Vector3(-3.00, 700.0, 1.0)
            end
            Camera:SetUp(UpVector)
        else
            tmp3 = false
        end

        if(tmp1 == false and tmp2 == false and tmp3 == false) then
            transition = false
            CreditsBack = false 
        end
    end

    if(Selected == 1) then
        StartScaling(dt)
    elseif(Selected == 2) then
        OptionScaling(dt)
    elseif(Selected == 3) then
        ExitScaling(dt)
    end

    if(ControllerPress("Up") and not transition) then
        ButtonChangeSFXEmitter:SetAndPlayAudioClip(ButtonChangeSFX)
        Selected = Selected - 1
        if(Selected < 1) then Selected = Selected + 3 end
        if(Selected == 1) then
            StartMesh:SetDiffuseTexture(TextureOfSelectedButtons[1])
            OptionsMesh:SetDiffuseTexture(TextureOfButtons[2])
            ExitMesh:SetDiffuseTexture(TextureOfButtons[3])
            OptionsXform:SetWorldScale(Vector3(3.50, 0.750, 0.750))
            ExitXform:SetWorldScale(Vector3(4.500, 0.750, 0.750))
        elseif(Selected == 2) then
            StartMesh:SetDiffuseTexture(TextureOfButtons[1])
            OptionsMesh:SetDiffuseTexture(TextureOfSelectedButtons[2])
            ExitMesh:SetDiffuseTexture(TextureOfButtons[3])
            StartXform:SetWorldScale(Vector3(5.5, 0.75, 0.75))
            ExitXform:SetWorldScale(Vector3(4.500, 0.750, 0.750))
        elseif(Selected == 3) then
            StartMesh:SetDiffuseTexture(TextureOfButtons[1])
            OptionsMesh:SetDiffuseTexture(TextureOfButtons[2])
            ExitMesh:SetDiffuseTexture(TextureOfSelectedButtons[3])
            StartXform:SetWorldScale(Vector3(5.5, 0.75, 0.75))
            OptionsXform:SetWorldScale(Vector3(3.50, 0.750, 0.750))
        end
    end

    if(ControllerPress("Down") and not transition) then
        ButtonChangeSFXEmitter:SetAndPlayAudioClip(ButtonChangeSFX)
        Selected = Selected + 1
        if(Selected > 3) then Selected = Selected - 3 end
        if(Selected == 1) then
            StartMesh:SetDiffuseTexture(TextureOfSelectedButtons[1])
            OptionsMesh:SetDiffuseTexture(TextureOfButtons[2])
            ExitMesh:SetDiffuseTexture(TextureOfButtons[3])
            OptionsXform:SetWorldScale(Vector3(3.50, 0.750, 0.750))
            ExitXform:SetWorldScale(Vector3(4.500, 0.750, 0.750))
        elseif(Selected == 2) then
            StartMesh:SetDiffuseTexture(TextureOfButtons[1])
            OptionsMesh:SetDiffuseTexture(TextureOfSelectedButtons[2])
            ExitMesh:SetDiffuseTexture(TextureOfButtons[3])
            StartXform:SetWorldScale(Vector3(5.5, 0.75, 0.75))
            ExitXform:SetWorldScale(Vector3(4.500, 0.750, 0.750))
        elseif(Selected == 3) then
            StartMesh:SetDiffuseTexture(TextureOfButtons[1])
            OptionsMesh:SetDiffuseTexture(TextureOfButtons[2])
            ExitMesh:SetDiffuseTexture(TextureOfSelectedButtons[3])
            StartXform:SetWorldScale(Vector3(5.5, 0.75, 0.75))
            OptionsXform:SetWorldScale(Vector3(3.50, 0.750, 0.750))
        end
    end

    if(GetControllerInput() == 0) then
        PaperMesh:SetDiffuseTexture("CreditsOpen")
    else
        PaperMesh:SetDiffuseTexture("CreditsOpenCon")
    end

    if(not transition) then
        if(GetControllerInput() == 0) then
            if(IsKeyPressed(KEY_SPACE)) then
                ButtonChangeSFXEmitter:SetAndPlayAudioClip(ButtonChangeSFX)
                if(Selected == 1) then
                    startMoving = true
                    transition = true
                elseif(Selected == 2) then
                    OptionsMove = true
                    transition = true
                    OptionsSelected = 1
                    OptionsWinMesh:SetColor(Color(1,0,0,1))
                    OptionsBackMesh:SetColor(Color(1,1,1,1))
                    SFXMesh:SetColor(Color(1,1,1,1))
                    BGMMesh:SetColor(Color(1,1,1,1))
                    SensitivityMesh:SetColor(Color(1,1,1,1))
                    if(OptionsHeader ~= nil) then
                        OptionsHeader:SetColor(Color(1,1,0,1))
                    end
                    owner:GetLayer():GetObject("monitor"):GetComponent("MeshRenderer"):SetDiffuseTexture("monitor")
                elseif(Selected == 3) then
                    StartMesh:SetColor(Color(0,0,0,0))
                    OptionsMesh:SetColor(Color(0,0,0,0))
                    ExitMesh:SetColor(Color(0,0,0,0))
                    AreYouSureMesh:SetColor(Color(1.0,1.0,0.0,1.0))
                    YesMesh:SetColor(Color(1.0,1.0,1.0,1.0))
                    NoMesh:SetColor(Color(1.0,0.0,0.0,1.0))
                    inConfirmationMode = true
                end
            end
        else
            if(ControllerPress("Shoot") or ControllerPress("Jump")) then
                ButtonChangeSFXEmitter:SetAndPlayAudioClip(ButtonChangeSFX)
                if(Selected == 1) then
                    startMoving = true
                    transition = true
                elseif(Selected == 2) then
                    OptionsMove = true
                    transition = true
                    OptionsSelected = 1
                    OptionsWinMesh:SetColor(Color(1,0,0,1))
                    OptionsBackMesh:SetColor(Color(1,1,1,1))
                    SFXMesh:SetColor(Color(1,1,1,1))
                    BGMMesh:SetColor(Color(1,1,1,1))
                    SensitivityMesh:SetColor(Color(1,1,1,1))
                    if(OptionsHeader ~= nil) then
                        OptionsHeader:SetColor(Color(1,1,0,1))
                    end
                    owner:GetLayer():GetObject("monitor"):GetComponent("MeshRenderer"):SetDiffuseTexture("monitor")
                elseif(Selected == 3) then
                    StartMesh:SetColor(Color(0,0,0,0))
                    OptionsMesh:SetColor(Color(0,0,0,0))
                    ExitMesh:SetColor(Color(0,0,0,0))
                    AreYouSureMesh:SetColor(Color(1.0,1.0,0.0,1.0))
                    YesMesh:SetColor(Color(1.0,1.0,1.0,1.0))
                    NoMesh:SetColor(Color(1.0,0.0,0.0,1.0))
                    inConfirmationMode = true
                end
            end
        end
    end

    if(startMoving and not done) then
        currPos = mTransform:GetWorldPosition()
        if(currPos:x() < EndPos:x() or currPos:y() < EndPos:y()) then
            x = currPos:x() + (EndPos:x() * dt)
            y = currPos:y() + (EndPos:y() * dt)
            z = currPos:z() + (EndPos:z() * dt)
            startY = startY - (yDiff * dt)
            if(startY < endY) then startY = endY end
            -- write(startY)
            Camera:SetUp(Vector3(-3.00, startY, 0.0))
            mTransform:SetWorldPosition(Vector3(x,y,z))
        else
            if(waitTimer > 0.0) then 
                waitTimer = waitTimer - dt
            else
                done = true
            end
        end
    end

    if(done) then
        currPos = mTransform:GetWorldPosition()
        if(currPos:x() < zoomInPos:x() or currPos:y() < zoomInPos:y() or currPos:z() > zoomInPos:z()) then
            x = currPos:x() + (zoomVector:x() * dt)
            y = currPos:y() + (zoomVector:y() * dt)
            z = currPos:z() + (zoomVector:z() * dt)
            mTransform:SetWorldPosition(Vector3(x,y,z))
            lightIntensity = lightIntensity - (intensity * dt)
            if(lightIntensity < 0.0) then lightIntensity = 0.0 end
            Camera:SetLightIntensity(lightIntensity)
            trans = owner:GetLayer():GetObject("MapLight"):GetComponent("Transform"):GetWorldPosition()
            trans.y = trans:y() - (dt * 100)
            owner:GetLayer():GetObject("MapLight"):GetComponent("Transform"):SetWorldPosition(trans)

            -- write(lightIntensity)
        else
            SceneLoad("Level_IntroVideo")
        end
    end

    if(transition and OptionsMove) then
        tmp1 = true
        tmp2 = true
        tmp3 = true
        currPos = mTransform:GetWorldPosition()    
        if(currPos:x() > OptionsPos:x() or currPos:y() > OptionsPos:y() or currPos:z() > OptionsPos:z()) then
            x = currPos:x() + (OptionsPos:x() * dt)
            y = currPos:y() + (OptionsPos:y() * dt)
            z = currPos:z() + (OptionsPos:z() * dt)
            mTransform:SetWorldPosition(Vector3(x,y,z))
            if(currPos:x() < OptionsPos:x() or currPos:y() < OptionsPos:y() or currPos:z() < OptionsPos:z()) then
                mTransform:SetWorldPosition(OptionsPos)
            end
        else
            tmp1 = false
        end
        LA = Camera:GetLookAt()
        if(LA:y() < LookAtOptions:y() or LA:z() < LookAtOptions:z()) then
            x = LA:x() + (OptionsLookAtVector:x() * dt)
            y = LA:y() + (OptionsLookAtVector:y() * dt)
            z = LA:z() + (OptionsLookAtVector:z() * dt)
            if(LA:y() > LookAtOptions:y() or LA:z() > LookAtOptions:z()) then
                x = 0
                y = 1
                z = -0.01
            end
            Camera:SetLookAt(Vector3(x,y,z))
        else
            Camera:SetLookAt(Vector3(0,1,-0.001))
            tmp2 = false
        end
        if(UpVector:x() < OptionsUp:x() or UpVector:z() > OptionsUp:z()) then
            x = UpVector:x() + (OptionsUpVector:x() * dt)
            y = UpVector:y() + (OptionsUpVector:y() * dt)
            z = UpVector:z() + (OptionsUpVector:z() * dt)
            UpVector.x = x
            UpVector.y = y
            UpVector.z = z
            if(UpVector:x() > OptionsUp:x() or UpVector:z() < OptionsUp:z()) then
            UpVector = Vector3(-0.0001, 700, 0.01)
            write(UpVector)
            end
            Camera:SetUp(UpVector)
        else
            tmp3 = false
        end

        if(tmp1 == false and tmp2 == false and tmp3 == false) then
            InOptions = true
        end

    end

    if(InOptions) then
        OptionsMode(dt)
    end

    if(BackMove) then
        InOptions = false
        tmp1 = true
        tmp2 = true
        tmp3 = true
        currPos = mTransform:GetWorldPosition()    
        if(currPos:x() < 0 or currPos:y() < 0 or currPos:z() < 0) then
            x = currPos:x() - (OptionsPos:x() * dt)
            y = currPos:y() - (OptionsPos:y() * dt)
            z = currPos:z() - (OptionsPos:z() * dt)
            mTransform:SetWorldPosition(Vector3(x,y,z))
            if(currPos:x() > 0 or currPos:y() > 0 or currPos:z() > 0) then
            mTransform:SetWorldPosition(Vector3(0,0,0))
            end
        else
            tmp1 = false
        end
        LA = Camera:GetLookAt()
        if(LA:x() < 0 or LA:y() > 0 or LA:z() > -1) then
            x = LA:x() - (OptionsLookAtVector:x() * dt)
            y = LA:y() - (OptionsLookAtVector:y() * dt)
            z = LA:z() - (OptionsLookAtVector:z() * dt)
            if(LA:x() > 0 or LA:y() < 0 or LA:z() < -1) then
                x = 0
                y = 0
                z = -1
            end
            Camera:SetLookAt(Vector3(x,y,z))
        else
            Camera:SetLookAt(Vector3(0,0,-1))
            tmp2 = false
        end
        if(UpVector:x() > LookAt:x() or UpVector:z() < LookAt:z()) then
            x = UpVector:x() - (OptionsUpVector:x() * dt)
            y = UpVector:y() - (OptionsUpVector:y() * dt)
            z = UpVector:z() - (OptionsUpVector:z() * dt)
            UpVector.x = x
            UpVector.y = y
            UpVector.z = z

            if(UpVector:x() < LookAt:x() or UpVector:z() > LookAt:z()) then
            UpVector = Vector3(-3.00, 700.0, 1.0)
            end
            Camera:SetUp(UpVector)
        else
            tmp3 = false
        end

        if(tmp1 == false and tmp2 == false and tmp3 == false) then
            transition = false
            BackMove = false 
        end
    end
else
    if(ConfirmationSelect == 0) then
        YesScaling(dt)
    elseif(ConfirmationSelect == 1) then
        NoScaling(dt)
    end

    if(ControllerPress("Right")) then
        ButtonChangeSFXEmitter:SetAndPlayAudioClip(ButtonChangeSFX)
        ConfirmationSelect = ConfirmationSelect + 1
        if(ConfirmationSelect >= 2) then 
            ConfirmationSelect = 0
        end
        if(ConfirmationSelect == 0) then
            NoMesh:SetColor(Color(1.0,1.0,1.0,1.0))
            YesMesh:SetColor(Color(1.0,0.0,0.0,1.0))
            NoXform:SetWorldScale(Vector3(1.50, 1.0, 1.0))
        elseif(ConfirmationSelect == 1) then
            NoMesh:SetColor(Color(1.0,0.0,0.0,1.0))
            YesMesh:SetColor(Color(1.0,1.0,1.0,1.0))
            YesXform:SetWorldScale(Vector3(2.0, 1.0, 1.0))
        end
    elseif(ControllerPress("Left")) then
        ButtonChangeSFXEmitter:SetAndPlayAudioClip(ButtonChangeSFX)
        ConfirmationSelect = ConfirmationSelect - 1
        if(ConfirmationSelect < 0) then 
            ConfirmationSelect = 1
        end
        if(ConfirmationSelect == 0) then
            NoMesh:SetColor(Color(1.0,1.0,1.0,1.0))
            YesMesh:SetColor(Color(1.0,0.0,0.0,1.0))
            NoXform:SetWorldScale(Vector3(1.50, 1.0, 1.0))
        elseif(ConfirmationSelect == 1) then
            NoMesh:SetColor(Color(1.0,0.0,0.0,1.0))
            YesMesh:SetColor(Color(1.0,1.0,1.0,1.0))
            YesXform:SetWorldScale(Vector3(2.0, 1.0, 1.0))
        end
    end
    if(GetControllerInput() == 0) then
        if(IsKeyPressed(KEY_SPACE)) then
            if(ConfirmationSelect == 0) then
                SceneQuit()
            elseif(ConfirmationSelect == 1) then
                StartMesh:SetColor(Color(1,1,1,1))
                OptionsMesh:SetColor(Color(1,1,1,1))
                ExitMesh:SetColor(Color(1,1,1,1))
                AreYouSureMesh:SetColor(Color(1.0,1.0,0.0,0.0))
                YesMesh:SetColor(Color(1.0,1.0,1.0,0.0))
                NoMesh:SetColor(Color(1.0,1.0,1.0,0.0))
                inConfirmationMode = false
            end
        end
    else
        if(ControllerPress("Shoot") or ControllerPress("Jump")) then
            if(ConfirmationSelect == 0) then
                SceneQuit()
            elseif(ConfirmationSelect == 1) then
                StartMesh:SetColor(Color(1,1,1,1))
                OptionsMesh:SetColor(Color(1,1,1,1))
                ExitMesh:SetColor(Color(1,1,1,1))
                AreYouSureMesh:SetColor(Color(1.0,1.0,0.0,0.0))
                YesMesh:SetColor(Color(1.0,1.0,1.0,0.0))
                NoMesh:SetColor(Color(1.0,1.0,1.0,0.0))
                inConfirmationMode = false
            end
        end
    end
end

end