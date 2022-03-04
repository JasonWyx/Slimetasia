spawnTime = 2
wordBlockPrefab = "WordBlock"

local GO = nil
local trans = nil
local block = nil

local timer = 0

function Constructor()
  timer = spawnTime
  GO = owner
  trans = GO:GetComponent("Transform")
end

function OnUpdate(dt)
  if (timer > 0) then
    timer = timer - dt
  else
    timer = spawnTime
    
    block = CreatePrefab(wordBlockPrefab)
    block:GetComponent("Transform"):SetWorldPosition(GO:GetComponent("Transform"):GetWorldPosition())
    
    blockRot = block:GetComponent("Transform"):GetWorldRotation()
    blockRot = Vector3(blockRot:x(), blockRot:y() + GO:GetComponent("Transform"):GetWorldRotation():y(), blockRot:z())
    block:GetComponent("Transform"):SetWorldRotation(blockRot)
    
    wordBlockScript = block:GetLuaScript("WordBlock.lua")    
    wordBlockScript:SetVariable("direction", trans:UpwardVector())
	  wordBlockScript:CallFunction("MoveBlock");
  end
end