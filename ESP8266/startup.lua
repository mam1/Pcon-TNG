--startup.lua

function runthermo()
    print("\n*** running thermo.lua")
    dofile("thermo.lua")
end

print('\n*** startup.lua ver 1.4')
print('  set mode=STATION (mode='..wifi.getmode()..')')
print('  MAC: ',wifi.sta.getmac())
print('  chip: ',node.chipid())
print('  heap: ',node.heap())
-- wifi config start
wifi.sta.config("FrontierHSI","")
-- wifi config end
wifi.sta.connect()
tmr.alarm(1, 1000, 1, function()
    if wifi.sta.getip()== nil then
        print("IP unavaiable, Waiting...")
    else
        tmr.stop(1)
        print("\n   ESP8266 mode is: " .. wifi.getmode())
        print("   The module MAC address is: " .. wifi.ap.getmac())
        print("   Config done, IP is "..wifi.sta.getip())
 --       dofile("getNets.lua")
    end
end)

tmr.alarm(2,8000,0,runthermo)
