/**
 * Stroker main program for Version 0.3 (using breakout boards)
 * File: <Stroker.ino>
 * Auth: <Bart Garcia-Nathan> 					<<bart.garcia.nathan@gmail.com>>
 * Auth: <Jose Espinoza Mendoza> 				<<joseluisesp@gmail.com>>
 * Date: <March 2019>
 */
#include <Stepper.h>
#include <Servo.h>
#include "Stroker_pinout.h"

// initialize the stepper library
Stepper Stepper_mot(Stepper_steps, STPR1, STPR2, STPR3, STPR4);
Servo Servo_mot;
int position_counter, programmed_length_data, programmed_lenght_back, mottor_speed_data, mottor_speed_back, servo_pos_data, servo_pos_back, commands_counter, commands_counter_back, servo_adjust_pos,home_return_speed;
float commands_stack_length[10],commands_stack_speed[10],commands_stack_servo[10];
float commands_stack_length_back[10],commands_stack_speed_back[10],commands_stack_servo_back[10];
String serial_input;
bool error_flag, ready_flag, ready_fordward_flag, ready_backward_flag, execute_flag, execute_flag_back, start_exec_pos, home_flag, first_start, toggle_led, busy_flag,start_exec_pos_init;
bool serial_listen, forward_flag;
float motor_movement_res = 10,total_time;
double return_length;

//Static configuration flags/defines
#define DEBUG true
#define LIVE_POS false


void setup() {
  // set the initial motor speed
  Stepper_mot.setSpeed(10);
  //Setup the servo motor
  Servo_mot.attach(Servo_pin);
  // initialize the serial port:
  Serial.begin(115200);
  Serial.setTimeout(50);
  // Set LED pins as outputs
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  digitalWrite(ON_LED,HIGH);//Set the ON LED
  digitalWrite(LED2,LOW);
  digitalWrite(LED3,LOW);
  digitalWrite(LED4,LOW);
  //Set digital inputs as inputs  
  pinMode(HOME_SW,  INPUT);
  pinMode(START_SW, INPUT);
  pinMode(STOP_SW,  INPUT);
  pinMode(IR_pin,   INPUT);
  //print
  if(DEBUG) Serial.println("Initializing robotic stroker V0.3 ...");
  //initializations
  position_counter=0;
  programmed_length_data=0;
  programmed_lenght_back=0;
  mottor_speed_data=0;
  mottor_speed_back=0;
  servo_pos_data=0;
  servo_pos_back=0;
  commands_counter=0;
  commands_counter_back=0;
  servo_adjust_pos=0;
  home_return_speed=10;//Variable to control the speed of the return to home, Init speed set to safety 10 (slow)
  total_time=0;//Variable used to calculate total time of movement
  ready_flag= false;
  ready_fordward_flag=false;
  ready_backward_flag=false;
  execute_flag=false;
  execute_flag_back=false;
  start_exec_pos=false;
  home_flag = true;
  first_start = true;
  toggle_led = false;
  busy_flag = false;
  start_exec_pos_init = false;
  serial_listen = false;
  servo_adjust_pos=90;
}

