
 tmr.alarm(0,30000, 1, function()
 t=444
 conn=net.createConnection(net.TCP, 0)
 conn:on("receive", function(conn, payload) print(payload) end )
 conn:connect(8080,"192.168.254.34")
 -- conn:send("GET /json.htm?type=command&param=udevice&idx=32&nvalue=0&svalue=" .. t .. " HTTP/1.1\r\nHost: 192.168.254.34\r\n"
--  .."Connection: keep-alive\r\nAccept: */*\r\n\r\n")
 connout:send("GET /update?api_key="
        .. key
        .. "&field"
        .. field
        .."=" 
        .. value
        .. " HTTP/1.1\r\n"
        .. "Host: 192.168.254.34\r\n"
        .. "Connection: close\r\n"
        .. "Accept: */*\r\n"
        .. "User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n"
        .. "\r\n")
 end)