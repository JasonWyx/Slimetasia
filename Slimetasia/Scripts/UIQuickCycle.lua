GO_Player = nil
Player_Transform = nil
Player_PlayerScript = nil
Player_TrapScript = nil

GO = nil
trans = nil

pos = Vector3()

UI_TrapText = nil
UI_TrapTextTR = nil
selectedTrap = 0
maxAmtTraps = 0


cam = nil

lateConstructor = true
iconL = nil
iconR = nil
iconLTrans = nil
iconRTrans = nil

resourceCountFrame = nil
rcfTrans = nil

function Constructor()
  GO = owner
  trans = GO:GetComponent("Transform")
 -- Find player
  GO_Player = CurrentLayer():GetObject("Player")
  if (GO_Player ~= nil) then
    Player_Transform      = GO_Player:GetComponent("Transform")
    Player_PlayerScript  = GO_Player:GetLuaScript("PlayerScript.lua")
    Player_TrapScript  = GO_Player:GetLuaScript("PlayerTrapPlacing.lua")
  end
  iconL           = GO:GetLayer():GetObject("UI_CycleLeft")
  iconR           = GO:GetLayer():GetObject("UI_CycleRight")
  iconLTrans      = iconL:GetComponent("Transform")
  iconRTrans      = iconR:GetComponent("Transform")
  
  
  
  resourceCountFrame = GO:GetLayer():GetObject("UI_Resource")
  rcfTrans = resourceCountFrame:GetComponent("Transform")
end

function OnUpdate(dt)   
  if(lateConstructor == true) then
    --Find respective traps
    UI_TrapText = GetLayer("UILayer"):GetObject("UI_TrapText")
    UI_TrapTextTrans = UI_TrapText:GetComponent("Transform")
    UI_TrapTextTR = UI_TrapText:GetComponent("TextRenderer")
    UpdateText()
    cam  =  GO:GetLayer():GetObject("MainCamera")
    lateConstructor = false
  end
  if(cam:GetActive()==false)then return end
  
  newpos = trans:GetWorldPosition()
  UI_TrapTextTrans:SetWorldPosition(newpos)
  
  if (Player_PlayerScript:GetVariable("currentMode") == true) then
      trans:SetWorldRotation(Vector3(0,0,0))  
      UI_TrapText:SetActive(false)
      iconL:SetActive(false)
      iconR:SetActive(false)
      return
  else
      trans:SetWorldRotation(Vector3(90,0,0))
      UI_TrapText:SetActive(true)
      iconL:SetActive(true)
      iconR:SetActive(true)
  end
   
  if (ControllerPress("SelectNext") or ControllerPress("SelectPrev")) then
    selectedTrap = Player_TrapScript:GetVariable("trapSelected")-1
    UI_TrapTextTR:SetText(Player_TrapScript:GetVariable("currentTrapName"))
    UpdateText()
  end
  
  if(iconL:GetActive() == true) then
    p = trans:GetWorldPosition()
    p.x = p:x() - trans:GetWorldScale():x()/2 + iconLTrans:GetWorldScale():x()/2
    iconLTrans:SetWorldPosition(p)
    p.x = p:x() + trans:GetWorldScale():x() - iconLTrans:GetWorldScale():x()/2 - iconRTrans:GetWorldScale():x()/2
    iconRTrans:SetWorldPosition(p)
  end
  newpos = rcfTrans:GetWorldPosition()
  newpos.y = newpos:y() + rcfTrans:GetWorldScale():z()/2.0 + trans:GetWorldScale():z()/2.0
  newpos.x = newpos:x() + rcfTrans:GetWorldScale():x()/2.0 - trans:GetWorldScale():x()/2.0
  trans:SetWorldPosition(newpos)
end

function UpdateText()
  t = Player_TrapScript:GetVariable("currentTrapName")
  if( t == "Trap_Blockade" ) then
    t = "Blockade"
  end
  if( t == "Trap_Napalm" ) then
    t = "Firecracker"
  end
  if( t == "Trap_BarbedWire" ) then
    t = "Barbed Wire"
  end
  if( t == "Trap_Remove" ) then
    t = "Remove"
  end
  UI_TrapTextTR:SetText(t)
end
