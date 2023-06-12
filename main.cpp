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
#include <cmath> //abs() ����� ���� ���

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

HRESULT LoadBitmapFromRes(ID2D1RenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, PCWSTR resourceName, PCWSTR resourceType, UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** ppBitmap);

typedef struct Ddong {
	ID2D1Bitmap* pDdong = NULL; //�� �̹��� �޾ƿ� ��Ʈ�� ��ü
	D2D1_SIZE_F Ddong_size; //��Ʈ�� ��ü ũ��
	D2D1_POINT_2F Ddong_LeftTop; //��Ʈ�� ��ü ���� ��ġ
	float ddong_speed = 0.f; //���� ������ �ӵ�
	bool destroyed = FALSE; //���� �Ҹ� ������ �����ߴ��� Ȯ��
}Ddong;

typedef struct Life {
	ID2D1Bitmap* pLife = NULL; //�����ִ� ���� �̹��� �޾ƿ� ��Ʈ�� ��ü
	ID2D1Bitmap* pBrokenLife = NULL; //�Ҹ�� ���� �̹��� �޾ƿ� ��Ʈ�� ��ü
	D2D1_SIZE_F Life_size; //��Ʈ�� ��ü ũ��
	D2D1_POINT_2F Life_LeftTop; //��Ʈ�� ��ü ���� ��ġ
}Life;

Ddong ddong;
Life life;

class MainWindow
{
public:
	MainWindow();
	~MainWindow(); //��� �ڿ��� �ݳ�
	HRESULT Initialize(HINSTANCE hInstance); //������ ����, CreateAppResource() ȣ��

private:
	HRESULT CreateAppResource(); //��ġ ������ �׸��� �ڿ��� ����
	HRESULT CreateDeviceResource(); //��ġ ���� �ڿ��� ����
	void DiscardDeviceResource(); //��ġ ���� �ڿ��� �ݳ�
	void OnPaint(); //������ �׸���
	void OnResize(); //����Ÿ���� resize
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); //������ ���ν���

	// player function
	void PlayermoveLeft(); //Ű���� ���� ȭ��ǥ ��ư ������ �� player �������� �̵�
	void PlayermoveRight(); //Ű���� ������ ȭ��ǥ ��ư ������ �� player ���������� �̵�
	void checkHit(); //�˰� player�� �ε������� Ȯ��

	// ddong function
	void Ddongmove(); //�� ��ü �����̴� �Լ�
	void DdongGEN(); //�� ��ü �����ϴ� �Լ�

private:
	//Ŭ������ �������� ����
	HWND hwnd;
	ID2D1Factory* pD2DFactory;
	ID2D1HwndRenderTarget* pRenderTarget;

	// ������ ������ ũ��
	RECT window_size;

	//DWrite
	IDWriteFactory* pDWriteFactory;

	//WIC ���� ����
	IWICImagingFactory* pWICFactory;

	// GameOver
	bool GameOver = false; //������ ����ƴ��� Ȯ��
	ID2D1Bitmap* pGameOverBitmap;
	D2D1_SIZE_F gameover_size;
	D2D1_POINT_2F gameover_LeftTop;

	// ����
	int score = 0;
	D2D1_POINT_2F score_LeftTop;
	bool score_calc = false; //���� ����� �ƴ��� Ȯ��
	IDWriteTextFormat* pScore;
	ID2D1SolidColorBrush* pScoreBrush;

	//player
	ID2D1Bitmap* pPlayer;
	ID2D1Bitmap* pDdongPlayer;
	D2D1_SIZE_F player_size;
	D2D1_POINT_2F player_LeftTop;
	bool player_hitted = false; //player�� �˿� �¾Ҵ��� Ȯ��

	//ddong
	std::vector<Ddong> Ddongs; //�� ��ü ���� ����
	ID2D1Bitmap* pDdong_bitmap;
	int period = 0; //�� ���� �ֱ�

	// Life
	int life_cnt = 0; //���� ���� ����
	std::vector<Life> Lifes; //���� ��ü ���� ����
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
	PlaySound(L"bgm.wav", NULL, SND_NOSTOP | SND_ASYNC | SND_LOOP); //������ ���۵Ǹ� bgm �÷���
}

