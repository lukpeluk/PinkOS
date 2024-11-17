#include "homeScreen.h"
#include "colors.h"
#include "pictures.h"
#include "graphicsLib.h"
#include "keyboard.h"
#include "syscallCodes.h"
#include "handlerIds.h"
#include "stdpink.h"
#include "permissions.h"

#define INC_INDEX(index) index = index + 1 < 4 ? index + 1 : index
#define DEC_INDEX(index) index = index - 1 >= 0 ? index - 1 : index

static int show_home_screen = 1;
static int show_settings_screen = 0;
static int show_about_screen = 0;

static char *logo_str = "PinkOS";
static char *version_str = "v0.6.9";
static char *options[] = {"START", "SETTINGS", "ABOUT", "EXIT"};
static char *is_selected_start = "> ";
static char *is_selected_end = " <";
static char *is_not_selected = "  ";
static int selected_option = 0;
static int previous_option = 0;


void home_screen_key_handler(unsigned char event_type, int hold_times, unsigned char ascii, unsigned char scan_code)
{
    if (event_type != 1 && event_type != 3)  // just register press events (not release or null events)
		return;

	// show_home_screen = 0;
	if(scan_code == 0x48){
        previous_option = selected_option;
        DEC_INDEX(selected_option);
        draw_selected_option();
        return;
	}else if(scan_code == 0x50){
        previous_option = selected_option;
        INC_INDEX(selected_option);
        draw_selected_option();
        return;
    }

    if (ascii == '\n')
    {
        switch (selected_option)
        {
            case 0:
                show_home_screen = 0;
                break;
            case 1:
                show_settings_screen = 1;
                break;
            case 2:
                show_about_screen = 1;
                break;
            case 3:
                show_home_screen = -1;
                break;
        }
    }
	
}

void draw_selected_option()
{
	syscall(INC_FONT_SIZE_SYSCALL, 1, 0, 0, 0, 0);
	syscall(INC_FONT_SIZE_SYSCALL, 1, 0, 0, 0, 0);

    int screen_width = getScreenWidth();
	int screen_height = getScreenHeight();

	Point options_position = {0, screen_height / 4 + 100};

    for (int i = 0; i < 4; i++)
	{
		options_position.x = (screen_width / 2) - ((strlen(options[i]) * getCharWidth() )/ 2);
		if (i == selected_option)
		{
			Point temp_position = options_position;
			temp_position.x -= strlen(is_selected_start) * getCharWidth();
			syscall(DRAW_STRING_AT_SYSCALL, is_selected_start, PinkOSColors.text, PinkOSColors.background, &temp_position, 0);
			temp_position.x += strlen(is_selected_end) * getCharWidth() + strlen(options[i]) * getCharWidth();
			syscall(DRAW_STRING_AT_SYSCALL, is_selected_end, PinkOSColors.text, PinkOSColors.background, &temp_position, 0);
		}else if (i == previous_option)
        {
            Point temp_position = options_position;
            temp_position.x -= strlen(is_not_selected) * getCharWidth();
            syscall(DRAW_STRING_AT_SYSCALL, is_not_selected, PinkOSColors.text, PinkOSColors.background, &temp_position, 0);
            temp_position.x += strlen(is_not_selected) * getCharWidth() + strlen(options[i]) * getCharWidth();
            syscall(DRAW_STRING_AT_SYSCALL, is_not_selected, PinkOSColors.text, PinkOSColors.background, &temp_position, 0);
        }
		options_position.y += 50;
	}

    syscall(DEC_FONT_SIZE_SYSCALL, 0, 0, 0, 0, 0);
    syscall(DEC_FONT_SIZE_SYSCALL, 0, 0, 0, 0, 0);
}

void home_screen()
{

	syscall(SET_HANDLER_SYSCALL, KEY_HANDLER, home_screen_key_handler, 0, 0, 0);

	Point position = {0};
	int scale = 12;

	int screen_width = getScreenWidth();
	int screen_height = getScreenHeight();

	// position.x = (screen_width - FOOTPRINT_WIDTH * scale) / 2;
	// position.y = (screen_height - FOOTPRINT_HEIGHT * scale) / 2;

	// drawBitmap(footprint, FOOTPRINT_WIDTH, FOOTPRINT_HEIGHT, (Point){position.x, position.y}, scale);
	// drawBitmap(mona_lisa, MONA_LISA_WIDTH, MONA_LISA_HEIGHT, position, scale);

	syscall(INC_FONT_SIZE_SYSCALL, 1, 0, 0, 0, 0);
	syscall(INC_FONT_SIZE_SYSCALL, 1, 0, 0, 0, 0);
	syscall(INC_FONT_SIZE_SYSCALL, 1, 0, 0, 0, 0);

	// center the text
	
	// Center the logo in x, and place it 1/3 of the screen height from the top
	Point logo_position = {(screen_width / 2) - strlen(logo_str) * getCharWidth() / 2, screen_height / 4};
	// The version most be placed at the bottom right corner
	Point version_position = {screen_width - strlen(version_str) * getCharWidth(), screen_height - 50}; 

	syscall(DRAW_STRING_AT_SYSCALL, logo_str, PinkOSColors.primary, PinkOSColors.background, &logo_position, 0);
	syscall(DEC_FONT_SIZE_SYSCALL, 0, 0, 0, 0, 0);

    Point options_position = {0, screen_height / 4 + 100};

    for (int i = 0; i < 4; i++)
	{
		options_position.x = (screen_width / 2) - ((strlen(options[i]) * getCharWidth() )/ 2);
		syscall(DRAW_STRING_AT_SYSCALL, options[i], PinkOSColors.text, PinkOSColors.background, &options_position, 0);
		options_position.y += 50;
	}

	syscall(DEC_FONT_SIZE_SYSCALL, 0, 0, 0, 0, 0);
	syscall(DEC_FONT_SIZE_SYSCALL, 0, 0, 0, 0, 0);

    draw_selected_option();


	syscall(DRAW_STRING_AT_SYSCALL, version_str, PinkOSColors.text, PinkOSColors.background, &version_position, 0);

	while (show_home_screen)
	{
		_hlt();
	}
}
