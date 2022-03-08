/////////////////////////////////////////////////////////////
// 程序名称：贝塞尔曲线
// 编译环境：Visual Studio 2019 (v142)，EasyX_20211109
// 作　　者：luuyiran@gmail.com
// 发布日期：2021-11-18
//
#include <math.h>				// sin
#include <stdint.h>				// uint64_t
#include <vector>				// array
#include <easyx.h>				// window

using std::vector;

#define	WIDTH			800		// 宽
#define	HEIGHT			600		// 高

struct Point { double x, y; };

// 初始化控制点
vector<Point> controlPoints;

Point operator+(const Point& a, const Point& b) {
	return Point({ a.x + b.x, a.y + b.y });
}
Point operator*(double f, const Point& p) {
	return Point({f*p.x, f*p.y});
}

// 计算二项式系数： C(n, k) = n! / (k!(n-k)!)
// 这里我们不用公式，使用 Pascal's Triangle
// [1],					n = 0
// [1, 1],				n = 1
// [1, 2, 1],			n = 2
// [1, 3, 3, 1],		n = 3
// [1, 4, 6, 4, 1],		n = 4
int C(int n, int k) {
	vector<int> array(n + 1, 1);
	for (int i = 2; i <= n; i++)
		for (int j = i - 1; j > 0; j--)
			array[j] += array[j - 1];
	return array[k];
}

// 绘制贝塞尔曲线，绘制 t [0 - end_t] 范围
void drawBezier(const vector<Point>& points, double end_t) {
	if (points.size() <= 1) return;
	int n = points.size() - 1; // 阶次为点数 - 1
	setfillcolor(RED);
	for (double t = 0.0; t <= end_t; t += 0.001) {
		Point p{ 0 };
		for (int k = 0; k <= n; k++)
			p = p + C(n, k) * pow(t, k) * pow(1 - t, (double)n - k) * points[k];
		// 位置四舍五入
		p.x += 0.5;
		p.y += 0.5;
		solidcircle((int)p.x, (int)p.y, 3);
	}
}

// 递归获取每一层的控制点
void bezierLevelPoints(vector<vector<Point>>& levels, double t) {
	vector<Point> &pre = levels.back();
	vector<Point> next;
	int n = pre.size();
	if (n <= 1) return;
	
	for (int i = 0; i < n - 1; i++) {
		Point point = (1 - t) * pre[i] + t * pre[i + 1];
		next.push_back(point);
	}
	levels.push_back(next);
	bezierLevelPoints(levels, t);
}

// 处理输入事件
void processInput() {
	ExMessage msg;
	while (peekmessage(&msg, EM_MOUSE | EM_KEY)) {
		// 按住左键拖动控制点
		if (WM_MOUSEMOVE == msg.message && msg.lbutton) {
			for (auto& p : controlPoints) {
				int dx = (int)p.x - msg.x;
				int dy = (int)p.y - msg.y;
				if (dx * dx + dy * dy < 500) {
					p.x = msg.x;
					p.y = msg.y;
				}
			}
		}
		// 点击右键，创建控制点
		else if (WM_RBUTTONDOWN == msg.message && msg.rbutton) {
			controlPoints.push_back({ (double)msg.x, (double)msg.y });
		}
		// 按空格键清空
		else if (WM_KEYDOWN == msg.message && msg.vkcode == VK_SPACE) {
			controlPoints.clear();
		}
	}
}

// 画点、画线
void drawControlLines(vector<vector<Point>>& levels) {
	int k = levels.size();
	COLORREF color[8] = { YELLOW, BROWN, CYAN, LIGHTRED, WHITE, GREEN, MAGENTA, RED };
	for (int i = 0; i < k; i++) {
		vector<Point> cur = levels[i];
		int n = cur.size();
		setlinecolor(color[i&0XF]);
		setfillcolor(color[i&0XF]);
		for (auto& point : cur)
			solidcircle((int)(point.x + 0.5), (int)(point.y + 0.5), 3);
		if (k <= 2) continue;
		for (int j = 0; j < n - 1; j++)
			line((int)(cur[j].x + 0.5), (int)(cur[j].y + 0.5), \
				(int)(cur[j + 1].x + 0.5), (int)(cur[j + 1].y + 0.5));
	}
}

int main() {
	TCHAR  buf[32];
	double t = 0.f;
	double time = -1.57079632679489661923;   // pi / 2;
	double deltaTime = 0.f; // 当前帧和上一帧的时间差
	double lastTime = 0.f;
	double currentTime = 0.f;
	uint64_t start, now, frequency;
    initgraph(WIDTH, HEIGHT);
    BeginBatchDraw();
	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
    while (1) {
        cleardevice();
		QueryPerformanceCounter((LARGE_INTEGER*)&now);
		currentTime = (double)(now - start) / frequency;
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// 保证不同帧率下绘制速度一致
		time += 1.5 * deltaTime;
		t = 0.5 * (1 + sin(time));
		
		// 根据鼠标拖动更新控制点
		processInput();
		// 得到每一层的控制点
		vector<vector<Point>> levels({ controlPoints });
		bezierLevelPoints(levels, t);
		// 绘制控制点、控制线
		drawControlLines(levels);
		// 绘制 0 - t 范围内的贝塞尔曲线
		drawBezier(controlPoints, t);

		// 提示信息
		outtextxy(0,  0, L"  Right Mouse Button to Create Points.");
		outtextxy(0, 16, L"  Left  Mouse Button to Drag Points.");
		outtextxy(0, 32, L"  Press SPACE to Clear.");
		swprintf_s(buf, _T("  t: %.3f"), t);
		outtextxy(0, 48, buf);
		swprintf_s(buf, _T("  time: %.0fms"), 1000 * deltaTime);
		outtextxy(0, 64, buf);
		swprintf_s(buf, _T("  fps: %d"), (int)(1.f / deltaTime));
		outtextxy(0, 80, buf);
        FlushBatchDraw();
    }
    return 0;
}