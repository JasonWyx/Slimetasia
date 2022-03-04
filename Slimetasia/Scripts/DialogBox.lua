local b = true
local InstructionsLayer = nil
local GOXform = nil
local GOMesh = nil
local center = Vector3(0, -4.0, -16.0)
local offscreen = Vector3(0.0, 0.0, -16.0)
local Textoffscreen = Vector3(0.0, 0.0, -15.950)
local scale = Vector3(0.0, 0.0, 0.0)
local movein = false
local moveout = false
local viewportWidth = 0
local viewportHeight = 0
local stopY = -4.0
local outY = -10
local speed = 50.0
up = false
local TextObj = nil
local GOTextRenderer = nil 
local TextTransform = nil
local NameObj = nil
local NameTextRenderer = nil
local NameTransform = nil

local StaticCounter = 0
local ReadlineCounter = 0

local dialogSpinner = nil
local dialogSpinnerMesh = nil
local dialogSpinnerXform = nil

done = false
NameText = ""
LineOfText = ""

AudioName = ""
AudioPos = Vector3()

local Audios = {}
local AudioPosition = {}
local AudioCounter = 0

local AudioPlayCounter = 0

local Texts = {}

local leftin = false
local leftout = false
local leftImage = nil
local leftTransform = nil
local leftMesh = nil
local leftPos = Vector3(-5.0, -0.50, -16.5)
local leftRot = Vector3(90.0, 0.0, 0.0)
local leftScale = Vector3(4.5, 1.0, 7.5)
leftTexture = ""
leftNextTexture = ""
local leftChanging = false
local ChangeLeft = false

local rightImage = nil
local rightTransform = nil
local rightMesh = nil
local rightPos = Vector3(5.0, -1.00, -16.5)
local rightin = false
local rightout = false
rightTexture = ""
rightNextTexture = ""
local rightChanging = false
local ChangeRight = false
completed = false

local AEmitter = nil

-- max length = 42

local globalDT = 0.0

function AddAudio()
    Audios[AudioCounter] = AudioName
    AudioPosition[AudioCounter] = AudioPos
    AudioCounter = AudioCounter + 1
end

function ChangeLeftTexture()
    if(not leftin) then return end
    ChangeLeft = true
end

function ChangeRightTexture()
    if(not rightin) then return end
    ChangeRight = true
end

function LeftChangeTexture()
    if(not ChangeLeft) then return end
    col = leftMesh:GetColor()
    a = col:a()
    if(not leftChanging) then
        if(a > 0.5) then
            a = a - globalDT
            if(a < 0.5) then
                a = 0.5
                leftMesh:SetDiffuseTexture(leftNextTexture)
                leftChanging = true
            end
            write(a)
            leftMesh:SetColor(Color(col:r(), col:g(), col:b(), a))

        end
    else
        if(a < 1.0) then
            a = a + globalDT
            if(a > 1.0) then
                a = 1.0
                leftChanging = false
                ChangeLeft = false
            end
            leftMesh:SetColor(Color(col:r(), col:g(), col:b(), a))
        end
    end
end

function RightChangeTexture()
    if(not ChangeRight) then return end
    col = rightMesh:GetColor()
    a = col:a()
    if(not rightChanging) then
        if(a > 0.5) then
            a = a - globalDT
            if(a < 0.5) then
                a = 0.5
                rightMesh:SetDiffuseTexture(rightNextTexture)
                rightChanging = true
            end
            write(a)
            rightMesh:SetColor(Color(col:r(), col:g(), col:b(), a))

        end
    else
        if(a < 1.0) then
            a = a + globalDT
            if(a > 1.0) then
                a = 1.0
                rightChanging = false
                ChangeRight = false
            end
            rightMesh:SetColor(Color(col:r(), col:g(), col:b(), a))
        end
    end
end

