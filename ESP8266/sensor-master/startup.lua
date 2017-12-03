
--startup.lua

version="1.0.1"
ip="192.168.254.205"

 -- setup I2c and connect display
function init_i2c_display()
     -- SDA and SCL can be assigned freely to available GPIOs
     local sda = 6 
     local scl = 5 
     local sla = 0x3c
     print("    initializng I2c OLED display on pins "..sda.." and "..scl)    
     i2c.setup(0, sda, scl, i2c.SLOW)
     disp = u8g.ssd1306_128x64_i2c(sla)
     disp:setFont(u8g.font_6x10)
     disp:setFontRefHeightExtendedText()
     disp:setDefaultForegroundColor()
     disp:setFontPosTop()
     disp:setColorIndex(1)
end

function draw(str)
    disp:drawStr(10,10,str)
end 

print('\n *** startup.lua ver 2.0')

wifi.setmode(wifi.STATION);
wifi.sta.config("FrontierHSI","")
wifi.sta.sethostname("sen06")

print('    set mode=STATION (mode='..wifi.getmode()..')')
print('    MAC: ',wifi.sta.getmac())
print('    chip: ',node.chipid())
print('    heap: ',node.heap())
print(' ')

tmr.alarm(1,1000,1,function()
  if wifi.sta.getip()==nil then
     print("    IP unavailable, waiting")
  else
     tmr.stop(1)
     print("\n    Config done, IP is "..wifi.sta.getip())
     print("    hostname: "..wifi.sta.gethostname())
     print("    ESP8266 mode is: " .. wifi.getmode())
     print("    MAC address is: " .. wifi.ap.getmac())
     init_i2c_display()
     print("\n    starting sensor read loop")
     print("    sending data to: " ..ip)
     dofile("post.lua")
  end
end)

