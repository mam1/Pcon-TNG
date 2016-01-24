
function runthermo()
    print("\n*** running thermo.lua")
    dofile("thermo.lua")
end

tmr.alarm(2,6000,0,runthermo)