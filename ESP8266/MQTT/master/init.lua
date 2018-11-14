-- init.lua
SSID="FrontierHSI" -- SSID 
PASSWORD="passfire" -- SSID password
HOSTNAME="MQTT-temp-shop-office"
TIMEOUT=100000    -- timeout to check the network status
EXSENSOR1="tempumid.lua"    -- module to run
MQTTSERVER="192.168.254.220"   -- mqtt broker address
MQTTPORT="1883"  -- mqtt broker port
MQTTQOS="0" -- qos used
 
print('\n *** MQTT init.lua ver 1.01')

-- setup I2c and connect display

-- function init_i2c_display()
--      -- SDA and SCL can be assigned freely to available GPIOs
--      local sda = 6 
--      local scl = 5 
--      local sla = 0x3c
--      print("    initializing I2c OLED display on pins "..sda.." and "..scl)    
--      i2c.setup(0, sda, scl, i2c.SLOW)
--      disp = u8g2.ssd1306_i2c_128x64_noname(id, sla)
--      disp:setFont(u8g.font_6x10)
--      disp:setFontRefHeightExtendedText()
--      disp:setDefaultForegroundColor()
--      disp:setFontPosTop()
--      disp:setColorIndex(1)
-- end

-- function draw(str) 
--     disp:drawStr(10,10,str)
-- end 

-- Set the network parameters and get ip address

station_cfg={}
station_cfg.ssid=SSID
station_cfg.pwd=PASSWORD
station_cfg.save=true
wifi.sta.config(station_cfg)
wifi.sta.sethostname(HOSTNAME)
wifi.sta.autoconnect(0)
wifi.setmode(wifi.STATION)

print('    set mode=STATION (mode='..wifi.getmode()..')')
print('    MAC: ',wifi.sta.getmac())
print('    chip: ',node.chipid())
print('    heap: ',node.heap())
print(' ')

-- Check the network status after the TIMEOUT interval
wifi.sta.connect()
tmr.alarm(1,1000,1,function()
  if wifi.sta.getip()==nil then
     print("    IP unavailable, waiting")
  else
     tmr.stop(1)
     print("\n    Config done, IP is "..wifi.sta.getip())
     print("    hostname: "..wifi.sta.gethostname())
     print("    ESP8266 mode is: " .. wifi.getmode())
     print("    MAC address is: " .. wifi.ap.getmac())
     -- init_i2c_display()
     print("\n    starting sensor read loop")
     print("    sending data to MQTT server @ " ..MQTTSERVER)
     dofile("tempumid.lua")
  end
end)
