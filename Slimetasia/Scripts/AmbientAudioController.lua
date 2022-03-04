--USAGE
-- Add an empty object with transform and tag it as "Ambient". 
-- The object will be automatically added to the list of points that will be randomly chosen to play a random sfx from a list found below


-- VARIABLES ===================================================================
local constructed = false

local audioPoints = nil
local player = nil
local player_transform = nil
distanceAway = 100
local playIntervals = 
{
  3,
  4,
  5,
  6
}
local playIndex    = 1

local timer        = 0
local randomDelay  = 0

local tekong_sfx = 
{
  "Tekong_Ocean_Wavebreak_1",
  "Tekong_Ocean_Wavebreak_2",
  "Tekong_Ocean_Wavebreak_3",
  "Tekong_Ocean_Wavebreak_4"
}

local chinatown_sfx = 
{
  "Amb_Wind_Lowloop_-AllLoc-",
  "Amb_Wind_Normloop_-AllLoc-",
  "Chinatown_Wind_Normalloop"
}

local changi_sfx = 
{
  "ambient_airplane_1",
  "ambient_airplane_2"
}

local yishun_sfx = 
{
  "Amb_Birds_Tek-Chin-Chan-Yis"
}

local sfx_list = nil
-- FUNCTIONS ===================================================================
function Constructor()
end

function MyConstructor()
  layer = owner:GetLayer()
  audioPoints = layer:GetObjectsListByTag("Ambient")
  player = layer:GetObject("Player")
  player_transform = player:GetComponent("Transform")
  
  levelName   = PlayerPref_GetString     ("CurrentLevel")
  
  if (levelName == "Level_Tekong") then
    sfx_list = tekong_sfx
    playIndex = 1
    distanceAway = 100
  elseif (levelName == "Level_ChinaTown") then
    sfx_list = chinatown_sfx
    playIndex = 2
    distanceAway = 30
  elseif (levelName == "Level_Changi") then
    sfx_list = changi_sfx
    playIndex = 3
    distanceAway = 150
  elseif (levelName == "Level_Yishun") then
    sfx_list = yishun_sfx
    playIndex = 4
  else
    write("AmbientAudioController: Your level is not valid. Please try again.")
  end
end

function OnUpdate(dt)
  if (constructed == false) then
    MyConstructor()
	constructed = true
  end
  
  if (timer > playIntervals[playIndex] + randomDelay) then
    rand = RandomRangeInt(1, #sfx_list)
    
    --Pick randomly from all those that is close enough to player
    arr = {}
    curr = 1
    for i = 1, #audioPoints do
      if (player_transform ~= nil) then
        distanceVec = audioPoints[i]:GetComponent("Transform"):GetWorldPosition() - player_transform:GetWorldPosition()
        if (VectorLength(distanceVec) < distanceAway) then
          arr[curr] = audioPoints[i]
          curr = curr + 1
        end
      end
    end
    randpt = RandomRangeInt(1, #arr)
    
    --write("Play index: ", playIndex)
    --write("Playing ", sfx_list[rand], " at ", arr[randpt]:GetComponent("Transform"):GetWorldPosition(), ", ", arr[randpt]:Name())
    AudioSystem_PlayAudioAtLocation(sfx_list[rand], arr[randpt]:GetComponent("Transform"):GetWorldPosition(), 1, 50, 500)
    timer = 0
    randomDelay = RandomRange(0,2)
  else
    timer = timer + dt
  end
end
