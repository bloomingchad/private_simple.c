// Build with: gcc -o simple simple.c `pkg-config --libs --cflags mpv`

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <curses.h>

#include <mpv/client.h>

#include <termios.h>

/*char getch() {
    char buf = 0;
    struct termios old = {0};
    fflush(stdout);
    if(tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if(tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if(read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if(tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    printf("%c\n", buf);
    return buf;
}*/

void check_error(int status) {
    if (status < 0) {
        printf("mpv API error: %s\n", mpv_error_string(status));
        exit(1);
    }
}

void ce(int s) {check_error(s);}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("pass a single media file as argument\n");
        return 1;
    }

    mpv_handle *ctx = mpv_create();
    if (!ctx) {
        printf("failed creating context\n");
        return 1;
    }

    // Enable default key bindings, so the user can actually interact with
    // the player (and e.g. close the window).
    //check_error(mpv_set_option_string(ctx, "input-default-bindings", "yes"));
    //mpv_set_option_string(ctx, "input-vo-keyboard", "yes");
    int val = 1;
    ce(mpv_set_option(ctx, "osc", MPV_FORMAT_FLAG, &val));
    ce(mpv_initialize(ctx));
    const char *cmd[] = {"loadfile", argv[1], NULL};
    ce(mpv_command(ctx, cmd));

    // Let it play, and wait until the user quits.
    while (1) {
        mpv_event *event = mpv_wait_event(ctx, 10000);
        printf("event: %s\n", mpv_event_name(event->event_id));
        if (event->event_id == MPV_EVENT_SHUTDOWN) break;
        if (event->event_id == MPV_EVENT_PLAYBACK_RESTART) {
        //mpv_node* val;
        //ce(mpv_get_property(ctx, "metadata", MPV_FORMAT_NODE_MAP, &val->u.list));

        char* res;
        ce(mpv_get_property(ctx, "media-title", MPV_FORMAT_STRING, &res));
        printf("title: %s\n", res);

		mpv_node result;
		ce(mpv_get_property(ctx, "metadata", MPV_FORMAT_NODE, &result.u.list));
		if (result.format == MPV_FORMAT_NODE_MAP) {
			for (int i = 0; i < result.u.list->num; i++) {
				printf("%s: %s\n", result.u.list->keys[i], result.u.list->values[i].u.string);
			}
		}

         }
       }

    mpv_terminate_destroy(ctx);
    return 0;
}
