#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <libnotify/notify.h>
#include <sys/inotify.h>
#include <sys/types.h>

#include "config.h"

#define EVENT_SIZE  ( sizeof( struct inotify_event ) )
#define BUF_LEN     ( 8 * ( EVENT_SIZE + 32 ) ) 
#define DEVICE_DIR  "/dev/"

#define error(e)    ( fprintf(stderr, e) )

static void
send_notification(const char* device, const char* action) {
    NotifyNotification *notification;
    
    notify_init("usbnotify");
    notification = notify_notification_new(action, device,
            ICON_PATH);

    notify_notification_show(notification, NULL);
    notify_uninit();
}

static void 
daemonize() {
    pid_t pid;
    pid = fork();
    if (pid < 0) {
        error("fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        // parent code
        exit(EXIT_SUCCESS);
    }

    // child code
    printf("started (%u)\n", getpid());

    umask(0);
}

int
main(int argc, char* argv[]) {
    daemonize();
    int fd;
    int wd;
    int len = 0;
    char buffer[ BUF_LEN ];

    fd = inotify_init();
    if ( fd < 0 ) {
        ( void ) fprintf(stderr, "error: inotify_init\n");
        exit(1);
    }

    wd = inotify_add_watch(fd, DEVICE_DIR, IN_CREATE | IN_DELETE);

    while ( 1 ) {
        len = read(fd, buffer, BUF_LEN);
        int i = 0;
        while ( i < len ) {
            struct inotify_event *event = 
                ( struct inotify_event * ) &buffer[i];
            if ( event->len ) {
                short name_len = 0;
                char device[16]; // current device name
                strcpy(device, DEVICE_DIR);
                strncat(device, event->name, 3);
                // result -> /dev/sdX
                name_len = strlen(event->name);
                
                if ( name_len == 3 ) {
                    if ( event->mask & IN_CREATE ) {
                        send_notification(device, DEVICE_CONNECTED);
                    }
                    else if ( event->mask & IN_DELETE ) {
                        send_notification(device, DEVICE_DISCONNECTED);
                    }
                }

            }
            i += EVENT_SIZE + event->len;
        }
    }

    ( void ) inotify_rm_watch(fd, wd);
    ( void ) close(fd);

    return 0;
}
