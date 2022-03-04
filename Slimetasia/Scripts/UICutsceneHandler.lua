--settings

local GO = nil
local trans = nil
event = nil


local healthbar = nil
local frame = nil
local icon = nil
local black = nil

rCB = nil
lCB = nil
rCC = nil
isChinatown = false
function Constructor()
  GO = owner:GetLayer():GetObject("MainCamera")
  trigger = CurrentLayer():GetObject("GameLogic")
  event = trigger:GetLuaScript("Event_Tekong.lua")
  if(event == nil) then
    event = trigger:GetLuaScript("Event_Chinatown.lua")
    isChinatown = true
  end
  if(event == nil) then
    event = trigger:GetLuaScript("Event_Changi.lua")
    isChinatown = true
  end
  
  if(event == nil) then
    isChinatown = true
  end
  if(isChinatown == false) then
    healthbar = owner:GetLayer():GetObject("UI_CoreHealth")
    frame = owner:GetLayer():GetObject("UI_CoreHealthFrame")
    icon = owner:GetLayer():GetObject("UI_CoreIcon")
    black = owner:GetLayer():GetObject("UI_CoreHealthBlack")
    charge = owner:GetLayer():GetObject("UI_Charger")
    chargeblack = owner:GetLayer():GetObject("UI_ChargerBlack")
    
    
    --Resource count assets
    rC = owner:GetLayer():GetObject("RCountN")
    lC = owner:GetLayer():GetObject("UCountN")
    rCB = owner:GetLayer():GetObject("RCountNB")
    lCB = owner:GetLayer():GetObject("UCountNB")
    rCC = owner:GetLayer():GetObject("RCCenter")
    res = owner:GetLayer():GetObject("UI_Resource")
    header = owner:GetLayer():GetObject("UI_Header")
  end
end

function OnUpdate(dt)
  if(event == nil) then
    return
  end
  if(isChinatown == false) then
    if(event:GetVariable("currentEvent") < 7) then
      healthbar:SetActive(false)
      frame:SetActive(false)
      icon:SetActive(false)
      black:SetActive(false)
      charge:SetActive(false)
      chargeblack:SetActive(false)
    else
      healthbar:SetActive(true)
      frame:SetActive(true)
      icon:SetActive(true)
      black:SetActive(true)
      charge:SetActive(true)
      chargeblack:SetActive(true)
    end
    
    if(event:GetVariable("currentEvent") < 4) then
      rC:SetActive(false)
      lC:SetActive(false)
      rCB:SetActive(false)
      lCB:SetActive(false)
      rCC:SetActive(false)
      res:SetActive(false)
      header:SetActive(false)
    else
      rC:SetActive(true)
      lC:SetActive(true)
      rCB:SetActive(true)
      lCB:SetActive(true)
      rCC:SetActive(true)
      res:SetActive(true)
      header:SetActive(true)
    end
  end
  if(event:GetVariable("inCutScene") == GO:GetActive()) then
    GO:SetActive(not event:GetVariable("inCutScene"))
  end
end
