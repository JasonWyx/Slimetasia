-- VARIABLES ===================================================================
local isConstruct  = false

-- FUNCTIONS ===================================================================
--Note: Constructor might call multiple copies, hence do not create stuff in
--Constructor
function Constructor()
end

--ONLY CALLS ONCE
function MyConstructor()
  transform    = owner:GetComponent("Transform")
  pos          = transform:GetWorldPosition()
  pos.y        = 0.5
  transform:SetWorldPosition(pos)
end

function OnUpdate(dt)
  if(isConstruct == false)then
    MyConstructor()
    isConstruct = true
  end
end