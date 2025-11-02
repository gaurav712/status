#include "bluetooth.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dbus/dbus.h>

#define BLUETOOTH_DEVICE_NAME_LEN 50
#define BLUETOOTH_BUS_NAME "org.bluez"
#define BLUETOOTH_ADAPTER_INTERFACE "org.bluez.Adapter1"
#define BLUETOOTH_DEVICE_INTERFACE "org.bluez.Device1"
#define DBUS_PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"
#define DBUS_OBJECTMANAGER_INTERFACE "org.freedesktop.DBus.ObjectManager"

/* Helper function to check D-Bus error */
static int dbus_check_error(DBusError *error) {
    if (dbus_error_is_set(error)) {
        dbus_error_free(error);
        return 0;
    }
    return 1;
}

/* Get property value from D-Bus object */
static DBusMessage* get_property(DBusConnection *conn, const char *path, 
                                 const char *interface, const char *property) {
    DBusError error;
    DBusMessage *msg, *reply;
    
    dbus_error_init(&error);
    
    msg = dbus_message_new_method_call(
        BLUETOOTH_BUS_NAME,
        path,
        DBUS_PROPERTIES_INTERFACE,
        "Get"
    );
    
    if (!msg) {
        return NULL;
    }
    
    dbus_message_append_args(msg,
        DBUS_TYPE_STRING, &interface,
        DBUS_TYPE_STRING, &property,
        DBUS_TYPE_INVALID
    );
    
    reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &error);
    dbus_message_unref(msg);
    
    if (!dbus_check_error(&error)) {
        if (reply) dbus_message_unref(reply);
        return NULL;
    }
    
    return reply;
}

/* Find adapter paths - try common paths first, then enumerate */
static int find_adapter_path(DBusConnection *conn, char *adapter_path, size_t path_len) {
    DBusError error;
    DBusMessage *msg, *reply;
    DBusMessageIter iter, array_iter, entry_iter, dict_iter, entry_iter2;
    const char *object_path;
    const char *interface_name;
    
    /* First try common adapter paths */
    const char *common_paths[] = {"/org/bluez/hci1", "/org/bluez/hci0"};
    int i;
    
    for (i = 0; i < 2; i++) {
        DBusMessage *test_reply = get_property(conn, common_paths[i], BLUETOOTH_ADAPTER_INTERFACE, "Powered");
        if (test_reply) {
            dbus_message_unref(test_reply);
            strncpy(adapter_path, common_paths[i], path_len - 1);
            adapter_path[path_len - 1] = '\0';
            return 1;
        }
    }
    
    /* If common paths don't work, enumerate via GetManagedObjects */
    dbus_error_init(&error);
    
    msg = dbus_message_new_method_call(
        BLUETOOTH_BUS_NAME,
        "/",
        DBUS_OBJECTMANAGER_INTERFACE,
        "GetManagedObjects"
    );
    
    if (!msg) {
        return 0;
    }
    
    reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &error);
    dbus_message_unref(msg);
    
    if (!dbus_check_error(&error) || !reply) {
        if (reply) dbus_message_unref(reply);
        return 0;
    }
    
    if (!dbus_message_iter_init(reply, &iter)) {
        dbus_message_unref(reply);
        return 0;
    }
    
    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY) {
        dbus_message_unref(reply);
        return 0;
    }
    
    dbus_message_iter_recurse(&iter, &array_iter);
    
    int found_count = 0;
    char last_adapter[256] = "";
    
    while (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_DICT_ENTRY) {
        dbus_message_iter_recurse(&array_iter, &entry_iter);
        
        /* Get object path */
        if (dbus_message_iter_get_arg_type(&entry_iter) != DBUS_TYPE_OBJECT_PATH) {
            dbus_message_iter_next(&array_iter);
            continue;
        }
        
        dbus_message_iter_get_basic(&entry_iter, &object_path);
        dbus_message_iter_next(&entry_iter);
        
        /* Check interfaces dictionary */
        if (dbus_message_iter_get_arg_type(&entry_iter) != DBUS_TYPE_ARRAY) {
            dbus_message_iter_next(&array_iter);
            continue;
        }
        
        dbus_message_iter_recurse(&entry_iter, &dict_iter);
        
        while (dbus_message_iter_get_arg_type(&dict_iter) == DBUS_TYPE_DICT_ENTRY) {
            dbus_message_iter_recurse(&dict_iter, &entry_iter2);
            
            if (dbus_message_iter_get_arg_type(&entry_iter2) != DBUS_TYPE_STRING) {
                dbus_message_iter_next(&dict_iter);
                continue;
            }
            
            dbus_message_iter_get_basic(&entry_iter2, &interface_name);
            
            if (strcmp(interface_name, BLUETOOTH_ADAPTER_INTERFACE) == 0) {
                found_count++;
                strncpy(last_adapter, object_path, sizeof(last_adapter) - 1);
                last_adapter[sizeof(last_adapter) - 1] = '\0';
            }
            
            dbus_message_iter_next(&dict_iter);
        }
        
        dbus_message_iter_next(&array_iter);
    }
    
    dbus_message_unref(reply);
    
    if (found_count > 0) {
        /* Use the last adapter found (skip first one used for volume) */
        strncpy(adapter_path, last_adapter, path_len - 1);
        adapter_path[path_len - 1] = '\0';
        return 1;
    }
    
    return 0;
}

