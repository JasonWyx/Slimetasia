--settings
local up = {}
local down = {}
local left = {}
local right = {}

local upSize = 0
local downSize = 0
local leftSize = 0
local rightSize = 0

receiveVariable = false
receiveVariableName = "gaga"
receiveVariableNames = {"P1"}
receiveVariableSize = 0
toGet = ""
currentName = 0

showDirection = false
allowInteraction = false
levelName = ""
levelPref = ""

function Constructor()
  GO = owner
  if(GO:Name() == "P11") then
    up = nil
    
    write(PlayerPref_GetBool("ClearedTekong"))
    if (PlayerPref_GetBool("ClearedTekong")) then
      downSize = 16
      for i=1,downSize,1 do
          down[i] = "P1" .. ToString(i)
      end
      down[16] = "P21"
    end
    
    allowInteraction = true
    levelName = "Level_Tekong"
    levelPref = "Level_Tekong"
  end
  
  if(GO:Name() == "P21") then
    allowInteraction = true
    downSize = 15
    for i = 15,1,-1 do
        down[16-i] = "P1" .. ToString(i)
    end
    
    if (PlayerPref_GetBool("ClearedChinaTown")) then
      upSize = 5
      for i = 1,upSize,1 do
          up[i] = "P2" .. ToString(i)
      end
    end
    
    
    levelName = "ChinatownVideo"
    levelPref = "Level_ChinaTown"
  end
  
  
  if(GO:Name() == "P25") then
    allowInteraction = true
    downSize = 4
    for i = 4,1,-1 do
        down[5-i] = "P2" .. ToString(i)
    end
    levelName = "ChangiVideo"
    levelPref = "Level_Changi"
  end
end

function OnUpdate(dt)
end

function GetObject()
  currentName = 0
  receiveVariable = false
  if(toGet == "up" and up ~= nil) then
    receiveVariable = true
    receiveVariableNames = up
    receiveVariableSize = upSize
  elseif (toGet == "down" and down ~= nil) then
    receiveVariable = true
    receiveVariableNames = down
    receiveVariableSize = downSize
  elseif (toGet == "left" and left ~= nil) then
    receiveVariable = true
    receiveVariableNames = left
    receiveVariableSize = leftSize
  elseif (toGet == "right" and right ~= nil) then
    receiveVariable = true
    receiveVariableNames = right
    receiveVariableSize = rightSize
  end
end

function NextName()
  currentName = currentName + 1
  receiveVariableName = receiveVariableNames[currentName]
end