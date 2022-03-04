--settings
local GO = nil
local TRANS = nil

startImage = nil
startImageTrans = nil
startImageRound = nil
startImageRoundTrans = nil

local WaveInfo = nil
local haveWaveStarted = false
startPos = nil

local activate = true
local firstUpdate = true
local shootToStart = false
--trigger

local tekongEvent = nil
anchorPosition = Vector3()
camObj = nil
cam = nil
function Constructor()
  GO = owner
  startImage = GO:GetLayer():GetObject("UI_PressXToStart")
  startImageTrans = startImage:GetComponent("Transform")
  startImageRound = GO:GetLayer():GetObject("UI_PressXRound")
  startImageRoundTrans = startImageRound:GetComponent("Transform")
  WaveInfo = CurrentLayer():GetObject("GameLogic"):GetLuaScript("GameLogic_VerticalSlice.lua")
  


  
  trigger = CurrentLayer():GetObject("GameLogic")
  tekongEvent = trigger:GetLuaScript("Event_Tekong.lua")
  if(tekongEvent ~= nil) then
      activate = false
  end
  
  
  camObj = owner:GetLayer():GetObject("MainCamera")
  if (camObj ~= nil) then
    cam = camObj:GetComponent("Camera")
  end
end

function OnUpdate(dt)
  if(firstUpdate == true) then
    firstUpdate = false
    startImage:SetActive(false)
    startImageRound:SetActive(false)
    return
  end
  if(activate == false) then
    if(tekongEvent:GetVariable("currentEvent") == 7) then
      activate = true
    end
    return
  end
  if(WaveInfo:GetVariable("spawnStart") == true) then
    startImage:SetActive(false)
    startImageRound:SetActive(false)
    return
  else
    startImage:SetActive(true)
    startImageRound:SetActive(true)
  end
  
  CalculateAnchoredPosition(0.025,0.94,1.0,0.0, startImageTrans)
  startImageTrans:SetWorldPosition(anchorPosition)
  anchorPosition.z = startImageRoundTrans:GetWorldPosition():z()
  startImageRoundTrans:SetWorldPosition(anchorPosition)
end

function CalculateAnchoredPosition(screenPosX,screenPosY,pivotX,pivotY, t)
  vertSize = 10
  
  --Get the new position as thought it's 0.5 at pivot
  y = (screenPosY*vertSize)-(vertSize/2)
  ratio = cam:GetViewporSize():x()/cam:GetViewporSize():y()
  horiSize = vertSize*ratio
  x = (screenPosX*horiSize)-(horiSize/2)
  v = Vector3(x, y, t:GetWorldPosition():z())

  --offset the new position base on the pivot
  pivotOffset = Vector3( pivotX - 0.5,  
                         pivotY - 0.5, 
                         0.0)
  x =  pivotOffset:x() * t:GetWorldScale():x()
  y =  pivotOffset:y() * t:GetWorldScale():y()
  z =  0-- pivotOffset:z() * t:GetWorldScale():z()
  pivotOffset = Vector3( x, y, z)
  v = v + pivotOffset
  anchorPosition =  v
end

