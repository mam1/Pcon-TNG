pin = 7

status, temp, humi, temp_dec, humi_dec = dht.read(pin)
temp = (temp*9)/5 + 32

if status == dht.OK then
    -- Float firmware using this example
    print("DHT Temperature:"..temp..";".."Humidity:"..humi)

elseif status == dht.ERROR_CHECKSUM then
    print( "DHT Checksum error." )
elseif status == dht.ERROR_TIMEOUT then
    print( "DHT timed out." )
end
