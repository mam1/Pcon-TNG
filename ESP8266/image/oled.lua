 -- setup I2c and connect display
function init_i2c_display()
     -- SDA and SCL can be assigned freely to available GPIOs
     local sda = 6 -- GPIO14
     local scl = 5 -- GPIO12
     local sla = 0x3c
     print("  initializng I2c OLED display on pins "..sda.." and "..scl)    
     i2c.setup(0, sda, scl, i2c.SLOW)
     disp = u8g.ssd1306_128x64_i2c(sla)
     disp:setFont(u8g.font_6x10)
     disp:setFontRefHeightExtendedText()
     disp:setDefaultForegroundColor()
     disp:setFontPosTop()
     disp:setColorIndex(1)
end

function draw()
    disp:drawStr(10,10,"*** %%% ***")
end  


   init_i2c_display()

  --picture loop
  disp:firstPage() 
  while disp:nextPage() do 
        draw()
        end
