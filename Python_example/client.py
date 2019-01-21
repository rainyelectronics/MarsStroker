#=========================================================================
#       Partial Stroker protocol implementation in Python 2.7
#               		Written by :
#			 Bart Garcia Nathan		bart.garcia.nathan@gmail.com
#			 Pepe Espinoza Mendoza  joseluisesp@gmail.com 
#							 Sep 2018
#=========================================================================
import serial
import time
import sys
#Stroker protocol
import stroker_protocol as stroker_prot
#================= General constants =================
#USB path in the OS, Change as necessary
usb_path='/dev/ttyACM0'
#usb_path='/dev/ttyUSB0'
#================= Stroker descriptor ================
stroker=stroker_prot.stroker_descriptor(usb_path)
#=================    Main program    ================
print '>> Stroker is initializing ...'
stroker.wait_hardware_init()
print '>> Initialization passed'
time.sleep(1)
print '>> Send complete movement configuration'
#The first value represents the lenght in 'digital units', when the total movement possible is
#100 units. The second value is the motor speed, which can be used between the values of 0-16
#The third value is the servo angle, which can be 0-180
err=stroker.send_movement_segment(100,12,20)
if err!= stroker_prot.ERR_OK:
	print "Error on the command"
#Clear the previous movement configuration
print '>> Clear the previous movement configuration'
err=stroker.clear_movement_segments()
if err!= stroker_prot.ERR_OK:
	print "Error on the command"
#Set a new movement configuration, parted in 3 segments. Increasing speed and decreasing angle
print '>> Send new movement configuration in 3 segments'
err=stroker.send_movement_segment(33,8,60)
if err!= stroker_prot.ERR_OK:
	print "Error on the command"
err=stroker.send_movement_segment(33,10,50)
if err!= stroker_prot.ERR_OK:
	print "Error on the command"
err=stroker.send_movement_segment(34,12,40)
if err!= stroker_prot.ERR_OK:
	print "Error on the command"
#Start movment of the robot, print position values until end of movement
print '>> START movement'
err=stroker.start_movement()
if err!= stroker_prot.ERR_OK:
	print "Error on the command"

#====== Close the connections
stroker.close_connection()
