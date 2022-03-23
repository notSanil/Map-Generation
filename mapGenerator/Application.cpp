#include "Application.h"

#include <stdlib.h>
#include <algorithm>
#include <stack>
#include <unordered_map>
#include <cmath>

#include <SDL_keycode.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_render.h>

#include "graph.h"

#include "PerlinNoise.hpp"
#define NOISE perlin.noise2D_01


double normalise(double val, double lower, double upper)
{
	return val / (upper - lower);
}

Application::Application(SDL_Renderer* renderer) : renderer(renderer)
{
	running = true;
	points.reserve(numberOfPoints);
	memset(&diagram, 0, sizeof(diagram));
	createRandomPoints();
	createVoronoiDiag();
	for (int i = 0; i < numberOfRelaxes; ++i)
	{
		relax();
	}
	buildGraphFromVoronoi();
	destroyJCV();
	createIslands();
}

void Application::createRandomPoints()
{
	srand(0);
	int width = 0, height = 0;
	SDL_GetRendererOutputSize(renderer, &width, &height);
	int boxWidth = width - (int)offset;
	int boxHeight = height - (int)offset;
	for (int i = 0; i < numberOfPoints; ++i)
	{
		points.push_back(jcv_point());
		points[i].x = (float)(rand() % boxWidth) + offset;
		points[i].y = (float)(rand() % boxHeight) + offset;
	}
}

void Application::createVoronoiDiag()
{
	int width = 0, height = 0;
	SDL_GetRendererOutputSize(renderer, &width, &height);
	jcv_rect rect{ {offset, offset}, {width - offset, height - offset} };
	jcv_diagram_generate(numberOfPoints, &*points.begin(), &rect, NULL, &diagram);
	sites = jcv_diagram_get_sites(&diagram);
}

void Application::relax()
{
	for (int i = 0; i < diagram.numsites; ++i)
	{
		const jcv_site* site = &sites[i];
		jcv_point sum = site->p;
		int count = 1;
		const jcv_graphedge* edge = site->edges;

		while (edge)
		{
			sum.x += edge->pos[0].x;
			sum.y += edge->pos[0].y;
			++count;
			edge = edge->next;
		}

		points[site->index].x = sum.x / count;
		points[site->index].y = sum.y / count;
	}

	createVoronoiDiag();
}

void Application::buildGraphFromVoronoi()
{
	//Centers
	centers.clear();
	for (int i = 0; i < diagram.numsites; ++i)
	{
		const jcv_site* site = &sites[i];
		Point p(site->p.x, site->p.y);
		Center center;
		center.location = p;
		centers.insert({ p, center });
	}
	//Corners
	corners.clear();
	for (int i = 0; i < diagram.numsites; ++i)
	{
		const jcv_site* site = &sites[i];
		const jcv_graphedge* edge = site->edges;
		while (edge)
		{
			Point p0(edge->pos[0].x, edge->pos[0].y);
			if (corners.count(p0) == 0)
			{
				Corner corner;
				corner.location = p0;
				corners.insert({ p0, corner });
			}
			Point p1(edge->pos[1].x, edge->pos[1].y);
			if (corners.count(p1) == 0)
			{
				Corner corner;
				corner.location = p1;
				corners.insert({ p1, corner });
			}
			edge = edge->next;
		}
	}
	for (const auto edge : edges)
	{
		delete edge;
	}
	edges.clear();

	std::unordered_map<Point, Edge*> uniqueEdges;
	//Edges
	for (int i = 0; i < diagram.numsites; ++i)
	{
		const jcv_site* site = &sites[i];
		const jcv_graphedge* edge = site->edges;
		while (edge)
		{
			//Corners
			Point v0(edge->pos[0].x, edge->pos[0].y);
			Point v1(edge->pos[1].x, edge->pos[1].y);
			//Centers
			Point d0(-1, -1);
			Point d1(-1, -1);
			if (edge->edge->sites[0])
			{
				d0.x = edge->edge->sites[0]->p.x;
				d0.y = edge->edge->sites[0]->p.y;
			}
			if (edge->edge->sites[1])
			{
				d1.y = edge->edge->sites[1]->p.y;
				d1.x = edge->edge->sites[1]->p.x;
			}

			Point center((v0.x + v1.x) / 2, (v0.y + v1.y) / 2);
			if (uniqueEdges.count(center) == 0)
			{
				Edge* e = new Edge();
				e->v0 = &corners[v0];
				e->v1 = &corners[v1];

				e->d0 = centers.count(d0) ? &centers[d0] : nullptr;
				e->d1 = centers.count(d1) ? &centers[d1] : nullptr;

				edges.push_back(e);

				//Edge-center
				if (e->d0)
				{
					e->d0->borders.push_back(e);
					if (!e->d1)
						e->d0->isBorder = true;
				}
				if (e->d1)
				{
					e->d1->borders.push_back(e);
					if (!e->d0)
						e->d1->isBorder = true;
				}
				//Edge-corner
				e->v0->protrudes.push_back(e);
				e->v1->protrudes.push_back(e);

				//Center-center
				if (e->d0 && e->d1)
				{
					e->d0->neigbours.push_back(e->d1);
					e->d1->neigbours.push_back(e->d0);
				}

				//Corner-corner
				e->v0->adjacent.push_back(e->v1);
				e->v1->adjacent.push_back(e->v0);

				//Center-corner
				if (e->d0)
				{
					e->d0->corners.push_back(e->v0);
					e->d0->corners.push_back(e->v1);
				}
				if (e->d1)
				{
					e->d1->corners.push_back(e->v0);
					e->d1->corners.push_back(e->v1);
				}

				//Corner-center
				if (e->d0)
				{
					e->v0->touches.push_back(e->d0);
					e->v1->touches.push_back(e->d0);
				}
				if (e->d1)
				{
					e->v0->touches.push_back(e->d1);
					e->v1->touches.push_back(e->d1);
				}

				uniqueEdges[center] = e;
			}
			edge = edge->next;
		}
	}

	for (auto& [point, center] : centers)
	{
		sortCornersCyclicly(center.corners);
	}
}

