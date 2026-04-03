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

struct Motor motorFrontLeft  = { 1, 800 };
struct Motor motorFrontRight = { 0, 1000 };
struct Motor motorRevolver   = { 1, -1250 };

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

struct Sensor lightSensor = { 3, 2800 };
struct Sensor groundSensorLeft = { 2, 3900 };
struct Sensor groundSensorRight = { 1, 3900 };
struct Sensor distanceSensorStart = { 0, 1250 };
struct Sensor distanceSensorClose = { 0, 2750 };

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

int seesWallStart(void)
{
	return isSensorAboveThreshold(distanceSensorStart);
}

int seesWallClose(void)
{
	return isSensorAboveThreshold(distanceSensorClose);
}

typedef int (*conditionFunction)(void);

// MOVE ---------------------------------------------------------

// base move functions

void moveRobot(int dir) // -1,0,1 = back, stop, forward
{
	setMotorVelocity(motorFrontLeft, dir);
    setMotorVelocity(motorFrontRight, dir);
}

void rotateRobot(int deg)
{   
    setMotorVelocity(motorFrontLeft, deg);
    setMotorVelocity(motorFrontRight, -deg);
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

void setArmPositionInc(int pos)
{
    int currentPos=get_servo_position(servoArm.port);
    if(pos>currentPos)
    {
        for (int i = currentPos; i <= pos; i++)
        {
            setArmPosition(i);
            msleep(1);
        }
    } else
    {
        for (int i = currentPos; i >= pos; i--)
        {
            setArmPosition(i);
            msleep(1);
        }
    }
    
}

// MAIN FUNCTIONS ----------------------------------

void run();

void waitUntilLight()
{
    while(!isSensorAboveThreshold(lightSensor)) {}
}

// Debug

int debug = 0;

void incrementDebug()
{
	printf("debug increment at %d\n", debug);
    debug++;
}

int main()
{
    enable_servo(servoArm.port);
    setArmPositionInc(0);
    msleep(1000);
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
    /*moveUntil(1,seesWallStart);
    rotateTime(-1,1175);*/
	moveUntil(1, bothSeesTape);
    moveUntil(1, bothSeesTape);
    moveUntil(1, seesWallClose);
    setArmPositionInc(2047);
}