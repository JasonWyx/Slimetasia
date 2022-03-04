-- VARIABLES ===================================================================
local areaOfEffect = 5
local areaCollider = nil
      origKDmg     = 0
      origDmgM     = 0
      origDmgR     = 0
      origIntAtk   = 1.0
-- FUNCTIONS ===================================================================
function Constructor()
  areaCollider = owner:GetComponent("SphereCollider")
  if (areaCollider ~= nil) then
	areaCollider:SetRadius(areaOfEffect)
  end
end

function OnUpdate(dt)
  
end

function OnCollisionEnter(other)
  --SETUP THE TOURGUIDE BUFF DMG TOWARDS melee, range, kamikaze
  if (other:Tag() == "Slime" and other:Name() ~= "Slime_TourGuide" and other:Name() ~= "Slime_Spawner") then
    script = nil
    if(other:Name() == "Slime_Kamikaze")then
      ---Increase kamikaze on core dmg
      script = GO_Core:GetLuaScript("CoreLogic.lua")
      kDmg = script:GetVariable("truekamikaze_Damage")
      origKDmg = kDmg
      kDmg = kDmg * 2
      script:SetVariable("kamikaze_Damage", kDmg)
      script = other:GetLuaScript("Enemy_Kamikaze.lua")
    else
    --Damage up
      script = other:GetLuaScript("EnemyBehavior.lua")
      dmgM = script:GetVariable("truedamage_melee")
      origDmgM = dmgM
      dmgM = dmgM * 2
      script:SetVariable("damage_melee",dmgM)
      dmgR = script:GetVariable("truedamage_slimeBullet")
      origDmgR = dmgR
      dmgR = dmgR * 2
      script:SetVariable("damage_slimeBullet",dmgR)
      --Adjust atk spd      
      origIntAtk = script:GetVariable("trueinterval_attack")
      int_atk = origIntAtk / 1.5
      script:SetVariable("interval_attack",int_atk)
    end
  end
end

function OnCollisionPersist(other)
  if (other:Name() == "Slime") then
    script = other:GetLuaScript("EnemyBehavior.lua")
    script:CallFunction("TourGuideBuff")
  end
  if (other:Name() == "Slime_Kamikaze") then
    script = other:GetLuaScript("Enemy_Kamikaze.lua")
    script:CallFunction("TourGuideBuff")
  end
end

function OnCollisionExit(other)
end