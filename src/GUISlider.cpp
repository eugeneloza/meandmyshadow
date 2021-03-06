/*
 * Copyright (C) 2012 Me and My Shadow
 *
 * This file is part of Me and My Shadow.
 *
 * Me and My Shadow is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Me and My Shadow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Me and My Shadow.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Functions.h"
#include "GUISlider.h"
using namespace std;

void GUISlider::calcPos(){
	//Floats ...
	float f,f2;
	
	//The value can't be below the minimum value or above the maximum.
	if(value<minValue)
		value=minValue;
	else if(value>maxValue)
		value=maxValue;
	
	//
	f=(float)(left+1);
	f2=(float)(width-2);

	if(largeChange<=0) f2=-1;
	if(f2>0){
		float f1=0.0f;
		valuePerPixel = (maxValue - minValue + largeChange) / f2;
		if(valuePerPixel > 0.0001f) f1 = largeChange / valuePerPixel;
		if(f1 < 4 && f2 > 4){
			valuePerPixel = (maxValue - minValue) / (f2 - 4);
			f1 = 4;
		}
		thumbStart = f + (value - minValue) / valuePerPixel;
		thumbEnd = thumbStart + f1;
	}else{
		valuePerPixel = -1;
		thumbStart = f;
		thumbEnd = f - 1;
	}
}

bool GUISlider::handleEvents(SDL_Renderer&,int x,int y,bool enabled,bool visible,bool processed){
	//Boolean if the event is processed.
	bool b=processed;
	
	//The GUIObject is only enabled when he and his parent are enabled.
	enabled=enabled && this->enabled;
	//The GUIObject is only enabled when he and his parent are enabled.
	visible=visible && this->visible;
	
	//Check enabled and visible.
	if (!(enabled && visible)) {
		state = 0;
	} else if (isKeyboardOnly) {
		//Do nothing on keyboard only mode.
	} else {
		//Check if the mouse button is released.
		if (event.type == SDL_MOUSEBUTTONUP){
			//It so we have lost any focus at all.
			state = 0;
		} else if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN){
			//The mouse button is down and it's moving
			int i, j, k, f, f1;
			state &= ~0xFF;
			k = SDL_GetMouseState(&i, &j);
			i -= x;
			j -= y;
			bool bInControl_0;

			f = left;
			f1 = f + width;
			bInControl_0 = (j >= top&&j < top + height);

			//===
			if ((state & 0x0000FF00) == 0x300 && (k&SDL_BUTTON(1)) && event.type == SDL_MOUSEMOTION && valuePerPixel>0){
				//drag thumb
				state |= 3;
				int val = criticalValue + (int)(((float)i - startDragPos) * valuePerPixel + 0.5f);
				if (val<minValue) val = minValue;
				else if (val>maxValue) val = maxValue;
				if (value != val){
					value = val;
					changed = true;
				}
				b = true;
			} else if (bInControl_0){
				if (i < f){ //do nothing
				} else if (valuePerPixel <= 0){ //do nothing
				} else if (i < (int)thumbStart){ //-largechange
					state = (state&~0xFF) | 2;
					if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) state = (state&~0x0000FF00) | ((state & 0xFF) << 8);
					else if ((state & 0x0000FF00) && ((state & 0xFF) != ((state >> 8) & 0xFF))) state &= ~0xFF;
					if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT){
						int val = value - largeChange;
						if (val < minValue) val = minValue;
						if (value != val){
							value = val;
							changed = true;
						}
						timer = 8;
					}
					if (state & 0xFF) criticalValue = minValue + (int)(float(i - f) * valuePerPixel + 0.5f);
					b = true;
				} else if (i < (int)thumbEnd){ //start drag
					state = (state&~0xFF) | 3;
					if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) state = (state&~0x0000FF00) | ((state & 0xFF) << 8);
					else if ((state & 0x0000FF00) && ((state & 0xFF) != ((state >> 8) & 0xFF))) state &= ~0xFF;
					if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT){
						criticalValue = value;
						startDragPos = (float)i;
					}
					b = true;
				} else if (i < f1){ //+largechange
					state = (state&~0xFF) | 4;
					if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) state = (state&~0x0000FF00) | ((state & 0xFF) << 8);
					else if ((state & 0x0000FF00) && ((state & 0xFF) != ((state >> 8) & 0xFF))) state &= ~0xFF;
					if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT){
						int val = value + largeChange;
						if (val > maxValue) val = maxValue;
						if (value != val){
							value = val;
							changed = true;
						}
						timer = 8;
					}
					if (state & 0xFF) criticalValue = minValue - largeChange + (int)(float(i - f) * valuePerPixel + 0.5f);
					b = true;
				}
			}
		}
	}
	
	return b;
}

void GUISlider::renderScrollBarButton(SDL_Renderer &renderer, int index, int x1, int y1, int x2, int y2, int srcleft, int srctop){
	//Make sure the button isn't inverted.
	if(x2<=x1||y2<=y1)
		return;

	//The color.
	int clr=-1;
	//Rectangle the size of the button.
	SDL_Rect r={x1,y1,x2-x1,y2-y1};
	
	//Check 
	if((state&0xFF)==index){
		if(((state>>8)&0xFF)==index){
			//Set the color gray.
			clr=0xDDDDDDFF;
		}else{
			//Set the color to lightgray.
			clr=0xFFFFFFFF;
		}
	}
	
	//Draw a box.
    drawGUIBox(r.x,r.y,r.w,r.h,renderer,clr);
	
	//Boolean if there should be an image on the button.
    const bool b = (x2-x1>=14);
	
	//Check if the image can be drawn.
	if(b&&srcleft>=0&&srctop>=0){
		//It can thus draw it.
        const SDL_Rect srcRect={srcleft,srctop,16,16};
        const SDL_Rect dstRect={(x1+x2)/2-8, (y1+y2)/2-8, srcRect.w, srcRect.h};

        SDL_RenderCopy(&renderer, bmGuiTex.get(), &srcRect, &dstRect);
	}
}

void GUISlider::render(SDL_Renderer &renderer, int x, int y, bool draw){
	//There's no use in rendering the scrollbar when invisible.
	if(!visible||!draw)
		return;

	//Draw highlight on keyboard only mode.
	if (isKeyboardOnly && state) {
		drawGUIBox(x + left, y + top, width, height, renderer, 0xFFFFFF40);
	}

	//Check if the scrollbar is enabled.
	if(enabled){
		//Check if the state is right.
		if((state&0xFF)==((state>>8)&0xFF)){
			//Switch the state (change)/
			switch(state&0xFF){
			case 2:
				//It's a lager negative change.
				//Check if it's time.
				if((--timer)<=0){
					if(value<criticalValue)
						state&=~0xFF;
					else{
						//Reduce the value.
						int val=value-largeChange;
						
						//Make sure it doesn't go too low.
						if(val<minValue)
							val=minValue;
						if(value!=val){
							value=val;
							changed=true;
						}
						
						//Set the time to two.
						timer=2;
					}
				}
				break;
			case 4:
				//It's a lager positive change.
				//Check if it's time.
				if((--timer)<=0){
					if(value>criticalValue)
						state&=~0xFF;
					else{
						//Increase the value.
						int val=value+largeChange;
						
						//Make sure it doesn't go too high.
						if(val>maxValue)
							val=maxValue;
						if(value!=val){
							value=val;
							changed=true;
						}
						
						//Set the time to two.
						timer=2;
					}
				}
				break;
			}
		}
	}
	
	//If the scrollbar changed then invoke a GUIEvent.
	if(changed){
		if(eventCallback){
			GUIEvent e={eventCallback,name,this,GUIEventChange};
			GUIEventQueue.push_back(e);
		}
		changed=false;
	}
	
	//We calculate the position since it could have changed.
	calcPos();
	
	//Now the actual drawing begins.
	if(valuePerPixel>0){
		//Draw the line the slider moves along.
        drawGUIBox(x+left,y+top+(height-4)/2,width,4,renderer,0);
        renderScrollBarButton(renderer,3,x-1+(int)thumbStart,y+top+(height/4),x+1+(int)thumbEnd,y+top+(height/4)*3,16,16);
	}else{
		//There are two buttons so draw them.
		int f=left+width/2;
        renderScrollBarButton(renderer,1,x+left,y+top,x+1+f,y+top+height,48,0);
        renderScrollBarButton(renderer,5,x+f,y+top,x+left+width,y+top+height,64,0);
	}
}
