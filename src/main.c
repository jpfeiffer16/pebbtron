#include <pebble.h>

#define DIRECTION_UP 1
#define DIRECTION_RIGHT 2
#define DIRECTION_DOWN 3
#define DIRECTION_LEFT 4
  
#define SCREEN_WIDTH 150
#define SCREEN_HEIGHT 150
  
#define FRAM_RATE 100
  

//Ui Stuffs...  
  
Window *my_window;
TextLayer *text_layer;
InverterLayer *inversion_layer;
Layer *canvas;



//And Game Vars...

typedef struct LinePoints {
  int first_pos_x;
  int first_pos_y;
  int second_pos_x;
  int second_pos_y;
} LinePoints;


//Create the arrays that will store the points in the trails of the bikes as they move
LinePoints human_line_points[50];
LinePoints ai_line_points[50];
int human_index = 0;
int ai_index = 0;
bool is_paused = false;
//And the directions for the two players
int human_travel_direction = DIRECTION_DOWN;
int ai_travel_direction = DIRECTION_UP;



//Functions for doing the tough-ass work dats gots to be done for this game....

void game_over(char game_over_text[]) {
  Layer *window_layer = window_get_root_layer(my_window);
  GRect bounds = layer_get_frame(window_layer);
  text_layer_set_text(text_layer, game_over_text);
  inversion_layer = inverter_layer_create(bounds);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  layer_add_child(window_layer, inverter_layer_get_layer(inversion_layer));
  is_paused = true;
}

void check_bounds_and_report(int x, int y, char text[]) {
  if(x > SCREEN_WIDTH || x < 0 || y > SCREEN_HEIGHT || y < 0) {
    game_over(text);
  }
}

void update_ai_direction() {
  ai_index++;
  ai_line_points[ai_index].first_pos_y = ai_line_points[ai_index - 1].second_pos_y;
  ai_line_points[ai_index].second_pos_y = ai_line_points[ai_index - 1].second_pos_y;
  ai_line_points[ai_index].first_pos_x = ai_line_points[ai_index - 1].second_pos_x;
  ai_line_points[ai_index].second_pos_x = ai_line_points[ai_index - 1].second_pos_x;
}

int random_number(int min_num, int max_num)
{
    int result=0,low_num=0,hi_num=0;
    if(min_num<max_num)
    {
        low_num=min_num;
        hi_num=max_num+1; // this is done to include max_num in output.
    }else{
        low_num=max_num+1;// this is done to include max_num in output.
        hi_num=min_num;
    }
    srand(time(NULL));
    result = (rand()%(hi_num-low_num))+low_num;
    return result;     
}

void ai_think() {
  int ran = random_number(0, 20);
  if(ran == 0) {
    int ran_dir = random_number(1, 2);
    if(ran_dir == 1){
      switch(ai_travel_direction) {
        case DIRECTION_UP:
          ai_travel_direction = DIRECTION_LEFT;
          break;
        case DIRECTION_RIGHT:
          ai_travel_direction = DIRECTION_UP;
          break;
        case DIRECTION_DOWN:
          ai_travel_direction = DIRECTION_RIGHT;
          break;
        case DIRECTION_LEFT:
          ai_travel_direction = DIRECTION_DOWN;
          break;
      }
    } else {
      switch(ai_travel_direction) {
        case DIRECTION_UP:
          ai_travel_direction = DIRECTION_RIGHT;
          break;
        case DIRECTION_RIGHT:
          ai_travel_direction = DIRECTION_DOWN;
          break;
        case DIRECTION_DOWN:
          ai_travel_direction = DIRECTION_LEFT;
          break;
        case DIRECTION_LEFT:
          ai_travel_direction = DIRECTION_UP;
          break;
  }
    }
    update_ai_direction();
  }
}

void canvas_next_frame() {
  //Check to see if the game is paused
  if (is_paused) {
    //call all ai functions...
    ai_think();
    
    //Do collision and bounds detection
    check_bounds_and_report(human_line_points[human_index].second_pos_x, human_line_points[human_index].second_pos_y, "Game Over!");
    check_bounds_and_report(ai_line_points[ai_index].second_pos_x, ai_line_points[ai_index].second_pos_y, "You Win!");
    
    //Update the human's position
    switch(human_travel_direction) {
      case DIRECTION_UP:
        human_line_points[human_index].second_pos_y--;
        break;
      case DIRECTION_RIGHT:
        human_line_points[human_index].second_pos_x++;
        break;
      case DIRECTION_DOWN:
        human_line_points[human_index].second_pos_y++;
        APP_LOG(1, "%i", human_line_points[human_index].second_pos_y);
        break;
      case DIRECTION_LEFT:
        human_line_points[human_index].second_pos_x--;
        break;
    }
    
    switch(ai_travel_direction) {
      case DIRECTION_UP:
        ai_line_points[ai_index].second_pos_y--;
        break;
      case DIRECTION_RIGHT:
        ai_line_points[ai_index].second_pos_x++;
        break;
      case DIRECTION_DOWN:
        ai_line_points[ai_index].second_pos_y++;
        APP_LOG(1, "%i", human_line_points[human_index].second_pos_y);
        break;
      case DIRECTION_LEFT:
        ai_line_points[ai_index].second_pos_x--;
        break;
    }
  }
  layer_mark_dirty(canvas);
}


