-- VARIABLES ===================================================================
resources = 100

-- Amount used in place of passing arguments to a function
amount = 0.0
gold_sfx = "Player_Pickup"

local player_GO

-- FUNCTIONS ===================================================================
function Constructor()
  player_GO = owner:GetLayer():GetObject("Sound")
  write("Player Resource Management Constructed!")
  --write("Number of resources: ", #resources)
  --for i = 1, #resources
  --  do
  --    write("Resource ", i, " amount: ", resources[i])
  --  end
  levelName        = PlayerPref_GetString("CurrentLevel")
  resources = PlayerPref_GetInteger("StartResource", levelName)
  write("Resource amount: ", resources)
end

function OnUpdate(dt)
  --For debug
  if (IsKeyPressed(KEY_0)) then
    --for i = 1, #resources
    --do
    --  write("Resource ", i, " amount: ", resources[i])
    --end
    write("Resource amount: ", resources)
  end
  
  --For debug (reduce resources by 10)
  if (IsKeyPressed(KEY_6)) then
    --for i = 1, #resources
    --do
    --  if (resources[i] > 0) then
    --    resources[i] = resources[i] - 10
    --  end
    --end
    
    resources = resources - 10
  end
  
  --For cheat (Reset resources to 1000)
  if (IsKeyPressed(KEY_7)) then
    --for i = 1, #resources
    --do
      --resources[i] = 1000
      resources = 1000
    --end
  end
end

function OnCollisionEnter(other)  
end

function AddGold()
  --resources[1] = resources[1] + amount
  --write(resources[1])
  resources = resources + amount
  --pos = player_GO:GetComponent("Transform"):GetWorldPosition()
  --playAudio = CreatePrefab("PlayAudioAndDie")
  --playAudio:GetComponent("Transform"):SetWorldPosition(pos)
  --playAudioScript = playAudio:GetLuaScript("PlayAudioAndDie.lua")
  --playAudioScript:SetVariable("audioClipName", gold_sfx)
  --playAudioScript:CallFunction("PlayAudio")
  
  AudioSystem_PlayAudioAtLocation(gold_sfx, player_GO:GetComponent("Transform"):GetWorldPosition())
  write("Resource left: ", resources)
  --Resets amount after function is done
  amount = 0
end

function ReduceGold()
  --resources[1] = resources[1] - amount
  --write(resources[1])
  resources = resources - amount
  write("Resource left: ", resources)
  --Resets amount after function is done
  amount = 0
end