MainWindow::~MainWindow()
{
	// ��ġ ������ �ڿ� �ݳ�
	DiscardDeviceResource();

	// ��ġ ������ �ڿ� �ݳ�
	SAFE_RELEASE(pD2DFactory);
	SAFE_RELEASE(pWICFactory);
	SAFE_RELEASE(pDWriteFactory);
}

HRESULT MainWindow::Initialize(HINSTANCE hInstance)
{
	// ��ġ ������ �ڿ��� ������.
	HRESULT hr = CreateAppResource();
	if (FAILED(hr)) return hr;

	// ������ Ŭ������ �����..
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

	// �����츦 ������.
	// ������ ũ�� ����: 380, ����: 540
	// ������ â �̸�: Avoiding Ddong
	hwnd = CreateWindow(L"Avoiding Ddong", L"Avoiding Ddong", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 380, 540, NULL, NULL, hInstance, this);
	hr = hwnd ? S_OK : E_FAIL;
	if (!hwnd) return E_FAIL;

	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);
	return hr;
}

HRESULT MainWindow::CreateAppResource()
{
	// D2D ���丮�� ������.
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
	if (FAILED(hr)) return hr;

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));
	if (FAILED(hr)) return hr;

	// ���� ǥ���� ���� IDWriteTextFormat ��ü ����
	hr = pDWriteFactory->CreateTextFormat(L"Verdana", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 18.f, L"", &pScore);
	if (FAILED(hr)) return hr;

	pScore->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pScore->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	// WIC ���丮�� ������.
	// ����: WIC ���丮�� �����ϴ� CoCreateInstance �Լ��� ���� ������ ������ CoInitialize�� ȣ�����־�� ��.
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWICFactory));
	if (FAILED(hr)) return hr;

	return hr;
}

// ��ġ ������ �ڿ����� ������. ��ġ�� �ҽǵǴ� ��쿡�� �̵� �ڿ��� �ٽ� �����ؾ� ��.
HRESULT MainWindow::CreateDeviceResource()
{
	HRESULT hr = S_OK;

	if (pRenderTarget) return hr;

	RECT rc;
	GetClientRect(hwnd, &rc);
	D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

	// D2D ����Ÿ���� ������.
	hr = pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hwnd, size), &pRenderTarget);
	if (FAILED(hr)) return hr;

	// ���� �ؽ�Ʈ ǥ���� �ܻ� �� ����
	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.f), &pScoreBrush);
	if (FAILED(hr)) return hr;

	//Player �̹��� �ε�
	hr = LoadBitmapFromRes(pRenderTarget, pWICFactory, L"Player", L"Image", 0, 0, &pPlayer);
	if (FAILED(hr)) return hr;

	//�˿� ���� player �̹��� �ε�
	hr = LoadBitmapFromRes(pRenderTarget, pWICFactory, L"DdongHead", L"Image", 0, 0, &pDdongPlayer);
	if (FAILED(hr)) return hr;

	//�� �̹��� �ε�
	hr = LoadBitmapFromRes(pRenderTarget, pWICFactory, L"Ddong", L"Image", 25, 25, &pDdong_bitmap);
	if (FAILED(hr)) return hr;

	//���� �̹��� �ε�
	hr = LoadBitmapFromRes(pRenderTarget, pWICFactory, L"Life", L"Image", 25, 25, &pLifeBitmap);
	if (FAILED(hr)) return hr;

	//�Ҹ�� ���� �̹��� �ε�
	hr = LoadBitmapFromRes(pRenderTarget, pWICFactory, L"BrokenLife", L"Image", 25, 25, &pBrokenLifeBitmap);
	if (FAILED(hr)) return hr;

	//���� ���Ḧ ��Ÿ�� �̹��� �ε�
	hr = LoadBitmapFromRes(pRenderTarget, pWICFactory, L"Gameover", L"Image", 350, 0, &pGameOverBitmap);
	if (FAILED(hr)) return hr;

	//������ ũ�� �޾ƿͼ� window_size ����(Ÿ��:RECT)�� ����
	::GetClientRect(hwnd, &window_size);

	// ���� ��� ��ġ �ʱ�ȭ
	score_LeftTop = D2D1::Point2F(window_size.right / 2 - 35.f, window_size.top);

	// Player ��ġ �ʱ�ȭ
	player_size = pDdongPlayer->GetSize();
	player_LeftTop = D2D1::Point2F(window_size.right / 2, window_size.bottom - player_size.height);

	// Life �̹��� ��ġ �ʱ�ȭ
	// �־����� Life�� �� 3�� �̹Ƿ� 3���� Life �̹��� ����ؾ���
	for (int i = 0; i < 3; i++)
	{
		life.pLife = pLifeBitmap; //������ Life ��ü life�� �����ִ� ���� �̹��� ��Ʈ�� ��ü ����
		life.pBrokenLife = pBrokenLifeBitmap; //Life ��ü life�� �Ҹ�� ���� �̹��� ��Ʈ�� ��ü ����
		life.Life_size = pLifeBitmap->GetSize(); //��Ʈ�� ũ�� �޾ƿ�
		life.Life_LeftTop = D2D1::Point2F(0.f, life.Life_size.width); //��Ʈ�� �׷��� ��ġ ����
		Lifes.push_back(life); //Life ��ü�� Lifes��� ���Ϳ� ����
	}
	life_cnt = Lifes.size(); //�����ִ� ���� ������ Lifes ������ ũ��

	// Gameover
	gameover_size = pGameOverBitmap->GetSize(); //���� ���� �̹��� ��Ʈ�� ũ�� �޾ƿ�
	gameover_LeftTop = D2D1::Point2F(0.f, (window_size.bottom - gameover_size.height) / 2); //���� ���� ��Ʈ�� �׷��� ��ġ ����(������ �߰��� �׸����� ����)

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

