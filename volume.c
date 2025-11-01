#include "volume.h"

#include <pulse/pulseaudio.h>
#include <string.h>

#define APP_NAME "status"

static short volume_result = 0;
static short mute_result = 0;
static int icon_type_result = VOLUME_ICON_SPEAKER;
static int results_cached = 0;

static void context_state_cb(pa_context *context, void *mainloop) {
  if (!context || !mainloop) return;

  pa_context_state_t state = pa_context_get_state(context);
  switch (state) {
    case PA_CONTEXT_READY:
    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_TERMINATED:
      *((int*)mainloop) = 1;
      break;
    default:
      break;
  }
}

static void sink_info_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
  if (eol > 0 || !i) {
    *((int*)userdata) = 1;
    return;
  }

  pa_volume_t vol = pa_cvolume_avg(&(i->volume));
  volume_result = (short)((vol * 100ULL) / PA_VOLUME_NORM);
  mute_result = i->mute ? 1 : 0;

  const char *form_factor = pa_proplist_gets(i->proplist, PA_PROP_DEVICE_FORM_FACTOR);
  const char *device_description = pa_proplist_gets(i->proplist, PA_PROP_DEVICE_DESCRIPTION);
  const char *description = i->description;

  int is_headphone = 0;
  int is_headset = 0;

  if (form_factor && strcmp(form_factor, "headset") == 0) {
    is_headset = 1;
  } else if (form_factor && strcmp(form_factor, "headphone") == 0) {
    is_headphone = 1;
  } else if (description && (strstr(description, "headphone") != NULL || 
                              strstr(description, "Headphone") != NULL)) {
    is_headphone = 1;
  } else if (description && (strstr(description, "Headset") != NULL ||
                              strstr(description, "headset") != NULL)) {
    is_headset = 1;
  } else if (device_description && (strstr(device_description, "headphone") != NULL || 
                                     strstr(device_description, "Headphone") != NULL)) {
    is_headphone = 1;
  } else if (device_description && (strstr(device_description, "Headset") != NULL ||
                                     strstr(device_description, "headset") != NULL)) {
    is_headset = 1;
  } else if (i->active_port && i->active_port->name && 
             (strstr(i->active_port->name, "headphone") != NULL ||
              strstr(i->active_port->name, "Headphone") != NULL)) {
    is_headphone = 1;
  } else if (i->active_port && i->active_port->description &&
             (strstr(i->active_port->description, "Headphone") != NULL ||
              strstr(i->active_port->description, "headphone") != NULL)) {
    is_headphone = 1;
  }

  if (is_headset && i->name && strstr(i->name, "bluez") != NULL) {
    icon_type_result = VOLUME_ICON_BLUETOOTH_HEADSET;
  } else if (is_headset || is_headphone) {
    icon_type_result = VOLUME_ICON_HEADPHONE;
  } else {
    icon_type_result = VOLUME_ICON_SPEAKER;
  }

  *((int*)userdata) = 1;
}

static void get_sink_info(void) {
  pa_mainloop *ml = NULL;
  pa_mainloop_api *api = NULL;
  pa_context *ctx = NULL;
  pa_operation *op = NULL;
  int ready = 0;

  if (results_cached) return;

  ml = pa_mainloop_new();
  if (!ml) return;

  api = pa_mainloop_get_api(ml);
  ctx = pa_context_new(api, APP_NAME);
  if (!ctx) {
    pa_mainloop_free(ml);
    return;
  }

  pa_context_set_state_callback(ctx, context_state_cb, &ready);
  pa_context_connect(ctx, NULL, PA_CONTEXT_NOFLAGS, NULL);

  while (!ready) pa_mainloop_iterate(ml, 1, NULL);

  if (pa_context_get_state(ctx) != PA_CONTEXT_READY) {
    goto cleanup;
  }

  ready = 0;
  op = pa_context_get_sink_info_by_name(ctx, NULL, sink_info_cb, &ready);

  while (!ready) pa_mainloop_iterate(ml, 1, NULL);

cleanup:
  if (op) pa_operation_unref(op);
  if (ctx) {
    pa_context_disconnect(ctx);
    pa_context_unref(ctx);
  }
  if (ml) pa_mainloop_free(ml);
  
  results_cached = 1;
}

short get_volume(void) {
  get_sink_info();
  return volume_result;
}

short get_mute(void) {
  get_sink_info();
  return mute_result;
}

int get_volume_icon_type(void) {
  get_sink_info();
  return icon_type_result;
}
