local start = false
local maxTimer = 2.0
timerIn = maxTimer
timerOut = maxTimer
local b = true
local deathLayer = nil
local GO = nil
local GOMesh = nil
local coreLogic = nil
local Player = nil
local playerScript = nil

function StartDeath()
    start = true
end

local function MerlionTakesDamageBitch()
    if(coreLogic == nil) then return end
    baseHealth = coreLogic:GetVariable("BaseCoreHealth")
    damage = baseHealth * 0.2
    for i = 1, damage do
        coreLogic:CallFunction("DealDamage")
    end
end

function OnUpdate(dt)
    if(b) then
        Player = owner:GetLayer():GetObject("Player")
        if(Player ~= nil) then
            playerScript = Player:GetLuaScript("PlayerScript.lua")
        end
        coreLogic = owner:GetLuaScript("CoreLogic.lua")
        deathLayer = GetLayer("DeathLayer")
        if(deathLayer == nil) then
            deathLayer = CreateLayer("DeathLayer")
        end
        cam = deathLayer:GetObject("MainCamera")
        camComp = cam:GetComponent("Camera")
        lookat = Vector3(0, 0, -1)
        camComp:SetLookAt(lookat)
        camComp:SetUICam(true)
        GO = deathLayer:Create("Death")
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
        write("Death.lua : Creation Complete")
    end

    if(playerScript ~= nil) then
        deads = playerScript:GetVariable("IsDead")
        if(deads) then
            StartDeath()
        end
    end

    if(start) then
        timerIn = timerIn - dt
        if(timerIn < 0.0) then 
            timerIn = 0.0
        end
        alpha = 1.0 - (timerIn / maxTimer)
        c = Color(0, 0, 0, alpha)
        GOMesh:SetColor(c)
        -- write(alpha)
        if(timerIn == 0) then
            timerOut = timerOut - dt
            alpha = (timerOut / maxTimer)
            if(timerOut < 0.0) then
                timerIn = maxTimer 
                timerOut = maxTimer
                start = false
                alpha = 0.0
                MerlionTakesDamageBitch()
            end
            c = Color(0, 0, 0, alpha)
            GOMesh:SetColor(c)
        end
    
    end

end