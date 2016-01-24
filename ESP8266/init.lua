function startup()
    if abort == true then
        print('startup aborted')
        return
        end
    print('\n*** running startup.lua')
    dofile('startup.lua')
    end
    
abort = false
print("run Snippet0 to abort")
tmr.alarm(0,5000,0,startup)
