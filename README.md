# esp32_serial_bridge

esp32, serial, serial_bridge, tcp_console, raw, slip, kiss, hdlc,

-- README --

Simple Arduino sketch, an esp32 serial bridge with builtin tcp remote console.
It is bridge the main usb-uart serial to a remote host on tcp with nodelay sideoption.
Have an interactive menusystem on the same uart with a timeout option to do not disturb the later dataflow.
Also, a builtin unsecure tcp console server with simple passwd auth for remotly change settings.

Developed and tested on esp32 devkitv1 (ESP-WROOM-32) but should work with others as well.

Features:
- Plug and Play, simple use
- RAW, SLIP, KISS, HDLC framing modes
- buffer size option, rx/tx led indicator
- Interactive serial and console server menusystem for settings, 
- wifi, serial speed, serial mode, bridge dst ip/port, console server settings, etc..

### Skeleton:

    serial_bridge/
    ── serial_bridge.ino              // main
    ── ConfigManager.h / .cpp         // Default settings + EEPROM
    ── Framer.h / .cpp                // Framing structure for (0=RAW, 1=SLIP, 2=KISS, 3=HDLC)
    ── MenuSystem.h / .cpp            // Serial console menusystem for interactive settings
    ── TCPBridge.h / .cpp             // TCP bridge structure + serial buffer size and rx/tx LED indicator pin settings
    ── TCPConsoleServer.h / .cpp      // TCP console server structure and menusystem for remote interactive settings, with ex.: netcat (nc)


### Boot log and menu:

    BOOTING...

    SSID: ap
    PASSWD: verysecretkey
    ...
    WiFi connected - IP: 
    192.168.1.6
    TCP console server starting on port 23

    +======================[menu control]=====================+
    |-type:|'e' to enter settings menu, or 'x' exit serial now|
    |-n:| autoexiting serial within 10 seconds...             |
    +---------------------------------------------------------+

    +==========[serial control]==========+
    |-i:| running settings:
    |------------------------------------|
    | [mode]: 0
    | [speed]: 115200
    | [ssid]: ap
    | [tcp client]: 192.168.1.11:99
    | [tcp console]: enabled - on @ 23
    |------------------------------------|
    |->| [ 0 ] = set serial speed
    |->| [ 1 ] = set serial mode
    |->| [ 2 ] = set wifi,tcp settings
    |------------------------------------|
    |-e| [ e ] = enter into settings menu
    |-s| [ s ] = save settings
    |-r| [ r ] = reboot
    |-x| [ x ] = exit
    +====================================+
    |-i:| Entered into menu...

    +==========[serial speed]===========+
    |-i:| [running_speed]: 115200
    |-----------------------------------|
    |->| [ 0 ] = 115200  bps - default
    |->| [ 1 ] = 9600    bps
    |->| [ 2 ] = 300     bps
    |->| [ 3 ] = 1200    bps
    |->| [ 4 ] = 2400    bps
    |->| [ 5 ] = 4800    bps
    |->| [ 6 ] = 19200   bps
    |->| [ 7 ] = 38400   bps
    |->| [ 8 ] = 57600   bps
    |-s| [ s ] = save setting
    |-r| [ r ] = reboot
    |-b| [ b ] = boot menu
    |-x| [ x ] = exit
    +-----------------------------------+

    +=============[serial mode]===============+
    |-i:| [running_mode]: 0
    |-----------------------------------------|
    |->| [ 0 ] = raw_mode - default
    |->| [ 1 ] = slip_mode
    |->| [ 2 ] = kiss_mode
    |->| [ 3 ] = hdlc_mode
    |-----------------------------------------|
    |-s| [ s ] = save setting
    |-r| [ r ] = reboot
    |-b| [ b ] = boot menu
    |-x| [ x ] = exit
    |-i:|-------------------------------------|
    | [raw] - should be @ speed, nortscts
    | [ppp] - ~ 10.8KB/s , rts/cts
    | [kissattach],[ax.25] - ~ 8.25KB/s
    | [tncattach],[ethernet] - ~ 10.5KB/s
    |  serial no control ideal for experiment
    | [slip] framing as (0xC0,0xDB,0xDC,0xDD)
    |  rts/cts, 8N1, ideal for rawip,tcp_rawip
    | [kiss] framing as (0xC0,0xDB,0xDC,0xDD)
    |  ideal for kiss,tnc,ax.25
    | [hdlc] framing as (0x7E,0x7D,0x20) 
    |  ideal for hdlc abm framing, reliable
    |  direct,frame relay,X.25 - no crc
    +-----------------------------------------+

    +=========[wifi,tcp settings]=========+
    |-i:| running settings:
    |-------------------------------------|
    | [ssid]: ap
    | [passwd]: verysecretkey
    | [tcpdstIP:port]: 192.168.1.11:99
    | [tcpServer]: enabled @ 23
    | [auth passwd]: admin
    |-------------------------------------|
    |->| [ 0 ] = set wifi ap ssid
    |->| [ 1 ] = set wifi password
    |->| [ 2 ] = set remote tcp dst_ip
    |->| [ 3 ] = set remote tcp dst_port
    |->| [ 4 ] = set tcpconsole server, toggle on/off
    |->| [ 5 ] = set tcpconsole port
    |->| [ 6 ] = set tcpconsole auth passwd
    |-s| [ s ] = save setting
    |-r| [ r ] = reboot
    |-b| [ b ] = boot menu
    |-x| [ x ] = exit
    +-------------------------------------+

    [exit menu]

    +====================================+
    |-i:| autoexiting boot menu, starting serial bridge...

    !or 'x'

    [|-i:| exit menu now - let i/o flow]

    Connecting to 192.168.1.11:99 ...
    [TCP connected, let i/o flow...]

    !or

    Connecting to 192.168.1.11:99 ...
    TCP connection failed.

### TCP console server:

    radiirr@:~$ nc 192.168.1.6 23

    welcome to serial bridge console
    please enter password:> 
    incorrect password, try again...
    please enter password:> admin
    [ authentication successful ]
    type 'help' for commands.

    > help
    available commands:
    -------------------
      help                - show this help
      status              - show current/running config and status
      set speed N         - set serial speed [ 115200, 9600, 300, 1200, 2400, 4800, 19200, 38400, 57600 ]
      set mode M          - set serial mode [ raw, slip, kiss, hdlc ]
      set tcpdstip IP     - set TCP destination IP (e.g., 192.168.1.100)
      set tcpdstport PORT - set TCP destination port (e.g., 2323)
      save                - save current config
      reboot              - restart device
      exit                - close this console
    
    > status
    -------------------
    buffer = 8192 
    serial mode = raw 
    serial speed = 115200 
    wifi ssid = ap 
    tcp client = 192.168.1.11:99 
    tcp console = [ ENABLED ] @ 23 
    serial bridge status = [ CONNECTED ] 
    last reconnect attempt = 3337 ms ago 
    > 
