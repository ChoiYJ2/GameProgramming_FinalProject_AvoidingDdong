#pragma once
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm")
#include <d2d1.h>
#pragma comment(lib, "d2d1")
#include <dwrite.h>
#pragma comment(lib, "dwrite")
#include <wincodec.h>
#pragma comment(lib, "WindowsCodecs")
#include <vector>
#include <cmath> //abs() 사용을 위한 헤더

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

HRESULT LoadBitmapFromRes(ID2D1RenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, PCWSTR resourceName, PCWSTR resourceType, UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** ppBitmap);

typedef struct Ddong {
	ID2D1Bitmap* pDdong = NULL; //똥 이미지 받아올 비트맵 객체
	D2D1_SIZE_F Ddong_size; //비트맵 객체 크기
	D2D1_POINT_2F Ddong_LeftTop; //비트맵 객체 시작 위치
	float ddong_speed = 0.f; //똥이 떨어질 속도
	bool destroyed = FALSE; //똥이 소멸 조건을 충족했는지 확인
}Ddong;

typedef struct Life {
	ID2D1Bitmap* pLife = NULL; //남아있는 생명 이미지 받아올 비트맵 객체
	ID2D1Bitmap* pBrokenLife = NULL; //소멸된 생명 이미지 받아올 비트맵 객체
	D2D1_SIZE_F Life_size; //비트맵 객체 크기
	D2D1_POINT_2F Life_LeftTop; //비트맵 객체 시작 위치
}Life;

Ddong ddong;
Life life;

class MainWindow
{
public:
	MainWindow();
	~MainWindow(); //모든 자원을 반납
	HRESULT Initialize(HINSTANCE hInstance); //윈도우 생성, CreateAppResource() 호출

private:
	HRESULT CreateAppResource(); //장치 독립적 그리기 자원을 생성
	HRESULT CreateDeviceResource(); //장치 의존 자원을 생성
	void DiscardDeviceResource(); //장치 의존 자원을 반납
	void OnPaint(); //내용을 그리기
	void OnResize(); //렌더타겟을 resize
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); //윈도우 프로시져

	// player function
	void PlayermoveLeft(); //키보드 왼쪽 화살표 버튼 눌렀을 때 player 왼쪽으로 이동
	void PlayermoveRight(); //키보드 오른쪽 화살표 버튼 눌렀을 때 player 오른쪽으로 이동
	void checkHit(); //똥과 player가 부딪혔는지 확인

	// ddong function
	void Ddongmove(); //똥 객체 움직이는 함수
	void DdongGEN(); //똥 객체 생성하는 함수

private:
	//클래스의 변수들을 선언
	HWND hwnd;
	ID2D1Factory* pD2DFactory;
	ID2D1HwndRenderTarget* pRenderTarget;

	// 생성한 윈도우 크기
	RECT window_size;

	//DWrite
	IDWriteFactory* pDWriteFactory;

	//WIC 변수 선언
	IWICImagingFactory* pWICFactory;

	// GameOver
	bool GameOver = false; //게임이 종료됐는지 확인
	ID2D1Bitmap* pGameOverBitmap;
	D2D1_SIZE_F gameover_size;
	D2D1_POINT_2F gameover_LeftTop;

	// 점수
	int score = 0;
	D2D1_POINT_2F score_LeftTop;
	bool score_calc = false; //점수 계산이 됐는지 확인
	IDWriteTextFormat* pScore;
	ID2D1SolidColorBrush* pScoreBrush;

	//player
	ID2D1Bitmap* pPlayer;
	ID2D1Bitmap* pDdongPlayer;
	D2D1_SIZE_F player_size;
	D2D1_POINT_2F player_LeftTop;
	bool player_hitted = false; //player가 똥에 맞았는지 확인

	//ddong
	std::vector<Ddong> Ddongs; //똥 객체 넣을 벡터
	ID2D1Bitmap* pDdong_bitmap;
	int period = 0; //똥 생성 주기

	// Life
	int life_cnt = 0; //남은 생명 개수
	std::vector<Life> Lifes; //생명 객체 넣을 벡터
	ID2D1Bitmap* pLifeBitmap;
	ID2D1Bitmap* pBrokenLifeBitmap;
};