/* Check if bluetooth adapter is powered (enabled) */
short bluetooth_is_blocked(void) {
    DBusConnection *conn;
    DBusError error;
    char adapter_path[256];
    DBusMessage *reply;
    DBusMessageIter iter, variant_iter;
    dbus_bool_t powered = 0;
    
    dbus_error_init(&error);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    
    if (!dbus_check_error(&error) || !conn) {
        return 1; /* Assume blocked on error */
    }
    
    if (!find_adapter_path(conn, adapter_path, sizeof(adapter_path))) {
        dbus_connection_unref(conn);
        return 1; /* No adapter found, assume blocked */
    }
    
    reply = get_property(conn, adapter_path, BLUETOOTH_ADAPTER_INTERFACE, "Powered");
    if (!reply) {
        dbus_connection_unref(conn);
        return 1;
    }
    
    if (dbus_message_iter_init(reply, &iter)) {
        if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_VARIANT) {
            dbus_message_iter_recurse(&iter, &variant_iter);
            if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_BOOLEAN) {
                dbus_message_iter_get_basic(&variant_iter, &powered);
            }
        }
    }
    
    dbus_message_unref(reply);
    dbus_connection_unref(conn);
    
    /* Return 1 if blocked (not powered), 0 if unblocked (powered) */
    return powered ? 0 : 1;
}

