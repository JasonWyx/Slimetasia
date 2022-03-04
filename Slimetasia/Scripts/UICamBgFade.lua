 cam = nil
 camObj = nil
 mr = nil
 oldA = nil
 activator = false
 
 firstUpdate = true
function Constructor()
  GO = owner
  mr = GO:GetComponent("MeshRenderer")
  camObj = owner:GetLayer():GetObject("MainCamera")
  oldA = mr:GetColor():a()
end


function OnUpdate(dt)
  if(firstUpdate) then 
    firstUpdate = false
    camObj:SetActive(false)
  end
  if(camObj:GetActive()==false) then return end
  oldColor = mr:GetColor()
  if(activator == true) then
    oldColor.a = oldColor:a() + UnscaledDT()
    if(oldColor:a() > oldA) then
      oldColor.a = oldA
    end
    mr:SetColor(oldColor)
  else
    oldColor.a = oldColor:a() - UnscaledDT()
    if(oldColor:a() < 0) then
      oldColor.a = 0
      camObj:SetActive(false)
    end
    mr:SetColor(oldColor)
  end
end

function activate()
  activator = true
  camObj:SetActive(true)
end


function deactivate()
  activator = false
end