MainWindow::MainWindow() :
	hwnd(NULL),
	pD2DFactory(NULL),
	pRenderTarget(NULL),
	pDWriteFactory(NULL),
	pGameOverBitmap(NULL),
	pScore(NULL),
	pScoreBrush(NULL),
	pWICFactory(NULL),
	pPlayer(NULL),
	pDdongPlayer(NULL),
	pDdong_bitmap(NULL),
	pLifeBitmap(NULL),
	pBrokenLifeBitmap(NULL)
{
	PlaySound(L"bgm.wav", NULL, SND_NOSTOP | SND_ASYNC | SND_LOOP); //게임이 시작되면 bgm 플레이
}

MainWindow::~MainWindow()
{
	// 장치 의존적 자원 반납
	DiscardDeviceResource();

	// 장치 독립적 자원 반납
	SAFE_RELEASE(pD2DFactory);
	SAFE_RELEASE(pWICFactory);
	SAFE_RELEASE(pDWriteFactory);
}

HRESULT MainWindow::Initialize(HINSTANCE hInstance)
{
	// 장치 독립적 자원을 생성함.
	HRESULT hr = CreateAppResource();
	if (FAILED(hr)) return hr;

	// 윈도우 클래스를 등록함..
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MainWindow::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof(LONG_PTR);
	wcex.hInstance = hInstance;
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
	wcex.lpszClassName = L"Avoiding Ddong";
	RegisterClassEx(&wcex);

	// 윈도우를 생성함.
	// 윈도우 크기 가로: 380, 세로: 540
	// 윈도우 창 이름: Avoiding Ddong
	hwnd = CreateWindow(L"Avoiding Ddong", L"Avoiding Ddong", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 380, 540, NULL, NULL, hInstance, this);
	hr = hwnd ? S_OK : E_FAIL;
	if (!hwnd) return E_FAIL;

	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);
	return hr;
}

HRESULT MainWindow::CreateAppResource()
{
	// D2D 팩토리를 생성함.
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
	if (FAILED(hr)) return hr;

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));
	if (FAILED(hr)) return hr;

	// 점수 표현을 위한 IDWriteTextFormat 객체 생성
	hr = pDWriteFactory->CreateTextFormat(L"Verdana", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 18.f, L"", &pScore);
	if (FAILED(hr)) return hr;

	pScore->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pScore->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	// WIC 팩토리를 생성함.
	// 주의: WIC 팩토리를 생성하는 CoCreateInstance 함수가 사용될 때에는 이전에 CoInitialize를 호출해주어야 함.
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWICFactory));
	if (FAILED(hr)) return hr;

	return hr;
}

// 장치 의존적 자원들을 생성함. 장치가 소실되는 경우에는 이들 자원을 다시 생성해야 함.
HRESULT MainWindow::CreateDeviceResource()
{
	HRESULT hr = S_OK;

	if (pRenderTarget) return hr;

	RECT rc;
	GetClientRect(hwnd, &rc);
	D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

	// D2D 렌더타겟을 생성함.
	hr = pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hwnd, size), &pRenderTarget);
	if (FAILED(hr)) return hr;

	// 점수 텍스트 표현할 단색 붓 생성
	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.f), &pScoreBrush);
	if (FAILED(hr)) return hr;

	//Player 이미지 로드
	hr = LoadBitmapFromRes(pRenderTarget, pWICFactory, L"Player", L"Image", 0, 0, &pPlayer);
	if (FAILED(hr)) return hr;

	//똥에 맞은 player 이미지 로드
	hr = LoadBitmapFromRes(pRenderTarget, pWICFactory, L"DdongHead", L"Image", 0, 0, &pDdongPlayer);
	if (FAILED(hr)) return hr;

	//똥 이미지 로드
	hr = LoadBitmapFromRes(pRenderTarget, pWICFactory, L"Ddong", L"Image", 25, 25, &pDdong_bitmap);
	if (FAILED(hr)) return hr;

	//생명 이미지 로드
	hr = LoadBitmapFromRes(pRenderTarget, pWICFactory, L"Life", L"Image", 25, 25, &pLifeBitmap);
	if (FAILED(hr)) return hr;

	//소멸된 생명 이미지 로드
	hr = LoadBitmapFromRes(pRenderTarget, pWICFactory, L"BrokenLife", L"Image", 25, 25, &pBrokenLifeBitmap);
	if (FAILED(hr)) return hr;

	//게임 종료를 나타낼 이미지 로드
	hr = LoadBitmapFromRes(pRenderTarget, pWICFactory, L"Gameover", L"Image", 350, 0, &pGameOverBitmap);
	if (FAILED(hr)) return hr;

	//윈도우 크기 받아와서 window_size 변수(타입:RECT)에 넣음
	::GetClientRect(hwnd, &window_size);

	// 점수 출력 위치 초기화
	score_LeftTop = D2D1::Point2F(window_size.right / 2 - 35.f, window_size.top);

	// Player 위치 초기화
	player_size = pDdongPlayer->GetSize();
	player_LeftTop = D2D1::Point2F(window_size.right / 2, window_size.bottom - player_size.height);

	// Life 이미지 위치 초기화
	// 주어지는 Life가 총 3개 이므로 3개의 Life 이미지 출력해야함
	for (int i = 0; i < 3; i++)
	{
		life.pLife = pLifeBitmap; //생성한 Life 객체 life에 남아있는 생명 이미지 비트맵 객체 저장
		life.pBrokenLife = pBrokenLifeBitmap; //Life 객체 life에 소멸된 생명 이미지 비트맵 객체 저장
		life.Life_size = pLifeBitmap->GetSize(); //비트맵 크기 받아옴
		life.Life_LeftTop = D2D1::Point2F(0.f, life.Life_size.width); //비트맵 그려질 위치 설정
		Lifes.push_back(life); //Life 객체를 Lifes라는 벡터에 넣음
	}
	life_cnt = Lifes.size(); //남아있는 생명 개수는 Lifes 벡터의 크기

	// Gameover
	gameover_size = pGameOverBitmap->GetSize(); //게임 오버 이미지 비트맵 크기 받아옴
	gameover_LeftTop = D2D1::Point2F(0.f, (window_size.bottom - gameover_size.height) / 2); //게임 오버 비트맵 그려질 위치 설정(윈도우 중간에 그리도록 설정)

	return hr;
}

