print('\n *** MQTT tempumid.lua ver 0.2')
-- tempumid.lua
CNAME="shop-office" -- Client name 
AMBIENT="home"  -- Ambient name
TTOPIC     = AMBIENT.."/"..CNAME.."/temperature"  -- Temperature topic
HTOPIC     = AMBIENT.."/"..CNAME.."/humidity"  -- Humidity topic 
STOPIC     = AMBIENT.."/"..CNAME.."/status"  -- Status topic
CTOPIC     = AMBIENT.."/"..CNAME.."/command" --Command topic   
MTOPIC     = AMBIENT.."/"..CNAME.."/monitor" --Monitor topic
 
sleep_in_seconds= nil --Sleep in seconds
 
TUPDATE = 15 -- Time interval for update monitor in seconds
TUPTEMP = 10 -- Time interval for update temperature and humidity values
SPIN    =  7 -- Sensor pin
TGEN    =  5 -- Time interval in seconds usend in various trm
 
-- Define Client Name and connection parameters
mqt = mqtt.Client(CNAME, 60, "", "", 1) 
 
-- Subscribe to the topics and send status to the status topic
mqt:on("connect", function() 
    mqt:subscribe(STOPIC, MQTTQOS)
    mqt:subscribe(CTOPIC, MQTTQOS)
    mqt:subscribe(MTOPIC, MQTTQOS)
    mqt:subscribe(TTOPIC, MQTTQOS)
    mqt:subscribe(HTOPIC, MQTTQOS)
    send_status("Sensor ready again")
end )
 
-- Callback when mqtt is offline
mqt:on("offline", function() print("mqtt offline");  end)
 
--Callback to manage messages 
mqt:on("message", function(client, topic, data) 
    manage_message(client, topic, data) 
end)
 
-- Function to filter the messages
function manage_message(client, topic, data) 
    -- If CTOPIC: command topic
    if topic == CTOPIC then
        --print("Command")
        -- If the command is Restart
        if data == "Restart" then
            send_status("Restarting")
            send_mstatus("Restarting")
            tmr.alarm(2, TGEN*1000, tmr.ALARM_SINGLE, node.restart)
            --print("restart")
        -- If the command is null    
        elseif data == nil then
           -- print("Command not valid")
            send_status("Command not valid")
        
        
        else
            -- Check the Sleep command           
            i=string.len(data)
            dato=string.sub(data,1,5)
          
            if dato == "Sleep" then
                ndato=string.sub(data,6)
                TSLEEP=tonumber(ndato)
                if TSLEEP ~= nil then
                    if TSLEEP >= 1 then
                        -- Sets the interval in seconds to sleep
                        sleep_in_seconds=TSLEEP*60
                        esp_sleep()
                    else
                        -- Send an error in Sleep command
                        send_status("Sleep command error")
                    
                    end
                else
                    -- Send an error in Sleep command 
                    send_status("Sleep command error")
                end
 
            else
                -- Command not managed           
                send_status("Command not valid")      
 
            end
            
        end
        
    end
end
 
-- Function to send messages to STOPIC: status topic
function send_status(message)
    mqt:publish(STOPIC, message, MQTTQOS, 0)
    
end
 
-- Fucntion to send messages to MTOPIC: monitor topic
function send_mstatus(message)
    mqt:publish(MTOPIC, message, MQTTQOS, 0)
    
end
 
-- Function to display temp and hum
function display_temphum(temp,humi)
    print("Temperature=%d, Humidity=%d",temp,humi)
end
 
 
-- Function to read and publish temperature and humidity
function read_temp_hum(client)
    status, temp, humi, temp_dec, humi_dec = dht.read(SPIN)
    if status == dht.OK then
    
        mqt:publish(TTOPIC, temp,MQTTQOS, 0)
        mqt:publish(HTOPIC, humi, MQTTQOS, 0)
        
       -- display_temphum(temp,humi)
       
    elseif status == dht.ERROR_CHECKSUM then
        print( "DHT Checksum error." )
    elseif status == dht.ERROR_TIMEOUT then
        print( "DHT timed out." )
    end
end
 
 
-- Function that sleeps esp8266
function esp_sleep()
 
    send_status("Sensor going to sleep now")
    send_mstatus("Sensor going to sleep now")
    -- Delayd sleep to send the message above.
    tmr.alarm(3, TGEN*1000, tmr.ALARM_SINGLE, function() 
        node.dsleep(sleep_in_seconds*1000000) 
    end)
end
 
-- Connection to the mqtt server at the mqttport
print("trying to connect ")
mqt:connect(MQTTSERVER, MQTTPORT, 0, 0)
print("connected ...")
--Sends periodically the temperature and humidity to the topics
tmr.alarm(1, TUPTEMP*1000, tmr.ALARM_AUTO, function() read_temp_hum(mqt) end)
 
-- Sends periodically a message to the monitor topic
tmr.alarm(4, TUPDATE*1000, tmr.ALARM_AUTO, function() send_mstatus("Sensor Ready") end)
