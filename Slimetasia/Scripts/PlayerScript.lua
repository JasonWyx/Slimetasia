-- VARIABLES ===================================================================

-- [Health and damage]
maxhealth    = 20
health       = 20
damage       = 0

-- [Components]
TRANSFORM = nil
Camera_PlayerCameraScript = nil

-- [Scripts]
Script_PlayerController  = nil
Script_PlayerTrapPlacing = nil
Script_PlayerShooting    = nil

-- [Respawn]
respawn_Pos = Vector3()

-- [Mode true - shooting, false - trap placing]
lockModeChanging = false
currentMode = true

-- RespawnTimer
local BaseRespawnTimer = 2.1
local RespawnTimer = BaseRespawnTimer
IsDead = false
local rigidbody = nil
local zeroVector = Vector3(0,0,0)
local ScreenBlockScript = nil

-- Flicker
local GO_playerModel      = nil
local playerModel_meshRenderer  = nil
local timer_flicker       = 0.2
local interval_flicker    = 0.1
local color_flicker       = Color(1, 0, 0, 1)
local color_OriginalColor = nil
local isGodMode           = false

-- Vibration
local vibrateControl      = false
local vibrateTimer        = 0
local vibrateDuration     = 0.2

local GUNDAMBITCHES = false

--Sfx
local hurtSfx = {"SFX_PlayerHurt1", "SFX_PlayerHurt2", "SFX_PlayerHurt3", "SFX_PlayerHurt4"}
local changeModeSfx = "SFX Button Change"

-- FUNCTIONS ===================================================================
function Constructor()
  -- Find components
  TRANSFORM = owner:GetComponent("Transform")
  
  -- Find scripts
  Script_PlayerController   = owner:GetLuaScript("PlayerController.lua")
  Script_PlayerTrapPlacing  = owner:GetLuaScript("PlayerTrapPlacing.lua")
  Script_PlayerShooting     = owner:GetLuaScript("PlayerShooting.lua")
  Camera_GO                 = CurrentLayer():GetObject("Camera")
  Camera_PlayerCameraScript = Camera_GO:GetLuaScript("PlayerCamera.lua")
  if(Camera_GO) then
    ScreenBlockScript = Camera_GO:GetLuaScript("ScreenBlock.lua")
  end


  -- Find respawn values
  respawn_Pos = TRANSFORM:GetWorldPosition()
  
  GO_playerModel = CurrentLayer():GetObject("PlayerModel")
  playerModel_meshRenderer  = GO_playerModel:GetComponent("MeshRenderer")

  color_OriginalColor  = playerModel_meshRenderer:GetColor()
  
  rigidbody = owner:GetComponent("RigidBody")

end

function OnUpdate(dt)
  if(IsKeyPressed(KEY_9)) then
    playerModel_meshRenderer:SetMesh("gundam_standing")
    playerModel_meshRenderer:SetDiffuseTexture("Gundam4kOptimised")
    GO_playerModel:GetComponent("Transform"):SetWorldScale(Vector3(0.125,0.125,0.125))
    GO_playerModel:GetComponent("Transform"):SetWorldRotation(Vector3(0,90,0))
    GUNDAMBITCHES = true
  end

  -- Change mode
  if (not lockModeChanging and ControllerUp("SwitchMode")) then
    currentMode = not currentMode
    Camera_PlayerCameraScript:CallFunction("ChangeCameraMode")
    Script_PlayerController:CallFunction("ChangeCameraMode")
    AudioSystem_PlayAudioAtLocation(changeModeSfx, GO_playerModel:GetComponent("Transform"):GetWorldPosition(), 0.5)
  end
  
  if(IsKeyPressed(KEY_2)) then
     isGodMode = not isGodMode
  end
  
  -- Damage flicker
  if (timer_flicker > 0) then
    timer_flicker = timer_flicker - dt
    if (timer_flicker <= 0) then
      playerModel_meshRenderer:SetColor(color_OriginalColor)
    end
  end
  
  -- Vibration
  if (vibrateControl) then
    if (vibrateTimer > 0) then
      vibrateTimer = vibrateTimer - dt
      vibrateStrength = (maxhealth - health / maxhealth) + 0.2
      if (vibrateStrength > 1) then 
        vibrateStrength = 1
      end
      ControllerVibrateRight(vibrateStrength)
    else
      ControllerVibrateRight(0)
      vibrateControl = false
    end
  end

  -- Respawn Timer
  if(IsDead) then
    RespawnTimer = RespawnTimer - dt
    if(RespawnTimer < BaseRespawnTimer - 1.0) then
      if(ScreenBlockScript) then
        ScreenBlockScript:CallFunction("Clear")
      end
    end
    if(RespawnTimer < 0.0) then
      RespawnTimer = BaseRespawnTimer
      IsDead = false
      Resume()
      Respawn()
    end
  end

end

function OnCollisionEnter(other)
  if (other:Name() == "DeathBox") then
    IsDead = true
    Pause()
    health = 0
    rigidbody:SetVelocity(zeroVector)
  end
end

function OnCollisionExit(other)
end

-- Deal Damage =================================================================
function DealDamage()
  if (isGodMode) then
    return 
  end
  
  health = health - damage
  write("CURRENT HEALTH: ", health)
  damage = 0
  
  --playAudio = CreatePrefab("PlayAudioAndDie")
  --playAudioScript = playAudio:GetLuaScript("PlayAudioAndDie.lua")
  --playAudioScript:SetVariable("audioClipName", "SFX_PlayerHurt")
  --playAudioScript:CallFunction("PlayAudio")
  
  AudioSystem_PlayAudioAtLocation(hurtSfx[RandomRangeInt(1, #hurtSfx)], TRANSFORM:GetWorldPosition())
  
  Flicker()
  
  vibrateControl = true
  vibrateTimer = vibrateDuration
  
  if (health <= 0) then
    IsDead = true
    Pause()
    health = 0
    rigidbody:SetVelocity(zeroVector)
    ControllerVibrateRight(0)
  end
end

-- Respawn =====================================================================
function Respawn()
  TRANSFORM:SetWorldPosition(respawn_Pos)
  health      = maxhealth
  
  playerModel_meshRenderer:SetColor(color_OriginalColor)
end

function Flicker()
  if (timer_flicker <= 0) then
    timer_flicker = interval_flicker
    playerModel_meshRenderer:SetColor(color_flicker)
  end
end

-- PAUSE/RESUME ================================================================
function Pause()
  Script_PlayerController:CallFunction("Pause")
  Script_PlayerTrapPlacing:CallFunction("Pause")
  Script_PlayerShooting:CallFunction("Pause")
end

function Resume()
  Script_PlayerController:CallFunction("Resume")
  Script_PlayerTrapPlacing:CallFunction("Resume")
  Script_PlayerShooting:CallFunction("Resume")
end

function PlayWalkAnim()
  Script_PlayerController:CallFunction("PlayWalkAnim")
end

function PlayIdleAnim()
  Script_PlayerController:CallFunction("PlayIdleAnim")
end

-- Trap placing ================================================================
function ToggleSpawnTrapOnGrid()
  Script_PlayerTrapPlacing:CallFunction("ToggleSpawnTrapOnGrid")
end