function TestDialogBox()
    LineOfText = "Hi all, Ha Ba Ba"
    AddText()
    LineOfText = "You are chosen to be a pokemon master"
    AddText()
    LineOfText = "But first please tell me if you are a male \nor a kiryuu"
    AddText()
    LineOfText = "Great, now what is your name?"
    AddText()
end

function FadeInRight()
    if(rightTexture ~= "") then
        rightMesh:SetDiffuseTexture(rightTexture)
    end
    rightin = true
end

function FadeOutRight()
    rightout = true
end

function FadeInLeft()
    if(leftTexture ~= "") then
        leftMesh:SetDiffuseTexture(leftTexture)
    end
    leftin = true
end

function FadeOutLeft()
    leftout = true
end

function ClearText()
    StaticCounter = 0
    AudioCounter = 0
end

function AddText()
    Texts[StaticCounter] = LineOfText
    StaticCounter = StaticCounter + 1
end

function ChangeName()
    NameTextRenderer:SetText(NameText)
end

function GoIn()
    movein = true
    completed = false
end

function GoOut()
    moveout = true
end

function PlayText()
if(up and ReadlineCounter <= StaticCounter) then
    if(ReadlineCounter == 0) then
        textyText = Texts[ReadlineCounter]
        GOTextRenderer:SetText(textyText)
        ReadlineCounter = ReadlineCounter + 1
        if(AudioPlayCounter < AudioCounter) then
            if(Audios[AudioPlayCounter] ~= "") then
                AEmitter:SetAndPlayAudioClip(Audios[AudioPlayCounter], AudioPosition[AudioPlayCounter])
            end
            AudioPlayCounter = AudioPlayCounter + 1
        end
    end
    if((ControllerPress("Shoot") or ControllerPress("Jump")) and ReadlineCounter < StaticCounter) then
        GOTextRenderer:SetText(Texts[ReadlineCounter])
        ReadlineCounter = ReadlineCounter + 1
        if(AudioPlayCounter < AudioCounter) then
            if(Audios[AudioPlayCounter] ~= "") then
                AEmitter:SetAndPlayAudioClip(Audios[AudioPlayCounter], AudioPosition[AudioPlayCounter])
            end
            AudioPlayCounter = AudioPlayCounter + 1
        end
    elseif(ReadlineCounter == StaticCounter) then
        -- write("DONE")
        done = true
        -- ReadlineCounter = 0
    end
end

end

function ShowText()
    textPosY = TextTransform:GetWorldPosition():y()
    if(textPosY ~= -3.400) then
        TextTransform:SetWorldPosition(TextTransform:GetWorldPosition():x(), -3.400, TextTransform:GetWorldPosition():z())
    end
    NamePosY = NameTransform:GetWorldPosition():y()
    if(NamePosY ~= -2.350) then
        NameTransform:SetWorldPosition(NameTransform:GetWorldPosition():x(), -2.350, NameTransform:GetWorldPosition():z())
    end
end

function HideText()
    textPosY = TextTransform:GetWorldPosition():y()
    if(textPosY ~= outY) then
        TextTransform:SetWorldPosition(TextTransform:GetWorldPosition():x(), outY, TextTransform:GetWorldPosition():z())
        NameTransform:SetWorldPosition(NameTransform:GetWorldPosition():x(), outY, NameTransform:GetWorldPosition():z())
    end
end

function ShowLeft()
    if(ChangeLeft) then return end
    if(not up) then return end
    if(not leftin) then return end 
    if(leftout) then return end
    col = leftMesh:GetColor()
    if(col:a() < 1.0) then
        a = col:a() + (globalDT * 2)
        if(a > 1.0) then 
            a = 1.0
            leftin = true
        end
        leftMesh:SetColor(Color(col:r(), col:g(), col:b(), a))
    end
end

