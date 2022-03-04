-- VARIABLES ===================================================================

BlockadeHealth = 3

-- FUNCTIONS ===================================================================
function Constructor()

end

function OnUpdate(dt)
end

-- Deal Core Damage ========================================================================
function DealDamage()
  BlockadeHealth = BlockadeHealth - 1
  write("reduce health of blockade!")
  if(BlockadeHealth <= 0 )then
    DestroyBlockade()
  end
end


function DestroyBlockade()
   ValidGrid(owner)
   slimeList = CurrentLayer():GetObjectsListByTag("Slime")
   --Update all slimes
   for i = 1, #slimeList
   do
    if(slimeList[i]:Name() ~= "Slime_Spawner" and slimeList[i]:Name() ~= "Slime_Kamikaze")then 
     PathFindScript = slimeList[i]:GetLuaScript("PathFindLogic.lua")
     EnemyScript = slimeList[i]:GetLuaScript("EnemyBehavior.lua")
     if(EnemyScript ~=nil)then
      EnemyScript:SetVariable("targetBlockExist", false)
     end
     if(PathFindScript ~= nil)then
      PathFindScript:SetVariable("attackBlock", false)
      PathFindScript:CallFunction("ComputePath_Indv")
     end     
    end
   end
   --Update Shared path
  -- myPlayer = CurrentLayer():GetObject("Player")
  -- ptpscript = myPlayer:GetLuaScript("PlayerTrapPlacing.lua")
   --ptpscript:CallFunction("UpdateNewOriginalPath")
   owner:Destroy()
end

function ValidGrid()
  blockPos = owner:GetComponent("Transform"):GetWorldPosition()
    --mark as invalid
  AISystem_SetPosValid(blockPos,true)
end
