-- VARIABLES ===================================================================
local disable = false

-- [Tekong Variables]
local GameLogic = nil
local TekongScript = nil

-- [Settings]
range     = 1000
gridScale = 1
onlyPlaceInGrid = false

-- [Traps]
local traps         = {"Trap_Blockade", "Trap_Napalm", "Trap_BarbedWire", "Trap_Remove"}
local trapCost      = { 2.0           ,  20.0         ,  5.0             ,  0.0        }
currentTrapCost     = 0
trapSelected  = 1
Placement_Allowed = true
local numOfRotation = 0
local changeMesh        = true
local RotateAllowed     = true
local trapToRemove      = nil
local PrefabRotation    = Vector3()

--For UI
currentTrapName    = "Trap_Blockade"

-- [Components]
local GO_Camera                             = nil
local Camera_Camera                         = nil
local GO_Core                               = nil
local Core_transform                        = nil

local GO_TrapPlacing                        = nil
local TrapPlacing_Transform                 = nil
local TrapPlacing_MeshRenderer              = nil
local TrapPlacing_TrapPlacingColliderScript = nil

local GO_Player                             = nil
local Player_Transform                      = nil
local Player_PlayerScript                   = nil
local Player_PlayerController               = nil
-- [PathFinding]
local FindNewPath                           = false
-- [Trap placing]
local hitInfo = nil

--Sfx
local sfx_NoResource = {"SFX_PlayerNoResource1", "SFX_PlayerNoResource2", "SFX_PlayerNoResource3", "SFX_PlayerNoResource6"}
local errorSfx       = "SFX_PlayerNoResource5"
local nextSfx       = "TickSound"
local prevSfx       = "TickSound"
-- [TrapPlacing Tag]

                                                                                -- Debug
                                                                                local scriptOk      = true
                                                                                local drawDebugLine = true
                                                                                local colorWHITE   = Color(1.0, 1.0, 1.0, 1.0)
                                                                                local colorBLACK   = Color(0.0, 0.0, 0.0, 1.0)
                                                                                local colorRED     = Color(1.0, 0.0, 0.0, 1.0)
                                                                                local colorGREEN   = Color(0.0, 1.0, 0.0, 1.0)
                                                                                local colorBLUE    = Color(0.0, 0.0, 1.0, 1.0)
                                                                                local colorYELLOW  = Color(1.0, 1.0, 0.0, 1.0)
                                                                                local colorYELLOW  = Color(1.0, 1.0, 0.0, 1.0)
                                                                                local colorCYAN    = Color(0.0, 1.0, 1.0, 1.0)
                                                                                local colorMagenta = Color(1.0, 0.0, 1.0, 1.0)
-- FUNCTIONS ===================================================================
function Constructor()
  -- Find camera
  GO_Camera = CurrentLayer():GetObject("Camera")
  if (GO_Camera ~= nil) then
    Camera_Camera = GO_Camera:GetComponent("Camera")
  end
  
  -- Find player
  GO_Player = CurrentLayer():GetObject("Player")
  if (GO_Player ~= nil) then
    Player_Transform         = GO_Player:GetComponent("Transform")
    Player_PlayerScript      = GO_Player:GetLuaScript("PlayerScript.lua")
    Player_PlayerController  = GO_Player:GetLuaScript("PlayerController.lua")
  end
  
  -- Find TrapPlacing
  GO_TrapPlacing = CurrentLayer():GetObject("TrapPlacingCollider")
  if (GO_TrapPlacing ~= nil) then
    TrapPlacing_Transform    = GO_TrapPlacing:GetComponent("Transform")
    TrapPlacing_MeshRenderer = GO_TrapPlacing:GetComponent("MeshRenderer")
    TrapPlacing_TrapPlacingColliderScript = GO_TrapPlacing:GetLuaScript("TrapPlacingCollider.lua")
  end
  
  -- setup tekong scripts if anything
  GameLogic = owner:GetLayer():GetObject("GameLogic")
  if(GameLogic ~= nil) then TekongScript = GameLogic:GetLuaScript("Event_Tekong.lua") end
  GO_Core      = CurrentLayer():GetObject("Core")
  -- Find other components
  Core_transform   = GO_Core:GetComponent("Transform")