function DoNotShowLeft()
    if(ChangeLeft) then return end
    if(not leftout) then return end
    col = leftMesh:GetColor()
    if(col:a() > 0.0) then
        a = col:a() - (globalDT * 2)
        if(a < 0.0) then 
            a = 0.0
            leftout = false
            leftin = false
        end
        leftMesh:SetColor(Color(col:r(), col:g(), col:b(), a))
    end
end


function ShowRight()
    if(ChangeRight) then return end
    if(not up) then return end
    if(not rightin) then return end 
    if(rightout) then return end
    col = rightMesh:GetColor()
    if(col:a() < 1.0) then
        a = col:a() + (globalDT * 2)
        if(a > 1.0) then 
            a = 1.0
            rightin = true
        end
        rightMesh:SetColor(Color(col:r(), col:g(), col:b(), a))
    end
end

function DoNotShowRight()
    if(ChangeRight) then return end
    if(not rightout) then return end
    col = rightMesh:GetColor()
    if(col:a() > 0.0) then
        a = col:a() - (globalDT * 2)
        if(a < 0.0) then 
            a = 0.0
            rightout = false
            rightin = false
        end
        rightMesh:SetColor(Color(col:r(), col:g(), col:b(), a))
    end
end


function OnUpdate(dt)
    globalDT = dt
if(b) then
    AEmitter = owner:AddComponent("AudioEmitter")
    AEmitter:SetChannelGroup("SFX")
    InstructionsLayer = GetLayer("InstructionsLayer")
    if(InstructionsLayer == nil) then
        InstructionsLayer = CreateLayer("InstructionsLayer")
    end
    cam = InstructionsLayer:GetObject("MainCamera")
    camComp = cam:GetComponent("Camera")
    lookat = Vector3(0,0,0-1)
    camComp:SetLookAt(lookat)
    camComp:SetUICam(true)
    GO = InstructionsLayer:Create("Textboard")
    GOXform = GO:AddComponent("Transform")
    GOMesh = GO:AddComponent("MeshRenderer")
    GOMesh:SetDiffuseTexture("KING_DIALOGBOX")

    -- Text Obj
    TextObj = InstructionsLayer:Create("Text")
    TextTransform = TextObj:AddComponent("Transform")
    GOTextRenderer = TextObj:AddComponent("TextRenderer")
    GOTextRenderer:SetColor(Color(0.0, 0.0, 0.0, 1.0))
    GOTextRenderer:SetFontSize(0.45)
    GOTextRenderer:SetFont("BD_CARTOON_SHOUT")

    -- Name Obj
    NameObj = InstructionsLayer:Create("Name")
    NameTransform = NameObj:AddComponent("Transform")
    NameTextRenderer = NameObj:AddComponent("TextRenderer")
    NameTextRenderer:SetColor(Color(243/255, 199/255, 104/255, 1.0))
    NameTextRenderer:SetFontSize(0.70)
    NameTextRenderer:SetFont("BD_CARTOON_SHOUT")
    NameTextRenderer:SetText("Jun Ze")
    NameTextRenderer:SetOutline(true)
    NameTextRenderer:SetOutlineSize(1.0)

    -- left Image
    leftImage = InstructionsLayer:Create("leftImage")
    leftTransform = leftImage:AddComponent("Transform")
    leftTransform:SetWorldPosition(leftPos)
    leftTransform:SetWorldRotation(leftRot)
    leftTransform:SetWorldScale(leftScale)
    leftMesh = leftImage:AddComponent("MeshRenderer")
    leftMesh:SetMesh("planeMesh")
    leftMesh:SetColor(Color(1.0, 1.0, 1.0, 0.0))

    -- right Image
    rightImage = InstructionsLayer:Create("rightImage")
    rightTransform = rightImage:AddComponent("Transform")
    rightMesh = rightImage:AddComponent("MeshRenderer")
    rightTransform:SetWorldPosition(rightPos)
    rightTransform:SetWorldRotation(leftRot)
    rightTransform:SetWorldScale(leftScale)
    rightMesh:SetMesh("planeMesh")
    rightMesh:SetColor(Color(1.0, 1.0, 1.0, 0.0))

    GOMesh:SetMesh("planeMesh")
    scale = Vector3(16.0, 3.0, 5.00)
    GOXform:SetWorldScale(scale)
    offscreen.y = outY
    GOXform:SetWorldPosition(offscreen)
    GOXform:SetWorldRotation(Vector3(90,0,0))
    Textoffscreen = offscreen
    Textoffscreen.x = (-scale:x() / 2.0) + 1.30
    TextTransform:SetWorldPosition(Textoffscreen)
    Textoffscreen.x = (-scale:x() / 2.0) + 1.00
    NameTransform:SetWorldPosition(Textoffscreen)

    -- dialog spinner
    dialogSpinner = InstructionsLayer:Create("Spinner")
    dialogSpinnerXform = dialogSpinner:AddComponent("Transform")
    dialogSpinnerMesh = dialogSpinner:AddComponent("MeshRenderer")
    dialogSpinnerMesh:SetColor(Color(0.95, 0.78, 0.4, 0))
    dialogSpinnerMesh:SetMesh("dialog_spinner")
    dialogSpinnerXform:SetWorldPosition(Vector3(6.400, -4.800, -15.000))
    dialogSpinnerXform:SetWorldScale(Vector3(0.4, 0.4, 0.4))

    b = false
    -- TestDialogBox()
    write("DialogBox.lua : Creation Complete")