// Ű���� ���� ȭ��ǥ ��ư ������ player �������� �̵�
void MainWindow::PlayermoveLeft()
{
	if (player_LeftTop.x <= 7.0f) //���� Player ��Ʈ���� ���� ���� x��ǥ�� 7���� ���� ��� �������� ���� -> ���� â ������ player�� ������� �� ����
		return;
	player_LeftTop.x -= 15.f; //���� ȭ��ǥ ��ư ���� ������ Player ��Ʈ���� ���� �� x ��ǥ�� 15�� ���� -> Player ��Ʈ�� �������� 15�� �̵�
}

// Ű���� ������ ȭ��ǥ ��ư ������ player ���������� �̵�
void MainWindow::PlayermoveRight()
{
	if (player_LeftTop.x >= (window_size.right - player_size.width - 5.f)) //���� Player ��Ʈ���� ���� ���� x��ǥ�� (���� â�� �ʺ� - Player ��Ʈ���� �ʺ� - 5)���� ���� ��� �������� ���� -> ���� â ������ player�� ������� �� ����
		return;
	player_LeftTop.x += 15.f; //������ ȭ��ǥ ��ư ���� ������ Player ��Ʈ���� ���� �� x ��ǥ�� 15�� ���� -> Player ��Ʈ�� ���������� 15�� �̵�
}

