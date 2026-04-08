#include <kipr/wombat.h>

// DECLARATIONS
// SENSORS
int tapeThreshold = 3650;
int lightThreshold = 200;

// CLAW MOVER
int clawMoverLeftUpVal = 1047;
int clawMoverRightUpVal = 1000;
int clawMoverLeftDownVal = 1947;
int clawMoverRightDownVal = 100;
int clawMoverLeftHalfVal = 1847; //
int clawMoverRightHalfVal = 200; //
int clawOpenVal = 1300;
int clawHalfVal = 1000; //
int clawCloseVal = 200;

// PLOW
int plowUpVal = 1082;
int plowDownVal = 0;

// WHEELS
int topLeftWheelVal = 525;
int topRightWheelVal = 500;
int bottomLeftWheelVal = 500;
int bottomRightWheelVal = 525;

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

struct Motor motorFrontLeft  = { 2, 1000 };
struct Motor motorFrontRight = { 0, 1000 };
struct Motor motorBackLeft   = { 3, 1000 };
struct Motor motorBackRight  = { 1, 1000 };

// SERVOS --------------------------------------------------------

int clawMoverLeft = 2;
int clawMoverRight = 0;
void setClawMoverPosition(int posLeft, int posRight)
{
    set_servo_position(clawMoverLeft,posLeft);
    set_servo_position(clawMoverRight,posRight);
} 

int plow = 3;
void setPlowPosition(int pos)
{
    set_servo_position(plow, pos);
}

