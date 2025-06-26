#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#define ROOT_PERMISSIONS 0xFFFFFFFF

// KERNEL Permissions 0x01000000 - 0xFF000000 [      ]
#define MANAGE_PROCESSES_PERMISSION         0b10000000000000000000000000000000 
#define MANAGE_FILES_PERMISSION             0b01000000000000000000000000000000 
#define MANAGE_WINDOWS_PERMISSION           0b00100000000000000000000000000000
#define INSTALL_PROGRAM_PERMISSION          0b00010000000000000000000000000000
//                                                   .
//                                                   .
// GUI Permissions 0x00000001 - 0x0000000F           .                    [  ]
#define DRAWING_PERMISSION                  0b00000000000000000000000000000001 
#define CHANGE_FONT_PERMISSION              0b00000000000000000000000000000010
#define CHANGE_FONT_SIZE_PERMISSION         0b00000000000000000000000000000100
#define DRAW_OVERLAY_PERMISSION             0b00000000000000000000000000001000
//                                                   .
// RTC Permissions 0x00000010 - 0x000000F0           .                [  ]
#define SET_TIMEZONE_PERMISSION             0b00000000000000000000000000010000
//                                                   .
// AUDIO Permissions 0x00000100 - 0x00000F00         .            [  ]
#define PLAY_AUDIO_PERMISSION               0b00000000000000000000000100000000
//                                                   .
// SERIAL Permissions 0x00001000 - 0x0000F000        .        [  ]
#define MAKE_ETHEREAL_REQUEST_PERMISSION    0b00000000000000000001000000000000
//                                                   .
// EVENT Permissions 0x00010000 - 0x000F0000         .    [  ]
#define LISTEN_GLOBAL_KEY_EVENTS            0b00000000000000010000000000000000


#endif