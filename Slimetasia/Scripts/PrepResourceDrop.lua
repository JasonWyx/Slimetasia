-- VARIABLES ===================================================================
-- Amount used in place of passing arguments to a function
resourceAmount = 0
coinComparison = 5
prefabName = "Coin"
position = Vector3()
constructed = false
drop = nil
framesToDelay = 2
-- FUNCTIONS ===================================================================
function OnUpdate(dt)
  if (not constructed) then
    drop = CreatePrefab(prefabName)
    constructed = true
  end
  if (framesToDelay > 0) then
    framesToDelay = framesToDelay - 1
  else
    CreateResource()
    owner:Destroy()
  end
end

function CreateResource()
  script = drop:GetLuaScript(prefabName .. "Behaviour.lua") -- .. is concating the strings together
  drop:GetComponent("Transform"):SetWorldPosition(position)
  script:SetVariable("gold", resourceAmount)
end