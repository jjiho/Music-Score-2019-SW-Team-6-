// Perfect_pitch_2.2.1.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"

#include<opencv2/opencv.hpp>
#include<iostream>
#include<stdlib.h>
#include<vector>
#include"Pretreatment.h"
#include"Midi.h"
#include"Score.h"
#include"ScoreProcessor.h"
#include"linearScore.h"
#include<sstream>

#pragma comment(lib,"winmm.lib")

using namespace std;

int linearScore::index = 0;


int main()
{
	/////////////make new midi and settings//////////////////////////////
	int LUT_original[2][20] = {
		{ 41,39,37,36,34,32,30,29,27,25,24,22,20,18,17,15,13,12,10,8 },
	{ 20,18,17,15,13,12,10,8,6,5,3,1,0,-2,-4,-6,-7,-9,-11,-12 } };

	HMIDIOUT     m_DevHandle;
	MMRESULT     m_MMErrCode;
	MIDIOUTCAPS  m_NowMidiOutCaps;

	m_DevHandle = Midi::Open(0);

	if (m_DevHandle == NULL)
		return 0;

	Midi::AllChannelSoundOff(m_DevHandle);
	Midi::SendShortMsg(m_DevHandle, 0xB0, 7, 127);

	Score myMusic(m_DevHandle,150);

	std::cout << "미디 생성 및 설정 완료" << endl;

	//////////////////////////read image and do pretreatment it///////////////////////////////

	bool EoI = false;
	cv::Mat image;
	vector<int> lineArr;

	image = cv::imread("score.jpg", CV_LOAD_IMAGE_COLOR);
	//image = cv::imread("RestTest.jpg", CV_LOAD_IMAGE_COLOR);

	Pretreatment::Binarization(image, 200);
	Pretreatment::DetectLine(image, lineArr);
	Pretreatment::RemoveDup(lineArr);
	
	std::cout << "이미지 로드 및 전처리 완료" << endl;

	////////////////////cutting score for left/right hands to one line(linScore)/////////////////////////////
	std::cout << "악보 자르기 작업 중..." << endl;

	vector<linearScore> LinScores;
	int NumofLinears = 0;
	while(!EoI)
	{
		linearScore temp0(image, lineArr,EoI);
		LinScores.push_back(temp0);
		NumofLinears++;
	}
	//lines5=[NumofLinears][2][5]
	int*** lines5 = new int**[NumofLinears];
	for (int i = 0; i < NumofLinears; i++)
	{
		lines5[i] = new int*[2];
		for (int j = 0; j < 2; j++)
		{
			lines5[i][j] = new int[5];
		}
	}
	for (int n = 0; n < lineArr.size(); n++)
	{
		int i = n / 10;
		int j = n % 5;
		int k;
		if (n % 10 < 5)
			k = 0;
		else k = 1;
		lines5[i][k][j] = lineArr[n];
	}
	std::cout << "\n악보 자르기 작업 완료" << endl;

	//////////////////////////////main processing///////////////////////////////
	
	std::cout << "악보 처리 시작" << endl;
	for (int i = 0; i < NumofLinears; i++)
	{
		for (int RL = 0; RL < 2; RL++)
		{
			ScoreProcessor myProc(LinScores[i].oneline[RL], &myMusic,lines5[i][RL],LUT_original, RL);
			if(RL==0)
				std::cout <<"\n"<< i + 1 << "번째 오른손 라인 탐색 시작" << endl;
			else
				std::cout << "\n" << i + 1 << "번째 왼손 라인 탐색 시작" << endl;
			myProc.DetectNote();
		}
	}
	myMusic.NewNote(0,8);
	myMusic.NewNote(1,8);

	////////////////////////악보 정리(도돌이표)////////////////////////////////
	Score FinalScore(m_DevHandle, 150);

	int RightRecurIndex = -1;
	int LeftRecurIndex = 0;
	bool jumpState = false;
	int jumpIndex = -1;
	std::cout<<"////////////////////////////////////////////////////////////////////////////////"<<endl;
	std::cout << "악보 정리 시작" << endl;
	int boundary = myMusic.Threads_R[0].notes.size();
	if (boundary > myMusic.Threads_L[0].notes.size())
		boundary = myMusic.Threads_L[0].notes.size();
	for (int i = 0; i < boundary; i++)
	{
		if (myMusic.Threads_R[0].notes[i] > -50) //오른손이 정상적인 음일때
		{
			if (myMusic.Threads_L[0].notes[i] > -50) //둘다 정상적인 음일때 그냥 푸쉬
			{
				for (int j = 0; j < 5; j++)
				{
					if (i < myMusic.Threads_R[j].notes.size())
					{
						FinalScore.Threads_R[j].notes.push_back(myMusic.Threads_R[j].notes[i]);
					}
					if (i < myMusic.Threads_L[j].notes.size())
					{
						FinalScore.Threads_L[j].notes.push_back(myMusic.Threads_L[j].notes[i]);
					}
				}
			}
			else //왼손이 비정상적인 음일때는 오른손만 푸쉬
			{
				for (int j = 0; j < 5; j++)
				{
					if (i < myMusic.Threads_R[j].notes.size())
					{
						FinalScore.Threads_R[j].notes.push_back(myMusic.Threads_R[j].notes[i]);
					}
				}
			}
		}
		else //오른손이 비정상적인 음일때
		{
			if (myMusic.Threads_L[0].notes[i] > -50) //왼손은 정상일때 왼손만 푸쉬
			{
				for (int j = 0; j < 5; j++)
				{
					if (i < myMusic.Threads_L[j].notes.size())
					{
						FinalScore.Threads_L[j].notes.push_back(myMusic.Threads_L[j].notes[i]);
					}
				}
			}
			else //둘다 비정상적인 음일때
			{
				if (myMusic.Threads_R[0].notes[i] == myMusic.Threads_L[0].notes[i]) //같은거 동시에 찾음
				{
					if (myMusic.Threads_R[0].notes[i] == -100) // 오른쪽 도돌이표 발견
					{
						RightRecurIndex = i;
						std::cout << "오른쪽 도돌표 발견, RightRecurIndex = " << RightRecurIndex << endl;////////////////
					}
					else if (myMusic.Threads_R[0].notes[i] == -101) // 왼쪽 도돌이표 발견
					{
						LeftRecurIndex = i;
						std::cout << "왼쪽 도돌이표 발견, JumpState =" << jumpState <<", jumpIndex:"<<jumpIndex<< ", LeftRecurIndex = " << LeftRecurIndex << endl;//
						if (jumpState&&RightRecurIndex < jumpIndex) //현재까지 건너뛰기 표시가 있었음
						{
							for (int j = RightRecurIndex + 1; j < jumpIndex; j++)
							{
								if (myMusic.Threads_R[0].notes[j] > -50 && myMusic.Threads_L[0].notes[j] > -50) //왼쪽 도돌이표 발견, state = true상태
								{
									for (int k = 0; k < 5; k++)
									{
										FinalScore.Threads_R[k].notes.push_back(myMusic.Threads_R[k].notes[j]);
										FinalScore.Threads_L[k].notes.push_back(myMusic.Threads_L[k].notes[j]);
									}
								}
							}
							std::cout << "state:true->false, " << RightRecurIndex + 1 << "부터 " << jumpIndex - 1 << "(점프인덱스)까지 복사함" << endl;///////////////
							jumpState = false;
						}
						else if (!jumpState)
						{
							for (int j = RightRecurIndex + 1; j < i; j++)
							{
								if (myMusic.Threads_R[0].notes[j] > -50 && myMusic.Threads_L[0].notes[i] > -50)
								{
									for (int k = 0; k < 5; k++)
									{
										FinalScore.Threads_R[k].notes.push_back(myMusic.Threads_R[k].notes[j]);
										FinalScore.Threads_L[k].notes.push_back(myMusic.Threads_L[k].notes[j]);
									}
								}
							}
							std::cout << RightRecurIndex + 1 << "부터 " << i - 1 << "까지 복사함" << endl;/////////////////
						}
					}
					else if (myMusic.Threads_R[0].notes[i] = -102) //건너뛰기 표시 발견
					{
						if (!jumpState)
						{
							jumpIndex = i;
							cout << "건너뛰기 표시 홀수 발견,jumpindex :" << jumpIndex << endl;
							jumpState = true;
						}
						else
						{
							cout << "건너뛰기 표시 짝수 발견, jumpIndex" << jumpIndex << endl;
							jumpIndex = -1;
							jumpState = false;
						}
					}
				}
			}
		}
	}
	

	std::cout << "악보 처리 완료" << endl;

	////////////////////////악보 출력///////////////////////////
	/*
	for (int i = NumofLinears-1; i >= 0; i--)
	{
		for (int j = 0; j < 2; j++)
		{
			ostringstream o;
			o << 2 * i + j;
			string str = o.str();
			cv::namedWindow(str, CV_WINDOW_AUTOSIZE);
			cv::imshow(str, LinScores[i].oneline[j]);
		}
	}
	cv::waitKey(0);
	for (int i = NumofLinears - 1; i >= 0; i--)
	{
		for (int j = 0; j < 2; j++)
		{
			ostringstream o;
			o << 2 * i + j;
			string str = o.str();
			cv::destroyWindow(str);
		}
	}
	*/
	////////////////////////////////Play music////////////////////////////////////////////////
	
	FinalScore.setTempo(90); //디폴트는 150, 버튼으로 템포 지정 가능하면 좋음. 75하면 두배빨라짐

	while (true)
	{
		int input;
		std::cout << "1:전체 재생, 2:오른손 재생, 3:왼손 재생, 4:종료";
		cin >> input;
		if (input == 1)
		{
			FinalScore.setVolume_R(100);
			FinalScore.setVolume_L(80);
			FinalScore.PlayMusic();
		}
		else if (input == 2)
		{
			FinalScore.setVolume_R(100);
			FinalScore.setVolume_L(0);
			FinalScore.PlayMusic();
		}
		else if (input == 3)
		{
			FinalScore.setVolume_R(0);
			FinalScore.setVolume_L(100);
			FinalScore.PlayMusic();
		}
		else break;
	}
	
	////////////////////////////////////////////////////

    return 0;
}