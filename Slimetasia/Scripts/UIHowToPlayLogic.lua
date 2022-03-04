tutorialTex = {"KILLALLDUMMIES","BARRICADETUT","BARBEDWIRETUT", "FIRECRACKERTUT","TOGGLEMODE","SLIMETUT","KAMIKAZETUT","MERLIONTUTNEWEST","ENCIKTUT","PATHTUT","ENDOFTEKONG"}

mr = nil
state = 1
prevState = 0
prevOffset = 0
maxTutorial = #tutorialTex
camObj = nil
cam = nil
 local selectSfx = "Menu_Select"
 local confirmSfx = "Game_Start_play"

leftArrowTrans = nil
rightArrowTrans = nil

firstUpdate = true
function Constructor()
  GO = owner
  trans = GO:GetComponent("Transform")
  mr = GO:GetComponent("MeshRenderer")
  
  
  camObj = owner:GetLayer():GetObject("MainCamera")
  if (camObj ~= nil) then
    cam = camObj:GetComponent("Camera")
  end
  leftArrowTrans = owner:GetLayer():GetObject("lArrow"):GetComponent("Transform")
  rightArrowTrans = owner:GetLayer():GetObject("rArrow"):GetComponent("Transform")
end
  

function OnUpdate(dt)   
  if(firstUpdate == true) then
    firstUpdate = false
    camObj:SetActive(false)
  end
  if(camObj:GetActive() == false) then
    state = 1
    prevState = 0
    return
  end
  vertSize = 10
  --Get the new position as thought it's 0.5 at pivot
  size = ((1.0*vertSize)-(vertSize/2)) - ((0.0*vertSize)-(vertSize/2))
  size = size/20
  trans:SetWorldScale(Vector3(-size,size,size))
  currentOffset =  -ControllerAxis( "Left" ) * UnscaledDT()
  currentOffset = currentOffset + ControllerAxis( "Right" ) * UnscaledDT()
  
  if(ControllerUp("Jump") or ControllerUp("Shoot")) then
    currentOffset = 1
    state = state + currentOffset
  elseif(prevOffset == 0) then
    if(currentOffset < 0)     then state = state - 1
    elseif(currentOffset > 0) then state = state + 1 end
  else
    state = state + currentOffset
  end
  prevOffset = currentOffset
  
  if(state < 1) then
    state = 1
    prevState = 1
  elseif(state > maxTutorial - 0.1) then
    AudioSystem_PlayAudioAtLocation(confirmSfx, Vector3(0, 0, 0), 1, 50, 500)
    camObj:SetActive(false)
  end
  if(ToInt(prevState) ~= ToInt(state)) then
    AudioSystem_PlayAudioAtLocation(selectSfx, Vector3(0, 0, 0), 1, 50, 500)
    mr:SetDiffuseTexture(tutorialTex[ToInt(state)])
  end
  prevState = state
  
  ratio = cam:GetViewporSize():x()/cam:GetViewporSize():y()
  horiSize = vertSize*ratio
  
  x = (0.1*horiSize)-(horiSize/2)
  p = trans:GetWorldPosition()
  p.x = x + leftArrowTrans:GetWorldScale():x()/2
  leftArrowTrans:SetWorldPosition(p)
  
  x = (0.9*horiSize)-(horiSize/2)
  p = trans:GetWorldPosition()
  p.x = x - rightArrowTrans:GetWorldScale():x()/2
  rightArrowTrans:SetWorldPosition(p)
  
  if(ControllerUp("GetOut")) then
    AudioSystem_PlayAudioAtLocation(confirmSfx, Vector3(0, 0, 0), 1, 50, 500)
    camObj:SetActive(false)
  end
end


