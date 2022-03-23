#pragma once
#include <vector>

struct Point;
struct Center;
struct Edge;
struct Corner;

enum class Biomes
{
	Water,
	Barren,
	Forest,
	Tundra,
};

struct Point
{
	float x, y;
	Point()
	{
		x = 0;
		y = 0;
	}
	
	Point(float x, float y) : x(x), y(y) {}

	bool operator==(const Point& other) const
	{
		return x == other.x && y == other.y;
	}
};

template<>
struct std::hash<Point>
{
	std::size_t operator()(Point const& s) const noexcept
	{
		std::size_t h1 = std::hash<float>{}(s.x);
		std::size_t h2 = std::hash<float>{}(s.y);
		return h1 ^ (h2 << 1);
	}
};

struct Center
{
	Point location;
	double elevation;

	std::vector<Center*> neigbours;
	std::vector<Edge*> borders;
	std::vector<Corner*> corners;

	bool isWater = false;
	bool isOcean = false;
	bool isBorder = false;
	Biomes biome;
};

struct Edge 
{
	Center* d0;
	Center* d1;
	Corner* v0;
	Corner* v1;
};

struct Corner
{
	Point location;
	double elevation;

	std::vector<Center*> touches;
	std::vector<Edge*> protrudes;
	std::vector<Corner*> adjacent;

	bool isWater = false;
	bool isCoast = false;
	bool isOcean = false;

	Biomes biome;
};