void loop() {
//-----Find the Home/start position----
//Set motor position to a slow movement (ensure safe return)
if(first_start){
  Stepper_mot.setSpeed(5);
  if(DEBUG) Serial.println("Getting the stroker to home...");
  first_start = false;
}
if(home_flag){
  //Get to the home position on a clockwise motion
  busy_flag = true;
  serial_listen = false;
  //Set motor movement X steps
  Stepper_mot.step(motor_movement_res);
  //Toggle LED
    if(toggle_led){
      digitalWrite(BUSY_LED,HIGH);//Set the ON LED
      toggle_led = false;
    }
    else{
     digitalWrite(BUSY_LED,LOW);//Clear the BUSY LED
     toggle_led = true;  
    }
  //check for IR to detect end of movement, and set flag
	if(!digitalRead(IR_pin)){ //If the IR pin is low, then home has been found
    home_flag=false;
    //Move the motor to start of execution position
    start_exec_pos=true;
    start_exec_pos_init=true;
    digitalWrite(BUSY_LED,LOW);//Clear the BUSY LED
    }   
}
//-----Wait for configuration commands
//Clear direction flag
forward_flag=false;
//Set segment value strings is SET,LXXX,MSXX,SVXX for forward and SET,RXXX,MSXX,SVXX for backward
//Example SET,L100,MS10,SV90
if(serial_listen & !busy_flag){
 while(Serial.available()){
        serial_input = Serial.readString();
        serial_input.trim();
        //-----For a SET command
        if(serial_input.substring(0,3) == "SET"){ //Check for SET command                  
          if(serial_input.length() != 18){ //Check for correct command lenght
          	error_flag=true;
          	break;
          	}
            //Check for length forward
           if(serial_input.substring(4,5) == "L"){ 
            programmed_length_data= serial_input.substring(5,8).toInt();
            forward_flag=true;//Set forward flag
            //Serial.println(programmed_length_data);
           }
           //Check for lenght backwards
           else if(serial_input.substring(4,5) == "R"){
           	programmed_lenght_back= serial_input.substring(5,8).toInt();
           	forward_flag=false;//Clear flag to set backward direction  
           }
           else{
           error_flag=true;
            break;
            }
            //Check for mottor speed
           if(serial_input.substring(9,11) == "MS"){
           	if(forward_flag){//Set for forward
	            mottor_speed_data= serial_input.substring(11,13).toInt();
	        }
	        else{//Set for backward
   	            mottor_speed_back= serial_input.substring(11,13).toInt();
	        } 
            //Serial.println(mottor_speed_data);
           }
           else{
           error_flag=true;
            break;
            }
           //Check for servo position
           if(serial_input.substring(14,16) == "SV"){
           	if(forward_flag){//Set for forward
            	servo_pos_data= serial_input.substring(16,18).toInt();
            }
            else{//Set for backward
            	servo_pos_back= serial_input.substring(16,18).toInt();            
            }
            	//Serial.println(servo_pos_data);
           }
           else{
           error_flag=true;
            break;
            }
     //-----End of data decoding for SET command,now process
            //Data obtained without errors, send an ACK back
            Serial.println("ACK");
            //Store obtained data
            if(forward_flag){//Set for forward
	            commands_stack_length[commands_counter]=programmed_length_data;
	            commands_stack_speed[commands_counter]=mottor_speed_data;
	            commands_stack_servo[commands_counter]=servo_pos_data;
	            //Increase commands counter
	            commands_counter+=1;
	            //Check if the 100 digital units of forward length have been configured
	            int summed_length=0;
	            for(int i=0; i<commands_counter;i++){
	              summed_length+=commands_stack_length[i];
	             }
    	        if(summed_length==100){
	              ready_fordward_flag=true;
	            }
	            //Check for overflow stack
	            if(summed_length>100){
	              if(DEBUG) Serial.println("Stack Overflow, last command has been removed from the stack");
	              commands_stack_length[commands_counter]=0;
	              commands_stack_speed[commands_counter]=0;
	              commands_stack_servo[commands_counter]=0;
	              commands_counter-=1;
	              error_flag=true;         
	            }
	         }
	         else {//Set for backward
	         	commands_stack_length_back[commands_counter_back]=programmed_lenght_back;
	            commands_stack_speed_back[commands_counter_back]=mottor_speed_back;
	            commands_stack_servo_back[commands_counter_back]=servo_pos_back;
	            //Increase commands counter
	            commands_counter_back+=1;
	            //Check if the 100 digital units of backward length have been configured
	            int summed_length=0;
	            for(int i=0; i<commands_counter_back;i++){
	              summed_length+=commands_stack_length_back[i];
	            }
	            if(summed_length==100){
	              ready_backward_flag=true;
	            }
	            //Check for overflow stack
	            if(summed_length>100){
	              if(DEBUG) Serial.println("Stack Overflow, last command has been removed from the stack");
	              commands_stack_length_back[commands_counter_back]=0;
	              commands_stack_speed_back[commands_counter_back]=0;
	              commands_stack_servo_back[commands_counter_back]=0;
	              commands_counter_back-=1;
	              error_flag=true;         
	            }                
	         }
	         //If both directions are ready, turn on READY LED
	         if(ready_fordward_flag && ready_backward_flag){
	         	digitalWrite(READY_LED,HIGH);//Set the Ready LED
	         	ready_flag=true; //Set ready flag
	         }
        }//End of SET command
        
        
        //------For Clear command
        else if(serial_input.substring(0,5) == "CLEAR"){ //Check for CLEAR command
          //Clear commands stack  
          for(int i=0; i<10;i++){
            commands_stack_length[i]=0;
            commands_stack_speed[i]=0;
            commands_stack_servo[i]=0;
            commands_stack_length_back[i]=0;
            commands_stack_speed_back[i]=0;
            commands_stack_servo_back[i]=0;
           }   
           //Clear completion flag
           ready_flag=false;
           digitalWrite(READY_LED,LOW);//Clear the READY LED
           //Clear commands counter
           commands_counter=0;
           commands_counter_back=0;
           Serial.println("ACK");
        }
        //------For Read command
        else if(serial_input.substring(0,4) == "READ"){ //Check for READ command
          //Print current list
          if(commands_counter>0 || commands_counter_back > 0){
            for(int i=0; i<commands_counter;i++){
              Serial.print("L:");
              Serial.print(commands_stack_length[i],0);
              Serial.print(" MS:");
              Serial.print(commands_stack_speed[i],0);
              Serial.print(" SV:");
              Serial.println(commands_stack_servo[i],0);           
             }
             
             for(int i=0; i<commands_counter_back;i++){
              Serial.print("R:");
              Serial.print(commands_stack_length_back[i],0);
              Serial.print(" MS:");
              Serial.print(commands_stack_speed_back[i],0);
              Serial.print(" SV:");
              Serial.println(commands_stack_servo_back[i],0);            
             }
          }
          else{
            if(DEBUG)Serial.println("Empty Stack");
          }
           Serial.println("ACK");          
        }
       //------For TIME command
        else if(serial_input.substring(0,4) == "TIME"){ //Check for TIME command
          //Print times forward
          if(commands_counter>0 || commands_counter_back > 0){
          	total_time=0.0;
          	//Calculate total time
          	for(int i=0; i<commands_counter;i++){
          		total_time+=(60/commands_stack_speed[i])*(commands_stack_length[i]/200);
          	}
          	for(int i=0; i<commands_counter_back;i++){
          		total_time+=(60/commands_stack_speed_back[i])*(commands_stack_length_back[i]/200);
          	}
          	//Print total time:
		  	Serial.print("T:");
		  	Serial.print(total_time);
		  	Serial.print(",P:");
            for(int i=0; i<commands_counter;i++){//Print forward segments
              Serial.print((60/commands_stack_speed[i])*(commands_stack_length[i]/200));
              if(i<(commands_counter-1)){
              	Serial.print(",");
              }
             }
             if(commands_counter_back>0){//Print comma if there are segments for backwards
             	Serial.print(",");
             }
            for(int i=0; i<commands_counter_back;i++){//Print backwards segments
              Serial.print((60/commands_stack_speed_back[i])*(commands_stack_length_back[i]/200));
              if(i<(commands_counter_back-1)){
              	Serial.print(",");
              }
             }
             
            Serial.println();              
          }else{
            if(DEBUG)Serial.println("Empty Stack");
          }
           Serial.println("ACK");          
        }
       //-------For Start command
       else if(serial_input.substring(0,5) == "START"){ //Check for START command
        //If the stack has been fully configured, set execute flag to start execution
        if(ready_flag){
          execute_flag=true;
          busy_flag = true;
          serial_listen = false;
          Serial.println("ACK");
        }
        else{
          if(DEBUG) Serial.println("The full length forward and backward has not been configured yet.");
          error_flag=true;    
        }
       }
       //-------Wrong command
       else{
           error_flag=true;
       }
      }
}
if(error_flag){
	Serial.println("ERR");
	if(DEBUG) Serial.println("ERR:Wrong command. Errors will not be tolerared");
  // Flash the error LED for 500 ms
  digitalWrite(ERROR_LED,HIGH);//Set the Error LED
  delay(100);
  digitalWrite(ERROR_LED,LOW);//Set the Error LED
  error_flag=false; 
}

//-----Execute program, send positions, return to previous state after execution
float completed_length=0;
int completed_stack_counter=0;
int completed_stac_length=0;
while(execute_flag){
  //Find current stack command to be executed
    if(completed_stack_counter<(commands_counter-1) & completed_length >=(completed_stac_length+commands_stack_length[completed_stack_counter])){
      completed_stac_length+=commands_stack_length[completed_stack_counter];
      completed_stack_counter+=1;      
  }
    //Set servo in position
    Servo_mot.write(commands_stack_servo[completed_stack_counter]);
    //Set motor speed
    Stepper_mot.setSpeed(commands_stack_speed[completed_stack_counter]);
    //Set motor movement X steps
    Stepper_mot.step(motor_movement_res);
    //Add steps to counter
    //Half a revolution of the motor represents the full 100 digital length units. Therefore, the linear increase
    //is obtained by deviding half a revolution steps by the amount of steps taken
    completed_length +=(motor_movement_res/(Stepper_steps/2))*100;
    //Send by Serial the new position
    if(LIVE_POS){ //Print in case the live position is defined as true
    	Serial.print("L:");
	    Serial.print(completed_length);
    	Serial.print(" MS:");
	    Serial.print(commands_stack_speed[completed_stack_counter]);
	    Serial.print(" SV:");
	    Serial.println(commands_stack_servo[completed_stack_counter]);
    }
    //Toggle LED
    if(toggle_led){
      digitalWrite(BUSY_LED,HIGH);//Set the ON LED
      toggle_led = false;
    }
    else{
     digitalWrite(BUSY_LED,LOW);//Set the ON LED
     toggle_led = true;  
    }
    //check for IR to detect end of movement, and set flag
    //if(!digitalRead(IR_pin)){ //If the IR pin is low, then the half turn has been completed
    if(completed_length >= 100){//End of forward movement      
        execute_flag=false;
        execute_flag_back=true;
        //Signal the start of return movement, so the HOST is aware of the direction of movement
        if(DEBUG) Serial.println("Starting return to init position");
    }
}//End of forward execution loop

//Execute the return sequence
float completed_length_back=0;
int completed_stack_counter_back=0;
int completed_stac_length_back=0;
while(execute_flag_back){
  //Find current stack command to be executed
    if(completed_stack_counter_back<(commands_counter_back-1) & completed_length_back >=(completed_stac_length_back+commands_stack_length_back[completed_stack_counter_back])){
      completed_stac_length_back+=commands_stack_length_back[completed_stack_counter_back];
      completed_stack_counter_back+=1;      
  }
    //Set servo in position
    Servo_mot.write(commands_stack_servo_back[completed_stack_counter_back]);
    //Set motor speed
    Stepper_mot.setSpeed(commands_stack_speed_back[completed_stack_counter_back]);
    //Set motor movement X steps
    Stepper_mot.step(-motor_movement_res);//Negative value, to move the arm clockwise
    //Add steps to counter
    //Half a revolution of the motor represents the full 100 digital length units. Therefore, the linear increase
    //is obtained by deviding half a revolution steps by the amount of steps taken
    completed_length_back +=(motor_movement_res/(Stepper_steps/2))*100;
    //Send by Serial the new position
    if(LIVE_POS){ //Print in case the live position is defined as true    
	    Serial.print("R:");
	    Serial.print(completed_length_back);
	    Serial.print(" MS:");
	    Serial.print(commands_stack_speed_back[completed_stack_counter_back]);
	    Serial.print(" SV:");
	    Serial.println(commands_stack_servo_back[completed_stack_counter_back]);
    }
    //Toggle LED
    if(toggle_led){
      digitalWrite(BUSY_LED,HIGH);//Set the ON LED
      toggle_led = false;
    }
    else{
     digitalWrite(BUSY_LED,LOW);//Set the ON LED
     toggle_led = true;  
    }
    if(completed_length_back >= 100){//End of backward movement      
        execute_flag_back=false;
        busy_flag = false;
	    digitalWrite(BUSY_LED,LOW);//Clear the BUSY LED
	    if(DEBUG) Serial.println("Ready to receive commands or start execution");
	    serial_listen=true;
	    Serial.println("ACK");
    }
}//End of backward execution loop

//-----Return to start of movement position for first init or home switch button
if(start_exec_pos_init){
  if(DEBUG) Serial.println("Starting return to init position");
    busy_flag = true;
    start_exec_pos_init=false;
    return_length=0;
}
while(start_exec_pos){
  //Set servo at 90
  Servo_mot.write(90);
  //Set motor speed to user set return speed
  Stepper_mot.setSpeed(home_return_speed);
  //Set motor movement X steps
  Stepper_mot.step(-motor_movement_res);
  //Add steps to counter
  return_length+=(motor_movement_res/(Stepper_steps/2))*100;
  //Toggle LED
  if(toggle_led){
    digitalWrite(BUSY_LED,HIGH);//Set the ON LED
    toggle_led = false;
  }
  else{
   digitalWrite(BUSY_LED,LOW);//Clear the BUSY LED
   toggle_led = true;  
  }
  //If all the necessary steps have been done, clear flags
  if(return_length>=100){
    start_exec_pos=false;
    busy_flag = false;
    digitalWrite(BUSY_LED,LOW);//Clear the BUSY LED
    if(DEBUG) Serial.println("Ready to receive commands or start execution");
    serial_listen=true;
    Serial.println("ACK");
  }
}
//-----Push Buttons Actions
if(digitalRead(START_SW) & ready_flag & !busy_flag){
  execute_flag=true; //Start execution if the system is ready and not busy
}
if(digitalRead(HOME_SW) & !busy_flag){
  home_flag=true;//Set flags necesarry for calculating home again
}
//if(digitalRead(HOME_SW) & !busy_flag){//Add a 5 degrees angle to the servo position
// servo_adjust_pos-=5;
// Servo_mot.write(servo_adjust_pos);
//}

if(digitalRead(STOP_SW)){
  while(1){
    delay(10);// In case the stop switch is pressed, stop everything forcing a reset
  }
}


}//--End of Main loop
