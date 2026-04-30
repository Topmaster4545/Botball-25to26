#include <kipr/wombat.h>

// DECLARATIONS
int colorList[8]={1,0,1,0,1,0,1,0};

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

struct Motor motorFrontLeft  = { 1, 1250 };  //1150
struct Motor motorFrontRight = { 0, 1400 };  //1400
struct Motor motorRevolver   = { 1, -1250 };

// SERVOS --------------------------------------------------------

struct Servo 
{
	int port;
};

void setServoPosition(struct Servo s, int pos) // 0/2047 = up/down   700 for door
{
    set_servo_position(s.port, pos);
} 

struct Servo servoArmRight = { 0 };
struct Servo servoArmLeft = { 1 };
struct Servo door = { 2 };
struct Servo micro = {3};

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
struct Sensor groundSensorLeft = { 2, 3300 };
struct Sensor groundSensorRight = { 1, 3300 };
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
void moveTime(int dir, int time);
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

//DRUMS
void drumCollectionSequence(){
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
        setServoPosition(door,500);
        mrp(2,75,225);
        msleep(2000);
        for(int i=0;i<8;i++)
        {
            printf("%d,",colorList[i]);
        }
        printf("\n");
        setServoPosition(door,2047);
        //msleep(5000);
        newImage();
        
    }
    printf("ended\n");
}
void shake(){
	int orig = gmpc(2);
    mav(2,-355);
    msleep(340);
    
   	mav(2,0);
    for(int i=1400;i>=1200;i-=50)
    {
        mav(2,0);
        mav(2,-i);
        msleep(200);
        mav(2,0);
        msleep(500);
        mav(2,i);
        msleep(200);
        mav(2,0);
        
    }
    
    mav(2,-275);
    msleep(500);
    mav(2, 200);
    msleep(1000);
    mav(2,-275);
    msleep(200);
    mtp(2,200,orig);
    msleep(2000);
    
}
void dropoffSequence()
{
	int firstColor = colorList[5];
    shake();
    colorList[5]=-1;
    int pCount = 0;
    int temp;
    while(pCount<3)
    {
        while(colorList[5]!=firstColor)
        {
            //cw turn
            mav(2,450);
            msleep(600);
            mav(2,0);
            msleep(300);
            temp = colorList[0];
            for(int j=0; j<=6; j++)
            {
                colorList[j]=colorList[j+1];
            }
            colorList[7] = temp;
        }
        shake();
        msleep(400);
        pCount++;
        printf("pcountinc");
        colorList[5]=-1;
        for(int i=0;i<8;i++)
        {
            printf("%d,",colorList[i]);
        }
    }
    if(firstColor==0)
    {
        while(colorList[5]!=1)
        {
            //cw turn
            mav(2,450);
            msleep(600);
            mav(2,0);
            msleep(300);
            temp = colorList[0];
            for(int j=0; j<=6; j++)
            {
                colorList[j]=colorList[j+1];
            }
            colorList[7] = temp;
        } 
    }else
    {
        while(colorList[5]!=0)
        {
            //cw turn
            mav(2,450);
            msleep(600);
            mav(2,0);
            msleep(300);
            temp = colorList[0];
            for(int j=0; j<=6; j++)
            {
                colorList[j]=colorList[j+1];
            }
            colorList[7] = temp;
        }
    }
    printf("\nDROP OFF SEQUENCE DONE");
}



int main()
{
    enable_servos();
    setServoPosition(door,400);
    setServoPosition(micro,4000);
    /*
    mtp(2,100,0);
    setArmPositionInc(1000);*/
    //msleep(5000); ///DELETE OR LESSEN TIME BEFORE COMP
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
			dropoffSequence();
            break;
            
        case 3:
            shake();
          	break;
        case 4:
            moveCorrectedUntil(1,bothSeesTape);
            moveTime(1,1000);
            moveCorrectedUntil(1,bothSeesTape);
            moveCorrectedTime(1,1500);
            break;
        default:
            break;
    }
    return 0;
}

void run()
{   
    //Moves out from starting box to the drums
    camera_open();
    printf("starting\n");
    double startTime=seconds();
    setArmPositionInc(950);
    msleep(1200);
    moveUntil(1, bothSeesTape);
    moveTime(1,1000);
    moveUntil(1, bothSeesTape);
    moveTime(1,2000);
    setArmPositionInc(2047);
    moveTime(-1,250);
    rotateTime(-1,250);
    printf("before delay with %f\n",seconds()-startTime);
    
    
    //Collects Drums
    //drumCollectionSequence();
    setArmPositionInc(1000);
    
     //Work in prog, to move from tape, back to first pole
    
    moveUntil(-1,bothSeesTape);
    msleep(300);
    
    rotateTime(-1,2250);
    setServoPosition(micro,1400);
    msleep(200);
    moveTime(1,5750);
    rotateTime(-1,150);
    
   	dropoffSequence();
    
    moveTime(-1,2000);
    moveUntil(-1,seesTapeRight);
    moveTime(-1,1500);
    moveUntil(-1,seesTapeRight);
    moveTime(-1,1500);
    msleep(300);
    rotateTime(-1,1000);
    rotateUntil(-1,seesTapeRight);
    moveCorrectedUntil(1,bothSeesTape);
    moveTime(1,1000);
    moveCorrectedUntil(1,bothSeesTape);
    moveTime(1,1000);
    moveCorrectedTime(1,1000);
    rotateTime(1,150);
    
    dropoffSequence();
   
    /*moveTime(-1,4000);
    msleep(400);
    mav(0,-1830);
    mav(1,-1000);
    msleep(1500);
    moveRobot(0);
    rotateTime(-1,5000);
    moveRobot(0);*/
    
  
    /*
    //ORIGINAL SEQUENCE AFTER BACKING UP FROM DISPENSER
    moveTime(-1,1000);
    rotateTime(1,3350);
    moveTime(1,7000);
    dropoffSequence();
    moveUntil(-1,bothSeesTape);
    moveTime(-1,500);
    moveUntil(-1,bothSeesTape);
    rotateUntil(-1,seesTapeLeft);
    rotateTime(-1,450);
    moveCorrectedUntil(1,bothSeesTape);
    moveTime(1,600);
    moveCorrectedUntil(1,bothSeesTape);
    moveTime(1,500);
    moveCorrectedTime(1,800);
    shake();
    */
    
    
}