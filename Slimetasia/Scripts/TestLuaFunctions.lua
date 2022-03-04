-------------------------------------------------------------------------------
-- TestMode
TESTMODE = 9

-- Vectors
local lhs = Vector3(0, 0, 0)
local rhs = Vector3(0, 0, 0)
-- Colors
local col1 = Color(0, 0, 0, 0)
local col2 = Color(0, 0, 0, 0)
-- GameObject
local GO1 = nil
local GO2 = nil
-- Components
local LUASCRIPT     = nil
local TRANSFORM     = nil
local RGBD          = nil
local AUDIOLISTENER = nil
local AUDIOEMITTER  = nil
-------------------------------------------------------------------------------
function Constructor()
  write("TestFunctions usage : change TESTMODE to state which test to use")
  write("TESTMODE 1 = TEST write"        )
  write("TESTMODE 2 = TEST Vector3"      )
  write("TESTMODE 3 = TEST Color"        )
  write("TESTMODE 4 = TEST GameObject"   )
  write("TESTMODE 5 = TEST Lua scripting")
  write("TESTMODE 6 = TEST Transform"    )
  write("TESTMODE 7 = TEST RigidBody"    )
  write("TESTMODE 8 = TEST Audio"        )
  write("TESTMODE 9 = TEST Controller"   )
end