void Application::createIslands()
{
	generateHeights();
	floodFill();
	reevaluateCorners();
}

void Application::generateHeights()
{
	constexpr double threshold = 0.39;

	const siv::PerlinNoise::seed_type seed = 10009u;
	const siv::PerlinNoise perlin{ seed };

	int width, height;
	SDL_GetRendererOutputSize(renderer, &width, &height);
	const double frequency = 2;
	for (auto& [point, corner] : corners)
	{
		double nx = (double)point.x / (double)width, ny = (double)point.y / (double)height;
		double elevation = 1 * NOISE(1 * frequency * nx, 1 * frequency * ny) +
			0.5 * NOISE(2 * frequency * nx + 5.3, 2 * frequency * ny + 5.3) +
			0.25 * NOISE(4 * frequency * nx + 19.7, 4 * frequency * ny + 19.7);
		elevation = normalise(elevation, 0, 1.75);

		elevation = pow(elevation * 1.3, 2.0);

		if (elevation < threshold)
		{
			corner.isWater = true;
			corner.biome = Biomes::Water;
		}
		else
		{
			corner.isWater = false;
			Biomes biome;
			if (elevation < .425)
				biome = Biomes::Barren;
			else if (elevation < 0.65)
				biome = Biomes::Forest;
			else
				biome = Biomes::Tundra;

			corner.biome = biome;
		}

		corner.elevation = elevation;
	}

	for (auto& [point, center] : centers)
	{
		uint8_t water = 0;
		uint8_t land = 0;
		double elevation = 0;

		for (const auto corner : center.corners)
		{
			water += corner->isWater;
			land += !(corner->isWater);
			elevation += corner->elevation;
		}
		elevation /= (double)center.corners.size();
		center.isWater = water > land;
		center.elevation = elevation;

		Biomes biome;
		if (elevation < .425)
			biome = Biomes::Barren;
		else if (elevation < 0.65)
			biome = Biomes::Forest;
		else
			biome = Biomes::Tundra;
		


		center.biome = biome;
	}
}

void Application::floodFill()
{
	for (auto& [point, center] : centers)
	{
		if (center.isWater && center.isBorder && !center.isOcean)
		{
			std::stack<Center*> toBeProcessed;
			toBeProcessed.push(&center);

			while (!toBeProcessed.empty())
			{
				auto current = toBeProcessed.top();
				toBeProcessed.pop();

				current->isOcean = true;
				for (auto& neighbour : current->neigbours)
				{
					if (neighbour->isWater && !neighbour->isOcean)
						toBeProcessed.push(neighbour);
				}
			}
		}
	}
}

void Application::reevaluateCorners()
{
	for (auto& [point, corner] : corners)
	{
		bool hasLand = false;
		for (auto site : corner.touches)
		{
			if (!site->isWater)
				hasLand = true;
		}
		corner.isCoast = hasLand;
		corner.isOcean = !hasLand && corner.isWater;
	}
}

