remove this line to use/enable this script
--Cracker64's Lua Multiplayer Script
--See forum post http://powdertoy.co.uk/Discussions/Thread/View.html?Thread=14352 for more info
--Update to build 185 to have the mouse wheel change brush size!

--Version 1.1.0

--TODO's
--sync entire screen with stamps, tpt.load/stave_stamp needs to be changed first
--make WIND work? try to make FAN work
--Add custom deco editor (already finished in favorites menu script)
--CAPS_MODE, replace mode (for build 185+) --except cracker64 forgot to add a way to get SLALT, so now it must be disabled
--More work on multi-line chat
--Line tool is still off slightly
--Hacky method for PROP (read particle before it runs, check for a change after)

--These features are impossible (in v81), don't suggest them, even though they would be great
--load a save/stamp simultaneously
--gravity/wind modes
--signs, STKM controls
--AIR/VAC , WIND, PGRV/NGRV. no pressure,velocity,gravity get functions
--Deco editor (script doesn't run there)

--run lua commands typed in the chat window
--disable things that can't sync, re-enable with   TODO: way to re-enable, disable more
--= key resets pressure & velocity/sparks
--don't send empty lines in chat
--multiple lines in chat, up to 5(needs more work, optimizations)  v1.1.0
--floodfill delete while holding
--ESC key leaves chatbox_focus
--function to add data handlers so other scripts can add on functions
--Hopefully fixed altgr key    v1.0.9
--Interface flashes a bit after a disconnect --FIXED
--some keys enter spaces in chat --FIXED
--different language keyboard support --just Finnish right now, taking requests for others.
--WL_STREAM draws a bit too much --FIXED
--Drawing on CLNE and such sets ctype --Use a small brush 8 radius or smaller v1.0.8
--Block a few elements/tools from sending and notify
--Zoom window, yes, you can now use the zoom whenever and however you want, pixel art and electronics fans rejoice v1.0.7
--Floodfill ONLY with erase tool  --v1.0.6
--I broke right click, my bad --FIXED v1.0.5
--HEAT/COOL are now perfect (but still laggy)
--FULL BRUSH SUPPORT, triangle,square,circle,ovals.   mouse wheel changing(if using build 185 or higher) v1.0.4
--version check (notify if other person is higher) v1.0.3
--GoL elements besides first don't work and cause errors FIXED v1.0.2
--cursor (flash?) in chat box that moves (delete key) DONE v1.0.1
--connect command from chat window --DONE
--need better chat entering, and general looks--DONE
--chat box DONE 
--Walls --DONE
--FIGH and LIGH should only spawn once per click -FIXED
--Energy particles spawning too much --FIXED

loader,msg = package.loadlib("luasocket.dll","luaopen_socket_core") --this is the socket core dll, a few shortcut helper scripts are missing. If you want those get the full lua socket download
if loader then loader() else socket=require("socket") end --try loading normal location for socket

local PORT = 3000 --Change 3000 to your desired port

local KEYBOARD = 1 --only change if you have issues. Only other option right now is 2(finnish).
local tcp = nil
local version = 110
local otherversion = 0
local tptversion = tpt.version.build
local usenewbrush = tptversion>=185
local serverclient = nil
local client = nil
local lastpacket = os.clock()
local lastpsent = 0
local lastping = 0
local pingtimer = 0
local waitingforpong = false
local pingtext = ""
local sleft = 1 local sright = 0 --local salt=0 --for replace_mode
local myleft = 1 local myright = 0 --local myalt=0
local lastmx = 0 local lastmy = 0
local lastsent = ""
local statustext = ""
local infotext = ""
local infoalpha = 0
local otheruser = ""
local mybrx = 2 local mybry = 2
local otherbrx = 2 local otherbry = 2
local mybrushmode = 0 --0 circle, 1 square, 2 tri
local otherbrushmode = 0
local kmod = 0
local startx = 0 local starty = 0
local releasetype = 0
local pausenextframe = false
local datasent = 0
local sendbuffer = 0.0167
local otherfps = 60 local myfps = 60
local fpstime = os.clock()
local fpstimer = 0
local fpscapped = false
local init = true

local chatbox_hidden = false
local chatbox_messages = {}
local chatbox_focus = false
local chatbox_textbox = ""
local chatbox_newmessage = false
local chatbox_cursorpos = 0
local wheel = 0

local ZSIZE = 16
local ZFACTOR = math.floor(256/ZSIZE)
local zoom_en = 0
local zoom_x = math.floor((612-ZSIZE)/2)
local zoom_y = math.floor((384-ZSIZE)/2)
local zoom_wx = 0
local zoom_wy = 0
local zoom_trig = 0

local GoLrule = {{0,0,0,0,0,0,0,0,0,2},{0,0,1,3,0,0,0,0,0,2},{0,0,1,3,0,0,2,0,0,2},{0,0,0,2,3,3,1,1,0,2},{0,1,1,2,0,1,2,0,0,2},{0,0,0,3,1,0,3,3,3,2},{0,1,0,3,0,3,0,2,1,2},{0,0,1,2,1,1,2,0,2,2},{0,0,1,3,0,2,0,2,1,2},{0,0,0,2,0,3,3,3,3,2},{0,0,0,3,3,0,0,0,0,2},{0,0,0,2,2,3,0,0,0,2},{0,0,1,3,0,1,3,3,3,2},{0,0,2,0,0,0,0,0,0,2},{0,1,1,3,1,1,0,0,0,2},{0,0,1,3,0,1,1,3,3,2},{0,0,1,1,3,3,2,2,2,2},{0,3,0,0,0,0,0,0,0,2},{0,3,0,3,0,3,0,3,0,2},{1,0,0,2,2,3,1,1,3,2},{0,0,0,3,1,1,0,2,1,4},{0,1,1,2,1,0,0,0,0,3},{0,0,2,1,1,1,1,2,2,6},{0,1,1,2,2,0,0,0,0,3},{0,0,2,0,2,0,3,0,0,3}}
--get different lists for other language keyboards
local keyboardshift = { {before=" qwertyuiopasdfghjklzxcvbnm1234567890-=.,/`|;'[]\\",after=" QWERTYUIOPASDFGHJKLZXCVBNM!@#$%^&*()_+><?~\\:\"{}|",},{before=" qwertyuiopasdfghjklzxcvbnm1234567890+,.-'´¨<",after=" QWERTYUIOPASDFGHJKLZXCVBNM!\"#¤%&/()=?;:_*`^>",}  }
local keyboardaltrg = { {nil},{before=" qwertyuiopasdfghjklzxcvbnm1234567890+,.-'¨<",after=" qwertyuiopasdfghjklzxcvbnm1@£$€6{[]}\\,.-'~|",},}

function shift(s)
    if keyboardshift[KEYBOARD]~=nil then
        return (s:gsub("(.)",function(c)return keyboardshift[KEYBOARD]["after"]:sub(keyboardshift[KEYBOARD]["before"]:find(c,1,true))end))
    else return s end
end
function altgr(s)
    if keyboardaltgr[KEYBOARD]~=nil then
        return (s:gsub("(.)",function(c)return keyboardaltgr[KEYBOARD]["after"]:sub(keyboardaltgr[KEYBOARD]["before"]:find(c,1,true))end))
    else return s end 
end

function onstartup()
    tcp = socket.bind("*",PORT,0)
    tcp:settimeout(0)
end
--some luasocket shortcuts
function socket.bind(host, port, backlog)
    local sock, err = socket.tcp()
    if not sock then return nil, err end
    sock:setoption("reuseaddr", true)
    local res, err = sock:bind(host, port)
    if not res then return nil, err end
    res, err = sock:listen(backlog)
    if not res then return nil, err end
    return sock
end
function socket.connect(address, port, laddress, lport)
    local sock, err = socket.tcp()
    if not sock then return nil, err end
    if laddress then
        local res, err = sock:bind(laddress, lport, -1)
        if not res then return nil, err end
    end
    local res, err = sock:connect(address, port)
    if not res then return nil, err end
    return sock
end

function step()
    if init then init=false onstartup() end
    local currenttime = os.clock()
    tpt.drawtext(240,10,statustext,255,255,0)
    tpt.drawtext(135,10,"Data sent: "..datasent)
    tpt.drawtext(85,10,pingtext)
    --tpt.drawtext(85,50,myleft)
    if infoalpha > 0 then tpt.drawtext(240,20,infotext,255,255,255,infoalpha) infoalpha = infoalpha-5 end
    if tpt.get_name() == "" then statustext = "You need to login first" return end
    if pausenextframe then pausenextframe=false tpt.set_pause(1) end
    --check for new connections
    if serverclient == nil and client == nil then
        statustext = "Waiting...  or type '/c ip port' right in here! \\/"
        serverclient, err = tcp:accept()
        if serverclient~= nil then serverclient:settimeout(0)
            statustext = "Got a connection..."
            init_connection()
        end
    end
    --check streams for data
    if serverclient~=nil then
        data, err = serverclient:receive('*l')
        if data~=nil then readdata(data) end
    elseif client~=nil then
        data, err = client:receive('*l')
        if data~=nil then readdata(data) end
    end
    --timeout/ping
    if (serverclient~=nil or client~=nil) then
        local timeout = currenttime-lastpacket
        local pingtimeout = currenttime-lastping
            if timeout>15 then
                if serverclient ~= nil then serverclient:close() serverclient=nil end
                if client ~= nil then client:close() client=nil end
                infotext = "Lost connection to " .. otheruser  infoalpha = 255
                otheruser = "" pingtext = ""
            end
        if pingtimeout>2 then
            lastping = currenttime
            send("ping")
        end
    end
    --update brush selections
    if myleft~=tpt.selectedl or myright~=tpt.selectedr then
        myleft = tpt.selectedl
        myright = tpt.selectedr
        send("s " .. myleft .. " " .. myright)
    end
    --new brush size sync
    if usenewbrush and (mybrx~=tpt.brushx or mybry~=tpt.brushy) then
        mybrx = tpt.brushx
        mybry = tpt.brushy
        send("b " .. mybrx .. " " .. mybry)
    end
    --fps sync
    if fpstimer>=60 then 
        fpstimer = 0
        myfps = math.floor(60/(currenttime-fpstime))
        fpstime = currenttime
        send("fps " .. myfps)
    end
    fpstimer = fpstimer + 1
    if otheruser~="" then tpt.drawtext(16,27,otheruser .. " FPS: " .. otherfps) end
    
    update_chatbox()
    if jacob1s_mod then update_options() end

    send("")--try sending buffer
end

function update_options()
    local ngrav = tpt.newtonian_gravity()
    if ngrav ~= oldngrav then send("ngrav " .. ngrav) end
    oldngrav = ngrav

    local aheat = tpt.ambient_heat()
    if aheat ~= oldaheat then send("aheat ".. aheat) end
    oldaheat = aheat

    local decoenable = tpt.decorations_enable()
    if decoenable ~= olddecoenable then send("deco ".. decoenable) end
    olddecoenable = decoenable

    local heatsim = tpt.heat()
    if heatsim ~= oldheatsim then send("heat ".. heatsim) end
    oldheatsim = heatsim
end

local datahandlers = {
b = function(words) otherbrx=tonumber(words[2]) otherbry=tonumber(words[3]) if words[4]~=nil then otherbrushmode=tonumber(words[4]) end end,
d = function(words) --draw left click
    local n = tonumber(words[2])
    x = n%1024
    y = math.floor(n/1024)
    if tonumber(words[3])~=nil then
        drawstuff(x,y,otherbrx,otherbry,sleft,true,otherbrushmode)
    else
        create_line(lastmx,lastmy,x,y,otherbrx,otherbry,sleft,otherbrushmode)
    end
    lastmx = x
    lastmy = y end,
r = function(words)
    local n = tonumber(words[2])
    x = n%1024
    y = math.floor(n/1024)
    if tonumber(words[3])~=nil then
        drawstuff(x,y,otherbrx,otherbry,sright,true,otherbrushmode)
    else
        create_line(lastmx,lastmy,x,y,otherbrx,otherbry,sright,otherbrushmode)
    end
    lastmx = x
    lastmy = y end,
l = function(words) --line left
    create_line(tonumber(words[2]),tonumber(words[3]),tonumber(words[4]),tonumber(words[5]),otherbrx,otherbry,sleft,otherbrushmode) end,
lr = function(words) --line right
    create_line(tonumber(words[2]),tonumber(words[3]),tonumber(words[4]),tonumber(words[5]),otherbrx,otherbry,sright,otherbrushmode) end,
bo = function(words) --box left
    create_box(tonumber(words[2]),tonumber(words[3]),tonumber(words[4]),tonumber(words[5]),sleft) end,
br = function(words) --box right
    create_box(tonumber(words[2]),tonumber(words[3]),tonumber(words[4]),tonumber(words[5]),sright) end,
s = function(words) --selected elements
    sleft = tonumber(words[2])
    sright = tonumber(words[3]) end,
clear = function(words)
    clearsim()
    infotext = otheruser .. " cleared the screen"
    infoalpha = 255 end,
pause = function(words)
    if words[2] ~= nil then
        tpt.set_pause(tonumber(words[2]))
    else
        tpt.toggle_pause()
    end
    infotext = otheruser .. " toggled pause"
    infoalpha = 255 end,
n = function(words) --username
    otheruser = words[2]
    statustext = "Connected with " .. otheruser end,
fps = function(words) --fps sync
    otherfps = tonumber(words[2])
    --if otherfps<myfps then fpscapped=true tpt.setfpscap(math.max(math.min(otherfps+1,60),3)) --fpscap sync fails epicly
    --elseif fpscapped then if otherfps>=59 then fpscapped=false end tpt.setfpscap(math.max(math.min(otherfps+1,60),3)) end
    sendbuffer = math.min(1/otherfps,0.1) end,
f = function(words) --framestep, syncs pause as well
    pausenextframe = true
    tpt.set_pause(0) end,
ch = function(words) --chat
    table.insert(chatbox_messages, "o:".. command:sub(4))
    if chatbox_hidden then chatbox_newmessage = true end end,
cm = function(words) --chat
    table.insert(chatbox_messages, "o:".. command:sub(4))
    if chatbox_hidden then chatbox_newmessage = true end
	runluacode(command:sub(4)) end,
ping = function(words)
    send("pong") end,
pong = function(words)
    pingtext = "Ping: " .. math.ceil((os.clock()-lastping)*1000) end,
ver = function(words)
    otherversion = tonumber(words[2])
    if otherversion>version then statustext = otheruser.." has a newer version" end end,
jver = function(words)
    otherjversion = tonumber(words[2])
    disabled = {[226]=true,[241]=true,[246]=true,[248]=true,[249]=true,[250]=true,[251]=true,[252]=true,[253]=true} end,
ff = function(words)
    x=tonumber(words[2]) y=tonumber(words[3]) c=tonumber(words[4])
    floodparts(x,y,c,-1,-1) end,
ngrav = function(words)
    oldngrav = tonumber(words[2]) tpt.newtonian_gravity(oldngrav)
    infotext = otheruser .. " toggled newtonian gravity"
    infoalpha = 255 end,
aheat = function(words)
    oldaheat = tonumber(words[2]) tpt.ambient_heat(oldaheat)
    infotext = otheruser .. " toggled ambient heat"
    infoalpha = 255 end,
deco = function(words)
    olddecoenable = tonumber(words[2]) tpt.decorations_enable(olddecoenable)
    infotext = otheruser .. " toggled decorations"
    infoalpha = 255 end,
heat = function(words)
    oldheatsim = tonumber(words[2]) tpt.heat(oldheatsim)
    infotext = otheruser .. " toggled heat simulation"
    infoalpha = 255 end,
resetp = function(words)
    if jacob1s_mod then tpt.reset_pressure()
    else tpt.set_pressure(0,0,152,95,0) end
    tpt.reset_velocity()
    infotext = otheruser .. " reset the pressure"
    infoalpha = 255 end,
resets = function(words)
    tpt.reset_spark()
    infotext = otheruser .. " reset sparks"
    infoalpha = 255 end,
cmode = function(words)
    if jacob1s_mod then tpt.display_mode(words[2]) end end, -- still says not implimented in tpt, even though I put back set_cmode
}
--function that you can call from other scripts to recieve new packets and run functions
function newdatahandler(mess,func)
    datahandlers[mess]=func
end
--newdatahandler("waffle",function(words) tpt.create(200,200,1) end)

function readdata(data)
    lastsent = data
    lastpacket = os.clock()
    local i = string.find(data,"|")
    --go through all commands from the line
    --infotext=lastsent  infoalpha=255
    while data~=nil do
        if i~=nil then
           command = string.sub(data,1,i-1)
           --cut the first command off
           data = string.sub(data,i+1)
           i = string.find(data,"|")
        else command=data data=nil --no data left
        end
        local words = {}
        for word in string.gmatch(command, "%w+") do table.insert(words,word) end
        --switch table for reading data, yay!
        if type(datahandlers[words[1]])=="function" then datahandlers[words[1]](words) end
    end
end

function split_message(mess)
    local message=mess
    local width=tpt.textwidth(message)
    local lines = {}
    local k=1
    while width>152 do
        local j=15 
        local nextwidth = tpt.textwidth(message:sub(1,j))
        while nextwidth<=152 do
            j=j+1
            nextwidth = tpt.textwidth(message:sub(1,j))
        end
        j=j-1
        lines[k]=message:sub(1,j)
        message=message:sub(j+1)
        width=tpt.textwidth(message)
        k=k+1
    end
    lines[k]=message
    return lines
end

function connect(ip,port)
    if tpt.get_name() == "" then tpt.log("You need to be logged in") return end
    client, err = socket.connect(ip,port)
    if client~= nil then
        tpt.log("Connected")
        client:setoption("keepalive",true)
        client:settimeout(0)
        init_connection()
    else
        tpt.log("Fail")
    end
end

function init_connection()
   tpt.set_pause(1)
   clearsim()
   lastpacket=os.clock()
   tpt.set_console(0)
   tpt.newtonian_gravity(0) oldngrav = 0
   tpt.ambient_heat(0) oldaheat = 0
   tpt.decorations_enable(1) olddecoenable = 1
   tpt.display_mode(3)
   tpt.heat(1) oldheatsim = 1
   tpt.setfpscap(60)
   chatbox_messages = {}
   send("n "..tpt.get_name().."|s "..myleft.." "..myright.."|b ".. mybrx.." "..mybry.." "..mybrushmode.."|ver "..version)--send name
   if jacob1s_mod then send("jver " .. jacob1s_mod) end
   disabled = { [226]=true,[236]=true,[239]=true,[241]=true,[243]=true,[244]=true,[246]=true,[248]=true,[249]=true,[250]=true,[251]=true,[252]=true,[253]=true}
end

local messagebuffer = ""
function send(message)
    local timesincelast = os.clock()-lastpsent
    if messagebuffer == "" then messagebuffer=message return end
    if (message~="") then
        messagebuffer = messagebuffer.."|"..message
    end
    if (timesincelast<= sendbuffer or (message=="" and messagebuffer=="")) then
        return
    end
    lastpsent = os.clock()
    if client~= nil then
        local sent = client:send(messagebuffer .. "\n")
        if sent then  datasent = datasent + sent end
    elseif serverclient~= nil then
        local sent = serverclient:send(messagebuffer .. "\n")
        if sent then datasent = datasent + sent end
    else
        --tpt.log("No client")
    end
    messagebuffer = ""
end

function movechatcursor(amount)
    chatbox_cursorpos = chatbox_cursorpos+amount
    if chatbox_cursorpos>string.len(chatbox_textbox) then chatbox_cursorpos = string.len(chatbox_textbox) return end
    if chatbox_cursorpos<0 then chatbox_cursorpos = 0 return end
end

function chatkeyprocess(key,nkey)
    --replace some keys for specific keyboard layouts, make a list later
    if KEYBOARD==2 then if key=="=" then key="´" end if key=="\\" then key="'" end if key=="]" then key="¨" end end
    if nkey==275 then movechatcursor(1) return false end
    if nkey==276 then movechatcursor(-1) return false end
    if nkey==27 then chatbox_focus=false tpt.set_shortcuts(1) return false end
    if nkey==8 then chatbox_textbox = chatbox_textbox:sub(1,chatbox_cursorpos-1)..chatbox_textbox:sub(chatbox_cursorpos+1) movechatcursor(-1) return false end
    if nkey==127 then chatbox_textbox = chatbox_textbox:sub(1,chatbox_cursorpos)..chatbox_textbox:sub(chatbox_cursorpos+2)  return false end
    if (nkey==13 or nkey==271) and chatbox_textbox~="" then --enter, about to send, check for commands
        if otheruser=="" and chatbox_textbox:find("/c") then
            local ip = chatbox_textbox:sub(4,chatbox_textbox:find(" ",4))
            local port = chatbox_textbox:sub(chatbox_textbox:find(" ",4)+1)
            if tonumber(port) then connect(ip,tonumber(port)) else infotext="Bad command" infoalpha=255 end
            chatbox_textbox = "" chatbox_cursorpos = 0
            return false
        end
        if chatbox_textbox:find("tpt.") then
		    send("cm "..chatbox_textbox)
			table.insert(chatbox_messages,chatbox_textbox)
            runluacode(chatbox_textbox)
		    chatbox_textbox = ""
            return false
        end
        send("ch "..chatbox_textbox) table.insert(chatbox_messages,chatbox_textbox) chatbox_textbox = "" return false
    end
    --tpt.drawtext(200,210,nkey)
    if nkey<32 or nkey>=127 then return false end
    local addkey = (kmod==1 or kmod==2) and shift(key) or key
    if (math.floor(kmod/512))==1 then addkey=altgr(key) end
    if tpt.textwidth(chatbox_textbox .. addkey)<760 then chatbox_textbox=chatbox_textbox:sub(1,chatbox_cursorpos)..addkey..chatbox_textbox:sub(chatbox_cursorpos+1) movechatcursor(1) end
    
    return false
end

function update_chatbox()
    --icon on right
    local chaty = 169
    if jacob1s_mod then chaty = chaty - 17 end
    if chatbox_hidden then
        if chatbox_newmessage then tpt.drawrect(613,chaty,14,14,255,0,0) else tpt.drawrect(613,chaty,14,14,255,255,255) end
        tpt.drawline(614,chaty+6,627,chaty+6,255,255,255,255)
        tpt.drawline(614,chaty+6,617,chaty+3,255,255,255,255)
        tpt.drawline(614,chaty+6,617,chaty+9,255,255,255,255)
    else
        chatbox_newmessage=false
        tpt.drawrect(613,chaty,14,14,255,255,255,255)
        tpt.drawline(614,chaty+6,627,chaty+6,255,255,255,255)
        tpt.drawline(627,chaty+6,624,chaty+3,255,255,255,255)
        tpt.drawline(627,chaty+6,624,chaty+9,255,255,255,255)
    end
    --the main box
    if not chatbox_hidden then
        tpt.fillrect(455,27,156,157,0,0,0,160)
        tpt.drawrect(455,27,156,157,255,255,255,95)
        tpt.drawrect(456,28,154,155,255,255,255,255)
        tpt.drawline(457,169,609,169,255,255,255,200)
        while #chatbox_messages >15 do table.remove(chatbox_messages,1) end
        local i = 0
        local offset = 0
        while chatbox_messages[i+1]~= nil do
            local mess = chatbox_messages[i+1]
            if mess:find("o:") then --other user message brighter than self
                local lines = split_message(mess:sub(3))
                for k=1,#lines do
                    tpt.drawtext(458,30 + (i+offset+(k-1))*9,lines[k])
                    if k==#lines then offset=offset+(k-1) end
                end
            else
                local lines = split_message(mess)
                for k=1,#lines do
                    tpt.drawtext( 609 - tpt.textwidth(lines[k]) ,30 + (i+offset+(k-1))*9,lines[k],200,200,200)
                    if k==#lines then offset=offset+(k-1) end
                end
            end
            if i+offset>14 then table.remove(chatbox_messages,1) end --crappy line handling
            i = i+1
        end
        --display only last 152 length of textbox
        local width=tpt.textwidth(chatbox_textbox)
        local display_textbox = chatbox_textbox
        local display_cursorpos = chatbox_cursorpos
        while width>152 do
            display_textbox = display_textbox:sub(2)
            width=tpt.textwidth(display_textbox)
            display_cursorpos = display_cursorpos - 1
        end
        tpt.drawtext(458,173,display_textbox)
        if chatbox_focus then
            local cursoradjust=tpt.textwidth(display_textbox:sub(1,display_cursorpos))+1
            tpt.drawline(457+cursoradjust,171,457+cursoradjust,183,255,255,255)
            tpt.drawrect(456,169,154,14,255,255,0)
        end
    end
end

function keyclicky(key,nkey,modifier,event)
    kmod = modifier%1024--ignore numlock and caps lock for now
    --tpt.drawtext(200,200,nkey)
    if key=="z" then --TPT still can still open zoom with shortcut lock, so we need to as well
        if event==1 then
            if kmod==256 then zoom_trig= (zoom_trig>=1 and 0 or 2)
            else zoom_trig=1 end
        end
        if event==2 and zoom_trig==1 then zoom_trig = 0 end
        if zoom_trig==0 and zoom_en==1 then zoom_en=0 end--might go in step
        if zoom_en==2 then zoom_en=1 end
    end
    --ignore releases and modifiers
    if (event==2 or (nkey>=300 and nkey<=308)) then return true end
    if chatbox_focus then
        return chatkeyprocess(key,nkey)
    end
    if nkey == 49 and (kmod == 1 or kmod == 2) then send("cmode 9") -- doesn't check if in debug mode
    elseif nkey > 48 and nkey < 58 then send("cmode ".. nkey-49)
    elseif nkey == 48 then send("cmode 10") end
    if key==" " then send("pause") end
    if key=="f" then send("f") end
    if key=="=" then if kmod == 64 or kmod == 128 then send("resets") else send("resetp") end end
    if nkey==9 then mybrushmode=(mybrushmode+1)%3 send("b "..mybrx.." "..mybry.." "..mybrushmode) end --tab
    if nkey == 91 then --Left bracket
       if kmod == 256 or kmod == 512 then mybrx=mybrx-1 mybry=mybry-1 --alt
       elseif kmod == 1 or kmod == 2 then mybrx=mybrx-1 --shift
       elseif kmod == 64 or kmod == 128 then mybry=mybry-1 --ctrl
       else mybrx=mybrx-math.ceil((mybrx/5)+.1) mybry=mybry-math.ceil((mybry/5)+.1)
       end
       if mybrx < 0 then mybrx = 0 end
       if mybry < 0 then mybry = 0 end
       if zoom_trig>0 then
           ZSIZE = ZSIZE-1
           if ZSIZE<2 then ZSIZE=2 end
           ZFACTOR = math.floor(256/ZSIZE)
       end
       send("b " .. mybrx .. " " .. mybry)
    elseif nkey == 93 then --Right bracket
       if kmod == 256 or kmod == 512 then mybrx=mybrx+1 mybry=mybry+1 --alt
       elseif kmod == 1 or kmod == 2 then mybrx=mybrx+1 --shift
       elseif kmod == 64 or kmod == 128 then mybry=mybry+1 --ctrl
       else mybrx=mybrx+math.ceil((mybrx/5)+.1) mybry=mybry+math.ceil((mybry/5)+.1)
       end
       if mybrx > 800 then mybrx = 800 end
       if mybry > 800 then mybry = 800 end
       if zoom_trig>0 then
           ZSIZE = ZSIZE+1
           if ZSIZE>60 then ZSIZE=60 end
           ZFACTOR = math.floor(256/ZSIZE)
       end
       send("b " .. mybrx .. " " .. mybry)
    end
    if key == "b" and (kmod == 64 or kmod == 128) then
        if olddecoenable == 1 then olddecoenable = 0 else olddecoenable = 1 end
        tpt.decorations_enable(olddecoenable)
        send("deco " .. olddecoenable)
        return false
     end
     if key == "u" then
        if oldaheat == 1 then oldaheat = 0 else oldaheat = 1 end         
        tpt.ambient_heat(oldaheat)
        send("aheat " .. oldaheat)
        return false
     end
   return true
end

function mouse_coords_window_to_sim(window_x,window_y)
    if (zoom_en>0 and window_x>=zoom_wx and window_y>=zoom_wy and window_x<(zoom_wx+ZFACTOR*ZSIZE) and window_y<(zoom_wy+ZFACTOR*ZSIZE)) then
        return (math.floor((window_x-zoom_wx)/ZFACTOR)+zoom_x) , (math.floor((window_y-zoom_wy)/ZFACTOR)+zoom_y)
    else
        return window_x,window_y
    end
end

local releasestrings = {[1]="l ",[2]="lr ",[3]="bo ",[4]="br "}
function mouseclicky(mousex,mousey,button,event,mouse_wheel)
    wheel = mouse_wheel --wheel should be set to 0 after all used per frame (if used outside this function)
    --no need for wheel checks to change brush size, we can get the size directly
    --tpt.drawtext(200,200,tostring(button))
    if zoom_trig>0 then
        ZSIZE = ZSIZE+wheel
        if ZSIZE>60 then ZSIZE=60 end
        if ZSIZE<2 then ZSIZE=2 end
        ZFACTOR = math.floor(256/ZSIZE)
        if zoom_en<2 then zoom_en=1 end
    end
    if event==2 and zoom_trig>0 and zoom_en<2 then
        local x = mousex-math.floor(ZSIZE/2) local y = mousey-math.floor(ZSIZE/2)
        if x<0 then x=0 end if y<0 then y=0 end
        if x>612-ZSIZE then x=612-ZSIZE end if y>384-ZSIZE then y=384-ZSIZE end
        zoom_x = x
        zoom_y = y
        zoom_wx = (x<306) and 612-ZSIZE*ZFACTOR or 0
        zoom_wy = 0
        zoom_en = 2
        zoom_trig=0
        return true
    end
    if zoom_trig>0 then return true end
    --because this function will now trigger on mouse wheel, the event can possibly be 0
    if event==0 then return true end
    if otheruser ~="" and (disabled[myleft] and button==1 or disabled[myright] and button==4) and mousex < 612 and mousey < 384 then
        infotext = "This element does NOT send to your partner" infoalpha=255 return false
    end
    mousex,mousey = mouse_coords_window_to_sim(mousex,mousey)
    if releasetype > 0 then
        if event==2 then
            send(releasestrings[releasetype] .. startx .. " " .. starty .. " " .. mousex .. " " .. mousey)
            releasetype = 0
        end
        return true
    end
    if (event==2) then
        return true 
    end
    
    if not chatbox_hidden and mousex>=457 and mousex<=609 and mousey>=170 and mousey<=182 then
        if event==3 and not chatbox_focus then return true end
        chatbox_focus = true
        tpt.set_shortcuts(0)
        return false
    elseif event~=3 then
        tpt.set_shortcuts(1)
        chatbox_focus = false
    end
    
    if (mousey > 404 and event == 1) then
        --clear button
        if (mousex >= 486 and mousex <= 502) then
            send("clear")
            return true
        --pause button
        elseif (mousex >= 613) then
            send("pause")
            return true
        end
    end
    if (mousex > 612 and mousex < 628 and event == 1) then
        --chat hide button
        local chaty = 169
        if jacob1s_mod then chaty = chaty - 17 end
        if (mousey>=chaty and mousey<=chaty+14) then
            chatbox_hidden = not chatbox_hidden
            if not chatbox_hidden then chatbox_focus = true end
        end

        local extra = 0 if jacob1s_mod then extra = 16 end
        if (mousey > 32+extra and mousey < 48+extra) then
           if olddecoenable == 1 then olddecoenable = 0 else olddecoenable = 1 end
           tpt.decorations_enable(olddecoenable)
           send("deco " .. olddecoenable)
           return false
        end
        if (mousey > 48+extra and mousey < 64+extra) then
           if oldngrav == 1 then oldngrav = 0 else oldngrav = 1 end
           tpt.newtonian_gravity(oldngrav)
           send("ngrav " .. oldngrav)
           return false
        end
        if (mousey > 64+extra and mousey < 80+extra) then
           if oldaheat == 1 then oldaheat = 0 else oldaheat = 1 end
           tpt.ambient_heat(oldaheat)
           send("aheat " .. oldaheat)
           return false
        end
    end

    if (mousey >= 384 or mousex >= 612) then
        return true
    end
    if (button%2 == 1) then
        if (kmod == 1) then startx,starty = mousex,mousey releasetype=1 return true end
        if (kmod == 64) then startx,starty = mousex,mousey releasetype=3 return true end
        if (kmod == 65) then send("ff "..mousex.." "..mousey.." "..sleft) return true end
        send("d " .. (mousex + mousey*1024) ..  (event==1 and " 1" or "") )
    elseif (button == 4) then
        if (kmod == 1) then startx,starty = mousex,mousey releasetype=2 return true end
        if (kmod == 64) then startx,starty = mousex,mousey releasetype=4 return true end
        if (kmod == 65) then send("ff "..mousex.." "..mousey.." "..sright) return true end
        send("r " .. (mousex + mousey*1024) ..  (event==1 and " 1" or "")  )
    end
end

function runluacode(luacode)
    local env = {
        ipairs = ipairs,
        next = next,
        pairs = pairs,
        pcall = pcall,
        tonumber = tonumber,
        tostring = tostring,
        string = string,
        table = table,
        math = math,
        os = { clock = os.clock, difftime = os.difftime, time = os.time, date = os.date },
        tpt = tpt
    }
    if luacode:byte(1) == 27 then return nil end
    local func, message = loadstring(luacode)
    if not func then return nil end
    setfenv(func, env)
    pcall(func)
end

if tpt.version.jacob1s_mod then
   disabled = { [226]=true,[236]=true,[239]=true,[241]=true,[243]=true,[244]=true,[246]=true,[248]=true,[249]=true,[250]=true,[251]=true,[252]=true,[253]=true}

   create_line = tpt.create_line

   function drawstuff(x,y,rx,ry,c,fillbool,brush)
      fill = 1
      if fillbool == false then
         fill = 0
      end
      return tpt.create_parts(x,y,rx,ry,c,fill,brush)
   end

   floodparts = tpt.floodfill

   clearsim = tpt.clear_sim

   function create_box(x1,y1,x2,y2,c)
      local i = 0 local j = 0
      if x1>x2 then i=x2 x2=x1 x1=i end
      if y1>y2 then j=y2 y2=y1 y1=j end
      for j=y1, y2 do
         for i=x1, x2 do
           tpt.create_parts(i,j,0,0,c,1)
         end
      end
   end

   jacob1s_mod = tpt.version.jacob1s_mod

else -- functions not needed when using jacob1's mod

energytypes = { [18]=true, [31]=true, [136]=true, }
validwalls = { [222]=true, [223]=true, [224]=true, [225]=true, [227]=true, [228]=true, [229]=true, [230]=true, [231]=true, [232]=true, [233]=true, [234]=true, [235]=true, [240]=true, [242]=true, [245]=true, }
tools = { [236]=true, [237]=true, [238]=true, [239]=true, }
disabled = { [226]=true,[236]=true,[239]=true,[241]=true,[243]=true,[244]=true,[246]=true,[248]=true,[249]=true,[250]=true,[251]=true,[252]=true,[253]=true}
setctype = { [9]=true,[74]=true,[83]=true,[85]=true,[93]=true,[153]=true,}

function create(x,y,c,small)
    local t = c%256
    local v = math.floor(c/256)
    if t==237 or t==238 then --heat and cool!
        local temp = tpt.get_property("type",x,y)>0 and tpt.get_property("temp",x,y) or nil
        if t==237 and temp~=nil and temp<9999 then
            local heatchange = 4 --implement more temp changes later (ctrl-shift)
            tpt.set_property("temp",math.min(temp+heatchange,9999),x,y)
        elseif t==238 and temp~=nil and temp>0 then
            local heatchange = 4
            tpt.set_property("temp",math.max(temp-heatchange,0),x,y)
        end
        return
    end
    if small then --set ctype for CLNE types if smallish brush
        if not setctype[c] then
            local cm = tpt.get_property("type",x,y)
            if setctype[cm] then
                if (cm==74 or cm==153) and (c==35 or c==36) then return end --don't give PCLN ctypes of PSCN or NSCN
                tpt.set_property("ctype",c,x,y)
                return
            end
        end
    end
    if t==0 then 
        tpt.delete(x,y)
    else
        local i = tpt.create(x,y,t)
        --special case for GoL creation
        if t==78 and i>=0 then tpt.set_property("ctype",v,i) tpt.set_property("tmp",GoLrule[(v+2)][10],i) end
    end
end

function create_line(x1,y1,x2,y2,rx,ry,c,brush) -- Taken From Powder Toy Source Code
   if c == 87 or c == 158 then return end --never do lines of FIGH and LIGH
   local cp = math.abs(y2-y1)>math.abs(x2-x1)
   local x = 0 local y = 0 local dx = 0 local dy = 0 local sy = 0 local e = 0.0 local de = 0.0 local first = true
   if cp then y = x1 x1 = y1 y1 = y y = x2 x2 = y2 y2 = y end
   if x1 > x2 then y = x1 x1 = x2 x2 = y y = y1 y1 = y2 y2 = y end
   dx = x2 - x1 dy = math.abs(y2 - y1) if dx ~= 0 then de = dy/dx end
   y = y1 if y1 < y2 then sy = 1 else sy = -1 end
   for x = x1, x2 do
      if cp then
         drawstuff(y,x,rx,ry,c,first,brush)
      else
         drawstuff(x,y,rx,ry,c,first,brush)
      end
      first = false
      e = e + de
      if e >= 0.5 then
         y = y + sy
         e = e - 1
         if y1<y2 then
             if y>y2 then return end
         elseif y<y2 then return end
         if (rx+ry)==0 or c>=222 then
            if cp then
               drawstuff(y,x,rx,ry,c,first,brush)
            else
               drawstuff(x,y,rx,ry,c,first,brush)
            end
         end
      end
   end
end

function create_box(x1,y1,x2,y2,c)
   local i = 0 local j = 0
   if x1>x2 then i=x2 x2=x1 x1=i end
   if y1>y2 then j=y2 y2=y1 y1=j end
   for j=y1, y2 do
      for i=x1, x2 do
        create(i,j,c)
      end
   end
end

function incurrentbrush(i,j,rx,ry,brush)
    if brush==0 then
        return (math.pow(i,2)*math.pow(ry,2)+math.pow(j,2)*math.pow(rx,2)<=math.pow(rx,2)*math.pow(ry,2))
    elseif brush==1 then
        return (math.abs(i)<=rx and math.abs(j)<=ry)
    elseif brush==2 then
        return ((math.abs((rx+2*i)*ry+rx*j) + math.abs(2*rx*(j-ry)) + math.abs((rx-2*i)*ry+rx*j))<=(4*rx*ry))
    else
        return false
    end
end

function drawstuff(x,y,rx,ry,c,fill,brush) -- Draws or erases elements
   local energycheck = energytypes[c]
   local small = (rx+ry)<=16
   if c == 87 or c == 158 then create(x,y,c) return end --only draw one pixel of FIGH and LIGH
   --walls!
   local dw = false local b = 0
   if validwalls[c] then
      dw = true
      if c~=230 then b = c-100 end
   end
   if tools[c] then fill = true end
   if dw then
      local r=math.floor(rx/4) 
      local wx = math.floor(x/4)- math.floor(r/2) 
      local wy = math.floor(y/4)- math.floor(r/2)
      if b==125 then
         wx = wx + math.floor(r/2)
         wy = wy + math.floor(r/2)
         for v=-1,1 do
            for u=-1,1 do
                if (wx+u>=0 and wx+u<153 and wy+v>=0 and wy+v<96 and tpt.get_wallmap(wx+u,wy+v)==125) then
                    return
                end end end
         tpt.set_wallmap(wx,wy,b)
         return
      end
      tpt.set_wallmap(wx,wy,r+1,r+1,b)
      return
   end
   if rx<=0 then--0 radius loop prevention
      for j=y-ry,y+ry do
         if valid(x,j,energycheck,c) then
            create(x,j,c,small) end
      end
      return
   end
   local tempy = y local oldy = y
   if brush==2 then tempy=y+ry end
   for i = x - rx, x do
      oldy = tempy
      local check = incurrentbrush(i-x,tempy-y,rx,ry,brush)
      if check then
          while check do
             tempy = tempy - 1
             check = incurrentbrush(i-x,tempy-y,rx,ry,brush)
          end
          tempy = tempy + 1
          if fill then
             local jmax = 2*y - tempy
             if brush == 2 then jmax=y+ry end
             for j = tempy, jmax do
                if valid(i,j,energycheck,c) then
                   create(i,j,c,small) end
                if i~=x and valid(x+x-i,j,energycheck,c) then
                   create(x+x-i,j,c,small) end
             end
          else
             if (oldy ~= tempy and brush~=1) or i==x-rx then oldy = oldy - 1 end
             for j = tempy, oldy+1 do
                local i2=2*x-i local j2 = 2*y-j
                if brush==2 then j2=y+ry end
                if valid(i,j,energycheck,c) then
                   create(i,j,c,small) end
                if i2~=i and valid(i2,j,energycheck,c) then
                   create(i2,j,c,small) end
                if j2~=j and valid(i,j2,energycheck,c) then
                   create(i,j2,c,small) end
                if i2~=i and j2~=j and valid(i2,j2,energycheck,c) then
                   create(i2,j2,c,small) end
             end
          end
       end
   end
end
function valid(x,y,energycheck,c)
    if x >= 0 and x < 612 and y >= 0 and y < 384 then 
        if energycheck then 
            if energytypes[tpt.get_property("type",x,y)] then return false end
        end
    return true end
end
function floodparts(x,y,c,cm,bm)
    local x1=x local x2=x
    if cm==-1 then
        if c==0 then
            cm = tpt.get_property("type",x,y)
            if cm==0 then return false end
        else
            cm = 0
        end
    end
    --wall check here
    while x1>=4 do
        if (tpt.get_property("type",x1-1,y)~=cm) then break end
        x1 = x1-1
    end
    while x2<=608 do
        if (tpt.get_property("type",x2+1,y)~=cm) then break end
        x2 = x2+1
    end
    for x=x1, x2 do
        if c==0 then tpt.delete(x,y) end
        if c>0 and c<222 then create(x,y,c) end
    end
    if y>=5 then
        for x=x1,x2 do
            if tpt.get_property("type",x,y-1)==cm then
                if not floodparts(x,y-1,c,cm,bm) then
                    return false
                end end end
    end
    if y<379 then
        for x=x1,x2 do
            if tpt.get_property("type",x,y+1)==cm then
                if not floodparts(x,y+1,c,cm,bm) then
                    return false
                end end end
    end
    return true
end

function clearsim()
    tpt.start_getPartIndex()
    while tpt.next_getPartIndex() do
       local index = tpt.getPartIndex()
       tpt.set_property("type",0,index)
    end
    tpt.reset_velocity(0,0,153,96)
    tpt.set_pressure(0,0,153,96,0)
    tpt.set_wallmap(0,0,153,96,0)
end

end

tpt.register_keypress(keyclicky)
tpt.register_mouseclick(mouseclicky)
tpt.register_step(step)
