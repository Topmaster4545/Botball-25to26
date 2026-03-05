#include <kipr/wombat.h>

// DECLARATIONS

// MOTORS ---------------------------------------------------------

struct Motor 
{
    int port;
    int velocity; // Sync motor velocities by modifying instances below
};

void setMotorVelocity(struct Motor m, int modifier)
{
    mav(m.port, m.velocity * modifier);
}

struct Motor motorFrontLeft  = { 2, 1250 };
struct Motor motorFrontRight = { 3, 1250 };
struct Motor motorBackLeft   = { 1, 1250 };
struct Motor motorBackRight  = { 0, 1250 };

// SERVOS --------------------------------------------------------

int armPort = 0;
void setArmPosition(int pos) // 0/2047 = up/down
{
    set_servo_position(armPort,pos);
} 

// SENSORS --------------------------------------------------------

struct Sensor {
	int port;
    int threshold; //may be above or below.
};

int isSensorAboveThreshold(struct Sensor s) 
{
    return analog(s.port) > s.threshold;
}

struct Sensor lightSensor = { 0, 2800 };
struct Sensor groundSensorLeft = { 1, 2000 };
struct Sensor groundSensorRight = { 2, 2000 };
struct Sensor distanceSensor = { 3, 2250 };

// FUNCTIONS 1!!!!

// SENSOR CONDITIONS -----------------------------------------------

int untilSeesTapeLeft(void) // at least one
{
	return !isSensorAboveThreshold(groundSensorLeft);
}

int untilSeesTapeRight(void) // at least one
{
	return !isSensorAboveThreshold(groundSensorRight);
}

int untilBothSeesTape(void)
{
	return !isSensorAboveThreshold(groundSensorLeft) ||
           !isSensorAboveThreshold(groundSensorRight);
}

int untilSeesWall(void)
{
	return !isSensorAboveThreshold(distanceSensor);
}

typedef int (*conditionFunction)(void);

// MOVE ---------------------------------------------------------

// base move functions

void moveRobot(int dir) // -1,0,1 = back, stop, forward
{
	setMotorVelocity(motorFrontLeft, dir);
    setMotorVelocity(motorFrontRight, dir);
    setMotorVelocity(motorBackLeft, dir);
    setMotorVelocity(motorBackRight, dir);
}

void rotateRobot(int deg, int time) // -1, 1 (only supports left and right 90d currently)
{	
    setMotorVelocity(motorFrontLeft, deg);
    setMotorVelocity(motorFrontRight, -deg);
    setMotorVelocity(motorBackLeft, deg);
    setMotorVelocity(motorBackRight, -deg);
    msleep(time);
    moveRobot(0);
}

// higher move funcctionss

void rotateRobotColor(int deg, char sensor)
{
    
    if(sensor=='L')
    {
        
        while (!isSensorAboveThreshold(groundSensorLeft))
        {
            setMotorVelocity(motorFrontLeft, deg);
            setMotorVelocity(motorFrontRight, -deg);
            setMotorVelocity(motorBackLeft, deg);
            setMotorVelocity(motorBackRight, -deg);
        }   
    } else
    {
        while (!isSensorAboveThreshold(groundSensorRight))
        {
            setMotorVelocity(motorFrontLeft, deg);
            setMotorVelocity(motorFrontRight, -deg);
            setMotorVelocity(motorBackLeft, deg);
            setMotorVelocity(motorBackRight, -deg);
        }   
    }
}

void move(int dir, conditionFunction condition)
{
	while (condition()) {
    	moveRobot(1*dir);
    }
    moveRobot(0);
}

void moveTime(int dir, int time)
{
	moveRobot(dir);
    msleep(time);
    moveRobot(0);
}

void moveCorrected(int dir, conditionFunction condition)
{
	while (condition()) 
    {
    	moveRobot(1);
        if (isSensorAboveThreshold(groundSensorLeft))
        {
        	rotateRobot(-1, 10);
        }
        else if (isSensorAboveThreshold(groundSensorRight)) 
       	{
        	rotateRobot(1, 10);
        }
    }
    moveRobot(0);
}

void moveCorrectedTime(int dir, int time)
{
	for(int i=0;i<=time;i++)
    {
    	moveRobot(1);
        if (isSensorAboveThreshold(groundSensorLeft))
        {
        	rotateRobot(-1, 10);
        }
        else if (isSensorAboveThreshold(groundSensorRight)) 
       	{
        	rotateRobot(1, 10);
        }
        msleep(1); //Changing this value doesnt effect speed unless >1
        printf("%5d",i);
    }
    moveRobot(0);
}

// MAIN FUNCTIONS ----------------------------------

void run();

void waitUntilLight()
{
    while(!isSensorAboveThreshold(lightSensor))
    {

    }
}

int main()
{
    enable_servo(armPort);
    //test individual functions easily (make sure to put break)
    switch(1)
    {
        case 0: // Run this in the competition
            waitUntilLight();
            shut_down_in(120);
            run();
            break;
        case 1: // do NOT run this in the competition
            run();
            break;
        case 2:
            moveTime(1, 1000);
            break;
        case 3:
            setArmPosition(1);
            msleep(500);
            setArmPosition(0);
            break;
        case 4:
            move(1, untilSeesTapeLeft);
        default:
            break;
    }
    return 0;
}

