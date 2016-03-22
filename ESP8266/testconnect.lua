 tmr.alarm(0,30000, 1, function()

 snum = 2
 temp = 780
 humidity = 450
 conn=net.createConnection(net.TCP, 0)
 conn:on("receive", function(conn, payload) print(payload) end )
 conn:connect(8080,"192.168.254.34")
 
 mess = "GET /cgi-bin/Scon?snesor=" .. snum .. "&temp=" .. temp .. "&humid=" .. humidity .. " HTTP/1.1\r\nHost: 192.168.254.34\r\n"
 .."Connection: keep-alive\r\nAccept: */*\r\nUser-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n\r\n"
 conn:send(mess)
 
 --conn:send("GET /cgi-bin/Scon?snesor=" .. snum .. "&temp=" .. t .. " HTTP/1.1\r\nHost: 192.168.254.34\r\n"
-- .."Connection: keep-alive\r\nAccept: */*\r\nUser-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n\r\n")

 -- conn:send("GET /update?api_key="
 --        .. key
 --        .. "&field"
 --        .. field
 --        .."=" 
 --        .. value
 --        .. " HTTP/1.1\r\n"
 --        .. "Host: 192.168.254.34\r\n"
 --        .. "Connection: close\r\n"
 --        .. "Accept: */*\r\n"
 --        .. "User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n"
 --        .. "\r\n")
 end)
