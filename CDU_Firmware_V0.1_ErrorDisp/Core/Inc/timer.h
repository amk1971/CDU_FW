/*
 * timer.h
 *
 *  Created on: Oct 18, 2024
 *      Author: Hamza Javed
 */

#ifndef INC_TIMER_H_
#define INC_TIMER_H_

extern void start_timer(void);
extern void stop_timer(void);
extern void time_up_notify_tft(void);

extern void cursor_notify_tft(void);
extern void stop_timer_cursor(void);
extern void start_timer_cursor(void);

#endif /* INC_TIMER_H_ */
