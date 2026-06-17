#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#define APP_WINDOW_TITLE "Friendly break reminder"

// Paths for the configuration directory and INI file
#define CONFIG_DIR ".\\config"
#define INI_FILE_PATH ".\\config\\settings.ini"

// Default paths for visual and audio assets
#define DEFAULT_WORK_GIF_PATH "./documents/kos.gif"
#define DEFAULT_BREAK_GIF_PATH "./documents/Break_Over_GIF.gif"
#define DEFAULT_ALARM_SOUND "./documents/hey.mp3"
#define DEFAULT_INFO_GIF_PATH "./documents/reze_pool.gif"
#define DEFAULT_GOODBYE_GIF_PATH "./documents/good_by.gif"

// Default duration and cycle settings
#define DEFAULT_WORK_MIN 25
#define DEFAULT_WORK_SEC 0
#define DEFAULT_BREAK_MIN 5
#define DEFAULT_BREAK_SEC 0
#define DEFAULT_CYCLES 3
#define DEFAULT_VOLUME 50

// Default panel dimensions for the setup screen
#define DEFAULT_PANEL_W 860
#define DEFAULT_PANEL_H 740

// Number of motivational quotes to be rotated
#define QUOTE_COUNT 5

#endif // APP_CONFIG_H

