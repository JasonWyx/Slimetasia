
local speed = 70.0
local current = Vector3(0,-100,0)
local currentTrans = nil
local target = nil
local targetTrans = nil

local moving = false
local step = nil
local stepVec = nil
local currentIndex = 0

local current_level = nil

local tekong_cleared = false
local chinatown_cleared = false
local changi_cleared = false
local do_something = false

local tekong_china = {"P11","P12","P13","P14","P15","P17","P18","P19","P110","P111","P112","P113","P114","P115",}
local china_changi = {"P21", "P22","P23","P24","P25",}
local movement_table = {}
local table_count = 0

function Constructor()
  GO = owner
  TRANS = GO:GetComponent("Transform")
  current_level = PlayerPref_GetString("CurrentLevel")
  tekong_cleared = PlayerPref_GetBool("ClearedTekong")
  chinatown_cleared = PlayerPref_GetBool("ClearedChinaTown")
  changi_cleared = PlayerPref_GetBool("ClearedChangi")

  if(tekong_cleared == true and chinatown_cleared == true and changi_cleared == true) then
    do_something = false
  else
    do_something = true
    if(current_level == "Level_Tekong") then
      current = GO:GetLayer():GetObject("P11")
    elseif(current_level == "Level_ChinaTown") then
      current = GO:GetLayer():GetObject("P21")
    end
    currentTrans = current:GetComponent("Transform"):GetWorldPosition()
    currentTrans.x = currentTrans:x() + 5
    currentTrans.y = currentTrans:y() + 5
    currentTrans.z = currentTrans:z() + 10
  end
  if(do_something == true) then
    if(tekong_cleared == true and chinatown_cleared == false) then
      movement_table = tekong_china
      table_count = 14
    else
      movement_table = china_changi
      table_count = 5
    end
    target = GO:GetLayer():GetObject(movement_table[2])
    targetTrans = target:GetComponent("Transform")
    currentIndex = 2
  end
  TRANS:SetWorldPosition(currentTrans)
  
  --write("do_something = ")
  --write(do_something)
  --write("current_level = ")
  --write(current_level)
  --write("tekong_cleared = ")
  --write(tekong_cleared)
  --write("chinatown_cleared = ")
  --write(chinatown_cleared)
  --write("changi_cleared = ")
  --write(changi_cleared)
end

function OnUpdate(dt)
  if(do_something == true) then
    autoMove(dt)
  end
  --write("TRANS:GetWorldPosition() = ")
  --write(TRANS:GetWorldPosition())
  write("table_count = ")
  write(table_count)
end

function autoMove(dt)
  --Set a direction to move towards
  direction = targetTrans:GetWorldPosition() - TRANS:GetWorldPosition()
  step = dt * speed
  stepVec = direction:Normalized()*dt*speed*0.5
  
  --Set a new rotation
  directiontwo = direction;
  rotation = TRANS:GetWorldRotation()
  rotation.y = directiontwo:PolarAngle():y() + 90
  rotation.z = directiontwo:PolarAngle():z()
  rotation.x = directiontwo:PolarAngle():x() + 270
  TRANS:SetWorldRotation(rotation)
  
  --if (currentAnimation == nil or currentAnimation ~= animWeaponRun01) then
  --    currentAnimation = animWeaponRun01
  --    playerModel_meshAnimator:Play(animWeaponRun01)
  --end
  if(direction:Length() > step + 1.5) then
    pos = TRANS:GetWorldPosition() +  stepVec
    TRANS:SetWorldPosition(pos)
    write("moving")
  else
    write("new point")
    pos = targetTrans:GetWorldPosition()
    write("1")
    TRANS:SetWorldPosition(pos)
    write("2")
    currentIndex = currentIndex + 1;
    write("3")
    if(currentIndex <= table_count) then
    write("first")
      target = GO:GetLayer():GetObject(locations[currentIndex])
      targetTrans = target:GetComponent("Transform")
    else
    write("second")
      current = target
      target = nil
      locationsSize = -1
      currentIndex = 1
    end
  end
end
