/*
This program is a version of flappy bird heavily based on Javidx9's flappy bird implementation which he made for the purpose of being an educational tool.
As stated, my version is very similair to his, as I primarily followed the tutorial he published to youtube. I've implemented a different scoring system
which tracks the number of portals the bird flies through, much like the original flappy bird game. I also added a different type of door to make the gameplay 
a little more challenging. There are the normal green doors that increase the score when passed, and there are red doors that will also invert
gravity until another red door is passed.

Javidx9 has his Flappy bird implemntation on his github:
https://github.com/OneLoneCoder/videos/blob/master/OneLoneCoder_FlappyBird.cpp
YouTube link:
https://youtu.be/b6A4XHkTjs8

To run this program, the olcConsoleGameEngine header file is required. Javidx9 is the author of the header file as well, and discovering it from his 
YouTube is what got me interested in playing with this code in the first place, as it seems like a fun header file to use to make simple console based games.
This FlappyBird implementation was my introduction into the olcConsoleGameEngine, and I see myself using it again in the future to make other silly little games.
(Note: the olcConsoleGameEngine.h file can only be used on Windows to to calling the windows.h header file)
https://github.com/OneLoneCoder/videos/blob/master/olcConsoleGameEngine.h
*/
#include <iostream>
#include <string>
using namespace std;

#include "olcConsoleGameEngine.h"

class OneLoneCoder_FlappyBird : public olcConsoleGameEngine {
public:
	OneLoneCoder_FlappyBird()
	{
		m_sAppName = L"Flappy Bird";
	}
private:
	//Bird
	float fBirdPosition = ScreenHeight() / 2.0f;//0.0f;
	float fBirdVelocity = 0.0f;
	float fBirdAcceleration = 0.0f;
	bool bHasCollided = false;

	float fGravity = 100.0f;
	bool bResetGame = false;

	//World
	struct sSection
	{
		int nDoorHeight = 0;
		bool bFlip = false;//will this gate flip gravity?

		sSection() {
			int i = rand() % 5;
			if (i == 0)
				bFlip = true;
		};
	};
	float fSectionWidth;
	list<sSection> listSection;
	float fLevelPosition = 0.0f;

	//inversion integer. should only ever be 1 or -1. 
	//will reverse gravity if -1
	int nInvert = 1;

	//scoring
	int nAttemptCount = 0;
	int nScore = 0;
	int nMaxScore = 0;
	int CurrentTime = 0;
	//bool to indicate if the current pipe was counted in the scoring
	bool bPipeCounted = false;

protected:
	virtual bool OnUserCreate()
	{
		//game starts with an empty word to give player time
		listSection = { sSection(), sSection(), sSection(), sSection() };
		CurrentTime = 0;

		//section width is screen width divided by number of sections in our list - 1 (one is hidden off camera)
		fSectionWidth = (float)ScreenWidth() / (float)(listSection.size() - 1);
		return true;
	}