void Application::sortCornersCyclicly(std::vector<Corner*>& corners)
{
	Corner center;
	center.location = { 0, 0 };
	for (const auto& corner : corners)
	{
		center.location.x += corner->location.x;
		center.location.y += corner->location.y;
	}

	center.location.x /= corners.size();
	center.location.y /= corners.size();

	auto compareFunc = [&](Corner* a, Corner* b)
	{
		if (a->location.x - center.location.x >= 0 && b->location.x - center.location.x < 0)
			return true;
		if (a->location.x - center.location.x < 0 && b->location.x - center.location.x >= 0)
			return false;
		if (a->location.x - center.location.x == 0 && b->location.x - center.location.x == 0) {
			if (a->location.y - center.location.y >= 0 || b->location.y - center.location.y >= 0)
				return a->location.y > b->location.y;
			return b->location.y > a->location.y;
		}

		// compute the cross product of vectors (center -> a) location.x (center -> b)
		float det = (a->location.x - center.location.x) * (b->location.y - center.location.y) -
			(b->location.x - center.location.x) * (a->location.y - center.location.y);
		if (det < 0)
			return true;
		if (det > 0)
			return false;

		// points a and b are on the same line from the center
		// check which point is closer to the center
		float d1 = (a->location.x - center.location.x) * (a->location.x - center.location.x) +
			(a->location.y - center.location.y) * (a->location.y - center.location.y);
		float d2 = (b->location.x - center.location.x) * (b->location.x - center.location.x) +
			(b->location.y - center.location.y) * (b->location.y - center.location.y);
		return d1 > d2;
	};

	std::sort(corners.begin(), corners.end(), compareFunc);
}

void Application::destroyJCV()
{
	jcv_diagram_free(&diagram);
	memset(&diagram, 0, sizeof(diagram));
	sites = nullptr;
	points.clear();
}

void Application::onExecute()
{
	while (running)
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			onEvent(e);
		}
		onDraw();
	}
}

void Application::onEvent(const SDL_Event& e)
{
	if (e.type == SDL_QUIT)
	{
		running = false;
	}

	if (e.type == SDL_KEYDOWN)
	{
		if (e.key.keysym.sym == SDLK_l)
		{
			own = !own;
		}
		else if (e.key.keysym.sym == SDLK_d)
		{
			siteOffset = (siteOffset + 1) % (int)centers.size();
		}
		else if (e.key.keysym.sym == SDLK_ESCAPE)
		{
			running = false;
		}
	}
}

void Application::onDraw()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	// Rendering goes here

	if (!own)
	{
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		for (auto& [point, center] : centers)
		{
			std::vector<Sint16> xArr;
			xArr.reserve(center.corners.size());
			std::vector<Sint16> yArr;
			yArr.reserve(center.corners.size());

			for (const auto& corner : center.corners)
			{
				xArr.push_back((int)corner->location.x);
				yArr.push_back((int)corner->location.y);
			}

			if (center.isWater)
			{
				if (!center.isOcean)
					filledPolygonRGBA(renderer, &*xArr.begin(), &*yArr.begin(), (int)xArr.size(), 0, 119, 190, 255);
				else
					filledPolygonRGBA(renderer, &*xArr.begin(), &*yArr.begin(), (int)xArr.size(), 1, 41, 139, 255);
			}
			else
			{
				SDL_Color color;
				

				switch (center.biome)
				{
				case Biomes::Barren:
					color = { 204, 169, 144 };
					break;
				case Biomes::Forest:
					color = { 34, 139, 34 };
					break;
				case Biomes::Tundra:
					color = { 246,244,233 };
					break;
				default:
					color = { 204, 169, 144 };
					break;
				}
				filledPolygonRGBA(renderer, &*xArr.begin(), &*yArr.begin(), (int)xArr.size(), color.r, color.g, color.b, 255);
			}
		}
	}
	else
	{
		//Center
		auto it = centers.begin();
		for (int i = 0; i < siteOffset; ++i)
		{
			++it;
		}
		const auto& center = it->second;
		circleRGBA(renderer, (int)center.location.x, (int)center.location.y, 4, 0, 255, 255, 255);
		//Corner
		for (const auto& corner : center.corners)
		{
			circleRGBA(renderer, (int)corner->location.x, (int)corner->location.y, 10, 255, 0, 0, 255);
		}
		//Edges
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		for (const auto& edge : center.borders)
		{
			SDL_RenderDrawLineF(renderer, edge->v0->location.x, edge->v0->location.y, edge->v1->location.x, edge->v1->location.y);
		}
		//Deluaney Edges
		SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
		for (const auto& edge : center.borders)
		{
			if (edge->d0 && edge->d1)
				SDL_RenderDrawLineF(renderer, edge->d0->location.x, edge->d0->location.y, edge->d1->location.x, edge->d1->location.y);
		}

	}

	SDL_RenderPresent(renderer);
}

Application::~Application()
{
	corners.clear();
	centers.clear();
	for (const auto edge : edges)
		delete edge;
	edges.clear();
}