// �˰� �÷��̾ �ε������� Ȯ��
void MainWindow::checkHit()
{
	for (auto& i : Ddongs) //���� Ddongs�� �ִ� Ddong ��ü ��ü �˻�
	{
		//Player�� ��� y��ǥ���� �� ��ü �ϴ��� y��ǥ�� �A ��
		float check_y = player_LeftTop.y - (i.Ddong_LeftTop.y + i.Ddong_size.height);
		//player�� ������ x��ǥ ������ �� ��ü�� ���� x��ǥ ���� ��(���밪�� player�� �� ������ �Ÿ�)
		float check_x1 = i.Ddong_LeftTop.x - player_LeftTop.x;
		if (check_y <= 0) //Player�� �Ӹ��� ��ġ�� y��ǥ ���Ϸ� �� ��ü�� ���� ���
		{
			if(check_x1 > 0) //���� player�� �����ʿ� ��ġ�� ���
			{
				//player�� �� ������ �Ÿ��� player �ʺ񺸴� ���� ��� player�� ���� �ε��� 
				if (std::abs(check_x1) <= player_size.width)
				{
					i.destroyed = true;
					player_hitted = true;
					if (!score_calc)
						score -= 10;
					score_calc = true;
				}
			}
			else //���� player�� ���ʿ� ��ġ�� ���
			{
				//player�� �� ������ �Ÿ��� �� �ʺ񺸴� ���� ��� player�� ���� �ε��� 
				if (std::abs(check_x1) <= i.Ddong_size.width)
				{
					i.destroyed = true;
					player_hitted = true;
					if (!score_calc)
						score -= 10;
					score_calc = true;
				}
			}
			if (i.Ddong_LeftTop.y <= window_size.bottom) //���� player�� �ε����� �ʰ� ����â �ϴܿ� ������ ��� 
			{
				i.destroyed = true; //���� �Ҹ� ���� �������� üũ
				if (!score_calc)
					score += 2; //�˰� �ε����� �ʾ����Ƿ� 2�� �߰�
				score_calc = true;
			}
		}
	}
}

// �� �����̴� �Լ�
void MainWindow::Ddongmove()
{
	for (int i = 0; i < Ddongs.size(); i++) //���� Ddongs�� ũ�� = ���� �����Ǿ� �ִ� Ddong��ü ����(���� â�� ���̴� �� ����)
	{
		if (Ddongs[i].destroyed) //Ddong��ü�� �Ҹ� ������ �����ϸ�
		{
			Ddongs.erase(Ddongs.begin() + i); //���Ϳ��� Ddong��ü ����
		}
		Ddongs[i].Ddong_LeftTop.y += Ddongs[i].ddong_speed; //�� ��ü�� ������ ������ �ӵ���ŭ �Ʒ��� ������
	}
}

// �� �����ϴ� �Լ�
void MainWindow::DdongGEN()
{
	ddong.pDdong = pDdong_bitmap; //������ Ddong ��ü ddong�� �� �̹��� ��Ʈ�� ��ü ����
	ddong.ddong_speed = rand() % 5 + 2.f; //���� ������ �ӵ� �����ϰ� ����
	ddong.Ddong_LeftTop = D2D1::Point2F(rand() % 345, window_size.top + 35.f); //���� ������ ��ġ(x��ǥ) �����ϰ� ����, y��ǥ�� Score�� Life �̹��� �ؿ� �����ǵ��� ����
	ddong.Ddong_size = ddong.pDdong->GetSize(); //�� ��Ʈ�� ũ�� �޾ƿ�
	Ddongs.push_back(ddong); //Ddong��ü�� Ddongs��� ���Ϳ� ����(���� ���� Ddong ��ü ���� ����)
}

