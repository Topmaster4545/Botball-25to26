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

struct Servo 
{
	int port;
};

void setServoPosition(struct Servo s, int pos) // 0/2047 = up/down
{
    set_servo_position(s.port, pos);
} 

struct Servo servoArm = { 0 };

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

int seesTapeLeft(void) // at least one
{
	return isSensorAboveThreshold(groundSensorLeft);
}

int seesTapeRight(void) // at least one
{
	return isSensorAboveThreshold(groundSensorRight);
}

int bothSeesTape(void)
{
	return isSensorAboveThreshold(groundSensorLeft) &&
           isSensorAboveThreshold(groundSensorRight);
}

int seesWall(void)
{
	return isSensorAboveThreshold(distanceSensor);
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

void rotateRobot(int deg)
{   
    setMotorVelocity(motorFrontLeft, deg);
    setMotorVelocity(motorFrontRight, -deg);
    setMotorVelocity(motorBackLeft, deg);
    setMotorVelocity(motorBackRight, -deg);
}

// higher move funcctionss

// basic move

void moveTime(int dir, int time)
{
	moveRobot(dir);
    msleep(time);
    moveRobot(0);
}

void moveUntil(int dir, conditionFunction condition)
{
	while (!condition()) // means repeat until condition is true
    {
    	moveRobot(dir);
    }
    moveRobot(0);
}

// rotate

void rotateTime(int deg, int time) // -1, 1 (only supports left and right 90d currently)
{	
    rotateRobot(deg);
    msleep(time);
    rotateRobot(0);
}

void rotateUntil(int deg, conditionFunction condition)
{
    while (!condition())
    {
        rotateRobot(deg);
    }
    rotateRobot(0);
}

// corrected

void moveCorrected(int dir)
{
    moveRobot(1);
    if (seesTapeLeft())
    {
        rotateTime(-1, 10);
    }
    else if (seesTapeRight()) 
    {
        rotateTime(1, 10);
    }
}

void moveCorrectedTime(int dir, int time)
{
	for (int i = 0; i <= time; i++)
    {
    	moveCorrected(dir);
        msleep(1); //Changing this value doesnt effect speed unless >1
        printf("%5d",i);
    }
    moveRobot(0);
}

void moveCorrectedUntil(int dir, conditionFunction condition)
{
	while (!condition())
    {
    	moveCorrected(dir);
    }
    moveRobot(0);
}

// SERVO

void setArmPosition(int pos)
{
	setServoPosition(servoArm, pos);
}

// MAIN FUNCTIONS ----------------------------------

void run();

void waitUntilLight()
{
    while(!isSensorAboveThreshold(lightSensor)) {}
}

int main()
{
    enable_servo(servoArm.port);
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
			moveUntil(1, seesWall);
            break;
        case 5:
            while(1==1)
            {
                moveCorrected(1);
            }
        default:
            break;
    }
    return 0;
}

void run()
{    
    // skip 2 lines of tapeL
    
    setArmPosition(0);
    msleep(1000);
    setArmPosition(1000);
	moveUntil(1, seesTapeLeft);
    moveTime(1,1000);
    moveUntil(1, seesTapeLeft);
    moveTime(1,1750);
    msleep(1000);

    //rotate and move along the line
    rotateUntil(1, seesTapeRight);
    moveTime(-1,500);
    rotateTime(1, 100);
    setArmPosition(2024);
    msleep(100);
    moveCorrectedUntil(1, bothSeesTape);
    moveTime(1,2000);
    moveCorrectedUntil(1, bothSeesTape);

    // turn 180 and go back
    rotateTime(1,2500);
    rotateUntil(1, seesTapeRight);
    setArmPosition(1950);
   	moveCorrectedUntil(1, bothSeesTape);
    moveTime(1,1500);
    
    //Turns to starting area
    setArmPosition(1950);
    moveCorrectedUntil(1, seesWall);
    rotateTime(-1,725);
    setArmPosition(2000);
    moveUntil(1, seesTapeLeft);
    rotateUntil(-1, seesTapeRight);
    moveTime(1,1250);
    moveUntil(-1, seesTapeRight);

    //Further turning  to ram into side wall pvc
    rotateUntil(-1, seesTapeLeft);
    setArmPosition(1900);
    moveTime(1,2500);
    rotateTime(-1,500);
    moveUntil(-1, bothSeesTape);
    setArmPosition(0);
    moveTime(1,1000);
    rotateUntil(1, seesTapeLeft);
    moveCorrectedTime(1,1000);
    
    //Even more turning to turn to corner and align itself with ramp
    rotateTime(-1,2000);
    moveTime(1,3500);
    rotateTime(-1,2000);
    setArmPosition(866);
    moveTime(-1,1000);
    moveTime(1,2000);
    rotateTime(-1,250);
    moveTime(1,500);
    
    rotateTime(1,250);
    moveTime(-1,3000);
    moveTime(1,1250);
    rotateTime(1,375);
    setArmPosition(2047);
    msleep(750);
    rotateTime(-1,275);
    
    //Moves until it hits the tape in upperstart box
    moveUntil(1, bothSeesTape);
    moveTime(1,750);
    moveCorrectedTime(1,1100);
    setArmPosition(2045);
    moveTime(1,2000);
    moveTime(-1,500);
    moveTime(1,750);
    moveCorrectedUntil(1, bothSeesTape);
    setArmPosition(0);
    moveTime(1,2250);
    moveTime(-1,1000);
    rotateTime(-1,750);
    moveTime(1,2750);
    rotateTime(-1,1250);
    //setArmPosition(715);
    
    
    /*
    pivotRobot(-1,1500);
    move(1, untilSeesTapeLeft);
    rotateRobotColor(-1,'L');
    setArmPosition(1250);
    moveTime(1,1000);
    move(1, untilSeesTapeLeft);*/
    
    
    
    //turn and go to ramp*/
}