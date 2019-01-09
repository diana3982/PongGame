#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "buzzer.h"

#define GREEN_LED BIT6

short point = 1;
char p1Score ='0';
char p2Score = '0';
u_int p1_count = 0;
u_int p2_count = 0;

AbRect paddle = {abRectGetBounds, abRectCheck, {15,3}};

AbRectOutline fieldOutline = {
abRectOutlineGetBounds, abRectOutlineCheck,
{screenWidth/2 - 2, screenHeight/2 - 6}
};

Layer fieldLayer = { /* border outline */
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2 - 3},
  {0,0}, {0,0},
  COLOR_WHITE,
  0
};

Layer ballLayer = {
  (AbShape *)&circle5,
  {(screenWidth/2), (screenHeight/2)}, /* will center the ball*/
  {0,0}, {0,0}, COLOR_CYAN, &fieldLayer,
};

Layer p1Layer = { /* bottom paddle */
  (AbShape *)&paddle,
  {(screenWidth/2), (screenHeight/2)+64},
  {0,0}, {52,144}, 
  COLOR_MAGENTA, &ballLayer,
};

Layer p2Layer ={ /* top paddle */
(AbShape *)&paddle,
{(screenWidth/2), (screenHeight/2)-70},
  {0,0}, {52,10},
COLOR_GREEN,&p1Layer,
};

typedef struct MovLayer_s{
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

MovLayer m1 = {&ballLayer, {5,5}, 0};
MovLayer m2 = {&p1Layer, {5,5}, 0};
MovLayer m3 = {&p2Layer, {5,5}, 0};

/* the following was retrieved from shapemotiondemo "shapemotion.c" */
void movLayerDraw(MovLayer *movLayers, Layer *layers){
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);

  for(movLayer = movLayers; movLayer; movLayer = movLayer->next){
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
   }

  or_sr(8);

  for(movLayer = movLayers; movLayer; movLayer = movLayer->next){ /* for each moving layer*/
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0],
		bounds.topLeft.axes[1],
		bounds.botRight.axes[0],
		bounds.botRight.axes[1]);
    for(row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++){
      for(col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++){
	Vec2 pixelPos ={col, row};
	u_int color = bgColor;
	Layer *probeLayer;

	for(probeLayer = layers; probeLayer; probeLayer = probeLayer -> next){
	  if(abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)){
	    color = probeLayer->color;
	    break;
	  }/* end if */
	}/* end probeLayer for */
	lcd_writeColor(color);
      }/* end col for loop */
    }/* end row for loop */
  }/* for moving layer being updated */
}

Region fence = {{0,LONG_EDGE_PIXELS}, {SHORT_EDGE_PIXELS, LONG_EDGE_PIXELS}}; /* Creates fence region */

void mlAdvance(MovLayer *ml,MovLayer *m1, MovLayer *m2,  Region *fence){
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  
  for(; ml; ml = ml->next){
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for(axis = 0; axis < 2; axis++){
      if((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	 (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis])){
	int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }	/**< if outside of fence */

      if((ml->layer->posNext.axes[1] >= 134) &&
	 (ml->layer->posNext.axes[0] <= m1->layer->posNext.axes[0]+18 &&
	  ml->layer->posNext.axes[0] >= m1->layer->posNext.axes[0] -18))
	{/*bottom player 1*/
        int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
        m1->layer->color = COLOR_MAGENTA;
        m2->layer->color = COLOR_GREEN;
	ml->layer->color = COLOR_CYAN;
        ml->velocity.axes[0]+= 1;
        newPos.axes[axis] += (2*velocity);
        
        int redrawScreen = 1;
      }/* ends if*/
      else if((ml->layer->posNext.axes[1] <= 21) &&
	      (ml->layer->posNext.axes[0] <=
	       m2->layer->posNext.axes[0] +18 &&
	       ml->layer->posNext.axes[0] >=
	       m2->layer->posNext.axes[0] - 18))
	{/* top player 2*/
        int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
        m2->layer->color = COLOR_GREEN;
	m1->layer->color = COLOR_MAGENTA;
        ml->layer->color = COLOR_CYAN;
        ml->velocity.axes[0] += 1;
        newPos.axes[axis] += (2*velocity);
        
        int redrawScreen = 1;
      }/* end else if */
      else if((ml->layer->posNext.axes[1] == 20))
      { /* upper bound */
        m2->layer->color = COLOR_GREEN;
        p1Score ++;
	p1_count++;
        drawChar5x7(52,152,p1Score, COLOR_WHITE, COLOR_BLACK);
        newPos.axes[0] = screenWidth/2;
        newPos.axes[1] = (screenHeight/2);
        point = 1;
        ml->velocity.axes[0] = 5;
        ml->layer->posNext = newPos;
	
        int redrawScreen = 1;
      }/* ends upper bound check */
      else if((ml->layer->posNext.axes[1] == 135))
      { /* lower bounds */
	m1->layer->color = COLOR_MAGENTA;
        p2Score++;
	p2_count++;
        drawChar5x7(120, 152, p2Score, COLOR_WHITE, COLOR_BLACK);
        newPos.axes[0] = screenWidth/2;
        newPos.axes[1] = (screenHeight/2);
        point = 1;
        ml->velocity.axes[0] = 5;
        ml->layer->posNext = newPos;
	
        int redrawScreen = 1;
       }/* ends lower bounds check */
      int redrawScreen = 1;
      if(point != 1) /* no score  */
      {
	ml->layer->posNext = newPos;	
      }
    } /**< for axis */
  } /**< for ml */
}


