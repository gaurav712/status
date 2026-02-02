#ifndef VOLUME_H
#define VOLUME_H

enum VolumeIcon { IC_SPEAKER, IC_HEADPHONE, IC_BT_HEADSET, IC_MUTE };

short get_volume(void);
short get_mute(void);
int get_volume_icon_type(void);

#endif // VOLUME_H
