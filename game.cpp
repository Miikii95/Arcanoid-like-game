#include <vector>
#include<iostream>
#include <algorithm>
#include "Framework.h"
#include <time.h>
#include <cstdlib>
#include <string>
#include <array>

class Element { //class I use as an extension of sprite, it contains not only graphics, but also it's position and size
public:

	Sprite* spr;
	int width, height;
	double x, y;

	Element(Sprite* spr, int w, int h, int x, int y)
	{
		this->spr = spr;
		this->width = w;
		this->height = h;
		this->x = x;
		this->y = y;
	}

	Element(Sprite* spr, int x, int y)
	{
		this->spr = spr;
		getSpriteSize(spr, width, height);
		this->x = x;
		this->y = y;
	}

	Element()
	{
		spr = NULL;
		x = 0;
		y = 0;
		width = 0;
		height = 0;
	}
	void draw()
	{
		drawSprite(spr, x, y);
	}
}; //class Element

void scaleBricsSprite(Sprite* spr, int bricsPerRow, int screenWidth, int screenHeight, float frameSize) // function scaling "bric" size to fit window
{
	int initialWidth, initialHeight;
	getSpriteSize(spr, initialWidth, initialHeight);
	float rowWidth = initialWidth * bricsPerRow;
	float FLscreenWidth = screenWidth - screenWidth *2* frameSize;	
	int bricWidth = initialWidth * FLscreenWidth / rowWidth;
	int	bricHeight = initialHeight * FLscreenWidth / rowWidth;
	setSpriteSize(spr, bricWidth, bricHeight);
}

void scaleSprite(Sprite* spr, float relativeSize, int screenWidth, int screenHeight) // scaling sprite relatively to screen size
{
	int initialWidth, initialHeight;
	getSpriteSize(spr, initialWidth, initialHeight);
	int width = screenWidth * relativeSize;
	int height = initialHeight * width / initialWidth;
	
	setSpriteSize(spr, width, height);
}

int scalePlatform(Sprite* spr, float relativeSize, int screenWidth, int screenHeight) // scaling platform width, ONLY width, returning new platform width
{
	int initialWidth, initialHeight;
	getSpriteSize(spr, initialWidth, initialHeight);
	int width = screenWidth * relativeSize;

	setSpriteSize(spr, width, initialHeight);

	int platformWidth, platformHeigth;
	getSpriteSize(spr, platformWidth, platformHeigth);

	return platformWidth;
}

void normalizeVector(double& x, double& y,float normalizingFactor) //function that normalize movement vector of the ball
																	// it could also modify speed of ball by normalizing factor
{
	double length = std::sqrt(std::pow(x, 2) + std::pow(y, 2));
	x = x / length;
	y = y / length;
	x = x * normalizingFactor;
	y = y * normalizingFactor;
}

class Bric : public Element //class I use to store infromation about breakability of brics, so I can create various types of it
{
public:
	int breakability ;
	bool isBreakable;

	Bric()
	{
		spr = NULL;
		x = 0;
		y = 0;
		width = 0;
		height = 0;
		breakability = 1;
		isBreakable = true;
	}
	Bric(Element el) 
	{
		spr = el.spr;
		width = el.width;
		height = el.height;
		x = el.x;
		y = el.y;
		breakability = 1;
		isBreakable = true;

	}
	Bric(Element el, int BA, bool IBA)
	{
		spr = el.spr;
		width = el.width;
		height = el.height;
		x = el.x;
		y = el.y;
		breakability = BA;
		isBreakable = IBA;
	}

}; //class Bric

enum class Effect { //I decided to create more effects than was mantioned in email
					//I hope it will be seen as extention of feature 3 from your messege
	WIDER,
	SLIMER,
	FASTER,
	SLOWER,
	ADDLIVE,
	MINUSLIVE
};
class Ability : public Element //class to manage addons that fall from random brics, abilities as were called in email
{
public:
	Effect effect;

