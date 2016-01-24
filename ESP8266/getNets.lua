
function dispNet()
    print("   active networks:")
    wifi.sta.getap(function(t)
        for k,v in pairs(t) do
            print("      "..k.." "..v:sub(3,5).."dbi")
        end
    end)
end
dispNet()
