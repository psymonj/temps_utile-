/*
*
*     deal w/ encoders + buttons 
*
*
*/

uint32_t LAST_UI;     // timestamp/timeout for screen saver

// buttons:
uint32_t button_timestamp; 
uint16_t button_states[] = {0,0}, button_events[] ={0,0};
const uint16_t LONGPRESSED = 800;

enum _button_states {
  READY,
  PRESSED,
  SHORT,
  HOLD,
  DONE
  
};

// encoders:

void left_encoder_ISR() {
  encoder[LEFT].process();
}

void right_encoder_ISR() {
  encoder[RIGHT].process();
}

/* ----------------------------------------------------------------  */

void update_ENC()  {
  
 
  if (UI_MODE == _BPM) {
    
    if (encoder[RIGHT].change())  { 
        
          int16_t right_encoder_data = encoder[RIGHT].pos();
          int16_t bpm_max = BPM_MAX[BPM_SEL];
          
          if      (right_encoder_data < BPM_MIN)    { BPM = BPM_MIN;  encoder[RIGHT].setPos(BPM_MIN); }
          else if (right_encoder_data > bpm_max)    { BPM = bpm_max;  encoder[RIGHT].setPos(bpm_max); }
          else BPM = right_encoder_data; 

          bpm_set_microseconds();

          MENU_REDRAW = 1;   
          LAST_UI = millis();    
    }
    
    if (encoder[LEFT].change()) {
              
           int16_t left_encoder_data = encoder[LEFT].pos();
      
           if (left_encoder_data > _16TH)      { BPM_SEL = _16TH; encoder[LEFT].setPos(BPM_SEL); }
           else if (left_encoder_data < _4TH)  { BPM_SEL = _4TH;  encoder[LEFT].setPos(BPM_SEL); } 
           else BPM_SEL = left_encoder_data;
           
           if (BPM > BPM_MAX[BPM_SEL]) BPM_SEL--;
           
           bpm_set_microseconds();

           MENU_REDRAW = 1;   
           LAST_UI = millis(); 
    }  
  } 
  
  else if (UI_MODE == _MAIN) {
    
          if (encoder[RIGHT].change())  {
                
             uint16_t _channel = ACTIVE_CHANNEL;
             uint16_t _menu_item = ACTIVE_MENU_ITEM-2;  // offset menu
             int16_t right_encoder_data = encoder[RIGHT].pos();   
             LAST_UI = millis();
      
             // update parameter:
             if (right_encoder_data!= allChannels[_channel].param[ACTIVE_MODE][_menu_item]) {
         
                   int16_t _min, _max; 
                   // param limits + deal with exceptions / interdependent parameters.
                   switch(ACTIVE_MODE) {
                     
                       case LFSR: {
                         
                           switch(_menu_item) {
                             
                             case 0: { // pw 
                               _min = allChannels[_channel].param_min[_menu_item];
                               _max = allChannels[_channel].param_max[_menu_item]; 
                               break;
                             }
                             case 1: { // length
                               int16_t _min1, _min2;  
                               _min1 = allChannels[_channel].param[ACTIVE_MODE][2]; // tap1
                               _min2 = allChannels[_channel].param[ACTIVE_MODE][3]; // tap2
                               _min = _min1 > _min2 ? _min1 : _min2; // LFSR length > tap1, tap2
                               _max = allChannels[_channel].param_max[_menu_item]; 
                               break;
                             }
                             default: { // taps 
                                _min = allChannels[_channel].param_min[_menu_item];
                                _max = allChannels[_channel].param[ACTIVE_MODE][1]; // tap < LFSR length
                                break;  
                             } 
                          }
                          break;
                       }
                       
                       case EUCLID: {
                         
                          switch(_menu_item) {
                             
                             case 0: { // pw 
                               _min = allChannels[_channel].param_min[_menu_item];
                               _max = allChannels[_channel].param_max[_menu_item]; 
                               break;
                             }
                             case 1: { // N
                               _min = allChannels[_channel].param[ACTIVE_MODE][2]; // N > K
                               _min = _min < 2 ? 2 : _min; // N > 2 
                               _max = allChannels[_channel].param_max[_menu_item]; 
                               break;
                             }
                             default: { // K, offset
                                _min = allChannels[_channel].param_min[_menu_item];
                                _max = allChannels[_channel].param[ACTIVE_MODE][1]; //  K < N 
                                break;
                             } 
                          }
                          break;
                       }
                       
                       default:  
                         _min = allChannels[_channel].param_min[_menu_item];
                         _max = allChannels[_channel].param_max[_menu_item]; 
                         break;
                   }
                   
                   // update params: 
                   if      (right_encoder_data < _min)  { allChannels[_channel].param[ACTIVE_MODE][_menu_item] = _min; encoder[RIGHT].setPos(_min);} 
                   else if (right_encoder_data > _max)  { allChannels[_channel].param[ACTIVE_MODE][_menu_item] = _max; encoder[RIGHT].setPos(_max); } 
                   else                                 { allChannels[_channel].param[ACTIVE_MODE][_menu_item] = right_encoder_data; } 
                   
                   MENU_REDRAW = 1;
         
         }
      }
      
         // mode select:    
         if (encoder[LEFT].change())  {
       
              uint8_t _channel = ACTIVE_CHANNEL;
              int16_t left_encoder_data  =  encoder[LEFT].pos();   
              LAST_UI = millis();
         
              if (left_encoder_data!= allChannels[_channel].mode) {
                  
                  uint16_t mode_limit = allChannels[_channel].channel_modes-1;   
                  if      (left_encoder_data > mode_limit)  { MODE_SELECTOR = 0; encoder[LEFT].setPos(0);}
                  else if (left_encoder_data < 0) { MODE_SELECTOR = mode_limit;  encoder[LEFT].setPos(mode_limit);} 
                  else  { MODE_SELECTOR = left_encoder_data;}
                  MENU_REDRAW = 1;
         }
         else { MODE_SELECTOR = left_encoder_data; }
    } 
  }
  
  else if (UI_MODE == _CV) {
    
         uint16_t _sel  = CV_MENU_ITEM;
         
         if (encoder[RIGHT].change()) { // select dest. param
          
                 int16_t right_encoder_data = encoder[RIGHT].pos();
                 
                 if (_sel < numADC) {
                   
                           uint16_t _channel = CV_DEST_CHANNEL[_sel];
                           uint16_t _prev_dest = CV_DEST_PARAM[_sel];
                           if (_channel) {  // channel selected ?
                                      uint16_t _limit   = allChannels[_channel-1].mode_param_numbers + 1; // + 1 because we need ' - '
                                      if  (right_encoder_data > _limit)  { right_encoder_data = 0; encoder[RIGHT].setPos(0);}
                                      else if (right_encoder_data < 0) { right_encoder_data = _limit;  encoder[RIGHT].setPos(_limit);}
                                      
                                      CV_DEST_PARAM[_sel] = right_encoder_data;                   // update display: selected param
                                      
                                      uint16_t free_param_slot;
                                      if (right_encoder_data > _prev_dest) free_param_slot = find_slot(&allChannels[_channel-1], right_encoder_data, _limit);
                                      else free_param_slot = find_prev_slot(&allChannels[_channel-1], right_encoder_data, _limit);
                                      if (free_param_slot) { 
                                                             allChannels[_channel-1].cvmod[free_param_slot] = _sel+1;
                                                             allChannels[_channel-1].cvmod[_prev_dest] = 0;              // clear CV channel
                                                             CV_DEST_PARAM[_sel] = free_param_slot;
                                                             encoder[RIGHT].setPos(free_param_slot);
                                       }
                                       else  { 
                                                             CV_DEST_PARAM[_sel] = 0;
                                                             allChannels[_channel-1].cvmod[_prev_dest] = 0;
                                                             encoder[RIGHT].setPos(0);
                                             }
                           }
                           else CV_DEST_PARAM[_sel] = 0;   
                 }
                 else if (_sel == numADC) { // select clock source
                           if  (right_encoder_data > 3)  { right_encoder_data = 0; encoder[RIGHT].setPos(0);}
                           else if (right_encoder_data < 0) { right_encoder_data = 3;  encoder[RIGHT].setPos(3);} 
                           CV_DEST_CHANNEL[_sel] = right_encoder_data;
                           right_encoder_data = right_encoder_data < 2 ? 0 : 1;
                           CLK_SRC = right_encoder_data;
                 }
                 MENU_REDRAW = 1;
                 LAST_UI = millis(); 
         }
         
         if (_sel < numADC && encoder[LEFT].change()) { // select dest. channel
         
                 int16_t left_encoder_data = encoder[LEFT].pos();
                 uint16_t _param_val = CV_DEST_PARAM[_sel];  // 0,1,2,3,4
                 uint16_t _prev_dest = CV_DEST_CHANNEL[_sel];
                 
                 if  (left_encoder_data > CHANNELS)  { left_encoder_data = 0; encoder[LEFT].setPos(0);}
                 else if (left_encoder_data < 0) { left_encoder_data = CHANNELS;  encoder[LEFT].setPos(CHANNELS);} 
                 CV_DEST_CHANNEL[_sel] = left_encoder_data; 
                 if (left_encoder_data) { // channel selected?
                                /*
                                if (_param_val > allChannels[left_encoder_data-1].mode_param_numbers + 1) {  // out of bounds? reset param value
                                          if (_prev_dest) allChannels[_prev_dest-1].cvmod[_param_val] = 0;
                                          _param_val = CV_DEST_PARAM[_sel] = 0;
                                          encoder[RIGHT].setPos(0);
                                 }         
                                uint8_t free_param_slot = allChannels[left_encoder_data-1].cvmod[_param_val];
                                if (!free_param_slot) {
                                      if (_param_val) allChannels[left_encoder_data-1].cvmod[_param_val] = _sel+1;      // assign parameter
                                      if (_prev_dest) allChannels[_prev_dest-1].cvmod[_param_val] = 0;  // clear parameter 
                                }
                                
                                else {
                                      if (_prev_dest) allChannels[_prev_dest-1].cvmod[_param_val] = 0;  // clear parameter
                                      CV_DEST_PARAM[_sel] = 0;
                                      encoder[RIGHT].setPos(0);    
                                }
                                */
                                if (_prev_dest) allChannels[_prev_dest-1].cvmod[_param_val] = 0;  // clear parameter
                                CV_DEST_PARAM[_sel] = 0;
                                encoder[RIGHT].setPos(0);  
                 }
                 else { // clear assigned CV
                                 allChannels[_prev_dest-1].cvmod[_param_val] = 0; 
                                 CV_DEST_PARAM[_sel] = 0;
                 }
                     
                 MENU_REDRAW = 1; 
                 LAST_UI = millis();
         }
    }  
}

