/**
* @file	Assignment1App.cpp
* CSE 274 - Fall 2012
* My solution for HW01.
*
* @author RJ Marcus
* @date 2012-08-27
*
* @note This file is (c) 2012. It is licensed under the
* CC BY 3.0 license (http://creativecommons.org/licenses/by/3.0/),
* which means you are free to use, share, and remix it as long as you
* give attribution. Commercial uses are allowed.
*
* @note This project satisfies goals A.1 (rectangle), A.2 (circle), B.1 (blur), A.6 (tint), E.2 (transparency),
* E.5 (animation) and E.6 (mouse interaction), E.3 (rotation), A.4 (gradient)
*/

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/CinderMath.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Assignment1App : public AppBasic {
public:
	void setup();
	void mouseDown( MouseEvent event );	
	void mouseMove(MouseEvent event);

	/**
	* This method varies the speed at which the 'mist' particles 
	* move away from the cursor. 
	*
	* This satisfies the interactivity requirement (E.6)
	*/
	void mouseWheel(MouseEvent event);

	/**
	* This method helps toggle booleans that dictate which
	* methods are drawn on the screen (like tint, rectangle, rotation, blur)
	*/
	void keyDown(KeyEvent event);
	void update();
	void draw();

private:
	/**
	* Draw a circle on the screen based on sinusoidal waves that 
	* make it look like a ripple in a pond, extending outward
	* until the radius is equal to one fourth of the screen width or height.
	*
	* This satisfies the "circle" requirement (A.2), and "mouse interaction" (E.6)
	*/
	void draw_circle(int x, int y, int r, float transparency);

	/**
	* This method writes every pixel on the image to white, except for when
	* the green tint flag is toggled, in which case it writes every pixel as green.
	*
	*/
	void clearScreen();

	/**
	* Blur one image, using another as a template.
	*
	* Blur the image_to_blur using a standard convolution blur, but the strength of the blur depends on the blur_pattern.
	* Pixels are fulling blurred if they are black not blurred at all if they are white. Both images must be exactly the
	* same size as the app window.
	*
	* This method is a modified version of  Dr. Bo Brinkman's blur method. This instance only
	* blurs a specific rectangular area defined by x, y, width, height
	*
	* This satisfies the "blur" requirement, goal B.1
	*/
	void blur_area_edges(int x, int y , int width, int height);

	static const int kAppWidth = 800;
	static const int kAppHeight = 800;
	static const int kTextureSize = 1024;

	double PI_; // won't let me initialize even if it's a static const

	int mouse_X_;
	int mouse_Y_;
	float bg_color_r_;
	float bg_color_g_;
	float bg_color_b_;

	Surface* bg_Surface_; //background
	Surface* work_Surface_; //drawing

	struct mist_info{
		int x;
		int y;
		int width;
		int height;
		float r;
		float g;
		float b;
		float transparency;
	};

	struct circle_info{
		int x;
		int y;
		int radius;
		float transparency;
	};

	int mist_speed_;
	bool mist_on_;
	float rotation_rate_;
	bool tint_green_on_;
	bool blur_on_;
	void create_mist();
	int update_count_;
	Rand random_;
	uint8_t* my_blur_pattern_;

	/**
	* This method draws 'mist' particles which are multicolored rectangles with
	* transparency parameters.
	*
	* This satisfies the rectangle requirement (A.1), and transparency (E.2)
	*/
	void draw_mist(uint8_t* pixels, mist_info m);
	deque<mist_info> mist_list_;
	deque<circle_info> circle_list_;

};

void Assignment1App::setup()
{
	PI_ = 3.1415927;
	update_count_ = 0;

	random_.seed(419); //random number generator

	mist_speed_ = 1;
	mist_on_ = true;
	rotation_rate_ = 50.0; //controls period on mist rotation around mouse

	tint_green_on_ = false;

	bg_Surface_ = new Surface(kTextureSize,kTextureSize,false);
	clearScreen(); //fill screen with white pixels
	work_Surface_ = new Surface(kTextureSize,kTextureSize,false);

	//get data for blurring method:
	blur_on_ = false;
	uint8_t* blur_data = (*bg_Surface_).getData();	
	my_blur_pattern_ = new uint8_t[kAppWidth*kAppHeight*3];
	for(int y=0;y<kAppHeight;y++){
		for(int x=0;x<kAppWidth;x++){
			int offset = 3*(x + y*kAppWidth);
			my_blur_pattern_[offset] = blur_data[offset];
		}
	}

}

void Assignment1App::clearScreen(){

	uint8_t* pixels = (*bg_Surface_).getData();

	for(int y = 0; y < kAppHeight; y++){
		for(int x = 0; x < kAppWidth; x++){
			int offset = 3* (x + y*kTextureSize);
			if(tint_green_on_){ //TINT!
				pixels[offset] = 0;
				pixels[offset+1] = 255;
				pixels[offset+2] = 0;
			}
			else{
				pixels[offset] = 255;
				pixels[offset+1] = 255;
				pixels[offset+2] = 255;
			}
		}
	}
}

