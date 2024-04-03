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

void metadata(mpv_handle* ctx) {
	mpv_node result;
	ce(mpv_get_property(ctx, "metadata", MPV_FORMAT_NODE, &result.u.list));
	printf("--------------NODEMAP---------\n");
	for (int i = 0; i < result.u.list->num; i++) {
		printf("%s: %s\n", result.u.list->keys[i], result.u.list->values[i].u.string);
	} printf("-----------endNODEMAP---------\n");

    char* res;
    ce(mpv_get_property(ctx, "media-title", MPV_FORMAT_STRING, &res));
    printf("title: %s\n", res);
}


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

    int val = 1;
    ce(mpv_set_option(ctx, "osc", MPV_FORMAT_FLAG, &val));
    ce(mpv_initialize(ctx));
    const char *cmd[] = {"loadfile", argv[1], NULL};
    ce(mpv_command(ctx, cmd));
    //uint8_t uval = 0;
    mpv_event *event = mpv_wait_event(ctx, 0);
    //printf("%s\n", mpv_event_name(event->event_id));
    int metadata_is_observed = 0;

    while (1) {
        //if(uval == 500) uval = 0;
        event = mpv_wait_event(ctx, 1000);
        //printf("event: %s\n", mpv_event_name(event->event_id));
        if (event->event_id == MPV_EVENT_SHUTDOWN) break;
        if (event->event_id == MPV_EVENT_END_FILE) return 1;
        if(event->event_id == MPV_EVENT_PLAYBACK_RESTART && !metadata_is_observed) {
        	mpv_observe_property(ctx, 0
        		//event->reply_userdata
        		, "metadata", MPV_FORMAT_NONE);
			//printf("set to observe\n");
			metadata_is_observed = 1;
        }
        //if (uval == 0) {
        printf("event: %s\n", mpv_event_name(event->event_id));//}
        //uval++;
        //printf("%d", uval);
/*        if (event->event_id == MPV_EVENT_PLAYBACK_RESTART) {
        	metadata(ctx);
        }*/

        if (event->event_id == MPV_EVENT_PROPERTY_CHANGE) {
        	metadata(ctx);
        	printf("property crhanged\n");
        }

	}

    mpv_terminate_destroy(ctx);
    return 0;
}