/* ------------------------- buttons -------------------------------------  */

void rightButton() { // set parameter: 
   
  if (!digitalReadFast(butR) && millis() - LAST_BUT > DEBOUNCE) {
     
      if (UI_MODE==_MAIN) update_channel_params();  
      else if (UI_MODE==_BPM || UI_MODE==_SCREENSAVER) {   // show menu 
      
         ACTIVE_MODE = allChannels[ACTIVE_CHANNEL].mode;
         encoder[RIGHT].setPos(allChannels[ACTIVE_CHANNEL].param[ACTIVE_MODE][ACTIVE_MENU_ITEM - 2]);
         UI_MODE = _MAIN;
      }
      else if (UI_MODE==_CV)  next_CV_menu_item();
      LAST_BUT = LAST_UI = millis();
  }
} 

void leftButton() { // set mode:
  
  if (!digitalReadFast(butL) && millis() - LAST_BUT > DEBOUNCE) {
    
      if (UI_MODE==_MAIN) {
           
            if (ACTIVE_MODE != _DAC) allChannels[ACTIVE_CHANNEL].param[MODE_SELECTOR][0] = allChannels[ACTIVE_CHANNEL].param[ACTIVE_MODE][0]; // keep pw
            if (MODE_SELECTOR == _DAC) allChannels[ACTIVE_CHANNEL].param[MODE_SELECTOR][0] = 0;
            update_channel_mode(&allChannels[ACTIVE_CHANNEL], MODE_SELECTOR);
      }
      else if (UI_MODE==_BPM || UI_MODE==_SCREENSAVER) {    // show menu 
            ACTIVE_MODE = allChannels[ACTIVE_CHANNEL].mode;
            encoder[RIGHT].setPos(allChannels[ACTIVE_CHANNEL].param[ACTIVE_MODE][ACTIVE_MENU_ITEM-2]);
            UI_MODE = _MAIN;
      }
      else if (UI_MODE==_CV) prev_CV_menu_item();
      LAST_BUT = LAST_UI = millis();
  }
}