u_int bgColor = COLOR_BLACK;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */


/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{
  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(15);
  buzzer_init();

  layerInit(&ballLayer);
  layerDraw(&ballLayer);


  layerGetBounds(&fieldLayer, &fieldFence);


  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */

  drawString5x7(3,152,"Player1: ", COLOR_MAGENTA, COLOR_BLACK);
  drawString5x7(72, 152, "Player2: ", COLOR_GREEN, COLOR_BLACK);
  drawChar5x7(52,152, p1Score, COLOR_WHITE, COLOR_BLACK);
  drawChar5x7(120,152, p2Score, COLOR_WHITE, COLOR_BLACK);

  for(;;) { 
    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
      or_sr(0x10);	      /**< CPU OFF */
    }
    P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;
    
    movLayerDraw(&m1, &ballLayer);
    movLayerDraw(&m2, &p1Layer);
    movLayerDraw(&m3, &p2Layer);
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  /* when someone score 5 points there is a winner*/
  if(p1_count == 5 || p2_count == 5){
    bgColor = COLOR_BLACK;
    if(p1_count == 5){
      winner(0);
      layerDraw(&p1Layer);
      drawString5x7(screenWidth/2 - 38, screenHeight/2, "PLAYER 1 WINS!", COLOR_MAGENTA, COLOR_BLACK);
    }
    else{
      winner(1);
      layerDraw(&p2Layer);
      drawString5x7(screenWidth/2 - 38, screenHeight/2, "PLAYER 2 WINS!", COLOR_GREEN, COLOR_BLACK);
    }
  }

  
  if (count == 20) {
    mlAdvance(&m1, &m2, &m3, &fieldFence);
    u_int switches = p2sw_read();

    /*player 1 (bottom)*/
    if(!(switches & (1 << 1))){
      if(m2.layer->posNext.axes[0] <= 102)
	{
	  m2.layer->posNext.axes[0] += 5;
	  redrawScreen = 1;
	  point = 0;
	}
    }
    else if(!(switches & (1<<0))){
     if(m2.layer->posNext.axes[0] >= 27){
	m2.layer->posNext.axes[0] -= 5;
	redrawScreen = 1;
	point = 0;
      }
    }
    /*player 2 top */
    else if(!(switches & (1<<2))){
      if(m3.layer->posNext.axes[0] >= 26){
	m3.layer->posNext.axes[0] -= 5;
	redrawScreen = 1;
	point = 0;
      }
    }
    else if(!(switches & (1<<3))){
      if(m3.layer->posNext.axes[0] <= 102){
	  m3.layer->posNext.axes[0] += 5;
	  redrawScreen = 1;
	  point = 0;
	}
    }
    redrawScreen = 1;
    count = 0;
  }
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}