	Ability(Effect ef,Bric bric)
	{
		effect = ef;
		switch (ef)
		{
		case Effect::WIDER:
			spr = createSprite("data\\47-Breakout-Tiles.png");
			break;
		case Effect::SLIMER:
			spr = createSprite("data\\46-Breakout-Tiles.png");
			break;
		case Effect::FASTER:
			spr = createSprite("data\\42-Breakout-Tiles.png");
			break;
		case Effect::SLOWER:
			spr = createSprite("data\\41-Breakout-Tiles.png");
			break;
		case Effect::ADDLIVE:
			spr = createSprite("data\\65-Breakout-Tiles.png");
			break;
		case Effect::MINUSLIVE:
			spr = createSprite("data\\48-Breakout-Tiles.png");
			break;
		default:
			break;
		}

		{ setSpriteSize(spr, bric.width, bric.height); }
		
		getSpriteSize(spr, width, height);
		x = bric.x;
		y = bric.y-1;
	}
	 void clearSpriteMemory() //abilities are the only spirtes creating during the gameplay, not on initialization
	{				// so to avoid memory leakage they must be deleted within the game
		destroySprite(spr);
	}
};//class Ability

class MyFramework : public Framework {

public:
		
	std::vector<Element> frameVector;	//store sprites used to draw frame
	std::vector <Bric> bricsVector;		//store all info about brics
	std::vector <Bric> initialBricsVector;	//store innitial positioning of brics
	Element platform, ball, background, gameoverBackground, victoryBackground;	//other elements needed to be drawn
	std::vector<Element> heartsVector;	//stores sprites used to indicade number of lives
	std::vector<Ability> abilityVector;	//stores information about "abilities" falling from the brics
	
	Sprite* brokenRedBric; //it has to be stored somewhere, as it is the only sprite that will be changed

	double ballMovement[2];		//vector describing current movement of a ball
	bool isBricsLeft = true;
	bool isVictory = false;
	bool isGameover = false;
	bool isBallMoving = false;
	bool isMovingRight = false; //is PLATFORM moving
	bool isMovingLeft = false;	//is PLATFORM moving

	int bricsPerRow = 9;	//number of brics per row, and total number of rowes
	int numOfRows = 9;		//helpful in creating brics pattern
	int initialLives = 3;
	int maxLives = 5;

	float initialBallSpeed = 0.8;//in points per tic;
	float frameSize = 0.025; //in percentage of screen width
	float initialPlatformSize = 0.2; // in percentage of screen width
	float ballSize = 0.02;	//in percentage of screen width
	float heartSize = 0.075; //in percentage of screen width
	float probabilityOfAbility = 0.9;

	int screenWidth, screenHeight,lives;
	float platformSize, ballSpeed;	//platform size, and ball speed are changing throught the game by catching "abilities"
	
	MyFramework(int windowWidth, int windowHeigth)		//I added this construction so that window size could be changed outside of Myframework class
	{
		screenWidth = windowWidth;
		screenHeight = windowHeigth;
	}

	virtual void PreInit(int& width, int& height, bool& fullscreen)
	{
		width = screenWidth;
		height = screenHeight;
		fullscreen = false;
		srand(time(NULL));
	}

