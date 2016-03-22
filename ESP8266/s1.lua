version = "0.9.7"
CHANNEL_API_KEY = "WLDS1EKH6GRTK2QN"
delay = 60000
PINS =   {7,7}  --DHT22 data pin
FIELDS = {1,2}  --ThingSpeak fields
pinptr = 1

tempStr = " "
hmdyStr = " "

-- setup I2c and connect display
function init_i2c_display()
     -- SDA and SCL can be assigned freely to available GPIOs
     local sda = 6 -- GPIO14
     local scl = 5 -- GPIO12
     local sla = 0x3c
     print("  initializng I2c OLED display on pins "..sda.." and "..scl)    
     i2c.setup(0, sda, scl, i2c.SLOW)
     disp = u8g.ssd1306_128x64_i2c(sla)
     disp:setFont(u8g.font_6x10)
     disp:setFontRefHeightExtendedText()
     disp:setDefaultForegroundColor()
     disp:setFontPosTop()
     disp:setColorIndex(1)
end

-- update OLED display
function dispPage(line,text1,text2,text3)
   --picture loop
  disp:firstPage() 
  while disp:nextPage() do 
    disp:drawStr(0,line,text1)
    disp:drawStr(0,(line+15),text2)
    disp:drawStr(0,(line+27),text3)
    disp:drawStr(0,(line+45),"v "..version)
  end
end

--get data from DHT22 sensor on <pin>
function rdDHT22(pin)
    local tmp, hmd
    print("\n  reading pin "..PINS[pinptr])
    dht22 = require("dht22_min")
    --read sensor
    dht22.read(PINS[pinptr])
    tmp = dht22.getTemperature()
    hmd = dht22.getHumidity()
    if tmp == nil then
        print("*** Error reading temperature from DHT22")
    end
    if hmd == nil then
        print("*** Error reading humidity from DHT22")
    end
    --release DHT22 module
    dht22 = nil
    package.loaded["dht22"]=nil
    return tmp, hmd 
end

--post data <value> to ThingSpeak api key <key>, field <field>
function post(key,field,value)
    print("    posting pin "..PINS[pinptr].." data to field "..field.." value is "..value)   
    connout = nil
    connout = net.createConnection(net.TCP, 0)
    connout:on("receive", function(connout, payloadout)
        if (string.find(payloadout, "Status: 200 OK") ~= nil) then
            print("    Posted OK");
        end
    end)
    connout:on("connection", function(connout, payloadout) 
        print ("    Posting...");       
        connout:send("GET /update?api_key="
        .. key
        .. "&field"
        .. field
        .."=" 
        .. value
        .. " HTTP/1.1\r\n"
        .. "Host: 192.168.254.34:8080\r\n"
        .. "Connection: close\r\n"
        .. "Accept: */*\r\n"
        .. "User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n"
        .. "\r\n")
    end)
   
    connout:on("disconnection", function(connout, payloadout)
        connout:close();
        collectgarbage(); 
    end)
    connout:connect(8080,'192.168.254.34')
end 

--post data <value> to local apache2 server api key <key>, field <field>
function posta(key,field,value)
    print("    posting pin "..PINS[pinptr].." data to field "..field.." value is "..value)   
    connout = nil
    connout = net.createConnection(net.TCP, 0)
    connout:on("receive", function(connout, payloadout)
        if (string.find(payloadout, "Status: 200 OK") ~= nil) then
            print("    Posted OK");
        end
    end)
    connout:on("connection", function(connout, payloadout) 
        print ("    Posting...");       
        connout:send("GET /cgi-bin/Scon"
        .. "&field"
        .. field
        .."=" 
        .. value
        .. " HTTP/1.1\r\n"
        .. "Host: 192.168.254.34:8080\r\n"
        .. "Connection: close\r\n"
        .. "Accept: */*\r\n"
        .. "User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n"
        .. "\r\n")
    end)
    connout:on("disconnection", function(connout, payloadout)
        connout:close();
        collectgarbage(); 
    end)
    connout:connect(80,"192.168.254.34")
end

-- update temperature and humidity
function update()
    local t, h,send
    t, h = rdDHT22(PINS[pinptr]) --read the HDT22 sensor
    print("***")
    if pinptr % 2 ~= 0 then
        send = (t*9)/5 + 320
        print("    posting temperature ")
        tempStr = "    "..tostring(send/10).."."..tostring(send % 10).." deg F"  
    else
        send = h
        print("    posting humidity ")
        hmdyStr = "    "..tostring(send/10).."."..tostring(send % 10).." %" 
    end 
    dispPage(10,"Office Sensor",tempStr,hmdyStr)
    post(CHANNEL_API_KEY,FIELDS[pinptr],tostring(send/10).."."..tostring(send % 10))
    pinptr = pinptr + 1
    if pinptr > #PINS then pinptr = 1 end
end

-- ************** start main function ********************

    print("\n\n*** thermo.lua  version "..version.." ***")
    if (#PINS ~= #FIELDS) then 
        print("\n***** pin count and field count do not match\naborting")
    else
        print("  reading "..(#PINS / 2).." HDT22 sensor\n  posting data to ThingSpeak api key "..CHANNEL_API_KEY)
        init_i2c_display()  -- initialise the OLED display 
        dispPage(10," thermo.lua - "," "," ")
        print("  running update every "..delay.."ms\n  starting timer")
        tmr.alarm(4, delay, 1, update) -- execute update function every <delay> micro seconds
    end
