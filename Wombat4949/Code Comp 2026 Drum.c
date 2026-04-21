#include <kipr/wombat.h>

// DECLARATIONS
int colorList[8];

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

struct Motor motorFrontLeft  = { 1, 1000 };
struct Motor motorFrontRight = { 0, 1300 };
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

struct Servo servoArmRight = { 0 };
struct Servo servoArmLeft = { 1 };
struct Servo door = { 2 };

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
struct Sensor groundSensorLeft = { 2, 2500 };
struct Sensor groundSensorRight = { 1, 2500 };
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

void setDoorPosition(int pos)
{
	setServoPosition(door, pos);
}

void setArmPositionInc(int pos)
{
    int currentPos=get_servo_position(0);
    if(pos>currentPos)
    {
        for (int i = currentPos; i <= pos; i++)
        {
            set_servo_position(servoArmRight.port,i);
            set_servo_position(servoArmLeft.port,2047-i);
            msleep(1);
        }
    } else
    {
        for (int i = currentPos; i >= pos; i--)
        {
            set_servo_position(servoArmRight.port,i);
            set_servo_position(servoArmLeft.port,2047-i);
            msleep(1);
        }
    }   
}
// Camera -----------------
void newImage()
{
	camera_update();
    camera_update();
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
    enable_servos();
    /*mtp(2,100,0);
    setArmPositionInc(50);
    setDoorPosition(1525);
    msleep(5000);*/ //DELETE OR LESSEN TIME BEFORE COMP
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
        default:
            break;
    }
    return 0;
}

void run()
{   
    /*moveUntil(1,seesWallStart);
    rotateTime(-1,1175);
	moveUntil(1, bothSeesTape);
    moveUntil(1, bothSeesTape);
    moveUntil(1, seesWallClose);
    setArmPositionInc(2047);*/
    
    //Moves out from starting box to the drums
    camera_open();
    printf("starting\n");
    /*double startTime=seconds();
    setArmPositionInc(950);
    moveUntil(1, bothSeesTape);
    moveTime(1,2750);
    setArmPositionInc(2047);
    moveTime(-1,250);
    rotateTime(-1,250);
    printf("before delay with %f\n",seconds()-startTime);*/
    
    //Collects Drums

    camera_update();
    newImage();
    for(int i=1; i<=8; i++)
    {
        printf("Looking for image ++ ");
        while(get_object_count(0)<1&&get_object_count(1)<1){
            newImage();
        }
        if(get_object_count(0)>0){
        	colorList[i-1]=0;
            printf("Channel 0 ");
        } else if(get_object_count(1)>0)
        {
        	colorList[i-1]=1;
            printf("Channel 1 ");
        }
        printf("pipe detected! ");
        msleep(600);
        mrp(2,75,225);
        msleep(1000);
        for(int i=0;i<8;i++)
        {
            printf("%d,",colorList[i]);
        }
        printf("\n");
        //msleep(5000);
        newImage();
        
    }
    printf("ended\n");

}