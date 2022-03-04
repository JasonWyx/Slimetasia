-- VARIABLES ===================================================================
local eventsScript = nil
local isConstruct  = false

-- FUNCTIONS ===================================================================
--Note: Constructor might call multiple copies, hence do not create stuff in
--Constructor
function Constructor()

end

--ONLY CALLS ONCE
function MyConstructor()
  eventsGO     = CurrentLayer():GetObject("GameLogic")
  eventsScript = eventsGO:GetLuaScript("Event_Tekong.lua")
  
  transform    = owner:GetComponent("Transform")
  pos          = transform:GetWorldPosition()
  pos.y        = 2.0
  transform:SetWorldPosition(pos)
end

function OnUpdate(dt)
  if(isConstruct == false)then
    MyConstructor()
    isConstruct = true
  end
end

function OnCollisionEnter(other)
  if(other:Name() == "Player") then
    eventsScript:CallFunction("ActivateCharger")
    owner:Destroy()
  end
end