/* Check if any bluetooth device is connected */
short bluetooth_is_connected(void) {
    DBusConnection *conn;
    DBusError error;
    DBusMessage *msg, *reply;
    DBusMessageIter iter, array_iter, entry_iter, dict_iter, entry_iter2;
    const char *object_path;
    const char *interface_name;
    dbus_bool_t connected = 0;
    
    dbus_error_init(&error);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    
    if (!dbus_check_error(&error) || !conn) {
        return 0;
    }
    
    msg = dbus_message_new_method_call(
        BLUETOOTH_BUS_NAME,
        "/",
        DBUS_OBJECTMANAGER_INTERFACE,
        "GetManagedObjects"
    );
    
    if (!msg) {
        dbus_connection_unref(conn);
        return 0;
    }
    
    reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &error);
    dbus_message_unref(msg);
    
    if (!dbus_check_error(&error) || !reply) {
        if (reply) dbus_message_unref(reply);
        dbus_connection_unref(conn);
        return 0;
    }
    
    if (!dbus_message_iter_init(reply, &iter)) {
        dbus_message_unref(reply);
        dbus_connection_unref(conn);
        return 0;
    }
    
    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY) {
        dbus_message_unref(reply);
        dbus_connection_unref(conn);
        return 0;
    }
    
    dbus_message_iter_recurse(&iter, &array_iter);
    
    while (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_DICT_ENTRY) {
        dbus_message_iter_recurse(&array_iter, &entry_iter);
        
        if (dbus_message_iter_get_arg_type(&entry_iter) != DBUS_TYPE_OBJECT_PATH) {
            dbus_message_iter_next(&array_iter);
            continue;
        }
        
        dbus_message_iter_get_basic(&entry_iter, &object_path);
        dbus_message_iter_next(&entry_iter);
        
        if (dbus_message_iter_get_arg_type(&entry_iter) != DBUS_TYPE_ARRAY) {
            dbus_message_iter_next(&array_iter);
            continue;
        }
        
        dbus_message_iter_recurse(&entry_iter, &dict_iter);
        
        while (dbus_message_iter_get_arg_type(&dict_iter) == DBUS_TYPE_DICT_ENTRY) {
            dbus_message_iter_recurse(&dict_iter, &entry_iter2);
            
            if (dbus_message_iter_get_arg_type(&entry_iter2) != DBUS_TYPE_STRING) {
                dbus_message_iter_next(&dict_iter);
                continue;
            }
            
            dbus_message_iter_get_basic(&entry_iter2, &interface_name);
            
            if (strcmp(interface_name, BLUETOOTH_DEVICE_INTERFACE) == 0) {
                DBusMessage *prop_reply = get_property(conn, object_path, BLUETOOTH_DEVICE_INTERFACE, "Connected");
                if (prop_reply) {
                    DBusMessageIter prop_iter, prop_variant;
                    if (dbus_message_iter_init(prop_reply, &prop_iter)) {
                        if (dbus_message_iter_get_arg_type(&prop_iter) == DBUS_TYPE_VARIANT) {
                            dbus_message_iter_recurse(&prop_iter, &prop_variant);
                            if (dbus_message_iter_get_arg_type(&prop_variant) == DBUS_TYPE_BOOLEAN) {
                                dbus_message_iter_get_basic(&prop_variant, &connected);
                                if (connected) {
                                    dbus_message_unref(prop_reply);
                                    dbus_message_unref(reply);
                                    dbus_connection_unref(conn);
                                    return 1;
                                }
                            }
                        }
                    }
                    dbus_message_unref(prop_reply);
                }
            }
            
            dbus_message_iter_next(&dict_iter);
        }
        
        dbus_message_iter_next(&array_iter);
    }
    
    dbus_message_unref(reply);
    dbus_connection_unref(conn);
    
    return 0;
}