	virtual bool Init() {
		
		platformSize = initialPlatformSize;
		ballSpeed = initialBallSpeed;
		lives = initialLives;

	//preparing background
		// backgrounds found on:
		//Image by rawpixel.com on Freepik
		Sprite* background = createSprite("data\\background.png");
		setSpriteSize(background, screenWidth, screenHeight);
		this->background = Element(background,0,0);

		Sprite* GOBackground = createSprite("data\\gameover.png");
		setSpriteSize(GOBackground, screenWidth, screenHeight);
		this->gameoverBackground = Element(GOBackground, 0, 0);

		Sprite* VBackground = createSprite("data\\victory.png");
		setSpriteSize(VBackground, screenWidth, screenHeight);
		this->victoryBackground = Element(VBackground, 0, 0);

	//preparing frame
			int frameOffsetX = screenWidth * frameSize;
			int frameOffsetY = screenHeight * frameSize;

			//side frames
			Sprite* greyFrameX = createSprite("data\\29-Breakout-Tiles.png");
			setSpriteSize(greyFrameX, screenWidth * frameSize, screenHeight);
			frameVector.push_back(Element(greyFrameX, 0, 0));
			frameVector.push_back(Element(greyFrameX, screenWidth - screenWidth * frameSize, 0));
			//top frame
			Sprite* greyFrameY = createSprite("data\\29-Breakout-Tiles.png");
			setSpriteSize(greyFrameY, screenWidth, screenHeight * frameSize);
			frameVector.push_back(Element(greyFrameY, 0, 0));
			//bottom frame
			Sprite* redFrameY = createSprite("data\\24-Breakout-Tiles.png");
			setSpriteSize(redFrameY, screenWidth, screenHeight * frameSize);
			frameVector.push_back(Element(redFrameY, 0, screenHeight - frameOffsetY));

	//preaparing brics
		
			Sprite* blueBric = createSprite("data\\01-Breakout-Tiles.png");
			scaleBricsSprite(blueBric, bricsPerRow, screenWidth, screenHeight, frameSize);

			Sprite* redBric = createSprite("data\\07-Breakout-Tiles.png");
			scaleBricsSprite(redBric, bricsPerRow, screenWidth, screenHeight, frameSize);

			Sprite* goldBric = createSprite("data//13-Breakout-Tiles.png");
			scaleBricsSprite(goldBric, bricsPerRow, screenWidth, screenHeight, frameSize);

			Sprite* brokenBric = createSprite("data//08-Breakout-Tiles.png");
			scaleBricsSprite(brokenBric, bricsPerRow, screenWidth, screenHeight, frameSize);
			this->brokenRedBric = brokenBric;

			int bricWidth, bricHeight;
			getSpriteSize(blueBric, bricWidth, bricHeight);
			
			//positioning brics, puting it to vector
			//loops and conditions below are the simplest way I found to create bric pattern
			
			for (int j = 0; j < numOfRows; j++)
			{
				for (int i = 0; i < bricsPerRow; i++)
				{
						if (i % 2)
						{
							if ((j % 2))
							{
								Bric el = Bric(Element(blueBric, bricWidth * i + frameOffsetX, bricHeight * j + frameOffsetY), 1, true);
								bricsVector.push_back(el);
							}
						}
						if (!(i % 2))
						{
							if ((j % 2))
							{
								if ((i==2)||(i==6))
								{// I assumed that if it needs milion hits to be destroyed it is indestructible
									Bric el = Bric(Element(goldBric, bricWidth * i + frameOffsetX, bricHeight * j + frameOffsetY), 999999, false);
									bricsVector.push_back(el);
								}
							}

							else {
								if ((i != 2) && (i != 6))
								{
									Bric el = Bric(Element(redBric, bricWidth * i + frameOffsetX, bricHeight * j + frameOffsetY), 2, true);
									bricsVector.push_back(el);
								}
							}
						
					}
				}
			}
			initialBricsVector = bricsVector;	//storing that layout for replaying after win or losing
	//preparing platform 
		
			Sprite* platform = createSprite("data\\56-Breakout-Tiles.png");
			scaleSprite(platform,platformSize , screenWidth,  screenHeight);
			int platformWidth, platformHeight;
			getSpriteSize(platform, platformWidth, platformHeight);
			int x, y;
			y = 0.8 * screenHeight;
			x = screenWidth / 2;
			x = x - (platformWidth / 2);
			Element el = Element(platform, x, y);
			this->platform = el;

	//preparing ball
		
			Sprite* ball = createSprite("data\\58-Breakout-Tiles.png");
			scaleSprite(ball, ballSize, screenWidth, screenHeight);
			int ballWidth, ballHeight;
			getSpriteSize(ball, ballWidth, ballHeight);
			x = screenWidth / 2;
			x = x - (ballWidth / 2);
			y = y - ballHeight;
			el = Element(ball, x, y);
			this->ball = el;

	//preparing hearts
			Sprite* heart = createSprite("data\\60-Breakout-Tiles.png");
			scaleSprite(heart, heartSize, screenWidth, screenHeight);
			int heartWidth, heartHeight;
			getSpriteSize(heart, heartWidth, heartHeight);
			
			for (int i = 0; i < maxLives; i++)
			{
				Element el = Element(heart, 2 * frameOffsetX + i * heartWidth, 0.875 * screenHeight);
				heartsVector.push_back(el);
			}

		return true;
	}

