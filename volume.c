#include "volume.h"

#include <alsa/asoundlib.h>
#include <alsa/mixer.h>
#include <stdlib.h>

short get_volume(void) {

    int return_value;
    long current_vol, min_volume, max_volume;
    snd_mixer_t *mixer_handle;
    snd_mixer_selem_id_t *selem_id;
    snd_mixer_elem_t *elem;
    const char *card = "default";
    const char *selem_name = "Master";

    snd_mixer_open(&mixer_handle, 0);
    snd_mixer_attach(mixer_handle, card);
    snd_mixer_selem_register(mixer_handle, NULL, NULL);
    snd_mixer_load(mixer_handle);
    snd_mixer_selem_id_alloca(&selem_id);
    snd_mixer_selem_id_set_index(selem_id, 0);
    snd_mixer_selem_id_set_name(selem_id, selem_name);

    elem = snd_mixer_find_selem(mixer_handle, selem_id);

    snd_mixer_selem_get_playback_volume_range(elem, &min_volume, &max_volume);

    if(return_value = snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &current_vol)) {
        perror("can't get volume");
        exit(1);
    }

    snd_mixer_close(mixer_handle);

    return ((float)current_vol/max_volume * 100);
}
