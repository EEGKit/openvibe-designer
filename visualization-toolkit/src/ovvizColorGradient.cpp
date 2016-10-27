#include "ovvizColorGradient.h"

#include <cstdio>
#include <string>
#include <map>

using namespace OpenViBE;
using namespace OpenViBEVisualizationToolkit;

namespace
{
	typedef struct
	{
		float64 percent;
		float64 red;
		float64 green;
		float64 blue;
	} SColor;
};

bool OpenViBEVisualizationToolkit::Tools::ColorGradient::parse(IMatrix& colorGradientMatrix, const CString& string)
{
	std::string colorString(string.toASCIIString());
	std::string::size_type startPosition = 0;
	std::string::size_type endPosition;

	std::map < float64, SColor > colorGradientVector;

	do
	{
		endPosition = colorString.find(OV_Value_EnumeratedStringSeparator, startPosition);
		if (endPosition == std::string::npos)
		{
			endPosition = colorString.length();
		}

		std::string colorSubString;
		colorSubString.assign(colorString, startPosition, endPosition - startPosition);

		int p,r,g,b;
		if(sscanf(colorSubString.c_str(), "%i:%i,%i,%i", &p, &r, &g, &b) == 4)
		{
			SColor color;
			color.percent=p;
			color.red=r;
			color.green=g;
			color.blue=b;
			colorGradientVector[color.percent] = color;
		}

		startPosition = endPosition + 1;
	}
	while (startPosition < colorString.length());

	colorGradientMatrix.setDimensionCount(2);
	colorGradientMatrix.setDimensionSize(0, 4);
	colorGradientMatrix.setDimensionSize(1, static_cast<uint32>(colorGradientVector.size()));

	uint32 i = 0;
	for (auto it = colorGradientVector.begin(); it != colorGradientVector.end(); it++, i++)
	{
		colorGradientMatrix[i*4  ]=it->second.percent;
		colorGradientMatrix[i*4+1]=it->second.red;
		colorGradientMatrix[i*4+2]=it->second.green;
		colorGradientMatrix[i*4+3]=it->second.blue;
	}

	return true;
}

bool OpenViBEVisualizationToolkit::Tools::ColorGradient::format(CString& string, const IMatrix& colorGradient)
{
	if (colorGradient.getDimensionCount() != 2)
	{
		return false;
	}

	if (colorGradient.getDimensionSize(0) != 4)
	{
		return false;
	}

	std::string separator("  ");
	separator[0] = OV_Value_EnumeratedStringSeparator;

	std::string result;
	for (uint32 i = 0; i < colorGradient.getDimensionSize(1); i++)
	{
		char buffer[1024];
		sprintf(
			buffer,
			"%.0lf:%i,%i,%i",
			colorGradient[i*4],
			static_cast<int>(colorGradient[i*4+1]),
			static_cast<int>(colorGradient[i*4+2]),
			static_cast<int>(colorGradient[i*4+3]));
		result += (i==0 ? "" : separator);
		result += buffer;
	}

	string=result.c_str();
	return true;
}

bool OpenViBEVisualizationToolkit::Tools::ColorGradient::interpolate(IMatrix& interpolatedColorGradient, const IMatrix& colorGradient, const uint32 steps)
{
	uint32 i;

	if (colorGradient.getDimensionCount() != 2)
	{
		return false;
	}

	if (colorGradient.getDimensionSize(0) != 4)
	{
		return false;
	}

	if (steps <= 1)
	{
		return false;
	}

	interpolatedColorGradient.setDimensionCount(2);
	interpolatedColorGradient.setDimensionSize(0, 4);
	interpolatedColorGradient.setDimensionSize(1, steps);

	std::map<float64, SColor> colors;

	for (i = 0; i < colorGradient.getDimensionSize(1); i++)
	{
		SColor color;
		color.percent = colorGradient[i*4];
		color.red = colorGradient[i*4+1];
		color.green = colorGradient[i*4+2];
		color.blue = colorGradient[i*4+3];
		colors[color.percent]=color;
	}

	if (colors.find(0) == colors.end())
	{
		SColor color;
		color = colors.begin()->second;
		color.percent=0;
		colors[0] = color;
	}

	if (colors.find(100) == colors.end())
	{
		SColor color;
		color = colors.rbegin()->second;
		color.percent=100;
		colors[100] = color;
	}

	auto it1 = colors.begin();
	auto it2 = colors.begin();
	it2++;

	for (i = 0; i < steps; i++)
	{
		float64 t = i * 100 / (steps - 1);
		while (it2->first < t)
		{
			it1++;
			it2++;
		}

		float64 a = it2->first - t;
		float64 b = t - it1->first;
		float64 d = it2->first - it1->first;

		interpolatedColorGradient[i*4  ] = t;
		interpolatedColorGradient[i*4+1] = (it1->second.red   * a + it2->second.red   * b) / d;
		interpolatedColorGradient[i*4+2] = (it1->second.green * a + it2->second.green * b) / d;
		interpolatedColorGradient[i*4+3] = (it1->second.blue  * a + it2->second.blue  * b) / d;
	}

	return true;
}
