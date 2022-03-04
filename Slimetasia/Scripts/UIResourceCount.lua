--settings
local player = nil
local resourceManagement  = nil
local trapInfo = nil
local c = nil
local prevColor = Color(1,1,1,1) 


GO = nil
TRANS = nil
  
local resourceCount = nil
local useCount = nil
local rcCenter = nil
local rcCenterTrans = nil
local rcTrans = nil
local ucTrans = nil
local rcbTrans = nil
local ucbTrans = nil
local rcText = nil
local ucText = nil
local rcbText = nil
local ucbText = nil

function Constructor()
    GO = owner
    TRANS = GO:GetComponent("Transform")
    player = CurrentLayer():GetObject("Player")
    resourceManagement =    player:GetLuaScript("PlayerResourceManagement.lua")
    trapInfo =    player:GetLuaScript("PlayerTrapPlacing.lua")
    c = owner:GetComponent("MeshRenderer")
    prevColor = c:GetColor()
    
    resourceCount = GO:GetLayer():GetObject("RCountN")
    useCount = GO:GetLayer():GetObject("UCountN")
    rcCenter = GO:GetLayer():GetObject("RCCenter")
    rcTrans = resourceCount:GetComponent("Transform")
    ucTrans = useCount:GetComponent("Transform")
    rcbTrans =  GO:GetLayer():GetObject("RCountNB"):GetComponent("Transform")
    ucbTrans =  GO:GetLayer():GetObject("UCountNB"):GetComponent("Transform")
    rcCenterTrans = rcCenter:GetComponent("Transform")
    rcText = resourceCount:GetComponent("TextRenderer")
    ucText = useCount:GetComponent("TextRenderer")    
    rcbText =  GO:GetLayer():GetObject("RCountNB"):GetComponent("TextRenderer")
    ucbText =  GO:GetLayer():GetObject("UCountNB"):GetComponent("TextRenderer")
end

function OnUpdate(dt)
  resourcesLeft = resourceManagement:GetVariable("resources")
   trapInfo:CallFunction("GetCurrentTrapCost")
  cost = trapInfo:GetVariable("currentTrapCost")
  --Replace 1.0 with number
  if(resourcesLeft <   cost) then
    rcText:SetColor(Color(1,0,0,1))
  else  
    rcText:SetColor(Color(1,1,1,1))
  end
  
  targetPos = TRANS:GetWorldPosition()
  targetPos.x =  targetPos:x() - (TRANS:GetWorldScale():x()/10)
  targetPos.z = -11
  rcTrans:SetWorldPosition(targetPos)
  targetPos.z = -11.05
  rcbTrans:SetWorldPosition(targetPos)
  
  targetPos = TRANS:GetWorldPosition()
  targetPos.x =  targetPos:x() + (TRANS:GetWorldScale():x()/10)
  targetPos.z = -11
  ucTrans:SetWorldPosition(targetPos)
  targetPos.z = -11.05
  ucbTrans:SetWorldPosition(targetPos)
  
  targetPos = TRANS:GetWorldPosition()
  targetPos.z = -11
  rcCenterTrans:SetWorldPosition(targetPos)
  
  rcText:SetText(resourcesLeft)
  ucText:SetText(cost)
  rcbText:SetText(resourcesLeft)
  ucbText:SetText(cost)
  
end