void canvas_render(Layer *layer, GContext *ctx) {
  //render stuff...
  GPoint first_point;
  GPoint second_point;
  graphics_context_set_stroke_color(ctx, GColorBlack);

  for(int i = 0; i <= human_index; i++) {
    first_point.x = human_line_points[i].first_pos_x;
    first_point.y = human_line_points[i].first_pos_y;
    second_point.x = human_line_points[i].second_pos_x;
    second_point.y = human_line_points[i].second_pos_y;
    graphics_draw_line(ctx, first_point, second_point);

  }
  
  for(int i = 0; i <= ai_index; i++) {
    first_point.x = ai_line_points[i].first_pos_x;
    first_point.y = ai_line_points[i].first_pos_y;
    second_point.x = ai_line_points[i].second_pos_x;
    second_point.y = ai_line_points[i].second_pos_y;
    graphics_draw_line(ctx, first_point, second_point);

  }
  
  //now set a timer to render the next frame..
  app_timer_register(FRAM_RATE, canvas_next_frame, NULL);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(is_paused) {
    is_paused = false;
  } else {
    is_paused = true;
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch(human_travel_direction) {
    case DIRECTION_UP:
      human_travel_direction = DIRECTION_LEFT;
      break;
    case DIRECTION_RIGHT:
      human_travel_direction = DIRECTION_UP;
      break;
    case DIRECTION_DOWN:
      human_travel_direction = DIRECTION_RIGHT;
      break;
    case DIRECTION_LEFT:
      human_travel_direction = DIRECTION_DOWN;
      break;
  }
  human_index++;
  human_line_points[human_index].first_pos_y = human_line_points[human_index - 1].second_pos_y;
  human_line_points[human_index].second_pos_y = human_line_points[human_index - 1].second_pos_y;
  human_line_points[human_index].first_pos_x = human_line_points[human_index - 1].second_pos_x;
  human_line_points[human_index].second_pos_x = human_line_points[human_index - 1].second_pos_x;
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch(human_travel_direction) {
    case DIRECTION_UP:
      human_travel_direction = DIRECTION_RIGHT;
      break;
    case DIRECTION_RIGHT:
      human_travel_direction = DIRECTION_DOWN;
      break;
    case DIRECTION_DOWN:
      human_travel_direction = DIRECTION_LEFT;
      break;
    case DIRECTION_LEFT:
      human_travel_direction = DIRECTION_UP;
      break;
  }
  human_index++;
  human_line_points[human_index].first_pos_y = human_line_points[human_index - 1].second_pos_y;
  human_line_points[human_index].second_pos_y = human_line_points[human_index - 1].second_pos_y;
  human_line_points[human_index].first_pos_x = human_line_points[human_index - 1].second_pos_x;
  human_line_points[human_index].second_pos_x = human_line_points[human_index - 1].second_pos_x;
}



static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
//   window_single_repeating_click_subscribe(BUTTON_ID_UP, interval, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
//   window_single_repeating_click_subscribe(BUTTON_ID_DOWN, interval, down_click_handler);
}









//Right! Let's get this party started, shall we...
void handle_init(void) {
  //Initialize them trail-point arrays!
  human_line_points[0].first_pos_x = 50;
  human_line_points[0].first_pos_y = 1;
  human_line_points[0].second_pos_x = 50;
  human_line_points[0].second_pos_y = 1;
  ai_line_points[0].first_pos_x = 50;
  ai_line_points[0].first_pos_y = 150;
  ai_line_points[0].second_pos_x = 50;
  ai_line_points[0].second_pos_y = 150;
//   APP_LOG(1, "%i", human_line_points[0].first_pos_x);
  my_window = window_create();
  
  Layer *window_layer = window_get_root_layer(my_window);
  GRect bounds = layer_get_frame(window_layer);
  
  canvas = layer_create(bounds);
  layer_set_update_proc(canvas, canvas_render);
  layer_add_child(window_layer, canvas);
//   APP_LOG(1, "Test");

  text_layer = text_layer_create(GRect(0, 70, bounds.size.w, 16));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(text_layer, GColorClear);
//   text_layer_set_size(text_layer,);
  window_set_click_config_provider(my_window, click_config_provider);
  window_stack_push(my_window, true);
}

void handle_deinit(void) {
  text_layer_destroy(text_layer);
  layer_destroy(canvas);
  inverter_layer_destroy(inversion_layer);
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
