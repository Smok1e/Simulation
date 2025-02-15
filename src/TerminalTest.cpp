#include <numbers>
#include <random>
#include <format>
#include <numeric>
#include <array>

#include "Terminal.hpp"
#include "SignalHandler.hpp"

using namespace std::chrono_literals;

//======================================

void DrawLine(
	Terminal& terminal,
	int x0,
	int y0,
	int x1,
	int y1,
	char ch,
	Terminal::Color background = Terminal::Color::Default,
	Terminal::Color foreground = Terminal::Color::Default
);

void DrawCircle(
	Terminal& terminal,
	int x,
	int y,
	int radius,
	char ch,
	size_t point_count = 64,
	Terminal::Color background = Terminal::Color::Default,
	Terminal::Color foreground = Terminal::Color::Default
);

//======================================

struct Point
{
	double x;
	double y;

	double vx;
	double vy;
};

//======================================

extern std::vector<int> govnovector;

int main()
{
	Terminal terminal;

	//std::string_view test = "aaaaaaaabcdddddddddddddd";
	//terminal.set(terminal.getWidth() - test.length(), 0, test);
	//terminal.display();
	//terminal.display();
	//
	//return 0;

	constexpr size_t point_count = 10;
	Point points[point_count] = {};

	std::mt19937 generator(123);
	for (auto& point: points)
	{
		point.x = std::uniform_int_distribution<>(0, terminal.getWidth () - 1)(generator);
		point.y = std::uniform_int_distribution<>(0, terminal.getHeight() - 1)(generator);

		point.vx = std::uniform_real_distribution<>(-1, 1)(generator);
		point.vy = std::uniform_real_distribution<>(-1, 1)(generator);
	}

	std::vector<double> fps_buffer;
	size_t fps_buffer_limit = 100;
	size_t frames = 100;

	terminal.enableAltBuffer(true);

	auto start_time = std::chrono::high_resolution_clock::now();

	auto last_time = start_time;
	for (size_t i = 0; i < frames || true; i++)
	{
		terminal.clear();

		for (size_t i = 0; i < point_count; i++)
		{
			auto& a = points[i];
			auto& b = points[(i + 1) % point_count];

			a.x += a.vx;
			a.y += a.vy;

			if (a.x < 0 || a.x >= terminal.getWidth())
				a.vx *= -1, a.x = std::clamp<double>(a.x, 0, terminal.getWidth() - 1);

			if (a.y < 0 || a.y >= terminal.getHeight())
				a.vy *= -1, a.y = std::clamp<double>(a.y, 0, terminal.getHeight() - 1);

			DrawLine(
				terminal,
				a.x,
				a.y,
				b.x,
				b.y,
				'*'
			);
		}

		auto current_time = std::chrono::high_resolution_clock::now();
		auto fps = 1.0 / std::chrono::duration<double>(current_time - last_time).count();
		last_time = current_time;

		terminal.set(0, 0, std::format("Frame: {} / {}", i, frames));
		terminal.set(0, 1, std::format("FPS: {:.0f}", fps), Terminal::Color::Default, Terminal::Color::Blue);
		terminal.display();

		// WaitSignal(10ms);
	}

	terminal.enableAltBuffer(false);

	auto total_time = std::chrono::high_resolution_clock::now() - start_time;
	auto avg_fps = frames / std::chrono::duration<double>(total_time).count();

	std::cout << "Average FPS: " << avg_fps << std::endl;
}

//======================================

void DrawLine(
	Terminal& terminal,
	int x0,
	int y0,
	int x1,
	int y1,
	char ch,
	Terminal::Color background /*= Terminal::Color::Default*/,
	Terminal::Color foreground /*= Terminal::Color::Default*/
)
{
	auto length = std::sqrt(std::pow(x1 - x0, 2) + std::pow(y1 - y0, 2));
	for (double t = 0; t < 1; t += 1.0 / length)
	{
		terminal.set(
			x0 + t * (x1 - x0),
			y0 + t * (y1 - y0),
			ch,
			background,
			foreground
		);
	}
}

void DrawCircle(
	Terminal& terminal,
	int x,
	int y,
	int radius,
	char ch,
	size_t point_count /*= 32*/,
	Terminal::Color background /*= Terminal::Color::Default*/,
	Terminal::Color foreground /*= Terminal::Color::Default*/
)
{
	double delta = 2 * std::numbers::pi / point_count;

	for (size_t i = 0; i < point_count; i++)
	{
		double angle0 = i * delta;
		double angle1 = angle0 + delta;

		DrawLine(
			terminal,
			x + 2 * radius * cos(angle0),
			y +     radius * sin(angle0),
			x + 2 * radius * cos(angle1),
			y +     radius * sin(angle1),
			ch,
			background,
			foreground
		);
	}
}

//======================================