// �׸� ������ ȭ�鿡 �׸�.
void MainWindow::OnPaint()
{
	HRESULT hr = CreateDeviceResource();
	if (FAILED(hr)) return;

	pRenderTarget->BeginDraw();

	// ������ 0 ������ ��� ���� ����
	if (GameOver)
	{
		Sleep(2000); //2�� ��� �� ����â ����
		PostQuitMessage(0);
		return;
	}

	//������ �˿� ���� ����� ��� ������ ��ġ�� �ʵ��� �ʱ�ȭ
	player_hitted = false;
	score_calc = false;
	period++;
	Ddongmove(); //OnPaint()�� ����� ������ �� ��ġ ����(�Ʒ��� ����������)
	if (period % 35 == 0) //35�ʸ��� �� ����
	{
		if (Ddongs.size() <= 7) //���� â�� Ȱ��ȭ�� ���� 7���� ���� ���� ��� ���ο� �� ����
			DdongGEN();
	}
	checkHit(); //player�� ���� �ε������� Ȯ��

	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	if (player_hitted) //player�� ���� �ε��� ���
	{
		life_cnt--; //���� 1�� �Ҹ�
		pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(player_LeftTop.x, player_LeftTop.y));
		pRenderTarget->DrawBitmap(pDdongPlayer, D2D1::RectF(0.f, 0.f, player_size.width, player_size.height)); //�˿� ���� player �̹����� ����

		if ((score <= 0) || (life_cnt == 0)) //������ 0 �����̰ų� ������ ��� �Ҹ�� ���
		{
			PlaySound(NULL, NULL, SND_ASYNC); //����� �÷��� ����
			PlaySound(L"gameover.wav", NULL, SND_ASYNC); //���� ����Ǿ��� �� ����� �÷���
			GameOver = true; //���� ����Ǿ����� üũ
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(gameover_LeftTop.x, gameover_LeftTop.y));
			pRenderTarget->DrawBitmap(pGameOverBitmap, D2D1::RectF(0.f, 0.f, gameover_size.width, gameover_size.height)); //���� ���� �̹��� ���
		}
	}
	else //player�� ���� �ε����� ���� ���
	{
		pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(player_LeftTop.x, player_LeftTop.y));
		pRenderTarget->DrawBitmap(pPlayer, D2D1::RectF(0.f, 0.f, player_size.width, player_size.height)); //�˿� ���� ���� player �̹��� ���
	}

	// Ddong �׸���
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

	//Score �׸���
	WCHAR score_buf[180];
	wsprintf(score_buf, L"Score: %d", score);
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	pRenderTarget->DrawText(score_buf, (int)wcslen(score_buf), pScore, D2D1::RectF(score_LeftTop.x - 70.f, score_LeftTop.y, score_LeftTop.x + 150.f, 35.f), pScoreBrush);

	// Life �׸���
	for (int i = 0; i < Lifes.size(); i++)
	{
		if (i >= life_cnt) //�˰� �ε����� �ε��� Ƚ����ŭ ������ ���� �̹������� �Ҹ� ���� �̹����� ����
		{
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(Lifes[i].Life_size.width * i + 2.f, 2.f));
			pRenderTarget->DrawBitmap(Lifes[i].pBrokenLife, D2D1::RectF(0.f, 0.f, Lifes[i].Life_size.width, Lifes[i].Life_size.height));
		}
		else //�����ִ� ���� �̹��� ���
		{
			pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(Lifes[i].Life_size.width * i + 2.f, 2.f));
			pRenderTarget->DrawBitmap(Lifes[i].pLife, D2D1::RectF(0.f, 0.f, Lifes[i].Life_size.width, Lifes[i].Life_size.height));
		}
	}

	hr = pRenderTarget->EndDraw();

	// �� �Լ��� ����Ǵ� ���ȿ� ��ġ�� �ҽǵǸ� ��ġ ������ �ڿ����� �ݳ���.
	// �� ���� OnPaint() ȣ�� �ÿ� ���ο��� ȣ��Ǵ� CreateDeviceResource()���� �ݳ��� �ڿ����� �ٽ� ������.
	if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
	{
		DiscardDeviceResource();
	}
}

// ����Ÿ���� ũ�⸦ �ٽ� ������.
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
	case WM_KEYDOWN: //Ű���� ��ư ������ �� ó��
	{
		switch (wParam)
		{
		case VK_LEFT: //Ű���� ���� ȭ��ǥ Ű ������ �� 
			pMainWindow->PlayermoveLeft(); //player �������� �̵��ϴ� �Լ� ȣ��
			break;
		case VK_RIGHT: //Ű���� ������ ȭ��ǥ Ű ������ ��
			pMainWindow->PlayermoveRight(); //player ���������� �̵��ϴ� �Լ� ȣ��
			break;
		case VK_ESCAPE: //Ű���� ESC Ű ������ �� ������ â ����(���� ����)
			PostQuitMessage(0);
			return 1;
		}
		pMainWindow->OnPaint(); //Ű���� ó�� �� ������ ȭ�鿡 ���� ���� �׸� �� �ֵ��� OnPaint() ȣ��
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