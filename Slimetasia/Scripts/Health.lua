-- NOTE : This is a general script do not add any additional info to this script
-- USAGE: script = GO:GetLuaScript("Health.lua")
--        script:SetVariable("damage", xxx)
--        script:CallFunction("DealDamge")
-- VARIABLES ===================================================================

-- [variables to add (User must change these)]
health         = 3.0
fullHP         = 3.0
damage         = 0.0  -- Damage taken everytime DealDamge is called
destroyOndeath = true

-- [components]
local owner_meshRenderer = nil

-- [settings]
local originalColor   = Color(1,1,1,1)
local flickerTimer    = 0.0
local flickerColor    = Color(1,0,0,1)
local flickerDuration = 0.2
local flickering      = false
-- [UI TakeDamage]
local popupScript = nil
--EnemySpawner
immune                = false
local secondForm      = false
local damageAccumulator = 0.0
local percHealth        = 0.0
ignoreMilestone       = false
local halfHealth      = 0.0
local flinchActivate = false
local flinchPerc     = 0.0
local flinchAccumulator = 0.0
-- FUNCTIONS ===================================================================
function Constructor()
  owner_meshRenderer = owner:GetComponent("MeshRenderer")
  -- pop up script
  popupScript = owner:GetLuaScript("popup.lua")
  if(owner:Name() == "Slime_Spawner")then
    levelName        = PlayerPref_GetString      ("CurrentLevel")
    health           = PlayerPref_GetFloat("SpawnerHealth", levelName)
    write("MY SPAWNER HEALTH IS ", health)
    fullHP           = health
    --perc to change boss state
    myperc       = PlayerPref_GetFloat("PercToChangeState", levelName)
    percHealth   = fullHP * myperc
    flinchPerc   = fullHP * 0.1
  end
  halfHealth = fullHP * 0.5
end


function OnUpdate(dt)
  if (flickering) then
    if(flickerTimer > 0.0) then
      flickerTimer = flickerTimer - dt
    else
      owner_meshRenderer:SetColor(originalColor)
      flickering = false
    end
  end
end

function DealDamge()
  if(popupScript ~= nil) then
    popupScript:CallFunction("Start")
  end
  
  
  if(immune == false)then
    -- Flicker
    Flicker()
  -- Take damage
    health = health - damage
    damageAccumulator = damageAccumulator + damage
    flinchAccumulator = flinchAccumulator + damage
    damage = 0
    if(flinchActivate == false)then
      if(health <= halfHealth)then
        es = owner:GetLuaScript("EnemyBehavior.lua")
        if(es ~= nil)then
          es:CallFunction("PlayFlinch")
        end
        etg = owner:GetLuaScript("Enemy_TourGuideBehaviour.lua")
        if(etg ~= nil)then
          etg:CallFunction("PlayFlinch")
        end
        flinchActivate = true
      end
    end
    -- Destroy object if health reaches 0
    if (destroyOndeath and health <= 0) then
      owner:Destroy()
      return
    end
    BossCheckHealth()
  end
end

function Flicker()
  if (owner_meshRenderer ~= nil and not flickering) then
    flickering    = true
    flickerTimer  = flickerDuration
    originalColor = owner_meshRenderer:GetColor()
    owner_meshRenderer:SetColor(flickerColor)
  end
end

function BossCheckHealth()  
  if(owner:Name() == "Slime_Spawner")then
   owner_EnemSpwn = owner:GetLuaScript("Enemy_Spawner.lua")
   owner_EnemSpwn:CallFunction("TurnOnDD")
   if(flinchAccumulator >= flinchPerc)then
    owner_EnemSpwn:CallFunction("PlayFlinch")
    flinchAccumulator = 0.0
   end
   
   if(damageAccumulator >= percHealth)then    
    if(ignoreMilestone == false)then
      immune = true
      owner_EnemSpwn:CallFunction("SwitchNextPlace")
    end
    ignoreMilestone = false
    damageAccumulator = 0.0
   end
  end
end