void topButton()   {
  
     if (!digitalReadFast(but_top) && millis() - LAST_BUT > DEBOUNCE) {
 
         if (!button_states[TOP]) { // ready ?
           
              button_states[TOP] = PRESSED;
              button_timestamp = millis(); 
         }
    }  
}

void lowerButton() {

      if (!digitalReadFast(but_bot) && millis() - LAST_BUT > DEBOUNCE) {
        
          if (!button_states[BOTTOM]) { // ready ?
          
              button_states[BOTTOM] = PRESSED;
              button_timestamp = millis(); 
          }
     }  
}

void checkbuttons(uint16_t _button) {
       
     uint16_t _b, _pin_state, _b_state, _b_event;
     _b  = _button; 
     _b_state = button_states[_b];
     _b_event = button_events[_b]; 
     _pin_state = _b ? digitalReadFast(but_bot) : digitalReadFast (but_top);
     
     if (_pin_state && _b_state == PRESSED)   button_events[_b] = SHORT; // button released
     else if (_pin_state && _b_event == DONE) button_states[_b] = READY; // button released 
     else if (_b_state == PRESSED && millis() - button_timestamp > LONGPRESSED)   button_events[_b] = HOLD;    
     
     if (_b_event == SHORT) {
  
          _b ? channel_select(BOTTOM) : channel_select(TOP);
          LAST_BUT = LAST_UI = millis();
          button_events[_b] = button_states[_b] = DONE;
          MENU_REDRAW = 1;
     }
     
     else if (_b_event == HOLD) {
          
          if (_b)  { // bottom button -> BPM page
                UI_MODE = _BPM;
                encoder[RIGHT].setPos(BPM);
          }
          else {    // top button -> CV page
                UI_MODE = _CV;
                uint16_t _tmp = CV_MENU_ITEM;
                encoder[RIGHT].setPos(CV_DEST_PARAM[_tmp]);
                encoder[LEFT].setPos(CV_DEST_CHANNEL[_tmp]);
                
          }  
          MENU_REDRAW = 1;
          LAST_BUT = LAST_UI = millis();
          button_events[_b] = button_states[_b] = DONE;     
     }  
}

