#ifndef SERVO_H
#define SERVO_H

#define LASER      22 /**< wiringPi pin connected to LED. */
#define ELEVATION  23 /**< wiringPi pin connected to top servo. */
#define AZIMUTH    26 /**< wiringPi pin connected to bottom servo. */
#define BASE_CLK   19200000 /**< Raspberry Pi base clock frequency (Hz). */
#define PWM_FREQ   50  /**< Desired PWM frequency (Hz). */
#define PWMRNG     2000 /**< Range of PWM period. */
#define AZ_MAX     90  /**<Maximum azimuthal angle (degrees). Used in limiting range of motion to prevent damage.*/
#define AZ_MIN    -90  /**<Minimum azimuthal angle (degrees). Used in limiting range of motion to prevent damage.*/
#define EL_MAX     70 /**<Maximum elevation angle (degrees). Used in limiting range of motion to prevent damage.*/
#define EL_MIN    -35 /**<Minimum elevation angle (degrees). Used in limiting range of motion to prevent damage.*/
#define DELAY      250  /**<Servo delay (milliseconds). Used in preventing jitter.*/


//Function signatures
/**
   * Sets up PWM control for the servo motors.
   */
void pwmSetup();

/**
   * Turns on the front LED.
   */
void laserOn();

/**
   * Turns off the front LED.
   */
void laserOff();

/**
   * Stops movement of both servos.
   */
void stopMotors();

/**
   * Brings both servos to their 0 position, i.e. facing straight ahead.
   * @param az_curr current position of azimuthal motor (degrees) 
   * @param el_curr current position of elevation motor (degrees)
   */
void zeroMotors(int *az_curr, int *el_curr);

/**
   * Turns a given motor by the specified angle in degrees. Prevents movements of the servos at their angular limits so as to avoid damage
   * @param angle The angle by which the motor should turn. 
   * @param motor An integer that distiguishes between the azimuthal or elevation servo.
   * @param curr The current angular position of the servo
   * @see AZIMUTH
   * @see ELEVATION
   */
void turnMotorBy(int angle, int motor, int *curr); // 



void zeroAzimuth(int *curr);
void zeroElevation(int *curr);
int  getPwmClk(int pwmRange);
int  getPwmValue(int angle);
void turnMotorTo(int angle, int motor);
void stopMotor(int motor);

#endif

