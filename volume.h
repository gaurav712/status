#ifndef VOLUME_H
#define VOLUME_H

#include <stdint.h>

enum VolumeIcon { IC_SPEAKER, IC_HEADPHONE, IC_BT_HEADSET, IC_MUTE };

uint8_t get_volume(void);
uint8_t get_mute(void);
uint8_t get_volume_icon_type(void);

#endif // VOLUME_H
