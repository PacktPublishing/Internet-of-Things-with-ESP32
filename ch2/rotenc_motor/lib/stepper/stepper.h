
#ifndef stepper_h_
#define stepper_h_

void stepper_init(int number_of_steps, int motor_pin_1, int motor_pin_2,
                  int motor_pin_3, int motor_pin_4);

void stepper_setSpeed(long revolution_per_min);

void stepper_step(int steps_to_move);

int stepper_getPos(void);


#endif
