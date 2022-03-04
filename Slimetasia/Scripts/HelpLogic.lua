--settings
GO = nil
text = nil
function Constructor()
  GO = owner
  text = GO:GetComponent("TextRenderer")  
end

function OnUpdate(dt)  
  if(GetControllerInput() == 1) then
    text:SetText("Press back to see help")
  else
    text:SetText("Press tab to see help")
  end
end