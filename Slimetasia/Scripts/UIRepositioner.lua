
GO = nil
playerTrans = nil
trans = nil
mat = nil
defaultPos = nil


fullFadeTIme = 1.0
startFadeTime = 1.0
appearTime = 3.0
currentTime = 0.0

appeared = true
isFading = false

colorVal = 1.0

prevPos = Vector3()

function Constructor()
  GO = owner
  playerTrans = CurrentLayer():GetObject("Player"):GetComponent("Transform")  
  trans = GO:GetComponent("Transform")
  defaultPos = trans:GetWorldPosition()
  mat =  GO:GetComponent("MeshRenderer")  
  prevPos = playerTrans:GetWorldPosition()
end

function OnUpdate(dt)
  if(appeared == false) then
    if((prevPos-playerTrans:GetWorldPosition()):Length() < 0.01) then
      currentTime = currentTime + dt
    else
      currentTime = 0
    end
    if(currentTime >= appearTime) then
        currentTime = 0.0     
        trans:SetWorldPosition(defaultPos)
        appeared = true
    end
  else
      if((prevPos-playerTrans:GetWorldPosition()):Length() > 0.01) then
        currentTime = currentTime + dt
      else
        currentTime = 0.0
      end
      if(currentTime >= startFadeTime) then
        currentTime = 0.0        
        pos = trans:GetWorldPosition()
        pos.z = 1.0
        trans:SetWorldPosition(pos)
        appeared = false
      end
  end
  prevPos = playerTrans:GetWorldPosition()
end