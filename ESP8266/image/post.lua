--read-post.lua
version="0.1.1"
sensor=2
-- update OLED display
function dispPage(sensor,temp,humid)
   --picture loop
  disp:firstPage() 
  while disp:nextPage() do 
    disp:drawStr(0,7,wifi.sta.getip())
    disp:drawStr(10,24,"temp "..temp)
    disp:drawStr(10,36,"humidity "..humid)
    disp:drawStr(0,55,"SID "..sensor)
    disp:drawStr(60,55,"v"..version)
  end
end

print("    post.lua "..version.." called\n")

--get data from DHT22 sensor on <pin>
function rdDHT22(pin)

    print(" ESP8266: reading pin ".. pin)
    --read sensor
    status, temp, humi, temp_dec, humi_dec = dht.read(pin)
    if status == dht.OK then
    -- Float firmware using this example
        temp = (temp*9)/5 + 32
        print(" ESP8266: DHT Temperature:"..temp.."; ".."Humidity:"..humi)
        dispPage(sensor,temp,humi)

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

 tmr.alarm(0,60000, 1, function()
 temp,humidity = rdDHT22(7)
 conn=net.createConnection(net.TCP, 0)
 conn:on("receive", function(conn, payload) print(payload) end )
 conn:connect(8080,"192.168.254.34")
 mess = "GET /cgi-bin/Scon?snesor=" .. sensor .. "&temp=" .. temp .. "&humid=" .. humidity .. "\r\n"
 conn:send(mess)
 end)