end

-- if(IsKeyPressed(KEY_W)) then GoIn() end
-- if(IsKeyPressed(KEY_S)) then GoOut() end

-- if(IsKeyPressed(KEY_A)) then
   -- ClearText()
   -- LineOfText = "Fuck Off You all"
   -- AddText()
   -- LineOfText = "Chee Bye I got sign the form meh"
   -- AddText()
   -- LineOfText = "Get the fucking lights off me"
   -- AddText()
-- end

-- if(IsKeyPressed(KEY_D)) then FadeInLeft() end
-- if(IsKeyPressed(KEY_Q)) then FadeInRight() end
-- if(IsKeyPressed(KEY_Z)) then ChangeLeftTexture() end
-- if(IsKeyPressed(KEY_X)) then ChangeRightTexture() end



if(movein and not moveout) then
    transformY = GOXform:GetWorldPosition():y()
    if(transformY >= stopY) then
        movein = false
        up = true
        dialogSpinnerMesh:SetColor(Color(0.95, 0.78, 0.4, 1.0))
        ShowText()
    else
        transformY = transformY + (speed * dt)
        if(transformY > stopY) then transformY = stopY end
        pos = Vector3(GOXform:GetWorldPosition():x(), transformY, GOXform:GetWorldPosition():z())
        GOXform:SetWorldPosition(pos)
    end
end

rot = dialogSpinnerXform:GetWorldRotation()
rot.y = rot:y() + (dt * 90)
if(rot:y() > 360.0) then rot.y = rot:y() - 360 end
dialogSpinnerXform:SetWorldRotation(rot)

if(moveout) then
    if(leftin or rightin) then
        FadeOutLeft()
        FadeOutRight()
    else
        transformY = GOXform:GetWorldPosition():y()
        if(transformY < outY) then
            moveout = false
            up = false
            done = false
            AudioPlayCounter = 0
            ReadlineCounter = 0
            completed = true
            leftout = false
            rightout = false
        else
            dialogSpinnerMesh:SetColor(Color(0.95, 0.78, 0.4, 0))
            HideText()
            transformY = transformY - (speed * dt)
            pos = Vector3(GOXform:GetWorldPosition():x(), transformY, GOXform:GetWorldPosition():z())
            GOXform:SetWorldPosition(pos)
        end
    end
end

PlayText()

ShowLeft()
ShowRight()
DoNotShowLeft()
DoNotShowRight()
LeftChangeTexture()
RightChangeTexture()

end