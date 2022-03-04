timeToLive = 2
speed = 10
direction = Vector3()

GO = nil
rb = nil
timer = nil
trans = nil

function Constructor()
  timer = timeToLive;
  GO = owner
  trans = GO:GetComponent("Transform")  
  rb = GO:GetComponent("RigidBody")
end

function OnUpdate(dt)
  if (timer > 0) then
    timer = timer - dt
  else
    GO:Destroy()
  end
end

function MoveBlock()
  rb:SetVelocity(direction * speed) 
end

function OnCollisionEnter(other)
  if(other:Name() == "Slime")then
    script = other:GetLuaScript("EnemyBehavior.lua")
    --brainwashStatus = script:SetVariable("brainwash", true)
    script:CallFunction("Brainwash")
    GO:Destroy()
  end
end