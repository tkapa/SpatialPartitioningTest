#include "CImg.h"
#include "kf/kf_vector.h"
#include "kf/kf_ray.h"
#include "windows.h"
#include "vector"
#include "iostream"

using namespace cimg_library;

// The resolution of the window and the output of the ray tracer. This can be overridden by the Lua startup script.
int g_windowWidth = 1024;
int g_windowHeight = 1024;

class Circle
{
public:
	kf::Vector2 m_pos;
	float m_radius;

	float xMin;
	float xMax;
	float yMin;
	float yMax;
};

std::vector<Circle> g_circles;

class Node {
public:
	float xMin;
	float xMax;
	float yMin;
	float yMax;

	std::vector<Circle> myCircles;
};

int main(int argc, char **argv)
{
	srand(0);
	// The floating point image target that the scene is rendered into.
	CImg<float> image(g_windowWidth, g_windowHeight, 1, 3, 0);
	// The display object used to show the image.
	CImgDisplay main_disp(image, "Spatial");
	main_disp.set_normalization(0);	// Normalisation 0 disables auto normalisation of the image (scales to make the darkest to brightest colour fit 0 to 1 range.

	int divisionNumber = 3;//number of squares across and down

	std::vector<Node> nodes = std::vector<Node>(divisionNumber * divisionNumber);

	double nodeWid = g_windowWidth / divisionNumber;
	double nodeHig = g_windowHeight / divisionNumber;


	for (int y = 0; y < divisionNumber; ++y)//set the sizes for the quads
	{
		for (int x = 0; x < divisionNumber; ++x)
		{
			int index = x + y * divisionNumber;

			nodes[index].xMin = nodeWid * x;
			nodes[index].yMin = nodeHig * y;
			nodes[index].xMax = nodeWid * (x + 1);
			nodes[index].yMax = nodeHig * (y + 1);
		}
	}

	for (int i = 0; i < 1000; ++i)
	{
		Circle c;
		c.m_pos.x = rand() % g_windowWidth;
		c.m_pos.y = rand() % g_windowHeight;
		c.m_radius = rand() % 50;
		g_circles.push_back(c);

		c.xMin = c.m_pos.x - c.m_radius;
		c.xMax = c.m_pos.x + c.m_radius;
		c.yMin = c.m_pos.y - c.m_radius;
		c.yMax = c.m_pos.y + c.m_radius;

		for (int index = 0; index < nodes.size(); ++index)
		{
			nodes[index].myCircles.push_back(c);
			continue;
		}
	}


	// Record the starting time.
	DWORD startTime = timeGetTime();

	for (Node n : nodes){
		// Primary loop through all screen pixels.
		for (int y = n.yMin; y < n.yMax; ++y)
		{
			for (int x = n.xMin; x < n.xMax; ++x)
			{
				kf::Colour output(0, 0, 0, 0);
				float e = 0;
				for (int i = 0; i < n.myCircles.size(); ++i)
				{
					if ((kf::Vector2(x, y) - n.myCircles[i].m_pos).lengthSquared() <= n.myCircles[i].m_radius*n.myCircles[i].m_radius)
					{
						float d = (kf::Vector2(x, y) - n.myCircles[i].m_pos).length();
						e = e + (1.0 - d / n.myCircles[i].m_radius);
					}
				}
				output.set(e, e, e, 1);

				// Clamp the output colour to 0-1 range before conversion.
				output.saturate();
				// Convert from linear space to sRGB.
				output.toSRGB();
				// Write the colour to the image (scaling up by 255).
				*image.data(x, y, 0, 0) = output.r * 255;
				*image.data(x, y, 0, 1) = output.g * 255;
				*image.data(x, y, 0, 2) = output.b * 255;
			}
			std::cout << y << std::endl;
			// Check for Escape key.
			if (main_disp.is_keyESC())
				return 0;
		}
		std::cout << "Done" << std::endl;
	}

	// Record ending time.
	DWORD endTime = timeGetTime();

	// Display elapsed time in the window title bar.
	main_disp.set_title("Render time: %dms", endTime - startTime);
	main_disp.display(image);

	// Keep refreshing the window until it is closed or escape is hit.
	while (!main_disp.is_closed())
	{
		if (main_disp.is_keyESC())
			return 0;
		main_disp.wait();
	}

	return 0;

}