-------------------------------------------------------------------------------
function OnUpdate(dt)
  
  -- TEST::WRITE TEST
  if (TESTMODE ==1) then
    if    (IsKeyPressed(KEY_Q)) then write("String" )
    elseif(IsKeyPressed(KEY_W)) then write(1.1      )
    elseif(IsKeyPressed(KEY_E)) then write(1        )
    elseif(IsKeyPressed(KEY_R)) then write(true     )
    elseif(IsKeyPressed(KEY_T)) then write(Vector3())
    elseif(IsKeyPressed(KEY_Y)) then write(Color  ())
    elseif(IsKeyPressed(KEY_U)) then write("String", 1.1, 1, true, Vector3(), Color())
    end
  
  -- TEST::VECTOR 3
  elseif (TESTMODE == 2) then
    if    (IsKeyPressed(KEY_Q)) then write ("LHS :", lhs)
                                     write ("RHS :", rhs)
    elseif(IsKeyPressed(KEY_W)) then write ("SET : LHS to (1, 1, 1)") lhs:Set(1, 1, 1) -- VectorSet(lhs, 1, 1, 1)
                                     write ("SET : RHS to (2, 2, 2)") rhs:Set(2, 2, 2) -- VectorSet(rhs, 2, 2, 2)
    elseif(IsKeyPressed(KEY_E)) then write ("VECTOR ONE = "                    , VectorOne())
    elseif(IsKeyPressed(KEY_R)) then write ("ADD : LHS + RHS = "               , lhs + rhs)
    elseif(IsKeyPressed(KEY_T)) then write ("SUBTRACT : LHS - RHS = "          , lhs - rhs)
    elseif(IsKeyPressed(KEY_Y)) then write ("MULTIPLY : LHS * 2 = "            , lhs * 2)
    elseif(IsKeyPressed(KEY_U)) then write ("DIVISION : LHS / 2 = "            , lhs / 2)
    elseif(IsKeyPressed(KEY_I)) then write ("UNARY MINUS : -LHS = "            , -lhs)
    elseif(IsKeyPressed(KEY_O)) then write ("Comparison : LHS == RHS = "       , lhs == rhs)
    elseif(IsKeyPressed(KEY_P)) then write ("ACESS : LHS.x = "                 , lhs:x())
    elseif(IsKeyPressed(KEY_A)) then write ("NORMALIZE : LHS = "               , lhs:Normalize())
    elseif(IsKeyPressed(KEY_S)) then write ("NORMALIZED : LHS = "              , lhs:Normalized()) -- VectorNormalized(lhs)
    elseif(IsKeyPressed(KEY_D)) then write ("PROJECTION : LHS on RHS = "       , lhs:Project(rhs)) -- VectorProject(lhs, rhs)
    elseif(IsKeyPressed(KEY_F)) then write ("CROSS PRODUCT : LHS on RHS = "    , lhs:Cross(rhs)) -- VectorCross(lhs, rhs)
    elseif(IsKeyPressed(KEY_G)) then write ("DOT : LHS on RHS = "              , lhs:Dot(rhs)) -- VectorDot(lhs, rhs)
    elseif(IsKeyPressed(KEY_H)) then write ("LENGTH : LHS = "                  , lhs:Length()) -- VectorLength(lhs)
    elseif(IsKeyPressed(KEY_J)) then write ("SQUARED LENGTH : LHS = "          , lhs:SquareLength()) -- VectorSquareLength(lhs)
    elseif(IsKeyPressed(KEY_K)) then write ("DISTANCE : LHS to RHS = "         , lhs:DistanceTo(rhs)) -- VectorDistance(lhs, ths)
    elseif(IsKeyPressed(KEY_L)) then write ("SQUARED DISTANCE : LHS to RHS = " , lhs:SquareDistanceTo(rhs)) -- VectorSquareDistance(lhs, rhs)
    elseif(IsKeyPressed(KEY_Z)) then write ("ROTATE : LHS by x 45 = "          , lhs:Rotate("x", 45)) -- VectorRotate(lhs, "x", 45)
  end
  
  -- TEST::COLOR
  elseif (TESTMODE == 3) then
    if    (IsKeyPressed(KEY_Q)) then write ("COL1 :", col1)
                                     write ("COL2 :", col2)
    elseif(IsKeyPressed(KEY_W)) then write ("SET : COL1 to (2, 2, 2, 2)") col1:Set(2, 2, 2, 2) -- ColorSet(col1, 2, 2, 2, 2)
                                     write ("SET : COL2 to (1, 1, 1, 0)") col2:Set(1, 1, 1, 0) -- ColorSet(col2, 1, 1, 1, 0)
    elseif(IsKeyPressed(KEY_E)) then write ("ADD : COL1 + COL2 = "        , col1 + col2)
    elseif(IsKeyPressed(KEY_R)) then write ("SUBTRACT : COL1 - COL2 = "   , col1 - col2)
    elseif(IsKeyPressed(KEY_T)) then write ("MULTIPLY : COL1 * 2 = "      , col1 * 2)
    elseif(IsKeyPressed(KEY_Y)) then write ("DIVISION : COL1 / 2 = "      , col1 / 2)
    elseif(IsKeyPressed(KEY_U)) then write ("COMPARISON : COL1 == COL2 = ", col1 == col2)
    elseif(IsKeyPressed(KEY_I)) then write ("ACESS : COL1.r = "           , COL1.r)
    end
    
  -- TEST::GAMEOBJECT
  elseif (TESTMODE == 4) then
    if    (IsKeyPressed(KEY_Q)) then GO1 = CurrentLayer():GetObject("GAMEOBJECT")
                                if(GO1 ~= nil) then 
                                  write ("GAMEOBJECT GET : PASS - Test case is ready to run")
                                else
                                  write ("GAMEOBJECT GET : FAILED - This test mode is not applicable")
                                end
    
    elseif(IsKeyPressed(KEY_W)) then write ("NAME :"           , GO1:Name())
    elseif(IsKeyPressed(KEY_E)) then write ("SET ACTIVE : TRUE", GO1:SetActive(true))
    elseif(IsKeyPressed(KEY_R)) then write ("GET ACTIVE : "    , GO1:GetActive())
    end
  
      -- TEST::LUA SCRIPT
  elseif (TESTMODE == 5) then
    if    (IsKeyPressed(KEY_Q)) then GO1 = CurrentLayer():GetObject("LUASCRIPT")
                                     LUASCRIPT = GO1:GetLuaScript("TestLuaFunctions_luaScript.lua")
                                     if(GO1 ~= nil and LUASCRIPT ~= nil) then
                                       write ("LUASCRIPT GET : PASS - Test case is ready to run")
                                     else
                                       write ("LUASCRIPT GET : FAILED - This test mode is not applicable")
                                     end
    elseif(IsKeyPressed(KEY_W)) then write ("GET : Owner = : "              , owner:Name())
    elseif(IsKeyPressed(KEY_E)) then write ("value in LUASCRIPT : "         , LUASCRIPT:GetVariable("value"))
    elseif(IsKeyPressed(KEY_R)) then write ("changing value in LUASCRIPT : ") LUASCRIPT:SetVariable("value", 2);
    end
  
  -- TEST::TRANSFORM
  elseif (TESTMODE == 6) then
    if    (IsKeyPressed(KEY_Q)) then GO1 = CurrentLayer():GetObject("TRANSFORM")
                                     TRANSFORM = GO1:GetComponent("Transform")
                                if(GO1 ~= nil and TRANSFORM ~= nil) then 
                                  write ("Transform GET : PASS - Test case is ready to run")
                                else
                                  write ("Transform GET : FAILED - This test mode is not applicable")
                                end
    elseif(IsKeyPressed(KEY_1)) then write ("GET OWNER : "                            , TRANSFORM:Owner():Name())
    elseif(IsKeyPressed(KEY_W)) then write ("GET LOCAL POSITION : "                   , TRANSFORM:GetlocalPosition())
    elseif(IsKeyPressed(KEY_R)) then write ("GET LOCAL ROTATION : "                   , TRANSFORM:GetlocalRotation())
    elseif(IsKeyPressed(KEY_Y)) then write ("GET LOCAL SCALE : "                      , TRANSFORM:GetlocalScale())
    elseif(IsKeyPressed(KEY_I)) then write ("GET WORLD POSITION : "                   , TRANSFORM:GetWorldPosition())
    elseif(IsKeyPressed(KEY_P)) then write ("GET WORLD ROTATION : "                   , TRANSFORM:GetWorldRotation())
    elseif(IsKeyPressed(KEY_S)) then write ("GET WORLD SCALE : "                      , TRANSFORM:GetWorldScale())
    elseif(IsKeyPressed(KEY_H)) then write ("FORWARD VECTOR : "                       , TRANSFORM:ForwardVector())
    elseif(IsKeyPressed(KEY_J)) then write ("UPWARD VECTOR : "                        , TRANSFORM:UpwardVector())
    
    elseif(IsKeyPressed(KEY_E)) then write ("SET LOCAL POSITION : LOCAL = (1, 0, 0) " ) TRANSFORM:SetLocalPosition(Vector3(1, 0, 0)) -- TRANSFORM:SetLocalPosition(1, 0, 0) -- TRANSFORM:SetLocalPosition("x", 1)
    elseif(IsKeyPressed(KEY_T)) then write ("SET LOCAL ROTATION : LOCAL = (45, 0, 0) ") TRANSFORM:SetLocalRotation(Vector3(45, 0, 0)) -- TRANSFORM:SetLocalRotation(45, 0, 0) -- TRANSFORM:SetLocalRotation("x", 45)
    elseif(IsKeyPressed(KEY_U)) then write ("SET LOCAL SCALE : LOCAL = (2, 2, 2) "    ) TRANSFORM:SetLocalScale(Vector3(2, 2, 2)) -- TRANSFORM:SetLocalScale(2, 2, 2) -- TRANSFORM:SetLocalScale("x", 2)
    elseif(IsKeyPressed(KEY_O)) then write ("SET WORLD POSITION : WORLD = (0, 0, 0) " ) TRANSFORM:SetWorldPosition(Vector3()) -- TRANSFORM:SetWorldPosition(0, 0, 0) -- TRANSFORM:SetLocalScale("x", 0)
    elseif(IsKeyPressed(KEY_A)) then write ("SET WORLD ROTATION : WORLD = (0, 0, 0) " ) TRANSFORM:SetWorldRotation(Vector3()) -- TRANSFORM:SetWorldRotation(0, 0, 0) -- TRANSFORM:SetWorldRotation("x", 0)
    elseif(IsKeyPressed(KEY_D)) then write ("TRANSLATE : TRANSLATE(1, 0, 0) "         ) TRANSFORM:Translate(Vector3(1, 0, 0))-- TRANSFORM:Translate(0, 0, 0)
    elseif(IsKeyPressed(KEY_F)) then write ("ROTATE : ROTATE(45, 0, 0) "              ) TRANSFORM:Rotate(Vector3(45, 0 ,0)) -- TRANSFORM:Rotate(0, 0, 0)
    elseif(IsKeyPressed(KEY_G)) then write ("SCALE : SCALE (2, 2, 2) "                ) TRANSFORM:Scale(Vector3(2, 2, 2)) -- TRANSFORM:Scale(0, 0, 0)
    end
  
  -- TEST::RIGIDBODY
  elseif (TESTMODE == 7) then
    if    (IsKeyPressed(KEY_Q)) then GO1 = CurrentLayer():GetObject("RIGIDBODY")
                                     RGBD = GO1:GetComponent("RigidBody")
                                if(GO1 ~= nil and RGBD ~= nil) then 
                                  write ("RigidBody GET : PASS - Test case is ready to run")
                                else
                                  write ("RigidBody GET : FAILED - This test mode is not applicable")
                                end
    elseif(IsKeyPressed(KEY_1)) then write ("GET OWNER : "                      , RGBD:Owner():Name())
    elseif(IsKeyPressed(KEY_W)) then write ("GET GHOST : "                      , RGBD:GetGhost())
    elseif(IsKeyPressed(KEY_R)) then write ("GET COLLIDED WITH STATIC : "       , RGBD:GetCollisionWithStatic())
    elseif(IsKeyPressed(KEY_Y)) then write ("GET MASS : "                       , RGBD:GetMass())
    elseif(IsKeyPressed(KEY_I)) then write ("GET DRAG : "                       , RGBD:GetDrag())
    elseif(IsKeyPressed(KEY_P)) then write ("GET BODY TYPE : "                  , RGBD:GetBodyType())
    elseif(IsKeyPressed(KEY_S)) then write ("GET FREEZE ROTATION : "            , RGBD:GetFreezeRotation())
    elseif(IsKeyPressed(KEY_F)) then write ("GET RESITUITION : "                , RGBD:GetRestitution())
    elseif(IsKeyPressed(KEY_H)) then write ("GET VELOCITY : "                   , RGBD:GetVelocity())
    elseif(IsKeyPressed(KEY_K)) then write ("GET ACCELERATION : "               , RGBD:GetAcceleration())
    elseif(IsKeyPressed(KEY_Z)) then write ("GET OFFSET : "                     , RGBD:GetOffset())
    elseif(IsKeyPressed(KEY_C)) then write ("GET ANGULAR VELOCITY : "           , RGBD:GetAngularVelocity())
    elseif(IsKeyPressed(KEY_B)) then write ("GET GRAVITY ENABLED : "            , RGBD:GetGravityEnabled())
    elseif(IsKeyPressed(KEY_M)) then write ("GET AFFECTED BY RESISTANCE : "     , RGBD:GetAffectedByResistance())
    
    elseif(IsKeyPressed(KEY_E)) then write ("SET GHOST : TRUE "                 ) RGBD:SetGhost(true)
    elseif(IsKeyPressed(KEY_T)) then write ("SET COLLIDED WITH STATIC : TRUE "  ) RGBD:SetCollisionWithStatic(true)
    elseif(IsKeyPressed(KEY_U)) then write ("SET MASS : 200 "                   ) RGBD:SetMass(200)
    elseif(IsKeyPressed(KEY_O)) then write ("SET DRAG : 200 "                   ) RGBD:SetDrag(200)
    elseif(IsKeyPressed(KEY_A)) then write ("SET BODY TYPE : 2 "                ) RGBD:SetBodyType(2)
    elseif(IsKeyPressed(KEY_D)) then write ("SET FREEZE ROTATION : TRUE "       ) RGBD:SetFreezeRotation(true)
    elseif(IsKeyPressed(KEY_G)) then write ("SET RESITUITION : 2 "              ) RGBD:SetRestitution(2)
    elseif(IsKeyPressed(KEY_J)) then write ("SET VELOCITY : (2, 0, 0) "         ) RGBD:SetVelocity(Vector3(2, 0, 0)) -- RGBD:SetVelocity(2, 0, 0) -- RGBD:SetVelocity("x", 2)
    elseif(IsKeyPressed(KEY_L)) then write ("SET ACCELERTION : (2, 0, 0) "      ) RGBD:SetAcceleration(Vector3(2, 0, 0)) -- RGBD:SetAcceleration(2, 0, 0) -- RGBD:SetAcceleration("x", 2)
    elseif(IsKeyPressed(KEY_X)) then write ("SET OFFEST : (2, 0, 0)"            ) RGBD:SetOffset(Vector3(2, 0, 0)) -- RGBD:SetOffset(2, 0, 0) -- RGBD:SetOffset("x", 2)
    elseif(IsKeyPressed(KEY_V)) then write ("SET ANGULAR VELOCITY : (2, 0, 0)"  ) RGBD:SetAngularVelocity(Vector3(2, 0, 0))  -- RGBD:SetAngularVelocity(2, 0, 0) -- RGBD:SetAngularVelocity("x", 2)
    elseif(IsKeyPressed(KEY_N)) then write ("SET GRAVITY ENABLED : TRUE"        ) RGBD:SetGravityEnabled(true)
    elseif(IsKeyPressed(KEY_1)) then write ("SET AFFECTED BY RESISTANCE : TRUE ") RGBD:SetAffectedByResistance(true)
    end
  
  -- TEST::AUDIO
  elseif (TESTMODE == 8) then
    if    (IsKeyPressed(KEY_Q)) then GO1 = CurrentLayer():GetObject("AUDIOLISTENER")
                                     AUDIOLISTENER = GO1:GetComponent("AudioListener")
                                     GO2 = CurrentLayer():GetObject("AUDIOEMITTER")
                                     AUDIOEMITTER  = GO2:GetComponent("AudioEmitter")
                                if(GO1 ~= nil and AUDIOLISTENER ~= nil and  GO2 ~=nil and AUDIOEMITTER~= nil) then 
                                  write ("AudioListener/AudioEmitter GET : PASS - Test case is ready to run")
                                else
                                  write ("AudioListener/AudioEmitter GET : FAILED - This test mode is not applicable")
                                end
    elseif(IsKeyPressed(KEY_1)) then write ("GET OWNER : "                      , AUDIOLISTENER:Owner():Name())
                                     write ("GET OWNER : "                      , AUDIOEMITTER:Owner():Name())
    elseif(IsKeyPressed(KEY_W)) then write ("IS MAIN : ", AUDIOLISTENER:IsMain())
    elseif(IsKeyPressed(KEY_E)) then write ("SET MAIN : FALSE ") AUDIOLISTENER:SetMain(false)
    
    elseif(IsKeyPressed(KEY_R)) then write ("GET AUDIOCLIP : "            , AUDIOEMITTER:GetAudioClip())
    elseif(IsKeyPressed(KEY_A)) then write ("IS PLAYING : "               , AUDIOEMITTER:IsPlaying())
    elseif(IsKeyPressed(KEY_S)) then write ("IS PAUSED : "                , AUDIOEMITTER:IsPaused())
    elseif(IsKeyPressed(KEY_F)) then write ("GET VOLUME : "               , AUDIOEMITTER:GetVolume())
    elseif(IsKeyPressed(KEY_H)) then write ("GET PITCH : "                , AUDIOEMITTER:GetPitch())
    elseif(IsKeyPressed(KEY_K)) then write ("GET MIN DISTANCE : "         , AUDIOEMITTER:GetMinDistance3D())
    elseif(IsKeyPressed(KEY_Z)) then write ("GET MAX DISTANCE : "         , AUDIOEMITTER:GetMaxDistance3D())
                                                                         
    elseif(IsKeyPressed(KEY_T)) then write ("SET AUDIO : MI.OGG"          ) AUDIOEMITTER:SetAudioClip("MI.ogg")
    elseif(IsKeyPressed(KEY_Y)) then write ("PLAY :"                      ) AUDIOEMITTER:Play()
    elseif(IsKeyPressed(KEY_U)) then write ("PAUSE : TRUE"                ) AUDIOEMITTER:Pause(true)
    elseif(IsKeyPressed(KEY_I)) then write ("PAUSE : FALSE"               ) AUDIOEMITTER:Pause(false)
    elseif(IsKeyPressed(KEY_O)) then write ("SET LOOP : TRUE "            ) AUDIOEMITTER:SetLoop(true)
    elseif(IsKeyPressed(KEY_P)) then write ("SET AND PLAY AUDIO : MI.OGG ") AUDIOEMITTER:SetAndPlayAudioClip("MI.ogg")
    elseif(IsKeyPressed(KEY_D)) then write ("SET VOLUME : 0.5 "           ) AUDIOEMITTER:SetVolume(0.5)
    elseif(IsKeyPressed(KEY_G)) then write ("SET PITCH : 2 "              ) AUDIOEMITTER:SetPitch(2)
    elseif(IsKeyPressed(KEY_J)) then write ("SET MIN DISTANCE : 1 "       ) AUDIOEMITTER:SetMinDistance3D(1)
    elseif(IsKeyPressed(KEY_L)) then write ("SET MAX DISTANCE : 200"      ) AUDIOEMITTER:SetMaxDistance3D(200)
    
    elseif(IsKeyPressed(KEY_2)) then Transform = GO2:GetComponent("Transform")
                                     if ( Transform ~= nil) then
                                       write ("MOVING AUDIOEMITTER : (0, 0, 0)")
                                       Transform:SetWorldPosition(0, 0, 0)
                                     end
    elseif(IsKeyPressed(KEY_3)) then Transform = GO2:GetComponent("Transform")
                                     if ( Transform ~= nil) then
                                       write ("MOVING AUDIOEMITTER : (1, 0, 0)")
                                       Transform:SetWorldPosition(1, 0, 0)
                                     end
    elseif(IsKeyPressed(KEY_4)) then Transform = GO2:GetComponent("Transform")
                                     if ( Transform ~= nil) then
                                       write ("MOVING AUDIOEMITTER : (-1, 0, 0)")
                                       Transform:SetWorldPosition(-1, 0, 0)
                                     end
    elseif(IsKeyPressed(KEY_5)) then Transform = GO2:GetComponent("Transform")
                                     if ( Transform ~= nil) then
                                       write ("MOVING AUDIOEMITTER : (0, 1, 0)")
                                       Transform:SetWorldPosition(0, 1, 0)
                                     end
    elseif(IsKeyPressed(KEY_6)) then Transform = GO2:GetComponent("Transform")
                                     if ( Transform ~= nil) then
                                       write ("MOVING AUDIOEMITTER : (0, -1, 0)")
                                       Transform:SetWorldPosition(0, -1, 0)
                                     end
    elseif(IsKeyPressed(KEY_7)) then Transform = GO2:GetComponent("Transform")
                                     if ( Transform ~= nil) then
                                       write ("MOVING AUDIOEMITTER : (0, 0, 1)")
                                       Transform:SetWorldPosition(0, 0, 1)
                                     end
    elseif(IsKeyPressed(KEY_8)) then Transform = GO2:GetComponent("Transform")
                                     if ( Transform ~= nil) then
                                       write ("MOVING AUDIOEMITTER : (0, 0, -1)")
                                       Transform:SetWorldPosition(0, 0, -1)
                                     end
    end
  
  -- TEST::Controller
  elseif (TESTMODE == 9) then
    write("LUA : ", InputAxis(0, "x"))
  
  -- INVALID TEST
  else
    write ("INVALID TESTMODE")
  end
end


function OnCollisionEnter(other)
write(" On Collision Enter : ", other:Name())
end

function OnCollisionPersist(other)
write("On Collision Persist : ", other:Name())
end

function OnCollisionExit(other)
write("On Collision Exit : ", other:Name())
end