end

function OnUpdate(dt)
  -- If player is in build mode
  if (not Player_PlayerScript:GetVariable("currentMode") and not disable) then
    
    -- For changing of proxy
    if (changeMesh) then
      -- Normal traps
      if (trapSelected ~= #traps) then
        trapPrefab = CreatePrefab(traps[trapSelected])
        trapPrefab:SetName("trapProj")
        
        meshName = trapPrefab:GetComponent("MeshRenderer"):GetMesh()
        TrapPlacing_MeshRenderer:SetMesh(meshName)
        prefabScale = trapPrefab:GetComponent("Transform"):GetWorldScale()
        TrapPlacing_Transform:SetWorldScale(prefabScale)
        PrefabRotation = trapPrefab:GetComponent("Transform"):GetWorldRotation()
        
        trapPrefab:Destroy()
        
      -- Recycle trap
      else
        TrapPlacing_MeshRenderer:SetMesh("SmallBin")
        TrapPlacing_Transform:SetWorldScale(Vector3(2, 2, 2))
      end
      
      write(traps[trapSelected], "'s cost: ", trapCost[trapSelected])
      
      --To prevent setting of proxy variables every frame
      changeMesh = false
    end
    
    -- Detect spawn point
    DetectSpawnPoint()
    
    -- Make trap
    if (ControllerPress("Shoot")) then
      resourceScript = GO_Player:GetLuaScript("PlayerResourceManagement.lua")
      resourceAmt = resourceScript:GetVariable("resources")
      
      if (resourceAmt >= trapCost[trapSelected]) then
        -- Normal traps
        if(trapSelected ~= #traps) then
          if (Placement_Allowed) then
            SpawnTrap(trapSelected)
            write("[TRAP PLACING] Placed trap")
          else
            AudioSystem_PlayAudioAtLocation(errorSfx, GO_Player:GetComponent("Transform"):GetWorldPosition())
            write("[TRAP PLACING] Invalid Position")
          end
        -- Remove trap
        else
          RemoveTrap()
        end
      -- Not enough resouce
      else
        AudioSystem_PlayAudioAtLocation(sfx_NoResource[RandomRangeInt(1, #sfx_NoResource)], GO_Player:GetComponent("Transform"):GetWorldPosition())
        write("[TRAP PLACING] Insufficient Resource")
      end
    end
    
    -- Select trap
    if (ControllerPress("SelectPrev")) then
      trapSelected = trapSelected - 1
      if (trapSelected <= 0) then
        trapSelected = # traps
      end
      changeMesh = true
      AudioSystem_PlayAudioAtLocation(prevSfx, GO_Player:GetComponent("Transform"):GetWorldPosition(), 0.2)
      currentTrapName = traps[trapSelected]
    end
    
    if (ControllerPress("SelectNext")) then
      trapSelected = trapSelected + 1
      if (trapSelected > # traps) then
        trapSelected = 1
      end
      changeMesh = true
      AudioSystem_PlayAudioAtLocation(nextSfx, GO_Player:GetComponent("Transform"):GetWorldPosition(), 0.2)
      currentTrapName = traps[trapSelected]
    end
    
    -- Rotation
    if (ControllerPress("Rotate")) then
      if (numOfRotation < 3) then
        numOfRotation = numOfRotation + 1
      else
        numOfRotation = 0
      end
    end
    playerRot   = Player_Transform:GetWorldRotation()
    remainder   = Mod(ToInt(playerRot:y()), 90)
    playerRot.y = ToInt(playerRot:y()) - remainder
    TrapPlacing_Transform:SetWorldRotation(PrefabRotation + Vector3(0, numOfRotation * 90, 0) + playerRot)
    
  -- if player is not in build mode
  else
    TrapPlacing_Transform:SetWorldPosition(Vector3(0, -100, 0))
  end
  
  --if(ControllerPress("Shoot")) then
  --  clicked = false
  --end
end

-- DEBUGRAY ====================================================================
function DebugRayCast (s, dir, range, c)
  if (drawDebugLine) then
    DebugDrawLine(s, s + dir * range, c)
  end
end

-- Detection ===================================================================
function DetectSpawnPoint()
  Placement_Allowed = false
  
  startPos  = Player_Transform:GetWorldPosition() - Player_Transform:ForwardVector() 
  direction = Camera_Camera:GetLookAt()
  hitInfo      = RayCast(startPos, direction, range, "Player", "TrapPlacingCollider", "Bullet")
  trapToRemove = nil
  buildingPos  =  Vector3(0, -100, 0)
  
  -- If there is a gameobject
  if (hitInfo:GameObject() ~= nil) then
                                                                          DebugRayCast(hitInfo:Point(), Vector3(0, 1, 0), 100, colorBLUE)
    -- If i'm not removing any traps
    if (trapSelected ~= #traps) then
      
      if ((hitInfo:GameObject():Tag() == "Ground" or hitInfo:GameObject():Name() == "TrapPlacing") and
           hitInfo:GameObject():Name() ~= "ExcludeTrapPlacement" and
           hitInfo:GameObject():Name() ~= "Box"                  and
           hitInfo:GameObject():Name() ~= "BoxDoor") then
        -- Use AI grid
        buildingPos   = hitInfo:Point()
        buildingPos   = AISystem_RetrieveGridPos(buildingPos)
        if (buildingPos:y() ~= 300) then
          if (hitInfo:GameObject():Name() == "TrapPlacing") then
            buildingPos.y = hitInfo:Point():y() - 0.2
          else
            buildingPos.y = hitInfo:Point():y() + 0.2
          end
          
          -- Place only in specific grid
          if ((onlyPlaceInGrid and hitInfo:GameObject():Name() == "TrapPlacing") or (not onlyPlaceInGrid)) then
            Placement_Allowed = true
          end
        end
      end
    
     -- Removing traps
    else
      if (hitInfo:GameObject():Tag() == "Trap") then
        transform   = hitInfo:GameObject():GetComponent("Transform")
        worldPos    = transform:GetWorldPosition()
        buildingPos = AISystem_RetrieveGridPosAll(worldPos)
        buildingPos.y = worldPos:y() + 0.2
        trapToRemove = hitInfo:GameObject()
      elseif (hitInfo:GameObject():Tag() == "Ground") then
        -- Use AI grid
        buildingPos = hitInfo:Point()
        buildingPos = AISystem_RetrieveGridPos(buildingPos)
        buildingPos.y = hitInfo:Point():y() + 0.2
      end
    end
  end
  
  -- Set trap placing proxy position
  TrapPlacing_Transform:SetWorldPosition(buildingPos)
end

-- SPAWN TRAP ==================================================================
function SpawnTrap(index)
  -- Check if detect 
  if (hitInfo:GameObject()        == nil      or
      (hitInfo:GameObject():Tag()  ~= "Ground" and
       hitInfo:GameObject():Name() ~= "TrapPlacing")) then
    return
  end
  
  -- Check if can spawn
  if (TrapPlacing_TrapPlacingColliderScript:GetVariable("validPlacement") == false) then
    return
  end
  
  write("TrapPlacing : ",traps[trapSelected] )
  
  -- Move trap placing collider position
  buildingPos = TrapPlacing_Transform:GetWorldPosition()
  TrapPlacing_Transform:SetWorldPosition(Vector3(0, -100, 0))
  
  -- Find trap spawn position
 -- buildingPos   = hitInfo:Point()
 -- buildingPos.x = Round(buildingPos:x() / gridScale) * gridScale
 -- buildingPos.z = Round(buildingPos:z() / gridScale) * gridScale
  
  -- Spawn trap
  newTrap = CreatePrefab(traps[trapSelected])
  newTrap:GetComponent("Transform"):SetWorldPosition(buildingPos)
  trapRot = TrapPlacing_Transform:GetWorldRotation()
  newTrap:GetComponent("Transform"):SetWorldRotation(trapRot)
  placementParticle = CreatePrefab("TrapPlacementParticle")
  placementParticle:GetComponent("Transform"):SetWorldPosition(buildingPos)
  --playAudio = CreatePrefab("PlayAudioAndDie")
  --playAudioScript = playAudio:GetLuaScript("PlayAudioAndDie.lua")
  --playAudioScript:SetVariable("audioClipName", "SFX_TrapPlacing.ogg")
  --playAudioScript:CallFunction("PlayAudio")
  
  AudioSystem_PlayAudioAtLocation("SFX_TrapPlacing.ogg", GO_Player:GetComponent("Transform"):GetWorldPosition())
  
  resourceScript = GO_Player:GetLuaScript("PlayerResourceManagement.lua")
  resourceAmt = resourceScript:GetVariable("resources")
  
  write("Resource amount ", resourceAmt)
  if (resourceAmt >= trapCost[trapSelected]) then
    resourceScript:SetVariable("amount", trapCost[trapSelected])
    resourceScript:CallFunction("ReduceGold")
  else
    write("Not enough resources!")
    Placement_Allowed = false
  end
  
  -- Recompute all Slimes path
  if(traps[trapSelected] == "Trap_Blockade")then
    slimeList = CurrentLayer():GetObjectsListByTag("Slime")
    InvalidGrid(newTrap)
    if(slimeList ~= nil)then
      AISystem_SetPathChanged(false)
      slimeSize = #slimeList
      for i = 1, slimeSize
      do
        enemyScript = slimeList[i]:GetLuaScript("PathFindLogic.lua")
        if(enemyScript ~= nil)then
          enemyScript:CallFunction("ComputePath_Indv")
        end
      end
    end
  end
end

function InvalidGrid(go)
   blockPos = go:GetComponent("Transform"):GetWorldPosition()
   AISystem_SetPosValid(blockPos,false)
end

function ValidGrid(go)
  blockPos = go:GetComponent("Transform"):GetWorldPosition()
  AISystem_SetPosValid(blockPos)
end

function GetCurrentTrapCost()
  currentTrapCost = trapCost[trapSelected]
  return trapCost[trapSelected]
end

function RemoveTrap()
  -- Check if there is any traps
  if(trapToRemove ~= nil) then
    trapTransform = trapToRemove:GetComponent("Transform")
    trapPosition  = trapTransform:GetWorldPosition()
    AISystem_SetPosValid(trapPosition,true)
    pos = 0
    for i = 1, #traps
    do
      if (traps[i] == trapToRemove:Name()) then
        pos = i
      end
    end
    
    if (trapToRemove:Name() == "Trap_BarbedWire") then
      Player_PlayerController:CallFunction("ExitBarbedWire")
    end
    trapToRemove:Destroy()
    trapToRemove = nil
	
	script = owner:GetLuaScript("PlayerResourceManagement.lua")
    if (script ~= nil) then
      script:SetVariable("amount", Round(trapCost[pos] * 0.5))
      script:CallFunction("AddGold")
    else
      write("PlayerResourceManagement luascript not found in Player!")
    end
	
  end
end

-- PAUSE/RESUME ================================================================
function Pause()
  disable = true
end

function Resume()
  disable = false;
end
--unused function below
function UpdateNewOriginalPath()
  --write("WENT IN UPDATED")
  sptList = CurrentLayer():GetObjectsListByName("Spawn_Point")
  boolArr = PlayerPref_GetBoolArray("isPathFound")
  for j = 1, #sptList
  do
    --write("testing")
    spf = sptList[j]:GetComponent("PathFinding")
    if(spf ~= nil) then
      script = sptList[j]:GetLuaScript("TrailPath_Logic.lua")
      indexNum = script:GetVariable("spawnIndex")
      AISystem_ReplacePathAtIndex(indexNum)
      pathFound = spf:FindPath(Core_transform:GetWorldPosition(),false)
    --  write("IS valid path found?", pathFound)
      boolArr[indexNum] = pathFound
    end
  end
  PlayerPref_SetBoolArray("isPathFound", boolArr)
end

-- Trap placing ================================================================
function ToggleSpawnTrapOnGrid()
  onlyPlaceInGrid = not onlyPlaceInGrid
end