-- VARIABLES ===================================================================

--[shooting variables]
direction    = Vector3()
bulletSpeed  = 120
bulletYForce = 300
damage       = 0
distApart    = 0.0
originalDirection  =  Vector3(0,0,-1)
--[core]
GO_Core            = nil
core_Transform     = nil
distanceToCore_Min = 0.5

--[components]
component_Transform = nil
component_Rigidbody = nil
--Vy_0 = (0.5gt^2 ) / t, t = 1 for now
gravity = 30.0
yVel = 0.5 * gravity

camera = nil 

function Constructor()
  GO_Core             = CurrentLayer():GetObject("Core")
  core_Transform      = GO_Core:GetComponent("Transform")
  component_Rigidbody = owner:GetComponent("RigidBody")
  component_Transform = owner:GetComponent("Transform")
  camera = CurrentLayer():GetObject("Camera"):GetLuaScript("ScreenBlock.lua")
end

function MoveBullet()
  myD = direction * distApart
  component_Rigidbody:SetVelocity(  Vector3(myD:x(), yVel, myD:z()))
end

function OnUpdate(dt)
  -- Check if bullet is near the core
  --posBullet = component_Transform:GetWorldPosition()
  --corePos   = core_Transform:GetWorldPosition()
  --distance  = posBullet:DistanceTo(corePos)
  --
  --if(distance < distanceToCore_Min) then
  --  script = GO_Core:GetLuaScript("CoreLogic.lua")
  --  script:CallFunction("DealDamage")
  --  owner:Destroy()
  --end
end

function OnCollisionEnter(go)
  if(go:Name() == "Trap_Blockade")then
      script = go:GetLuaScript("Blockade.lua")
      script:CallFunction("DealDamage")
      write("I hit, im range")
      owner:Destroy()
      return
  end
  
  if (go:Tag() == "Player") then
    script = go:GetLuaScript("PlayerScript.lua")
    script:SetVariable("damage", damage)
    script:CallFunction("DealDamage")
    camera:CallFunction("BlockScreen")
    camera:CallFunction("BlockScreen")
    camera:CallFunction("BlockScreen")
    
    owner:Destroy()
    return
  end
  
  if (go:Tag() ~= "Slime") then
    owner:Destroy()
  end
  
  if (go:Tag() == "Merlion") then
    script = go:GetLuaScript("CoreLogic.lua")
    script:CallFunction("DealDamage")
    owner:Destroy()
    return
  end
end