void MainWindow::DiscardDeviceResource()
{
	SAFE_RELEASE(pRenderTarget);
	SAFE_RELEASE(pScore);
	SAFE_RELEASE(pScoreBrush);
	SAFE_RELEASE(pGameOverBitmap);
	SAFE_RELEASE(pPlayer);
	SAFE_RELEASE(pDdongPlayer);
	SAFE_RELEASE(pDdong_bitmap);
	SAFE_RELEASE(pLifeBitmap);
	SAFE_RELEASE(pBrokenLifeBitmap);
}

// 키보드 왼쪽 화살표 버튼 누르면 player 왼쪽으로 이동
void MainWindow::PlayermoveLeft()
{
	if (player_LeftTop.x <= 7.0f) //만약 Player 비트맵의 왼쪽 위의 x좌표가 7보다 작은 경우 움직이지 않음 -> 게임 창 밖으로 player가 사라지는 것 방지
		return;
	player_LeftTop.x -= 15.f; //왼쪽 화살표 버튼 누를 때마다 Player 비트맵의 왼쪽 위 x 좌표가 15씩 감소 -> Player 비트맵 왼쪽으로 15씩 이동
}

// 키보드 오른쪽 화살표 버튼 누르면 player 오른쪽으로 이동
void MainWindow::PlayermoveRight()
{
	if (player_LeftTop.x >= (window_size.right - player_size.width - 5.f)) //만약 Player 비트맵의 왼쪽 위의 x좌표가 (게임 창의 너비 - Player 비트맵의 너비 - 5)보다 작은 경우 움직이지 않음 -> 게임 창 밖으로 player가 사라지는 것 방지
		return;
	player_LeftTop.x += 15.f; //오른쪽 화살표 버튼 누를 때마다 Player 비트맵의 왼쪽 위 x 좌표가 15씩 감소 -> Player 비트맵 오른쪽으로 15씩 이동
}

