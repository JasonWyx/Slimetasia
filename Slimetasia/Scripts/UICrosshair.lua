GO_Player = null
Player_Transform = null
Player_PlayerScript = null

GO = nil
trans = nil
mr = nil
pos = Vector3()
lateStart = true

 inverse= true
function Constructor()

  GO = owner
  trans = GO:GetComponent("Transform")  
  
  -- Find player
  GO_Player = CurrentLayer():GetObject("Player")
  if (GO_Player ~= nil) then
    Player_Transform      = GO_Player:GetComponent("Transform")
    Player_PlayerScript  = GO_Player:GetLuaScript("PlayerScript.lua")
    Player_ShootScript  = GO_Player:GetLuaScript("PlayerShooting.lua")
  end
  mr =  GO:GetComponent("MeshRenderer")
  
  pos = trans:GetWorldPosition()
  if(GO:Name() == "UI_Cursor") then
    inverse = false
  end
end

function OnUpdate(dt)
  if (Player_PlayerScript:GetVariable("currentMode")) then
      if(inverse == false) then
        trans:SetWorldPosition(pos)
      else
        trans:SetWorldPosition(Vector3(0,0,0))
      end
  else
      if(inverse == false) then
        trans:SetWorldPosition(Vector3(0,0,0))
      else
        trans:SetWorldPosition(pos)
      end
  end
  
  if(Player_ShootScript ~= nil) then
    if(Player_ShootScript:GetVariable("lookingAtEnemy")==true)then
      mr:SetColor(Color(1.0,0.0,0.0,1.0))
    else
      mr:SetColor(Color(1.0,1.0,1.0,1.0))
    end
  end
end