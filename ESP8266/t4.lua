-- print ap list
function listap(t)
      print("\nactive networks:")
      for k,v in pairs(t) do
        print("  "..k.." : "..v)
      end
end
wifi.sta.getap(listap)