/* Get name of connected bluetooth device */
void get_connected_bluetooth_device_name(char *device_name) {
    DBusConnection *conn;
    DBusError error;
    DBusMessage *msg, *reply;
    DBusMessageIter iter, array_iter, entry_iter, dict_iter, entry_iter2;
    const char *object_path;
    const char *interface_name;
    
    device_name[0] = '\0';
    
    dbus_error_init(&error);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    
    if (!dbus_check_error(&error) || !conn) {
        return;
    }
    
    msg = dbus_message_new_method_call(
        BLUETOOTH_BUS_NAME,
        "/",
        DBUS_OBJECTMANAGER_INTERFACE,
        "GetManagedObjects"
    );
    
    if (!msg) {
        dbus_connection_unref(conn);
        return;
    }
    
    reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &error);
    dbus_message_unref(msg);
    
    if (!dbus_check_error(&error) || !reply) {
        if (reply) dbus_message_unref(reply);
        dbus_connection_unref(conn);
        return;
    }
    
    if (!dbus_message_iter_init(reply, &iter)) {
        dbus_message_unref(reply);
        dbus_connection_unref(conn);
        return;
    }
    
    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY) {
        dbus_message_unref(reply);
        dbus_connection_unref(conn);
        return;
    }
    
    dbus_message_iter_recurse(&iter, &array_iter);
    
    while (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_DICT_ENTRY) {
        dbus_message_iter_recurse(&array_iter, &entry_iter);
        
        if (dbus_message_iter_get_arg_type(&entry_iter) != DBUS_TYPE_OBJECT_PATH) {
            dbus_message_iter_next(&array_iter);
            continue;
        }
        
        dbus_message_iter_get_basic(&entry_iter, &object_path);
        dbus_message_iter_next(&entry_iter);
        
        if (dbus_message_iter_get_arg_type(&entry_iter) != DBUS_TYPE_ARRAY) {
            dbus_message_iter_next(&array_iter);
            continue;
        }
        
        dbus_message_iter_recurse(&entry_iter, &dict_iter);
        
        while (dbus_message_iter_get_arg_type(&dict_iter) == DBUS_TYPE_DICT_ENTRY) {
            dbus_message_iter_recurse(&dict_iter, &entry_iter2);
            
            if (dbus_message_iter_get_arg_type(&entry_iter2) != DBUS_TYPE_STRING) {
                dbus_message_iter_next(&dict_iter);
                continue;
            }
            
            dbus_message_iter_get_basic(&entry_iter2, &interface_name);
            
            if (strcmp(interface_name, BLUETOOTH_DEVICE_INTERFACE) == 0) {
                DBusMessage *connected_reply = get_property(conn, object_path, BLUETOOTH_DEVICE_INTERFACE, "Connected");
                dbus_bool_t connected = 0;
                
                if (connected_reply) {
                    DBusMessageIter connected_iter, connected_variant;
                    if (dbus_message_iter_init(connected_reply, &connected_iter)) {
                        if (dbus_message_iter_get_arg_type(&connected_iter) == DBUS_TYPE_VARIANT) {
                            dbus_message_iter_recurse(&connected_iter, &connected_variant);
                            if (dbus_message_iter_get_arg_type(&connected_variant) == DBUS_TYPE_BOOLEAN) {
                                dbus_message_iter_get_basic(&connected_variant, &connected);
                            }
                        }
                    }
                    dbus_message_unref(connected_reply);
                }
                
                if (connected) {
                    DBusMessage *name_reply = get_property(conn, object_path, BLUETOOTH_DEVICE_INTERFACE, "Name");
                    if (name_reply) {
                        DBusMessageIter name_iter, name_variant;
                        const char *name_value = NULL;
                        
                        if (dbus_message_iter_init(name_reply, &name_iter)) {
                            if (dbus_message_iter_get_arg_type(&name_iter) == DBUS_TYPE_VARIANT) {
                                dbus_message_iter_recurse(&name_iter, &name_variant);
                                if (dbus_message_iter_get_arg_type(&name_variant) == DBUS_TYPE_STRING) {
                                    dbus_message_iter_get_basic(&name_variant, &name_value);
                                    if (name_value) {
                                        strncpy(device_name, name_value, BLUETOOTH_DEVICE_NAME_LEN - 1);
                                        device_name[BLUETOOTH_DEVICE_NAME_LEN - 1] = '\0';
                                        dbus_message_unref(name_reply);
                                        dbus_message_unref(reply);
                                        dbus_connection_unref(conn);
                                        return;
                                    }
                                }
                            }
                        }
                        dbus_message_unref(name_reply);
                    }
                }
            }
            
            dbus_message_iter_next(&dict_iter);
        }
        
        dbus_message_iter_next(&array_iter);
    }
    
    dbus_message_unref(reply);
    dbus_connection_unref(conn);
}

/* Legacy functions kept for compatibility but not used */
void find_bluetooth_rfkill_device(char *rfkill_device) {
    rfkill_device[0] = '\0';
}

short bluetooth_is_enabled(char *rfkill_device) {
    (void)rfkill_device; /* Unused */
    return !bluetooth_is_blocked();
}