	virtual void Close() {

	}

	virtual bool Tick() {
	//drawing background
		background.draw();

		if (isGameover)
		{
			gameoverBackground.draw();
			return false;
		}
		if (isVictory)
		{
			victoryBackground.draw();
			return false;
		}

    //drawing frame
		for (Element el : frameVector)
		{
			el.draw();
		}
	//drawing brics
		for (Bric el : bricsVector)
		{
			el.draw();
		}

	//drawing hearts
		for (int i = 0; i < lives; i++)
		{
			heartsVector[i].draw();
		}

	//platform movement management
		if (isMovingLeft && platform.x > 0)
		{ platform.x--;
			if (!isBallMoving) ball.x--;
		}
		if (isMovingRight && platform.x < screenWidth - platform.width)
		{ platform.x++;
			if (!isBallMoving) ball.x++;
		}
		platform.draw();

	//ball movement management
		if (isBallMoving) { ball.x = ball.x + ballMovement[0]; ball.y = ball.y - ballMovement[1]; }
		//ball is drawn lastly, so it is above all other elements

	//collision management
	// I assumed that ball is so small, that we could use coordinations of its top left corner only in collision management
	// in other words, in my calculation I assumed it is a single point
	// frame collision
		if (ball.x < frameVector[0].x + frameVector[0].width)	{ ballMovement[0] = (-1) * ballMovement[0];}// right frame collision
		if (ball.x > frameVector[1].x)							{ ballMovement[0] = (-1) * ballMovement[0];}// left frame collision
		if (ball.y < frameVector[2].y + frameVector[2].height)	{ ballMovement[1] = (-1) * ballMovement[1];}// top frame collision
		if (ball.y > frameVector[3].y)																		// bottom frame collision
		{ 
			//stoping ball, and repositioning it on platform
			isBallMoving = false; 
			ball.y = platform.y-ball.height;
			ball.x = platform.x + platform.width / 2 - ball.width / 2;
			lives--;
			showCursor(true);
			
		}
	//platform collision
		if ((ball.y > platform.y && ball.y<platform.y+platform.height ) 
			&& (ball.x>platform.x && ball.x < platform.x + platform.width) 
			&& isBallMoving) 
		{	
			//catching the ball after braking all brics
			if (!isBricsLeft)
			{
				isVictory = true;
				return false;
			}

			//I found it "unnatural" when ball reflects the same regardles of where it hit the platform
	
			//finding where ball impact and scaling it from -0.5 to 0.5, where 0 is center of platform
			double contactPoint = ball.x - platform.x; 
			contactPoint = contactPoint / platform.width;
			contactPoint = contactPoint - 0.5;
			
			double xMove, yMove;
			xMove = ballMovement[0];
			yMove = ballMovement[1];
			
			if (xMove < 0 && contactPoint < 0) //impact from right on left side of platform
			{
				xMove = xMove - contactPoint; //making angle "smaller", 
			}								// it is possible to do it that way, becouse program normalizes movement vector afterwards
			else if (xMove > 0 && contactPoint < 0) // impact from left on left side
			{
				xMove = (-1) * xMove;
				xMove = xMove + contactPoint; //adding more speed in X movement, angle is steeper
			}
			else if (xMove <= 0 && contactPoint >= 0) // impact from right on right side
			{
				xMove = (-1) * xMove;
				xMove = xMove + contactPoint;
			}
			else if (xMove > 0 && contactPoint > 0)// impact from left on right side
			{
				xMove = xMove - contactPoint;
			}
			ball.y = platform.y - 2; //prevents ball from stucking in platform, which sometimes happen when ball hits platform from side
			
			//I found that adding or substracting X factor in the ball movement and then normalizing(scaling) vector,
			//is much easier to calculate, than precisly shifting angles

			normalizeVector(xMove, yMove, ballSpeed);

			yMove =  abs(yMove); // so ball goes always up // Y factor in ball movement vector is reversed!

			ballMovement[0] = xMove;
			ballMovement[1] = yMove;
			
			if (abs(ballMovement[1])<0.1 ) { ballMovement[1] = -0.1; } // in case that after normalization Y become to small and rounded to 0
										// also prevents situation when angle is too steep and ball just keeps on bouncing of left and right side
		}

	//bric collision
		int iterator = 0;
		for (; iterator < bricsVector.size(); iterator++)
		{
			// cheking if ball is "inside" any of brics

			if ((ball.y < bricsVector[iterator].y + bricsVector[iterator].height && ball.y>bricsVector[iterator].y) 
				&& (ball.x > bricsVector[iterator].x && ball.x < bricsVector[iterator].x + bricsVector[iterator].width))
			{
				//it feels unnatural when ball reflects the same from the bottom as from the side of bric
				if (ball.y > bricsVector[iterator].y + 2 && ball.y < bricsVector[iterator].y + bricsVector[iterator].height - 2)
				{
					//side collision
					ballMovement[0] = (-1) * ballMovement[0];
					ball.x = ball.x + ballMovement[0]; //moving ball from the bric prevents from "stucking" it inside the bric
				}
				else
				{	//vertical collision
					ballMovement[1] = (-1) * ballMovement[1];
					ball.y = ball.y - ballMovement[1]; // Y factor is inverted in ball movement vector
				}

				bricsVector[iterator].breakability--;
				if (bricsVector[iterator].breakability == 1) //with three types of brics, it occurs only when we hit red-2-hits-bric
				{ bricsVector[iterator].spr = brokenRedBric; }

				if (bricsVector[iterator].breakability < 1)  //bric destroyed
				{ 
					//generating abilities
					if ((float)rand() / RAND_MAX <probabilityOfAbility) //abilities creation 
					{
						Effect effect = static_cast<Effect>(rand() % 6);	//also effect is chosen randomly
						Ability ability= Ability(effect, bricsVector[iterator]);
						
						abilityVector.push_back(ability);
					};
					bricsVector.erase(bricsVector.begin() + iterator);
				}

				//victory condition
				isBricsLeft = false; //after breaking bric it is victory...
				for (Bric briciterator : bricsVector) //unless there are more any breakable brics
				{
					if (briciterator.isBreakable) 
					{ isBricsLeft = true; }
				}
				break;
			}// if hit bric
		}// for through brics

		//abilities management
		for (int i = 0; i < abilityVector.size(); i++)
		{
			if (abilityVector[i].y > frameVector[3].y)
			{
				abilityVector[i].clearSpriteMemory();
				abilityVector.erase(abilityVector.begin() + i);
			}
		}

		for (int i = 0; i < abilityVector.size(); i++)
		{
			abilityVector[i].y = abilityVector[i].y + ballSpeed;
			abilityVector[i].draw();

			//brics collision
			for (int iterator = 0; iterator < bricsVector.size(); iterator++)
			{
				int centerX = abilityVector[i].x + (abilityVector[i].width / 2); //I used center point to prevent colisions with brics to the side of freshly created ability

				if ((abilityVector[i].y < bricsVector[iterator].y + bricsVector[iterator].height && abilityVector[i].y>bricsVector[iterator].y)
					&& (centerX > bricsVector[iterator].x && centerX < bricsVector[iterator].x + bricsVector[iterator].width))
				{
					abilityVector[i].clearSpriteMemory();
					abilityVector.erase(abilityVector.begin() + i);
					
					break;
				}
			}
		}
		for (int i = 0; i < abilityVector.size(); i++) {
			//platform collision - catching
			if ((abilityVector[i].y > platform.y && abilityVector[i].y < platform.y + platform.height)
				&& 
				(
					(abilityVector[i].x > platform.x && abilityVector[i].x < platform.x + platform.width)
					||
					(abilityVector[i].x + abilityVector[i].width > platform.x && abilityVector[i].x + abilityVector[i].width < platform.x + platform.width)
				)
				)
			{
				switch (abilityVector[i].effect)
				{
				case Effect::WIDER:
					platformSize = platformSize * 1.10;
					platform.width = scalePlatform(platform.spr, platformSize, screenWidth, screenHeight);
					std::cout << "wider chatch";
					break;
				case Effect::SLIMER:
					platformSize = platformSize * 0.9;
					platform.width = scalePlatform(platform.spr, platformSize, screenWidth, screenHeight);
					
					std::cout << "slimer catch";
					break;
				case Effect::FASTER:
					ballSpeed = ballSpeed * 1.2;
					std::cout << "faster catch";
					break;
				case Effect::SLOWER:
					ballSpeed = ballSpeed * 0.8;
					std::cout << "slower catch";
					break;
				case Effect::ADDLIVE:
					if (lives<maxLives){ lives++; }
					std::cout << "lives ++ catch";
					break;
				case Effect::MINUSLIVE:
					lives--;
					std::cout << "lives-- catch";
					break;
				default:
					break;
				}//switch

				abilityVector[i].clearSpriteMemory();
				abilityVector.erase(abilityVector.begin() + i);

			}//if catch

		} // for through abilities vector

		ball.draw(); // it is here, so ball is drawn above additional abilities

		// gameover condition
		if (lives < 1) { isGameover = true; }
	
		return false;
	} // TICK

