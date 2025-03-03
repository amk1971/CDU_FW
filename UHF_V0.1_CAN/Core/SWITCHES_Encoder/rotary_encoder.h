#ifndef D__JIM_CDU_HF_RCU_RCU_WORKSPACE_RCU_FIRMWARE_V0_0_CORE_INC_ROTARY_ENCODER_H_
#define D__JIM_CDU_HF_RCU_RCU_WORKSPACE_RCU_FIRMWARE_V0_0_CORE_INC_ROTARY_ENCODER_H_

void read_encoder_standby_mhz();
void read_encoder_standby_khz();
void read_encoder_channel_mhz(void );
void read_encoder_channel_khz(void );
void encoder_change_channel(void );
void read_encoder_volume();
void scroll_freqs_memory();
void change_saved_channel_khz();
void change_saved_channel_mhz();
void move_cursor();
void  adjust_digit_interrupt();
void adjust_digit(int direction);
#endif  // D__JIM_CDU_HF_RCU_RCU_WORKSPACE_RCU_FIRMWARE_V0_0_CORE_INC_ROTARY_ENCODER_H_
