wifi.setmode(wifi.STATION);
wifi.sta.config("FrontierHSI","")
wifi.sta.sethostname("sen01")

print('\n *** startup.lua ver 2.0')
print('   set mode=STATION (mode='..wifi.getmode()..')')
print('   MAC: ',wifi.sta.getmac())
print('   chip: ',node.chipid())
print('   heap: ',node.heap())

tmr.alarm(1,1000,1,function()
  if wifi.sta.getip()==nil then
     print("IP unavailable, waiting")
  else
     tmr.stop(1)
     print("Config done, IP is "..wifi.sta.getip())
     print("hostname: "..wifi.sta.gethostname())
     print("ESP8266 mode is: " .. wifi.getmode())
     print("The module MAC address is: " .. wifi.ap.getmac())
     print("\n   Starting sensor read loop")
  end
end)

dofile(read-post.lua)
