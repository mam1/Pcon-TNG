--init.lua

function runsensor_post()
    print("\n *** running sensor_post.lua")
    dofile("sensor_post.lua")
end

print('\n *** startup.lua ver 1.7')
print('   set mode=STATION (mode='..wifi.getmode()..')')
print('   MAC: ',wifi.sta.getmac())
print('   chip: ',node.chipid())
print('   heap: ',node.heap())
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
        print("\n   Starting sensor read loop")
--        dofile("testconnect.lua")
    end
end)


