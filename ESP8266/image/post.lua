--read-post.lua

print("    post.lua called\n")

--get data from DHT22 sensor on <pin>
function rdDHT22(pin)
   local tmp, hmd
    print("    ESP8266: reading pin ".. pin)
    dht22 = require("dht22_min")
    --read sensor
    dht22.read(pin)
    tmp = dht22.getTemperature()
    hmd = dht22.getHumidity()
    if tmp == nil then
        print("ESP8266: *** Error reading temperature from DHT22")
    end
    if hmd == nil then
        print("ESP8266: *** Error reading humidity from DHT22")
    end
    --release DHT22 module
    dht22 = nil
    package.loaded["dht22"]=nil
    tmp = (tmp*9)/5 + 320
    return tmp, hmd 
end
 
 tmr.alarm(0,60000, 1, function()
 snum = 2
 temp,humidity = rdDHT22(7)
 --temp = 55
 --humidity = 15
 conn=net.createConnection(net.TCP, 0)
 conn:on("receive", function(conn, payload) print(payload) end )
 conn:connect(8080,"192.168.254.34")
-- mess = "GET /cgi-bin/Scon?snesor=" .. snum .. "&temp=" .. temp .. "&humid=" .. humidity .. " HTTP/1.1\r\nHost: 192.168.254.34\r\n"
--.."Connection: keep-alive\r\nAccept: */*\r\nUser-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n\r\n"
 mess = "GET /cgi-bin/Scon?snesor=" .. snum .. "&temp=" .. temp .. "&humid=" .. humidity .. "\r\n"
 conn:send(mess)
 end)
