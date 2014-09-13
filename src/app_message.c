#include <pebble.h>

#define NUM_MENU_SECTIONS 2
#define NUM_MENU_ICONS 3
#define NUM_FIRST_MENU_ITEMS 3
#define NUM_SECOND_MENU_ITEMS 1
  
//GLOBAL-VARIABLES-------------------------------------------------------------------------------------*
Window *mainWindow;	
static MenuLayer *mainMenu; 

//int daySelect = 0;

char *tag1 = "Loading...";
int length_Tag1 = 1;
char *tag2 = "Loading...";
int length_Tag2 = 1;

char *data_tag[2][50];


// Key values for AppMessage Dictionary
enum {
	TAG1 = 100,	
	TAG2 = 102,
  UPDATE = 104
};


static void menu_cell_track_draw (GContext* ctx, const Layer *cell_layer, const char *text) {
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, text, fonts_get_system_font(FONT_KEY_GOTHIC_18), layer_get_bounds(cell_layer), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return length_Tag1;

    case 1:
      return length_Tag2;

    default:
      return 0;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  // This is a define provided in pebble.h that you may use for the default height
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // There is title draw for something more simple than a basic menu item
  menu_cell_track_draw(ctx, cell_layer, data_tag[cell_index->section][cell_index->row]);
  
}

void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {

}
static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case 0:
      // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, tag1);
      break;

    case 1:
      menu_cell_basic_header_draw(ctx, cell_layer, tag2);
      break;
  }
}
	
//MAIN_WINDOW------------------------------------------------------------------------------------------*

//setup Top Bar w/ date(placeholder)
void mainWindow_load(Window *thisWindow){
  Layer *window_layer = window_get_root_layer(thisWindow);
  GRect bounds = layer_get_frame(window_layer);
  
  mainMenu = menu_layer_create(bounds);
  menu_layer_set_callbacks(mainMenu, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });
  menu_layer_set_click_config_onto_window(mainMenu, thisWindow);
  
  layer_add_child(window_layer, menu_layer_get_layer(mainMenu));
}

//deinit for mainWindow
void mainWindow_unload(Window *thisWindow){
  menu_layer_destroy(mainMenu);
}


//CLICK_HANDLING---------------------------------------------------------------------------------------*




//APP-MESSAGE-STUFF-------------------------------------------------------------------------------------*

// Write message to buffer & send
void getNewData(void){
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, UPDATE, 0x1);
	
	dict_write_end(iter);
  app_message_outbox_send();
}

// Called when a message is received from PebbleKitJS
// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *tuple = dict_read_first(received);
  
  length_Tag1 = 0;
  length_Tag2 = 0;
  
  while(tuple){
    int tmpKey = (int)tuple->key;
    switch(tuple->key){
      case TAG1:
        tag1 = tuple->value->cstring;
        break;
      case TAG2:
        tag2 = tuple->value->cstring;
        break;
      default:
        if(tmpKey < 50){
          //APP_LOG(APP_LOG_LEVEL_DEBUG, "Key: %d", tmpKey);
          data_tag[0][tmpKey] = tuple->value->cstring;
          length_Tag1++;
        }else{
          //APP_LOG(APP_LOG_LEVEL_DEBUG, "Key: %d", tmpKey-50);
          data_tag[1][tmpKey-50] = tuple->value->cstring;
          length_Tag2++;
        }
        break;
    }
    tuple = dict_read_next(received);
  }
  
  menu_layer_reload_data(mainMenu);
}
	

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {	
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
}

//GENERAL-INIT&DE-INIT-------------------------------------------------------------------------------------*

void init(void) {
	
	// Register AppMessage handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
		
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  //Create mainWindow & set handlers
  mainWindow = window_create();
  window_set_window_handlers(mainWindow, (WindowHandlers) {
    .load = mainWindow_load,
    .unload = mainWindow_unload,
  });
  
  
  //push Window
	window_stack_push(mainWindow, true);
	
  //fetch data
	getNewData();
}

void deinit(void) {
	app_message_deregister_callbacks();
	window_destroy(mainWindow);
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}