// 똥과 플레이어가 부딪혔는지 확인
void MainWindow::checkHit()
{
	for (auto& i : Ddongs) //벡터 Ddongs에 있는 Ddong 객체 전체 검사
	{
		//Player의 상단 y좌표에서 똥 객체 하단의 y좌표를 뺸 값
		float check_y = player_LeftTop.y - (i.Ddong_LeftTop.y + i.Ddong_size.height);
		//player의 오른쪽 x좌표 값에서 똥 객체의 왼쪽 x좌표 값을 뺌(절대값은 player와 똥 사이의 거리)
		float check_x1 = i.Ddong_LeftTop.x - player_LeftTop.x;
		if (check_y <= 0) //Player의 머리가 위치한 y좌표 이하로 똥 객체가 들어온 경우
		{
			if(check_x1 > 0) //똥이 player의 오른쪽에 위치한 경우
			{
				//player와 똥 사이의 거리가 player 너비보다 작은 경우 player와 똥이 부딪힘 
				if (std::abs(check_x1) <= player_size.width)
				{
					i.destroyed = true;
					player_hitted = true;
					if (!score_calc)
						score -= 10;
					score_calc = true;
				}
			}
			else //똥이 player의 왼쪽에 위치한 경우
			{
				//player와 똥 사이의 거리가 똥 너비보다 작은 경우 player와 똥이 부딪힘 
				if (std::abs(check_x1) <= i.Ddong_size.width)
				{
					i.destroyed = true;
					player_hitted = true;
					if (!score_calc)
						score -= 10;
					score_calc = true;
				}
			}
			if (i.Ddong_LeftTop.y <= window_size.bottom) //똥이 player와 부딪히지 않고 게임창 하단에 도달한 경우 
			{
				i.destroyed = true; //똥이 소멸 조건 충족함을 체크
				if (!score_calc)
					score += 2; //똥과 부딪히지 않았으므로 2점 추가
				score_calc = true;
			}
		}
	}
}

// 똥 움직이는 함수
void MainWindow::Ddongmove()
{
	for (int i = 0; i < Ddongs.size(); i++) //벡터 Ddongs의 크기 = 현재 생성되어 있는 Ddong객체 개수(게임 창에 보이는 똥 개수)
	{
		if (Ddongs[i].destroyed) //Ddong객체가 소멸 조건을 만족하면
		{
			Ddongs.erase(Ddongs.begin() + i); //벡터에서 Ddong객체 삭제
		}
		Ddongs[i].Ddong_LeftTop.y += Ddongs[i].ddong_speed; //똥 객체가 가지는 랜덤한 속도만큼 아래로 떨어짐
	}
}

// 똥 생성하는 함수
void MainWindow::DdongGEN()
{
	ddong.pDdong = pDdong_bitmap; //생성한 Ddong 객체 ddong에 똥 이미지 비트맵 객체 저장
	ddong.ddong_speed = rand() % 5 + 2.f; //똥이 떨어질 속도 랜덤하게 지정
	ddong.Ddong_LeftTop = D2D1::Point2F(rand() % 345, window_size.top + 35.f); //똥이 생성될 위치(x좌표) 랜덤하게 지정, y좌표는 Score와 Life 이미지 밑에 생성되도록 지정
	ddong.Ddong_size = ddong.pDdong->GetSize(); //똥 비트맵 크기 받아옴
	Ddongs.push_back(ddong); //Ddong객체를 Ddongs라는 벡터에 넣음(여러 개의 Ddong 객체 관리 용이)
}