void channel_select(uint16_t _select) {
  
   if (UI_MODE==_MAIN) { 
     
          if (_select) { 
                  ACTIVE_CHANNEL--;
                  ACTIVE_CHANNEL = (ACTIVE_CHANNEL < 0) ? (CHANNELS-1) : ACTIVE_CHANNEL;
          }
          else {
                  ACTIVE_CHANNEL++;
                  ACTIVE_CHANNEL = (ACTIVE_CHANNEL >= CHANNELS) ? 0 : ACTIVE_CHANNEL;
          }
          ACTIVE_MODE = allChannels[ACTIVE_CHANNEL].mode;
          MODE_SELECTOR = ACTIVE_MODE;
          ACTIVE_MENU_ITEM = 2;
          encoder[RIGHT].setPos(allChannels[ACTIVE_CHANNEL].param[ACTIVE_MODE][ACTIVE_MENU_ITEM-2]);
          encoder[LEFT].setPos(ACTIVE_MODE);
   }
       
   else { // show menu
          // ACTIVE_MENU_ITEM = 2;
             ACTIVE_MODE = allChannels[ACTIVE_CHANNEL].mode;
             encoder[RIGHT].setPos(allChannels[ACTIVE_CHANNEL].param[ACTIVE_MODE][ACTIVE_MENU_ITEM-2]);
   }  
     
   LAST_BUT = LAST_UI = millis();
   UI_MODE = _MAIN;  
}
 
/* -----------------------------------------------------  */   

uint8_t find_slot(struct params* _p, uint16_t _param, uint16_t _limit) {
    
        int16_t _slot, _x = _param;
        _x = (_x > _p->mode_param_numbers+1) ? 0 : _x;
        if (!_x) _slot = 0;
        else if (!_limit) _slot = 0;                   // nope
        else if (!_p->cvmod[_x]) _slot = _x;           // slot ok?
        else   _slot = find_slot(_p, _x+1, _limit-1);  // try again
  
        return _slot;
}  

uint8_t find_prev_slot(struct params* _p, int16_t _param, uint16_t _limit) {
    
        int16_t _slot, _x = _param;
        _x = (_x < 0 ) ? _p->mode_param_numbers+1 : _x;
        if (!_x) _slot = 0;
        else if (!_limit) _slot = 0;                   // nope
        else if (!_p->cvmod[_x]) _slot = _x;           // slot ok?
        else   _slot = find_slot(_p, _x-1, _limit-1);  // try again
       
        return _slot;
}  
   
void init_menu(void) {

      ACTIVE_MODE = allChannels[ACTIVE_CHANNEL].mode;
      encoder[RIGHT].setPos(allChannels[ACTIVE_CHANNEL].param[ACTIVE_MODE][ACTIVE_MENU_ITEM-2]);
      MENU_REDRAW = 1;  
      MODE_SELECTOR = ACTIVE_MODE; 
      encoder[LEFT].setPos(ACTIVE_MODE);      
}

