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
import struct

#================= General constants =================
#Error codes definitions
ERR_OK = 0
ERR_NO_REPLY = 1
ERR_ERROR = 2
#Baud rate used in the USB serial communication
baud_rate=115200
#=================Stroker class descriptor=================
class stroker_descriptor:
	#Init function
	def __init__(self,usb_path):
		self.baud_rate = baud_rate
		self.serial=serial.Serial(usb_path,self.baud_rate)
	#Write to serial function
	def ser_write(self,data_to_write):
		self.serial.write(data_to_write)
		time.sleep(0.05)
	#Write to read function
	def ser_read(self):
		line=self.serial.readline()
		return line	
	#Close connection to Stroker
	def close_connection(self):
		self.serial.close()
	#Check for acknowledge reply
	def check_ack(self):
		data=self.ser_read()
		time.sleep(0.05)
		if data=='ACK\r\n':
			return ERR_OK
		else:
			return ERR_ERROR
	#Wait for end of initialization of stroker
	def wait_hardware_init(self):
		flag_done=False
		while(not(flag_done)):
			err=self.check_ack()
			if err==ERR_OK:
				flag_done=True
	#Send a movement segment
	def send_movement_segment(self,distance,speed,angle):
		#Create Text string, the format is
		# SET,LXXX,MSXX,SVXX
		#Where after L the 3 digit value is the distance, from 0 to 100
		#MS 2 digit value is the speed (2-16) and SV is the servo angle (0-180)
		data="SET,L%03d,MS%02d,SV%02d" % (distance,speed,angle)
		self.ser_write(data)
		time.sleep(0.05)
		return self.check_ack()
	# Clear all the movement segments
	def clear_movement_segments(self):
		self.ser_write("CLEAR")
		time.sleep(0.05)
		return self.check_ack()
	#Start movement of the arm. The movement only starts if the robot state is 'ready'
	def start_movement(self):
		self.ser_write('START')
		time.sleep(0.05)
		err=self.check_ack()
		if err==ERR_OK:
			data_incoming= True
			while data_incoming:
				#Start reading incoming data
				data=self.ser_read()
				if data=='ACK\r\n':
					data_incoming=False
				else:
					print data
			return ERR_OK
		else:
			return err


