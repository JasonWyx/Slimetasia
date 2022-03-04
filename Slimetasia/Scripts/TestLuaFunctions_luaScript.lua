value = 1
local oldValue;

function Constructor()
  oldValue = value
end


function OnUpdate(dt)
  if (value ~= oldValue) then
    oldValue = value;
    write("Value in LuaScript have been changed : ", value)
  end
end