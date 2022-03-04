player = {
    pos = {
         X = 20,
         Y = 30,
    },
    filename = "res/images/player.png",
    HP = 20,
-- you can also have comments
}

-- Global Variables
boolean = false
tmpglobal = Vector3()
vec3 = nil
float = 0.0
lalala = 1

-- Global statement a.k.a will only run once
if write("Hello") then write("World") end

-- Thinking should create Constructor and destructor for the scripts

-- Using lightuserdata will only pass pointers therefore may mess up metatable (outdated)
-- Solutions: newuserdata and memcpy but uses 2 times memory and may crash on destructor
--            create global functions to force metatable to change (recommended, may be abit tedious)
--            LOL we can use pointer to pointer for newuserdata then we no need memcpy

-- Constructor
function Constructor()
write("Constructed")
end

-- Destructor
function Destructor()
write("Destructed")
end

-- Update function
function OnUpdate(dt)
ly = CurrentLayer()
if(IsKeyPressed(KEY_P)) then write(float) end
if(IsKeyPressed(KEY_L)) then write(lalala) end
if(IsKeyPressed(KEY_A)) 
then
	if(boolean == false) then
	write(this:Name())
	boolean = true
	else
	CurrentLayer()
	go = CurrentLayer():GetObject("hoho")
	if(go == nil) then write("HOSEI OUT")
	else write(go:GetActive()) end
	end
end 
if(IsKeyPressed(KEY_S)) then
	vec3 = Vector3()
end
if(IsKeyPressed(KEY_D)) then
    vec3.x = 3.0123
    tmp = Vector3()
tmp.y = 0.5

vec3 = vec3 + tmp
	write(vec3) 
end
if(IsKeyPressed(KEY_SPACE)) then
	gameObj = ly:GetObject("Junze")
	if(gameObj ~= nil) then
		gameObj:AddComponent("Transform")
		comps = gameObj:GetComponents()
		size = #comps -- size of
		write(size)
		for i = 1, size 
		do 
		write(comps[i]) 
		if(comps[i] == "Transform") then write("Matches") end
		end
	else write("GameObject does not exists")
	end
end

if(IsKeyPressed(KEY_Z)) then
write(#this:GetComponents())
vec3:Normalize()
write(vec3)
end

if(IsKeyPressed(KEY_Q)) then
  gameobjectlist = ly:GetObjectsList()
  for i = 1, #gameobjectlist do
  write(gameobjectlist[i]:Name())
  end
end

end

if(IsPadPressed(GAMEPAD_A)) then write("GamePad up is pressed") end

-- Test function to be replaced with collision function
function OnCollisionEnter(go)
  write("OnCollisionEnter", go:Name())
end

function OnCollisionPersist(go)
  write("OnCollisionPersist", go:Name())
end

function OnCollisionExit(go)
  write("OnCollisionExit", go:Name())
end

--[[
Add OnCollisionEnter, Add OnCollisionPersist, Add OnCollisionEnded Functions to
Lua Script, then we will run the function from C++ side
Similar to the Test function above
--]]