void Assignment1App::mouseMove(MouseEvent event){

	mouse_X_ = event.getX();
	mouse_Y_ = event.getY();

	//Here we get "background color" which is a color dependant 
	//on the position of the mouse on the background.
	//This fufills the gradient requirement (A.4)
	bg_color_r_ = 255 * mouse_X_ / ((float)kAppWidth);
	bg_color_g_ = 255 * sqrt(pow(mouse_X_,2.0) * pow(mouse_Y_,2.0)) /
		(float) sqrt(pow(kAppHeight,2.0) * pow(kAppWidth,2.0));
	bg_color_b_ = 255 * mouse_Y_ / ((float)kAppWidth);

}

void Assignment1App::mouseDown( MouseEvent event ){

	//create a circle:
	circle_info c;
	c.x = event.getX();
	c.y = event.getY();
	c.radius = 10;
	c.transparency = 1; //NOTE: this is a little bit different than the transparency for mist
	circle_list_.push_front(c);
}

void Assignment1App::keyDown(KeyEvent event){

	switch(event.getChar()){
	case 'm':
		mist_on_ = !mist_on_;
		break;
	case '[':
		if(rotation_rate_ > 55.0)
			rotation_rate_ -= 5.0;
		break;
	case ']':
		if(rotation_rate_ < 495.0)
			rotation_rate_ += 5.0;
		break;
	case 'g':
		tint_green_on_ = !tint_green_on_;
		clearScreen();
		break;
	case 'b':
		blur_on_ = !blur_on_;
		break;
	default:
		break;
	}
}

void Assignment1App::mouseWheel(MouseEvent event){
	int change = event.getWheelIncrement();

	if((mist_speed_ > -20 && change < 0) || (mist_speed_ < 20 && change > 0))
		mist_speed_ += change;
}

void Assignment1App::create_mist(){

	//create a mist packet
	mist_info m;
	m.x = mouse_X_;
	m.y = mouse_Y_;
	m.width = 5;
	m.height = 5;
	m.transparency = 0;
	m.r = bg_color_r_;
	m.g = bg_color_g_;
	m.b = bg_color_b_;

	if(tint_green_on_){ // tints green
		m.r /= 4;
		m.b /= 4;
	}

    // adds new mist to the list
	mist_list_.push_front(m);
	if(mist_list_.size() > 15) // pops off the last
		mist_list_.pop_back();

}

void Assignment1App::draw_mist(uint8_t* pixels, mist_info m){

	for(int y = m.y; y < m.y + m.height; y++){
		for(int x = m.x; x < m.x + m.width; x++){
			if(y < 0 || x < 0 || x >= kAppWidth || y >= kAppHeight)
				continue;

            // draws pixels with an alpha value taken into consideration
			int offset = 3* (x + y*kTextureSize);
			pixels[offset] = pixels[offset]*m.transparency + m.r*(1-m.transparency);
			pixels[offset+1] = pixels[offset+1]*m.transparency + m.g*(1-m.transparency);
			pixels[offset+2] = pixels[offset+2]*m.transparency + m.b*(1-m.transparency);
	

		}
	}

	if(blur_on_){
		blur_area_edges(m.x, m.y, m.width, m.height);
	}

}

void Assignment1App::draw_circle(int center_x, int center_y, int r, float transparency){

	uint8_t* pixels = (*work_Surface_).getData();

	//Bounds test
	if(r >= kAppWidth/4 || r >= kAppHeight/4) return;

	for(int y=center_y-r; y<=center_y+r; y++){
		for(int x=center_x-r; x<=center_x+r; x++){
			//Bounds test, to make sure we don't access array out of bounds
			if(y < 0 || x < 0 || x >= kAppWidth || y >= kAppHeight)
				continue;

			int dist = (int)sqrt((double)((x-center_x)*(x-center_x) + (y-center_y)*(y-center_y)));
			float ratio = ((float)dist/r);

			if(ratio > 0.5 && dist < r){
				int offset = 3*(x + y*kTextureSize);
				//By blending the colors I get a semi-transparent effect
				pixels[offset] = pixels[offset]*sin(2*PI_*ratio)/transparency;
				pixels[offset+1] = pixels[offset+1]*sin(2*PI_*ratio)/transparency;
				pixels[offset+2] = pixels[offset+2]*sin(2*PI_*ratio)/transparency;
				//check too see if i can attenuate negative side
				//or just say 0 - pi
			}
		}
	}
}