void run()
{
    // NOTE: functions now take a condition parameter so 
    // instead of "moveUntilSeesTape(1);"
    // it is "move(1, untilSeesTape);"
    
    // skip 2 lines of tapeL
    
    //moveCorrectedSingleTime(1,'L',2000);
    setArmPosition(0);
    msleep(1000);
    setArmPosition(1000);
	move(1, untilSeesTapeLeft);
    moveTime(1,1500);
    move(1, untilSeesTapeLeft);
    moveTime(1,1750);
    msleep(1000);

    //rotate and move along the line
    rotateRobotColor(1,'R');
    moveTime(-1,500);
    setArmPosition(2024);
    moveCorrected(1, untilBothSeesTape);
    moveTime(1,2000);
    moveCorrected(1, untilBothSeesTape);

    // turn 180 and go back
    rotateRobot(1,2500);
    rotateRobotColor(1,'R');
   	moveCorrected(1, untilBothSeesTape);
    moveTime(1,1500);


    
    //Turns to starting area
    setArmPosition(1950);//DELETE THIS AFTER TESTING
    moveCorrected(1, untilSeesWall);
    rotateRobot(-1,800);
    setArmPosition(1900);
    move(1, untilSeesTapeLeft);
    rotateRobotColor(-1,'R');
    moveTime(1,1250);
    move(-1, untilSeesTapeRight);
    //moveTime(-1,250);

    //Further turning  to ram into side wall pvc
    rotateRobotColor(-1,'L');
    moveTime(1,2500);
    setArmPosition(0);
    move(-1,untilBothSeesTape);
    moveTime(1,1000);
    rotateRobotColor(1,'L');
    moveCorrectedTime(1,1000);
    
    //Even more turning to turn to corner and align itself with ramp
    rotateRobot(-1,2000);
    moveTime(1,750);
    setArmPosition(0);
    rotateRobot(-1,750);
    rotateRobot(1,750);
    setArmPosition(0);
    moveTime(2250);
    rotateRobot(-1,2000);
    moveTime(-1,1000);
    rotateRobot(-1,250);
    setArmPosition(2024);
    msleep(750);
    rotateRobot(1,325);
    
    //Moves until it hits the tape in upperstart box
    move(1,untilBothSeesTape);
    moveTime(1,1500);
    moveCorrectedTime(1,1100);
    setArmPosition(2045);
    moveTime(1,4500);
    moveCorrected(1,untilBothSeesTape);
    setArmPosition(0);
    moveTime(1,4000);
    rotateRobot(-1,2400);
    moveTime(1,750);
    
    
    /*
    pivotRobot(-1,1500);
    move(1, untilSeesTapeLeft);
    rotateRobotColor(-1,'L');
    setArmPosition(1250);
    moveTime(1,1000);
    move(1, untilSeesTapeLeft);*/
    
    
    
    //turn and go to ramp*/
}

// DEPRECIATED 
/*
void moveUntilSeesWall(int val){
    while(!isSensorAboveThreshold(distanceSensor))
    {
        moveRobot(1);
    }
    moveRobot(0);
}

void moveUntilSeesTape(int dir) {
    while (!isSensorAboveThreshold(groundSensorLeft))
    {
    	moveRobot(1);
    }
    moveRobot(0);
}

void moveUntilSeesBothTape(int dir) {
 	while (!isSensorAboveThreshold(groundSensorLeft) &&
           !isSensorAboveThreshold(groundSensorRight))
    {
    	moveRobot(1);
    }
    moveRobot(0);
}

void moveWithCorrection(int dir)
{
    while (!isSensorAboveThreshold(groundSensorLeft) ||
           !isSensorAboveThreshold(groundSensorRight)) 
    {
        
    	moveRobot(1);
        if (isSensorAboveThreshold(groundSensorLeft)) {
        	rotateRobot(-1, 200);
        }
        else if (isSensorAboveThreshold(groundSensorRight)) {
        	rotateRobot(1, 200);
        }
    }
    moveRobot(0);
}

void moveWithCorrectionD(int dir)
{
    while (!isSensorAboveThreshold(distanceSensor)) 
    {
    	moveRobot(1);
        if (isSensorAboveThreshold(groundSensorLeft)) {
        	rotateRobot(-1, 200);
        }
        else if (isSensorAboveThreshold(groundSensorRight)) {
        	rotateRobot(1, 200);
        }
    }
    moveRobot(0);
}
*/
/*
void pivotRobot(int deg, int time)
{
    if(deg==1)
    {
        setMotorVelocity(motorFrontLeft, deg);
    	setMotorVelocity(motorBackLeft, deg);
    } else
    {
        setMotorVelocity(motorFrontRight, deg);
    	setMotorVelocity(motorBackRight, deg);
    }
    msleep(time);
    setMotorVelocity(motorFrontLeft, 0);
    setMotorVelocity(motorFrontRight, 0);
    setMotorVelocity(motorBackLeft, 0);
    setMotorVelocity(motorBackRight, 0);
}
*/
/*

void moveCorrectedSingleTime(int dir, char sensor, int time)
{
    for(int i=0;i<=time;i++)
    {
    	moveRobot(1);
        if(sensor=='L'&&!isSensorAboveThreshold(groundSensorLeft))
        {
        	rotateRobot(-1, 10);
            rotateRobot(1,150);
        } else if(sensor=='R'&&!isSensorAboveThreshold(groundSensorRight))
        {
            rotateRobot(1,10);
        }
        msleep(1); //Changing this value doesnt effect speed unless >1
        printf("%5d",i);
    }
    moveRobot(0);
}
*/