int claw = 1;
void setClawPosition(int pos)
{
    set_servo_position(claw, pos);
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

struct Sensor lightSensor = { 0, 200 };
struct Sensor groundSensorLeft = { 1, 3650 };
struct Sensor groundSensorRight = { 2, 3650 };

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

typedef int (*conditionFunction)(void);

// MOVE ---------------------------------------------------------

// base move functions

void moveRobotVert(int dir) // -1,0,1 = back, stop, forward
{
    setMotorVelocity(motorFrontLeft, dir);
    setMotorVelocity(motorFrontRight, dir);
    setMotorVelocity(motorBackLeft, dir);
    setMotorVelocity(motorBackRight, dir);
}

void moveRobotHoriz(int dir) // -1,0,1 = left, stop, right
{
    setMotorVelocity(motorFrontLeft, -dir);
    setMotorVelocity(motorFrontRight, dir);
    setMotorVelocity(motorBackLeft, dir);
    setMotorVelocity(motorBackRight, -dir);
}

void rotateRobot(int deg, int time) // -1, 1 (only supports left and right 90d currently)
{	
    setMotorVelocity(motorFrontLeft, deg);
    setMotorVelocity(motorFrontRight, -deg);
    setMotorVelocity(motorBackLeft, deg);
    setMotorVelocity(motorBackRight, -deg);
    msleep(time);
    setMotorVelocity(motorFrontLeft, 0);
    setMotorVelocity(motorFrontRight, 0);
    setMotorVelocity(motorBackLeft, 0);
    setMotorVelocity(motorBackRight, 0);
}

void pivotRobot(int deg, int time)
{
    if(deg==1)
    {
        setMotorVelocity(motorFrontLeft, deg);
        setMotorVelocity(motorBackLeft, deg);
    } else
    {
        setMotorVelocity(motorFrontRight, -deg);
        setMotorVelocity(motorBackRight, -deg);
    }
    msleep(time);
    setMotorVelocity(motorFrontLeft, 0);
    setMotorVelocity(motorFrontRight, 0);
    setMotorVelocity(motorBackLeft, 0);
    setMotorVelocity(motorBackRight, 0);
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

void moveVert(int dir, conditionFunction condition)
{
    while (condition()) {
        moveRobotVert(1*dir);
    }
    moveRobotVert(0);
}

void moveHoriz(int dir, conditionFunction condition)
{
    while (condition()) {
        moveRobotHoriz(1*-dir);
    }
    moveRobotHoriz(0);
}

void moveTimeVert(int dir, int time)
{
    moveRobotVert(dir);
    msleep(time);
    moveRobotVert(0);
}

void moveTimeHoriz(int dir, int time)
{
    moveRobotHoriz(-dir);
    msleep(time);
    moveRobotHoriz(0);
}

void moveCorrected(int dir, conditionFunction condition)
{
    while (condition()) 
    {
        moveRobotVert(1);
        if (isSensorAboveThreshold(groundSensorLeft) && condition())
        {
            rotateRobot(-1, 100);
        }

        else if (isSensorAboveThreshold(groundSensorRight) && condition()) 
        {
            rotateRobot(1, 100);
        }
    }
    moveRobotVert(0);
}

void moveCorrectedTime(int dir, int time)
{
    for(int i=0;i<=time;i++)
    {
        moveRobotVert(1);
        if (isSensorAboveThreshold(groundSensorLeft))
        {
            rotateRobot(-1, 100);
        }
        else if (isSensorAboveThreshold(groundSensorRight)) 
        {
            rotateRobot(1, 100);
        }
        msleep(1); //Changing this value doesnt effect speed unless >1
        printf("%5d",i);
    }
    moveRobotVert(0);
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
    enable_servos();
    //test individual functions easily (make sure to put break)
    switch(6)
    {
        case 0: // Run this in the competition
            waitUntilLight();
            shut_down_in(120);
            run();
            break;
        case 1: // do NOT run this in the competition
            enable_servos();
            setPlowPosition(plowDownVal);
            setClawPosition(clawOpenVal);
            msleep(1000);
            setClawPosition(clawCloseVal);
            msleep(500);
            setClawMoverPosition(clawMoverLeftUpVal, clawMoverRightUpVal);
            msleep(1000);
            setClawMoverPosition(clawMoverLeftDownVal, clawMoverRightDownVal);
            msleep(100);
            setPlowPosition(plowUpVal);
            msleep(1000);
            run();
            break;
        case 2: // pink tape start
            setPlowPosition(plowDownVal);
            msleep(100);
            setClawPosition(clawCloseVal);
            msleep(100);
            setClawMoverPosition(clawMoverLeftUpVal, clawMoverRightUpVal);
            run();
            break;
        case 3: // claw positioning tests
            setClawMoverPosition(clawMoverLeftUpVal, clawMoverRightUpVal);
            msleep(500);
            setClawMoverPosition(clawMoverLeftDownVal, clawMoverRightDownVal);
            msleep(500);
            setClawMoverPosition(1247, 800); // values to latch onto the left door
            msleep(500);
            setClawMoverPosition(1147, 900); // values to latch onto the right door
            msleep(500);

            setClawPosition(clawHalfVal);
            msleep(500);
            setClawPosition(clawOpenVal);
            msleep(500);
            setClawPosition(clawCloseVal);
            break;
        case 4:
            moveHoriz(1, untilSeesTapeLeft);
        case 5: // resets claw positions
            setClawMoverPosition(clawMoverLeftHalfVal, clawMoverRightHalfVal);
            msleep(500);
            setClawPosition(clawCloseVal);
            msleep(500);
            setPlowPosition(plowDownVal);
        default:
            break;
        case 6:
            run();
            break;
    }
    return 0;
}

void run()
{

    // NOTE: functions now take a condition parameter so 
    // instead of "moveUntilSeesTape(1);"
    // it is "move(1, untilSeesTape);"

  /*  
    // Bring plow down and drive past both tapes, grabbing four poms before bringing the claw up and preparing for a turn
    setPlowPosition(plowDownVal);
    msleep(500);
    moveVert(1, untilBothSeesTape);
    moveTimeVert(1, 1000);
    moveVert(1, untilBothSeesTape);
    setClawMoverPosition(clawMoverLeftUpVal, clawMoverRightUpVal);
    moveTimeVert(1, 450);


    // Turn and gather poms up to the first traffic cone, aligning to prepare to open doors
    rotateRobot(1, 1750);
    moveTimeVert(-1, 500);
    moveCorrected(1, untilBothSeesTape);
    moveTimeVert(1, 2000);
    moveTimeVert(-1, 2000);
    rotateRobot(-1, 175);
    moveTimeHoriz(1, 2500);
    moveTimeHoriz(-1, 2250);
    moveTimeHoriz(1, 500);

    // Open left door
    moveTimeHoriz(1, 500); // go back so bringing claw down does not hit the doors yet
    msleep(100);
    setClawMoverPosition(1797, 250); // values to latch onto the left door
    setClawPosition(clawHalfVal); // open claw so it can hook onto the door
    moveTimeHoriz(-1, 500); // move towards door
    msleep(100);
    moveTimeHoriz(-1, 700); // move towards door
    moveTimeVert(-1, 2350); // we are latched on, just move left to move the door left
    moveTimeVert(1, 800); // unhook from the door 

    // Open right door
    moveTimeVert(1, 500); // unhook from the left door
    moveTimeHoriz(1, 800); // move away from the doors
    setClawPosition(clawHalfVal); // values to latch onto the right door
    moveTimeVert(1, 1500); // move towards the middle (ish)
    moveTimeHoriz(-1, 500); // come closer to doors
    moveTimeVert(1, 700);
    moveTimeHoriz(-1, 700); // hook onto door
    moveTimeVert(1, 2075); // move door right

    // Prepare to grab Botguy
    moveTimeVert(-1, 400); // unhook from right door
    moveTimeHoriz(1, 700); // move back from right door so we do not get caught on it
    setClawMoverPosition(clawMoverLeftDownVal, clawMoverRightDownVal);
    moveVert(-1, untilBothSeesTape); // go back to middle
    moveTimeVert(1, 650); // position back to middle
    setClawPosition(clawCloseVal); // close claw to enter the opening
    setClawMoverPosition(clawMoverLeftDownVal, clawMoverRightDownVal);
    msleep(100);
    moveTimeHoriz(-1, 300); // get into the opening
    msleep(200);
    setClawPosition(clawHalfVal); // open claw halfway
    msleep(200);
    moveTimeHoriz(-1, 100); // get into botguy range
    setClawPosition(clawOpenVal); // open claw
    moveTimeHoriz(-1, 200); // get into botguy range

    // Grab Botguy
    moveTimeHoriz(-1, 650); // fully enter botguy range
    msleep(500);
    setClawPosition(clawCloseVal); // grab botguy

    // Get out of jail and go to second cone
    msleep(500);
    setClawMoverPosition(1697, 350);
    msleep(500);
    moveHoriz(1, untilSeesTapeRight);
    moveTimeHoriz(1, 700);
    setClawMoverPosition(clawMoverLeftUpVal, clawMoverRightUpVal);
    moveCorrected(1, untilBothSeesTape);
    rotateRobot(-1, 175);

    // Turn around and get the items into the lower starting box
    rotateRobot(1, 3500);
    moveCorrected(1, untilBothSeesTape); */
    moveTimeVert(1, 500);
    moveCorrectedTime(1, 750);
    moveTimeVert(1, 2600); // PINK TAPE STARTS HERE, position for a turn
    pivotRobot(-1, 3750); // pivot 90 degrees
    moveTimeVert(1, 2000); // plow items just before the ramp

    // Pivot to get into the corner after pushing cubes away
    setPlowPosition(2047); // bring plow fully up
    moveTimeVert(-1, 1700); // back up
    moveTimeHoriz(1, 1250); // go right to move cubes
    setPlowPosition(plowDownVal);
    moveTimeVert(1, 1700); // go towards cubes
    moveTimeHoriz(-1, 1100); // move cubes left
    setPlowPosition(2047); // bring plow fully up
    moveTimeVert(-1, 1700); // back up
    moveTimeHoriz(1, 4250); // align with wall left of lower starting box
    moveTimeHoriz(-1, 750); // move a little off the wall
    rotateRobot(-1, 1750); // rotate 90 degrees
    moveTimeVert(-1, 1200); // align with wall behind robot
    moveTimeHoriz(1, 4750); // move right to get into position to plow

    // Get plow down
    moveTimeVert(-1, 500); // align with wall behind robot
    moveTimeVert(1, 2500); // drive into cubes
    moveTimeVert(-1, 2500); // go back
    moveTimeHoriz(1, 400); // get into corner
    moveTimeVert(-1, 400); // get into corner
    moveTimeHoriz(-1, 400); // go left a little to execute the turn
    pivotRobot(1, 800); // turn right
    setPlowPosition(plowDownVal); // drop the plow on the pvc
    moveTimeVert(1, 200); // return back to the corner
    moveTimeHoriz(1, 200); // return back to the corner
    moveTimeVert(1, 200); // return back to the corner
    moveTimeHoriz(1, 200); // return back to the corner
    pivotRobot(-1, 400); // turn left to bring the plow down onto the playing field
    msleep(200); 
    setPlowPosition(100); // bring plow up a little before getting onto the ramp so it does not get caught on anything
   
    // Go up the ramp
    moveVert(1, untilBothSeesTape); // go to the start of the ramp
    setPlowPosition(plowDownVal); // plow goes down
    moveTimeVert(1, 2000); // get off the tape
    moveCorrectedTime(1, 6000); // go up the ramp
    moveTimeVert(1, 2000); // get past top
    moveCorrected(1, untilBothSeesTape); // go to the start of the upper starting box
    
}