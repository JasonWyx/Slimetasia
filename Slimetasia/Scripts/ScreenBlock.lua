local b = true
local screenLayer = nil
local tailsmanXform = {}
local tailsmanMesh = {}
local tailsmanActive = {}
local tailsmanTimer = {}

function Clear()
    for i = 1, 12 do
        tailsmanTimer[i] = 0.0
        tailsmanActive[i] = false
        tailsmanMesh[i]:SetColor(Color(1,1,1,0))
    end
end

function BlockScreen()
    for i = 1, 12 do
        if(tailsmanActive[i] ~= true) then
            tailsmanMesh[i]:SetColor(Color(1,1,1,1))
            time = RandomRange(15, 20)
            tailsmanTimer[i] = time
            tailsmanActive[i] = true
            return
        end
    end
end

function OnUpdate(dt)
    if(b) then
        screenLayer = GetLayer("screenLayer")
        if(screenLayer == nil) then
            screenLayer = CreateLayer("screenLayer")
        end
        cam = screenLayer:GetObject("MainCamera")
        camComp = cam:GetComponent("Camera")
        lookat = Vector3(0,0,-1)
        camComp:SetLookAt(lookat)
        camComp:SetUICam(true)
        for i = 1, 12 do
            GO = screenLayer:Create("tailsman")
            tailsmanXform[i] = GO:AddComponent("Transform")
            tailsmanMesh[i] = GO:AddComponent("MeshRenderer")
            tailsmanMesh[i]:SetMesh("planeMesh")
            tailsmanMesh[i]:SetDiffuseTexture("talisman_part")
            tailsmanMesh[i]:SetColor(Color(1,1,1,0))
            tailsmanActive[i] = false
        end

        pos = Vector3(3.307, 0.781, -5.02)
        rot = Vector3(90.0, 0.0, 15.564)
        scale = Vector3(1.0, 1.0, 3.0)
        tailsmanXform[1]:SetWorldPosition(pos)
        tailsmanXform[1]:SetWorldRotation(rot)
        tailsmanXform[1]:SetWorldScale(scale)

        pos = Vector3(3.48, -0.990, -5.03)
        rot = Vector3(90.0, 0.0, -6.741)
        tailsmanXform[2]:SetWorldPosition(pos)
        tailsmanXform[2]:SetWorldRotation(rot)
        tailsmanXform[2]:SetWorldScale(scale)

        pos = Vector3(2.218, 0.749, -5.004)
        rot = Vector3(90.0, -0.0, -2.282)
        tailsmanXform[3]:SetWorldPosition(pos)
        tailsmanXform[3]:SetWorldRotation(rot)
        tailsmanXform[3]:SetWorldScale(scale)

        pos = Vector3(2.044, -1.797, -5.001)
        rot = Vector3(90.0, 0.0, 11.196)
        tailsmanXform[4]:SetWorldPosition(pos)
        tailsmanXform[4]:SetWorldRotation(rot)
        tailsmanXform[4]:SetWorldScale(scale)

        pos = Vector3(1.529, 1.954, -5.005)
        rot = Vector3(90.0, 0.0, -22.482)
        tailsmanXform[5]:SetWorldPosition(pos)
        tailsmanXform[5]:SetWorldRotation(rot)
        tailsmanXform[5]:SetWorldScale(scale)

        pos = Vector3(1.011, 2.091, -5.006)
        rot = Vector3(90.00, -0.0, -77.908)
        tailsmanXform[6]:SetWorldPosition(pos)
        tailsmanXform[6]:SetWorldRotation(rot)
        tailsmanXform[6]:SetWorldScale(scale)

        pos = Vector3(-1.423, 2.466, -5.007)
        rot = Vector3(90.0, 0.0, 72.299)
        tailsmanXform[7]:SetWorldPosition(pos)
        tailsmanXform[7]:SetWorldRotation(rot)
        tailsmanXform[7]:SetWorldScale(scale)

        pos = Vector3(-1.905, -1.328, -5.008)
        rot = Vector3(90.0, 0.0, -13.082)
        tailsmanXform[8]:SetWorldPosition(pos)
        tailsmanXform[8]:SetWorldRotation(rot)
        tailsmanXform[8]:SetWorldScale(scale)

        pos = Vector3(-2.169, 0.984, -5.009)
        rot = Vector3(90.0, 0.0, 9.122)
        tailsmanXform[9]:SetWorldPosition(pos)
        tailsmanXform[9]:SetWorldRotation(rot)
        tailsmanXform[9]:SetWorldScale(scale)

        pos = Vector3(-2.823, 0.071, -5.0011)
        rot = Vector3(90.0, 0.0, 0.631)
        tailsmanXform[10]:SetWorldPosition(pos)
        tailsmanXform[10]:SetWorldRotation(rot)
        tailsmanXform[10]:SetWorldScale(scale)

        pos = Vector3(-3.442, -1.446, -5.0012)
        rot = Vector3(90.0, 0.0, 6.178)
        tailsmanXform[11]:SetWorldPosition(pos)
        tailsmanXform[11]:SetWorldRotation(rot)
        tailsmanXform[11]:SetWorldScale(scale)

        pos = Vector3(-3.608, 1.322, -5.0013)
        rot = Vector3(90.0, 0.0, -19.953)
        tailsmanXform[12]:SetWorldPosition(pos)
        tailsmanXform[12]:SetWorldRotation(rot)
        tailsmanXform[12]:SetWorldScale(scale)

        b = false

        write("ScreenBlock.lua : Creation Complete")
    end

    for i = 1, 12 do
        if(tailsmanActive[i] == true) then
            tailsmanTimer[i] = tailsmanTimer[i] - dt
            if(tailsmanTimer[i] <= 5.0) then
              alpha = tailsmanTimer[i] / 5.0
              tailsmanMesh[i]:SetColor(Color(1,1,1,alpha))
            end
            if(tailsmanTimer[i] <= 0.0) then
                tailsmanTimer[i] = 0.0
                tailsmanActive[i] = false
                tailsmanMesh[i]:SetColor(Color(1,1,1,0))
            end
        end
    end


end