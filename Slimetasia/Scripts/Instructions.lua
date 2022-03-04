local b = true
local GO = nil
local InstructionsLayer = nil
local GOXform = nil
local GOMesh = nil
local center = Vector3(0, 0, -16.0)
local offscreen = Vector3(50.0, 0, -16.0)
local speed = -100.0
local exitscreen = -100
local reached = false
local exit = false
movein = false
completed = false

local camera_GO = nil
local cameraXform = nil
local enterSound = "Tutorial_Page_Paper_Rustle_In"
local exitSound  = "Tutorial_Page_Paper_Rustle_Out"
local myBoolean = false


-- texture names
local arrayoftextures = {}
currTexture = ""

function Start()
    if(currTexture ~= nil) then
        GOMesh:SetDiffuseTexture(currTexture)
    end
    movein = true
    completed = false
    if(cameraXform ~= nil) then
        pos = cameraXform:GetWorldPosition()
        if(myBoolean == false) then
            myBoolean = true
            AudioSystem_PlayAudioAtLocation(enterSound, pos)
        end
    end
end

function OnUpdate(dt)
    if(b) then
        InstructionsLayer = GetLayer("InstructionsLayer")
        if(InstructionsLayer == nil) then
            InstructionsLayer = CreateLayer("InstructionsLayer")
        end
        cam = InstructionsLayer:GetObject("MainCamera")
        camComp = cam:GetComponent("Camera")
        lookat = Vector3(0,0,-1)
        camComp:SetLookAt(lookat)
        camComp:SetUICam(true)
        GO = InstructionsLayer:Create("BlackBoard")
        GOXform = GO:AddComponent("Transform")
        GOMesh = GO:AddComponent("MeshRenderer")
        GOMesh:SetMesh("Blackboard")
        rotation = Vector3(0, 180, 0)
        GOXform:SetWorldRotation(rotation)
        scale = Vector3(0.75, 0.75, 0.75)
        GOXform:SetWorldScale(scale)
        GOXform:SetWorldPosition(offscreen)
        -- to be commented away
        -- currTexture = "KILLALLDUMMIES"
        -- GOMesh:SetDiffuseTexture(currTexture)
        -- end of to be commented
        b = false
        camera_GO = owner:GetLayer():GetObject("Camera")
        if(camera_GO ~= nil) then
            cameraXform = camera_GO:GetComponent("Transform")
        end
        write("Instructions.lua : Creation Complete")
    end
    currPos = GOXform:GetWorldPosition()
    if(movein) then
        if(currPos:x() ~= center:x()) then
            newX = currPos:x() + speed * dt
            if(newX < 0) then
                newX = 0
                reached = true
            end
            newPos = Vector3(newX, 0, -16.0)
            GOXform:SetWorldPosition(newPos)
        end
    end
    if(reached) then
        if(ControllerDown("Shoot") or ControllerDown("Jump")) then
            exit = true
            if(cameraXform ~= nil) then
                pos = cameraXform:GetWorldPosition()
                if(myBoolean) then
                    AudioSystem_PlayAudioAtLocation(exitSound, pos)
                    myBoolean = false
                end
            end
        end
    end
    if(exit) then
        if(currPos:x() ~= exitscreen) then
            exitX = currPos:x() + speed * dt
            if(exitX < exitscreen) then
                exitX = exitscreen
                reached = false
                exit = false
                movein = false
                completed = true
                GOXform:SetWorldPosition(offscreen)
            else
                exitPos = Vector3(exitX, 0, -16.0)
                GOXform:SetWorldPosition(exitPos)
            end
        end
    end


end
