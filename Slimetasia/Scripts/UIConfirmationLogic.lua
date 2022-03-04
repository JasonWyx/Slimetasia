--settings
YesT = nil
NoT = nil
camObj = nil
firstUpdate = true
selectingYes = false
 local selectSfx = "Menu_Select"
 local confirmSfx = "Game_Start_play"
function Constructor()
  camObj = owner:GetLayer():GetObject("MainCamera")
  YesT = owner:GetLayer():GetObject("Yes"):GetComponent("TextRenderer")
  NoT = owner:GetLayer():GetObject("No"):GetComponent("TextRenderer")
end

function OnUpdate(dt)
  if(camObj:GetActive() == false) then return end
  if(firstUpdate == true) then
  firstUpdate = false
  camObj:SetActive(false)
  end
  currentOffset =  -ControllerAxis( "Left" ) * UnscaledDT() * 4 
  currentOffset = currentOffset + ControllerAxis( "Right" ) * UnscaledDT() * 4
  if(currentOffset < 0 and selectingYes == false) then
    AudioSystem_PlayAudioAtLocation(selectSfx, Vector3(0, 0, 0), 1, 50, 500)
    selectingYes = true
  elseif(currentOffset > 0 and selectingYes == true) then
    AudioSystem_PlayAudioAtLocation(selectSfx, Vector3(0, 0, 0), 1, 50, 500)
    selectingYes = false
  end
  if( selectingYes ) then
    YesT:SetColor(Color(0.815,0.552,0.058,1.0))
    NoT:SetColor(Color(1.0,1.0,1.0,1.0))
  else
    YesT:SetColor(Color(1.0,1.0,1.0,1.0))
    NoT:SetColor(Color(0.815,0.552,0.058,1.0 ))
  end
  if(ControllerUp("Jump")) then
    AudioSystem_PlayAudioAtLocation(confirmSfx, Vector3(0, 0, 0), 1, 50, 500)
    if( selectingYes == true) then
      SetTimeScale(1)
      SceneQuit()
    else
      camObj:SetActive(false)
    end
  end  
end
