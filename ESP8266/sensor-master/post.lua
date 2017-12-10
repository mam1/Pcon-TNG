--read-post.lua

sensor=6
US_TO_MS = 1000
flip = 0
status = 0
temp = 0
humi = 0
temp_dec = 0
humi_dec = 0
beat = '#'

-- update OLED display
function dispPage()
   --picture loop
  disp:firstPage() 
  while disp:nextPage() do 
    disp:drawStr(0,7,wifi.sta.getip())
    disp:drawStr(10,24,"temp "..temp)
    disp:drawStr(10,36,"humidity "..humi)
    disp:drawStr(0,55,"SID "..sensor)
    disp:drawStr(60,55,"v"..version)
    disp:drawStr(123,58,flipit())
  end
end

function flipit()
    if flip==0 
        then
            flip = 1
            return '*'
        else
            flip = 0
            return ' '
     end
 end

--get data from DHT22 sensor on <pin>
function rdDHT22(pin,flip)
    print(" ESP8266: reading pin ".. pin)
    --read sensor
    status, temp, humi, temp_dec, humi_dec = dht.read(pin)
    if status == dht.OK then
    -- Float firmware using this example
        temp = (temp*9)/5 + 32
        print(" ESP8266: DHT Temperature:"..temp.."; ".."Humidity:"..humi)
        
    elseif status == dht.ERROR_CHECKSUM then
        print( " *** DHT Checksum error." )
    elseif status == dht.ERROR_TIMEOUT then
        print( " *** DHT timed out." )
    end
    --release DHT22 module
    dht22 = nil
    package.loaded["dht22"]=nil
    return temp, humi 
end

 print("    post.lua "..version.." called\n")
 tmr.alarm(4, 600, tmr.ALARM_AUTO, dispPage)
 tmr.alarm(0,60000, 1, function()
 temp,humidity = rdDHT22(7,flip)
 conn=net.createConnection(net.TCP, 0)
 conn:on("receive", function(conn, payload) print(payload) end )
 conn:connect(8080,ip)
 mess = "GET /cgi-bin/Scon&snesor=" .. sensor .. "&temp=" .. temp .. "&humid=" .. humidity .. "\r\n"
 conn:send(mess)
 end)
