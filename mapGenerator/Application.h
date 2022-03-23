#pragma once
#include <vector>
#include <unordered_map>

#include <SDL_events.h>
#include <SDL_render.h>
#include "graph.h"

#define JC_VORONOI_IMPLEMENTATION
#include "jc_voronoi.h"


class Application
{
public:
	Application(SDL_Renderer* renderer);
	~Application();

	void onExecute();

private:
	void onDraw();
	void onEvent(const SDL_Event& e);
	void createVoronoiDiag();
	void createRandomPoints();
	void relax();
	void buildGraphFromVoronoi();
	void destroyJCV();
	void createIslands();
	void generateHeights();
	void floodFill();
	void reevaluateCorners();

	void sortCornersCyclicly(std::vector<Corner*>& corners);

private:
	bool running = false;
	SDL_Renderer* renderer = nullptr;

	const int numberOfPoints = 5000;
	const float offset = 5.0f;
	std::vector<jcv_point> points;
	jcv_diagram diagram;
	const jcv_site* sites;

	std::unordered_map<Point, Center> centers;
	std::unordered_map<Point, Corner> corners;
	std::vector<Edge*> edges;
	
	bool own = false;
	int siteOffset = 0;

	const int numberOfRelaxes = 4;
};
