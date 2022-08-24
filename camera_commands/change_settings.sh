# curl "http://192.168.0.240/control?var=framesize&val=9"  # 800x600
curl "http://192.168.0.240/control?var=framesize&val=10"  # 1024x768

# set gainceiling to 5 gives brighter image
curl "http://192.168.0.240/control?var=gainceiling&val=4"

# this is not enabled, but should control the led while streaming.
# curl "http://192.168.0.240/control?var=led_intensity&val=255" 
# we will however do something different and allow turning on/off led flash with a button