void Assignment1App::blur_area_edges(int origin_x, int origin_y, int width, int height){
	uint8_t* pixels = (*work_Surface_).getData();
	
	//Convolution filters tend to overwrite the data that you need, so
	// we keep a temporary copy of the image_to_blur. There are certainly
	// more efficient ways to deal with this problem, but this is simple to
	// understand.
	static uint8_t work_buffer[3*kTextureSize*kTextureSize];
	//This memcpy is not much of a performance hit.
	memcpy(work_buffer,pixels,3*kTextureSize*kTextureSize);

	uint8_t kernelA[9] =
		{4,3,4,
		4,3,4,
		4,3,4};
	uint8_t kernelB[9] =
		{4,3,4,
		4,2,4,
		4,3,4};

	uint8_t total_red =0;
	uint8_t total_green = 0;
	uint8_t total_blue =0;
	int offset;	
	int k, ky,kx;

	//Visit every pixel in the image, except the ones on the edge.
	//TODO Special purpose logic to handle the edge cases
	for(int y = origin_y; y < origin_y + height-1; y++){
		for(int x = origin_x; x < origin_x + width-1; x++){

			//check bounds so we don't access pixels outside of image array:
			if(x >= kAppWidth || y >= kAppHeight 
				|| x <= 0 || y <= 0){
					continue;
			} 

			offset = 3*(x + y*kAppWidth);

			if(my_blur_pattern_[offset] > 2*256/3){
				//Compute the convolution of the kernel with the region around the current pixel
				//I use ints for the totals and the kernel to avoid overflow
				total_red=0;
				total_green=0;
				total_blue=0;
				for( ky=-1;ky<=1;ky++){
					for( kx=-1;kx<=1;kx++){
						offset = 3*(x + kx + (y+ky)*kTextureSize);
						k = kernelA[kx+1 + (ky+1)*3];
						total_red += (work_buffer[offset ] >> k);
						total_green += (work_buffer[offset+1] >> k);
						total_blue += (work_buffer[offset+2] >> k);
					}
				}
			} else if(my_blur_pattern_[offset] > 256/3){
				//Compute the convolution of the kernel with the region around the current pixel
				//I use ints for the totals and the kernel to avoid overflow
				total_red=0;
				total_green=0;
				total_blue=0;
				for( ky=-1;ky<=1;ky++){
					for( kx=-1;kx<=1;kx++){
						offset = 3*(x + kx + (y+ky)*kTextureSize);
						k = kernelB[kx+1 + (ky+1)*3];
						total_red += (work_buffer[offset ] >> k);
						total_green += (work_buffer[offset+1] >> k);
						total_blue += (work_buffer[offset+2] >> k);
					}
				}
			} else {
				offset = 3*(x + y*kTextureSize);
				total_red = work_buffer[offset];
				total_green = work_buffer[offset+1];
				total_blue = work_buffer[offset+2];
			}

			offset = 3*(x + y*kTextureSize);
			pixels[offset] = total_red;
			pixels[offset+1] = total_green;
			pixels[offset+2] = total_blue;
		}
	}

}

void Assignment1App::update()
{
	update_count_++; // increment update count for mist creation

	if(update_count_ % 5 == 0){ //assuming 60 hz update, create new mist every second
		create_mist();
		if(update_count_ > 10000000) //don't want to overflow INT
			update_count_ = 0; //reset counter
	}

	*work_Surface_ = (*bg_Surface_).clone();

	uint8_t* pixels = (*work_Surface_).getData();

	//ANIMATION SECTION FOR CIRCLES AND RECTANGLES: 

	if(!mist_on_)
		mist_list_.clear();
	else{ // draws rectangles
		for(int i = 0; i < mist_list_.size(); i++){
			draw_mist(pixels,mist_list_[i]);
			if(mist_list_[i].height < 50 && mist_list_[i].width < 50){
				mist_list_[i].height += random_.nextInt(5);//get int from 0-1
				mist_list_[i].width += random_.nextInt(5);
			}
			mist_list_[i].x += mist_speed_*cos(2*PI_*update_count_/rotation_rate_);
			mist_list_[i].y += mist_speed_*sin(2*PI_*update_count_/rotation_rate_);
			if(mist_list_[i].transparency < 1)
				mist_list_[i].transparency += 0.005;
		}
	}

    // draws circles
	for(int i = 0; i < circle_list_.size(); i++){
		if(circle_list_[0].radius <= 0)
			circle_list_.pop_back();

		draw_circle(circle_list_[i].x, circle_list_[i].y, circle_list_[i].radius, circle_list_[i].transparency);
		circle_list_[i].radius += 1;
		circle_list_[i].transparency -= 0.005;
	}


}

void Assignment1App::draw()
{
	gl::draw(*work_Surface_); // redraws surface
}

CINDER_APP_BASIC( Assignment1App, RendererGl )