void update_channel_params(void) {
  
      ACTIVE_MENU_ITEM++;
      if ( ACTIVE_MENU_ITEM >= allChannels[ACTIVE_CHANNEL].mode_param_numbers+3) ACTIVE_MENU_ITEM = 2;
      
      uint16_t _param_val = allChannels[ACTIVE_CHANNEL].param[ACTIVE_MODE][ACTIVE_MENU_ITEM-2];
      encoder[RIGHT].setPos(_param_val);
      
      MENU_REDRAW = 1;      
}

void update_channel_mode(struct params* _p, uint16_t _mode) {
  
      channel_set_mode(_p,_mode);
      // update menu
      encoder[LEFT].setPos(_mode);
      encoder[RIGHT].setPos(_p->param[_mode][0]);
      if (_mode == DIV) sync();
      /* clear CV */
      _p->cvmod[1] = 0;
      _p->cvmod[2] = 0;
      _p->cvmod[3] = 0;
      _p->cvmod[4] = 0;
      uint16_t _channel = ACTIVE_CHANNEL+1;
      if (CV_DEST_CHANNEL[0] == _channel) { CV_DEST_CHANNEL[0] = CV_DEST_PARAM[0] = 0; }
      if (CV_DEST_CHANNEL[1] == _channel) { CV_DEST_CHANNEL[1] = CV_DEST_PARAM[1] = 0; }
      if (CV_DEST_CHANNEL[2] == _channel) { CV_DEST_CHANNEL[2] = CV_DEST_PARAM[2] = 0; }
      if (CV_DEST_CHANNEL[3] == _channel) { CV_DEST_CHANNEL[3] = CV_DEST_PARAM[3] = 0; }
      ACTIVE_MODE = _mode;
      ACTIVE_MENU_ITEM = 2;
      MENU_REDRAW = 1;   
}  

void next_CV_menu_item() {
  
         CV_MENU_ITEM++;
         CV_MENU_ITEM = CV_MENU_ITEM > MAIN_ITEMS-1 ? 0 : CV_MENU_ITEM; 
         encoder[LEFT].setPos(CV_DEST_CHANNEL[CV_MENU_ITEM]);
         encoder[RIGHT].setPos(CV_DEST_PARAM[CV_MENU_ITEM]);
         MENU_REDRAW = 1;
}


void prev_CV_menu_item() {
  
         CV_MENU_ITEM--;
         CV_MENU_ITEM = CV_MENU_ITEM < 0 ? MAIN_ITEMS-1 : CV_MENU_ITEM; 
         encoder[LEFT].setPos(CV_DEST_CHANNEL[CV_MENU_ITEM]);
         encoder[RIGHT].setPos(CV_DEST_PARAM[CV_MENU_ITEM]);
         MENU_REDRAW = 1;
}

void time_out() {
  
      uint16_t _param_val, _mode;
      _mode = allChannels[ACTIVE_CHANNEL].mode;
      _param_val = allChannels[ACTIVE_CHANNEL].param[ACTIVE_MODE][ACTIVE_MENU_ITEM-2];
      encoder[LEFT].setPos(_mode);
      encoder[RIGHT].setPos(_param_val);
      UI_MODE = 0;
      MENU_REDRAW = 1;
}  

/*   -----------------------------------------  */

void sync() {
 
   allChannels[0].param[DIV][3] = 0;
   allChannels[1].param[DIV][3] = 0;
   allChannels[2].param[DIV][3] = 0;
   allChannels[3].param[DIV][3] = 0;
   allChannels[4].param[DIV][3] = 0;
   allChannels[5].param[DIV][3] = 0;
}  

void printstuff() {
  
  for (int i = 0; i < 6; i ++) {
  Serial.print(allChannels[i].cvmod[0]); 
  Serial.print(" - "); 
  Serial.print(allChannels[i].cvmod[1]);
  Serial.print(" - "); 
  Serial.print(allChannels[i].cvmod[2]); 
  Serial.print(" - "); 
  Serial.print(allChannels[i].cvmod[3]);
  Serial.print(" - "); 
  Serial.print(allChannels[i].cvmod[4]);
  Serial.print(" | "); 
  }
  Serial.println(" ");
}

void printADC() {
  
  for (int i = 0; i <= numADC; i ++) {
  Serial.print(CV[i]); 
  Serial.print(" | "); 
  }
  Serial.println(" ");
}