	virtual void onMouseMove(int x, int y, int xrelative, int yrelative) {
		if (!isBallMoving)
		{
			double xMove;
			double yMove;
			xMove = x - ball.x;
			yMove = abs(y - ball.y); //calculating initial movement vector based on ball and mouse position
			normalizeVector(xMove, yMove,ballSpeed); //scaling that vector so that speed remains the same
			ballMovement[0] = xMove;
			ballMovement[1] = yMove;
		}
	}

	virtual void onMouseButtonClick(FRMouseButton button, bool isReleased) {
		if (isGameover || isVictory) //reseting game
		{ 
			isGameover = false; 
			isVictory = false;
			isBricsLeft = true;
			showCursor(true);
			bricsVector = initialBricsVector;
			lives = initialLives;
			platformSize = initialPlatformSize;
			platform.width = scalePlatform(platform.spr, platformSize, screenWidth, screenHeight); 
			platform.x= (screenWidth / 2) - (platform.width / 2);
			ball.x = (screenWidth / 2) - (ball.width / 2);
			ball.y = platform.y - ball.height;
			isBallMoving = false;
			return;
		}
		if (!isGameover && !isReleased) //starting the ball
		{
			isBallMoving = true;
			showCursor(false);
		}
			
	}

	virtual void onKeyPressed(FRKey k) {
		//moving platform
		if (k == FRKey::LEFT) { isMovingLeft = true; }
		if (k == FRKey::RIGHT) { isMovingRight = true; }

	}

	virtual void onKeyReleased(FRKey k) {
		//stoping platform
		if(k == FRKey::LEFT) { isMovingLeft = false; }
		if (k == FRKey::RIGHT) { isMovingRight = false; }
		
	}
	
	virtual const char* GetTitle() override
	{
		return "Arcanoid";
	}
};


int main(int argc, char* argv[])
{
	std::cout << "Welcome to my version of Arcanoid-like-test-program\n\n";
	//I couldnt find the way to use command line after fremwork was on, so it is my way to fulfill resolution scaling requirement
	// Yet I admit it is impractical and uncomfortable
	int a = 800; int b = 600;
	//std::cout << "Resolution:\n"; std::cin >> a; std::cin >> b;
	
	return run(new MyFramework(a, b));
}