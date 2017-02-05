 --US_TO_MS = 100
 flip=0
 
 tmr.alarm(0,1000, 1, function()
   disp:firstPage()
   if flip==0 
    then
        flip = 1
        while disp:nextPage() do 
       --   disp:drawStr(85,7,"*")
          disp:drawStr(123,58," ")
        end
    else
        flip = 0
        while disp:nextPage() do 
    --      disp:drawStr(85,7," ")
          disp:drawStr(123,58,"*")
        end
   end
 -- tmr.delay(60 * US_TO_MS)
 end)