	virtual bool OnUserUpdate(float fElapsedTime)
	{
		//bird x position doesn't change
		int nBirdx = (int)(ScreenWidth() / 3.0f);
		if (bResetGame)
		{
			bHasCollided = false;
			bResetGame = false;
			listSection = { sSection(), sSection(), sSection(), sSection() };
			fBirdAcceleration = 0.0f;
			fBirdVelocity = 0.0f;
			fBirdPosition = ScreenHeight() / 2.0f;
			nScore = 0;
			nAttemptCount++;
			CurrentTime = fElapsedTime;
			nInvert = 1;
		}

		if (bHasCollided)
		{
			//do nothing until player presses space

			if (m_keys[VK_SPACE].bReleased)
				bResetGame = true;
		}
		else
		{

		
			//check for spacebar and if bird has traveled down some distance before flapping
			if (m_keys[VK_SPACE].bPressed && fBirdVelocity >= fGravity / 10.0f)
			{
				fBirdAcceleration = 0.0f;
				fBirdVelocity = -fGravity / 4.0f;
			}
			else
				fBirdAcceleration += fGravity * fElapsedTime * nInvert;

			if (fBirdAcceleration > fGravity)
				fBirdAcceleration = fGravity;
			//update bird position and acceleration
			fBirdVelocity += fBirdAcceleration * fElapsedTime * nInvert;
			fBirdPosition += fBirdVelocity * fElapsedTime * nInvert;

			//update world position offset
			fLevelPosition += 14.0f * fElapsedTime;

			if (fLevelPosition > fSectionWidth)
			{
				fLevelPosition -= fSectionWidth;
				listSection.pop_front();
				//new obstacle
				sSection section = sSection();
				section.nDoorHeight = rand() % (ScreenHeight() - 20);
				if (section.nDoorHeight <= 10)
				{
					section.nDoorHeight = 0;
					section.bFlip = false;
				}
				listSection.push_back(section);
				
			}

			//before drawing, clear the screen
			Fill(0, 0, ScreenWidth(), ScreenHeight(), L' ');

			//draw sections 
			int nSection = 0;
			int nRand = rand() % 5; //random number to determine gravity flip 1/20 chance
			for (auto s : listSection)
			{
				if (s.nDoorHeight != 0)
				{
					if (!s.bFlip)
					{
						//drawing green rectangles
						Fill(nSection * fSectionWidth + 10 - fLevelPosition, ScreenHeight() - s.nDoorHeight,
							nSection * fSectionWidth + 15 - fLevelPosition, ScreenHeight(),
							PIXEL_SOLID, FG_GREEN);
						Fill(nSection * fSectionWidth + 10 - fLevelPosition, 0,
							nSection * fSectionWidth + 15 - fLevelPosition, ScreenHeight() - s.nDoorHeight - 15,
							PIXEL_SOLID, FG_GREEN);
					}
					else
					{
						//drawing red rectangles
						Fill(nSection * fSectionWidth + 10 - fLevelPosition, ScreenHeight() - (s.nDoorHeight - 2),
							nSection * fSectionWidth + 15 - fLevelPosition, ScreenHeight(), 
							PIXEL_SOLID, FG_RED);
						Fill(nSection * fSectionWidth + 10 - fLevelPosition, 0,
							nSection * fSectionWidth + 15 - fLevelPosition, ScreenHeight() - (s.nDoorHeight + 2) - 15,
							PIXEL_SOLID, FG_RED);
					}
					

					//scoring
					if ( ((int)(nSection * fSectionWidth + 15 - fLevelPosition) == nBirdx) && !bPipeCounted)
					{
						bPipeCounted = true;
						++nScore;
						if (nScore > nMaxScore)
							nMaxScore = nScore;
						//if gate was red, flip gravity
						if (s.bFlip && s.nDoorHeight != 0)
						{
							nInvert *= -1;
							fBirdAcceleration = 0.0f;
							fBirdVelocity = 0.0f;
						}
							
					}
					else
					{
						if (bPipeCounted && ((int)(nSection * fSectionWidth + 15 - fLevelPosition) == nBirdx -1))
						{
							bPipeCounted = false;
						}	
					}
						
				}
				++nSection;
			}

			//collision detection 
			bHasCollided = fBirdPosition < 2 || fBirdPosition > ScreenHeight() - 2 ||
				m_bufScreen[(int)(fBirdPosition + 0) * ScreenWidth() + nBirdx].Char.UnicodeChar != L' ' ||
				m_bufScreen[(int)(fBirdPosition + 1) * ScreenWidth() + nBirdx].Char.UnicodeChar != L' ' ||
				m_bufScreen[(int)(fBirdPosition + 0) * ScreenWidth() + nBirdx + 5].Char.UnicodeChar != L' ' ||
				m_bufScreen[(int)(fBirdPosition + 1) * ScreenWidth() + nBirdx + 5].Char.UnicodeChar != L' ';

			//draw Bird
			if (nInvert == 1)
			{
				if (fBirdVelocity > 0) //Bird descending
				{
					DrawString(nBirdx-1, fBirdPosition + 0, L"\\\\\\");
					DrawString(nBirdx, fBirdPosition  +1, L"<\\\\'>");
				}
				else				//Bird ascending
				{
					DrawString(nBirdx, fBirdPosition + 0, L"<//'>");
					DrawString(nBirdx -1, fBirdPosition +1, L"///");
				}
			}
			else if(nInvert == -1)//reversed gravity
			{
				if (fBirdVelocity > 0) //Bird descending
				{
					DrawString(nBirdx, fBirdPosition + 0, L"<//.>");
					DrawString(nBirdx-1, fBirdPosition +1, L"///");
				}
				else				//Bird ascending
				{
					DrawString(nBirdx-1, fBirdPosition +0, L"\\\\\\");
					DrawString(nBirdx, fBirdPosition +1, L"<\\\\.>");
				}
			}
			

			DrawString(1, 1, L"Attempt: " + to_wstring(nAttemptCount) + L" Score: " + to_wstring(nScore) + L" High Score: " + to_wstring(nMaxScore));

		}//end if(hasCollided) else

		return true;
	}
};

int main()
{
	//create instance of olc console engine derived game
	OneLoneCoder_FlappyBird game;

	game.ConstructConsole(80,48,16,16);
	game.Start();

	return 0;
}