// 그릴 내용을 화면에 그림.
void MainWindow::OnPaint()
{
	HRESULT hr = CreateDeviceResource();
	if (FAILED(hr)) return;

	pRenderTarget->BeginDraw();

	// 점수가 0 이하인 경우 게임 종료
	if (GameOver)
	{
		Sleep(2000); //2초 대기 후 게임창 종료
		PostQuitMessage(0);
		return;
	}

	//이전에 똥에 맞은 결과가 계속 영향을 미치지 않도록 초기화
	player_hitted = false;
	score_calc = false;
	period++;
	Ddongmove(); //OnPaint()가 수행될 때마다 똥 위치 변경(아래로 떨어지도록)
	if (period % 35 == 0) //35초마다 똥 생성
	{
		if (Ddongs.size() <= 7) //게임 창에 활성화된 똥이 7개를 넘지 않은 경우 새로운 똥 생성
			DdongGEN();
	}
	checkHit(); //player와 똥이 부딪혔는지 확인

	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	if (player_hitted) //player와 똥이 부딪힌 경우
	{
		life_cnt--; //생명 1개 소멸
		pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(player_LeftTop.x, player_LeftTop.y));
		pRenderTarget->DrawBitmap(pDdongPlayer, D2D1::RectF(0.f, 0.f, player_size.width, player_size.height)); //똥에 맞은 player 이미지로 변경

		if ((score <= 0) || (life_cnt == 0)) //점수가 0 이하이거나 생명이 모두 소멸된 경우
		{
			PlaySound(NULL, NULL, SND_ASYNC); //배경음 플레이 정지
			PlaySound(L"gameover.wav", NULL, SND_ASYNC); //게임 종료되었을 때 배경음 플레이
			GameOver = true; //게임 종료되었음을 체크
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(gameover_LeftTop.x, gameover_LeftTop.y));
			pRenderTarget->DrawBitmap(pGameOverBitmap, D2D1::RectF(0.f, 0.f, gameover_size.width, gameover_size.height)); //게임 오버 이미지 출력
		}
	}
	else //player와 똥이 부딪히지 않은 경우
	{
		pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(player_LeftTop.x, player_LeftTop.y));
		pRenderTarget->DrawBitmap(pPlayer, D2D1::RectF(0.f, 0.f, player_size.width, player_size.height)); //똥에 맞지 않은 player 이미지 출력
	}

	// Ddong 그리기
	for (auto& i : Ddongs)
	{
		pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(i.Ddong_LeftTop.x, i.Ddong_LeftTop.y));
		for (auto& i : Ddongs)
		{
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(i.Ddong_LeftTop.x, i.Ddong_LeftTop.y));
			if (i.destroyed)
				pRenderTarget->DrawBitmap(i.pDdong, D2D1::RectF(0.f, 0.f, i.Ddong_size.width, i.Ddong_size.height), 0.f);
			else
				pRenderTarget->DrawBitmap(i.pDdong, D2D1::RectF(0.f, 0.f, i.Ddong_size.width, i.Ddong_size.height));
		}
	}

	//Score 그리기
	WCHAR score_buf[180];
	wsprintf(score_buf, L"Score: %d", score);
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	pRenderTarget->DrawText(score_buf, (int)wcslen(score_buf), pScore, D2D1::RectF(score_LeftTop.x - 70.f, score_LeftTop.y, score_LeftTop.x + 150.f, 35.f), pScoreBrush);

	// Life 그리기
	for (int i = 0; i < Lifes.size(); i++)
	{
		if (i >= life_cnt) //똥과 부딪히면 부딪힌 횟수만큼 오른쪽 생명 이미지부터 소멸 생명 이미지로 변경
		{
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(Lifes[i].Life_size.width * i + 2.f, 2.f));
			pRenderTarget->DrawBitmap(Lifes[i].pBrokenLife, D2D1::RectF(0.f, 0.f, Lifes[i].Life_size.width, Lifes[i].Life_size.height));
		}
		else //남아있는 생명 이미지 출력
		{
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(Lifes[i].Life_size.width * i + 2.f, 2.f));
			pRenderTarget->DrawBitmap(Lifes[i].pLife, D2D1::RectF(0.f, 0.f, Lifes[i].Life_size.width, Lifes[i].Life_size.height));
		}
	}

	hr = pRenderTarget->EndDraw();

	// 이 함수는 실행되는 동안에 장치가 소실되면 장치 의존적 자원들을 반납함.
	// 그 다음 OnPaint() 호출 시에 내부에서 호출되는 CreateDeviceResource()에서 반납된 자원들이 다시 생성됨.
	if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
	{
		DiscardDeviceResource();
	}
}

// 렌더타겟의 크기를 다시 설정함.
void MainWindow::OnResize()
{
	if (!pRenderTarget) return;

	RECT rc;
	GetClientRect(hwnd, &rc);

	D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
	pRenderTarget->Resize(size);
}

LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CREATE)
	{
		CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
		MainWindow* pDemoApp = (MainWindow*)pCreate->lpCreateParams;

		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pDemoApp);

		return 1;
	}

	MainWindow* pMainWindow = (MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (!pMainWindow)
		return DefWindowProc(hwnd, message, wParam, lParam);


	switch (message)
	{
	case WM_SIZE:
	{
		pMainWindow->OnResize();
		return 0;
	}
	case WM_DISPLAYCHANGE:
	{
		InvalidateRect(hwnd, NULL, FALSE);
		return 0;
	}
	case WM_PAINT:
	{
		pMainWindow->OnPaint();
		return 0;
	}
	case WM_KEYDOWN: //키보드 버튼 눌렸을 때 처리
	{
		switch (wParam)
		{
		case VK_LEFT: //키보드 왼쪽 화살표 키 눌렸을 때 
			pMainWindow->PlayermoveLeft(); //player 왼쪽으로 이동하는 함수 호출
			break;
		case VK_RIGHT: //키보드 오른쪽 화살표 키 눌렸을 때
			pMainWindow->PlayermoveRight(); //player 오른쪽으로 이동하는 함수 호출
			break;
		case VK_ESCAPE: //키보드 ESC 키 눌렀을 떄 윈도우 창 종료(게임 종료)
			PostQuitMessage(0);
			return 1;
		}
		pMainWindow->OnPaint(); //키보드 처리 후 윈도우 화면에 변경 사항 그릴 수 있도록 OnPaint() 호출
		return 0;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 1;
	}
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	if (SUCCEEDED(CoInitialize(NULL)))
	{
		MainWindow mwindow;
		if (FAILED(mwindow.Initialize(hInstance))) return 0;

		MSG msg = {};

		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	CoUninitialize();
	return 0;
}

HRESULT LoadBitmapFromRes(ID2D1RenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, PCWSTR resourceName, PCWSTR resourceType, UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** ppBitmap)
{
	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pSource = NULL;
	IWICStream* pStream = NULL;
	IWICFormatConverter* pConverter = NULL;
	IWICBitmapScaler* pScaler = NULL;

	HRSRC imageResHandle = NULL;
	HGLOBAL imageResDataHandle = NULL;
	void* pImageFile = NULL;
	DWORD imageFileSize = 0;

	// Locate the resource.
	imageResHandle = FindResourceW(NULL, resourceName, resourceType);
	HRESULT hr = imageResHandle ? S_OK : E_FAIL;
	if (FAILED(hr)) return hr;

	// Load the resource.
	imageResDataHandle = LoadResource(NULL, imageResHandle);
	hr = imageResDataHandle ? S_OK : E_FAIL;
	if (FAILED(hr)) return hr;

	// Lock it to get a system memory pointer.
	pImageFile = LockResource(imageResDataHandle);
	hr = pImageFile ? S_OK : E_FAIL;
	if (FAILED(hr)) return hr;

	// Calculate the size.
	imageFileSize = SizeofResource(NULL, imageResHandle);
	hr = imageFileSize ? S_OK : E_FAIL;
	if (FAILED(hr)) return hr;

	// Create a WIC stream to map onto the memory.
	hr = pIWICFactory->CreateStream(&pStream);
	if (FAILED(hr)) return hr;

	// Initialize the stream with the memory pointer and size.
	hr = pStream->InitializeFromMemory(reinterpret_cast<BYTE*>(pImageFile), imageFileSize);
	if (FAILED(hr)) return hr;

	// Create a decoder for the stream.
	hr = pIWICFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeMetadataCacheOnLoad, &pDecoder);
	if (FAILED(hr)) return hr;

	// Create the initial frame.
	hr = pDecoder->GetFrame(0, &pSource);
	if (FAILED(hr)) return hr;

	// Convert the image format to 32bppPBGRA (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
	hr = pIWICFactory->CreateFormatConverter(&pConverter);
	if (FAILED(hr)) return hr;

	// If a new width or height was specified, create an IWICBitmapScaler and use it to resize the image.
	if (destinationWidth != 0 || destinationHeight != 0)
	{
		UINT originalWidth, originalHeight;
		hr = pSource->GetSize(&originalWidth, &originalHeight);
		if (FAILED(hr)) return hr;

		if (destinationWidth == 0)
		{
			FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
			destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
		}
		else if (destinationHeight == 0)
		{
			FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
			destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
		}

		hr = pIWICFactory->CreateBitmapScaler(&pScaler);
		if (FAILED(hr)) return hr;

		hr = pScaler->Initialize(pSource, destinationWidth, destinationHeight, WICBitmapInterpolationModeCubic);
		if (FAILED(hr)) return hr;

		hr = pConverter->Initialize(pScaler, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
		if (FAILED(hr)) return hr;
	}
	else
	{
		// If destinationWidth == 0 and destinationHeight == 0 then, create bitmap with the original image size.
		hr = pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
		if (FAILED(hr)) return hr;
	}

	//create a Direct2D bitmap from the WIC bitmap.
	hr = pRenderTarget->CreateBitmapFromWicBitmap(pConverter, NULL, ppBitmap);
	if (FAILED(hr)) return hr;

	SAFE_RELEASE(pDecoder);
	SAFE_RELEASE(pSource);
	SAFE_RELEASE(pStream);
	SAFE_RELEASE(pConverter);
	SAFE_RELEASE(pScaler);

	return hr;
}